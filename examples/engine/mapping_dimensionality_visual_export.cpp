// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// Native metric.visual.v1 exporter for mapping and dimensionality evidence.
//
// The evidence is computed by the C++ process-curve PHATE pipeline examples:
// records are native process curves, distances are the aligned-curve metric, and
// mapped coordinates/diagnostics come from native_phate_autoencoder_with_codec.

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <metric/engine.hpp>
#include <metric/solve/parametric/dnn.hpp>

#include "mtrc_visual.hpp"

namespace {

namespace visual = mtrc::visual;

constexpr const char *kDatasetId = "mapping-dimensionality-process-curves";
constexpr const char *kSourceRelationId = "process-curve-aligned-metric";
constexpr const char *kLatentRelationId = "process-curve-latent-euclidean-metric";
constexpr const char *kSourceSpaceId = "process-curve-source-space";
constexpr const char *kLatentSpaceId = "process-curve-latent-space";
constexpr const char *kGraphId = "process-curve-source-knn";
constexpr std::size_t kFeatureCount = 8;
constexpr std::size_t kNeighborCount = 1;
constexpr double kEps = 1.0e-9;

struct ProcessCurve {
	std::string id;
	std::string family;
	std::vector<double> values;
};

struct AlignedCurveDistance {
	double gap_cost{2.0};

	auto operator()(const ProcessCurve &lhs, const ProcessCurve &rhs) const -> double
	{
		std::vector<double> previous(rhs.values.size() + 1, 0.0);
		for (std::size_t index = 0; index < previous.size(); ++index) {
			previous[index] = static_cast<double>(index) * gap_cost;
		}

		for (std::size_t lhs_index = 1; lhs_index <= lhs.values.size(); ++lhs_index) {
			std::vector<double> current(rhs.values.size() + 1, 0.0);
			current[0] = static_cast<double>(lhs_index) * gap_cost;
			for (std::size_t rhs_index = 1; rhs_index <= rhs.values.size(); ++rhs_index) {
				const auto substitution =
					previous[rhs_index - 1] +
					std::min(std::abs(lhs.values[lhs_index - 1] - rhs.values[rhs_index - 1]), 2 * gap_cost);
				const auto deletion = previous[rhs_index] + gap_cost;
				const auto insertion = current[rhs_index - 1] + gap_cost;
				current[rhs_index] = std::min({substitution, deletion, insertion});
			}
			previous = std::move(current);
		}

		return previous.back();
	}
};

struct CurveFeatureEncoder {
	std::size_t feature_count{0};

	auto operator()(const ProcessCurve &record) const -> std::vector<double>
	{
		if (feature_count == 0) {
			throw std::invalid_argument("curve feature encoder requires a positive feature count");
		}
		if (record.values.empty()) {
			throw std::invalid_argument("curve feature encoder requires non-empty curve values");
		}
		if (feature_count == 1 || record.values.size() == 1) {
			return std::vector<double>(feature_count, record.values.front());
		}

		std::vector<double> features;
		features.reserve(feature_count);
		for (std::size_t index = 0; index < feature_count; ++index) {
			const auto position = static_cast<double>(index) * static_cast<double>(record.values.size() - 1) /
								  static_cast<double>(feature_count - 1);
			const auto left = static_cast<std::size_t>(std::floor(position));
			const auto right = std::min(left + 1, record.values.size() - 1);
			const auto fraction = position - static_cast<double>(left);
			features.push_back(record.values[left] * (1.0 - fraction) + record.values[right] * fraction);
		}
		return features;
	}
};

struct MetricLawDiagnostics {
	double min_distance{std::numeric_limits<double>::infinity()};
	double max_distance{};
	double mean_distance{};
	double diagonal_max_abs{};
	double symmetry_max_abs{};
	double triangle_max_violation{};
	std::size_t pair_count{};
	std::size_t triangle_triplets{};
	bool finite{true};
};

auto json_object(const std::vector<std::pair<std::string, std::string>> &fields) -> std::string
{
	std::string out = "{";
	for (std::size_t index = 0; index < fields.size(); ++index) {
		if (index != 0) {
			out += ",";
		}
		out += visual::quote(fields[index].first);
		out += ":";
		out += fields[index].second;
	}
	out += "}";
	return out;
}

auto json_bool(bool value) -> const char * { return value ? "true" : "false"; }

auto json_size(std::size_t value) -> std::string { return std::to_string(value); }

auto json_number_array(const std::vector<double> &values) -> std::string
{
	return visual::number_array(values);
}

auto process_records() -> std::vector<ProcessCurve>
{
	return {{"normal_reference", "normal", {0, 0, 1, 1, 1, 2, 3, 3}},
			{"flat_hold", "flat", {0, 0, 0, 0, 1, 1, 1, 1, 1, 1}},
			{"late_ramp", "late_ramp", {0, 0, 0, 1, 1, 1, 2, 3, 3}},
			{"spike", "spike", {0, 0, 1, 7, 1, 2, 3, 3}},
			{"early_ramp", "early_ramp", {0, 1, 1, 1, 2, 3, 3, 3}},
			{"normal_smooth", "normal", {0, 0, 1, 1, 2, 2, 3, 3}},
			{"flat_hold_long", "flat", {0, 0, 0, 0, 0, 1, 1, 1, 1, 1}},
			{"late_ramp_shifted", "late_ramp", {0, 0, 0, 0, 1, 1, 2, 3, 3}},
			{"spike_shifted", "spike", {0, 0, 0, 7, 1, 2, 3, 3}},
			{"early_ramp_short", "early_ramp", {0, 1, 1, 2, 2, 3, 3}},
			{"late_ramp_long_delay", "late_ramp", {0, 0, 0, 0, 0, 1, 2, 3, 3}},
			{"flat_hold_extra_long", "flat", {0, 0, 0, 0, 0, 0, 1, 1, 1, 1}},
			{"normal_soft_start", "normal", {0, 0, 1, 1, 1, 1, 2, 3, 3}},
			{"spike_wide", "spike", {0, 0, 0, 1, 8, 1, 2, 3, 3}},
			{"early_ramp_plateau", "early_ramp", {0, 1, 1, 1, 1, 2, 3, 3}}};
}

auto held_out_queries() -> std::vector<ProcessCurve>
{
	return {{"gallery_delay_A", "late_ramp", {0, 0, 0, 0, 1, 1, 1, 2, 3, 3}},
			{"gallery_delay_B", "late_ramp", {0, 0, 0, 0, 0, 1, 1, 2, 3, 3}},
			{"gallery_delay_C", "late_ramp", {0, 0, 0, 0, 1, 1, 2, 3}},
			{"gallery_delay_D", "late_ramp", {0, 0, 0, 0, 0, 1, 2, 3}},
			{"gallery_delay_E", "late_ramp", {0, 0, 0, 0, 0, 0, 1, 1, 2, 3, 3}},
			{"gallery_delay_F", "late_ramp", {0, 0, 0, 0, 1, 1, 1, 1, 2, 3, 3}}};
}

auto record_ids(const std::vector<ProcessCurve> &records) -> std::vector<std::string>
{
	std::vector<std::string> ids;
	ids.reserve(records.size());
	for (const auto &record : records) {
		ids.push_back(record.id);
	}
	return ids;
}

auto add_identity_dense_layer(mtrc::solve::parametric::dnn::Network<double> &network, std::size_t input_size,
							  std::size_t output_size) -> void
{
	mtrc::solve::parametric::dnn::FullyConnected<double, mtrc::solve::parametric::dnn::Identity<double>> layer(
		input_size, output_size);
	layer.initConstant(0.01, 0.0);
	network.addLayer(layer);
}

auto make_autoencoder_network(std::size_t feature_count) -> mtrc::solve::parametric::dnn::Network<double>
{
	mtrc::solve::parametric::dnn::Network<double> network;
	add_identity_dense_layer(network, feature_count, 2);
	add_identity_dense_layer(network, 2, feature_count);
	network.setOutput(mtrc::solve::parametric::dnn::RegressionMSE<double>());
	network.setOptimizer(mtrc::solve::parametric::dnn::RMSProp<double>(0.003, 1.0e-8, 0.0));
	return network;
}

auto find_epoch_term(const mtrc::solve::parametric::dnn::EpochReport<double> &epoch, const std::string &name)
	-> double
{
	for (const auto &term : epoch.terms) {
		if (term.name == name) {
			return term.value;
		}
	}
	throw std::runtime_error("missing native training report term: " + name);
}

template <typename Space> auto dense_distance_matrix(const Space &space) -> std::vector<std::vector<double>>
{
	const auto size = space.size();
	std::vector<std::vector<double>> distances(size, std::vector<double>(size, 0.0));
	const auto &metric = space.metric();
	for (std::size_t row = 0; row < size; ++row) {
		for (std::size_t column = 0; column < size; ++column) {
			distances[row][column] = metric(space.record(space.id(row)), space.record(space.id(column)));
		}
	}
	return distances;
}

auto diagnose_metric_law(const std::vector<std::vector<double>> &distances) -> MetricLawDiagnostics
{
	MetricLawDiagnostics diagnostics;
	const auto size = distances.size();
	for (std::size_t row = 0; row < size; ++row) {
		diagnostics.diagonal_max_abs = std::max(diagnostics.diagonal_max_abs, std::abs(distances[row][row]));
		for (std::size_t column = 0; column < size; ++column) {
			const auto value = distances[row][column];
			diagnostics.finite = diagnostics.finite && std::isfinite(value);
			diagnostics.min_distance = std::min(diagnostics.min_distance, value);
			diagnostics.max_distance = std::max(diagnostics.max_distance, value);
			diagnostics.symmetry_max_abs =
				std::max(diagnostics.symmetry_max_abs, std::abs(value - distances[column][row]));
			if (row != column) {
				diagnostics.mean_distance += value;
				++diagnostics.pair_count;
			}
			for (std::size_t via = 0; via < size; ++via) {
				const auto violation = distances[row][column] - distances[row][via] - distances[via][column];
				diagnostics.triangle_max_violation = std::max(diagnostics.triangle_max_violation, violation);
				++diagnostics.triangle_triplets;
			}
		}
	}
	if (diagnostics.pair_count != 0) {
		diagnostics.mean_distance /= static_cast<double>(diagnostics.pair_count);
	}
	if (!std::isfinite(diagnostics.min_distance)) {
		diagnostics.min_distance = 0.0;
	}
	return diagnostics;
}

auto metric_law_json(const MetricLawDiagnostics &law, const std::string &operator_name) -> std::string
{
	return json_object({{"checked", visual::quote("full finite native C++ distance matrix")},
						{"operator", visual::quote(operator_name)},
						{"finite", json_bool(law.finite)},
						{"non_negative", json_bool(law.min_distance >= -kEps)},
						{"diagonal_zero", json_bool(law.diagonal_max_abs <= kEps)},
						{"symmetric", json_bool(law.symmetry_max_abs <= kEps)},
						{"triangle", json_bool(law.triangle_max_violation <= kEps)},
						{"min_distance", visual::num(law.min_distance)},
						{"max_distance", visual::num(law.max_distance)},
						{"mean_distance", visual::num(law.mean_distance)},
						{"diagonal_max_abs", visual::num(law.diagonal_max_abs)},
						{"symmetry_max_abs", visual::num(law.symmetry_max_abs)},
						{"triangle_max_violation", visual::num(law.triangle_max_violation)},
						{"pair_count", json_size(law.pair_count)},
						{"triangle_triplets", json_size(law.triangle_triplets)}});
}

auto relation_values_json(const std::vector<std::string> &ids, const std::vector<std::vector<double>> &distances)
	-> std::string
{
	std::vector<std::string> values;
	values.reserve(ids.size() * ids.size());
	for (std::size_t row = 0; row < ids.size(); ++row) {
		for (std::size_t column = 0; column < ids.size(); ++column) {
			values.push_back(json_object({{"row_id", visual::quote(ids[row])},
										  {"column_id", visual::quote(ids[column])},
										  {"value", visual::num(distances[row][column])}}));
		}
	}
	return visual::array_of(values);
}

auto relation_json(const std::string &id, const std::string &name, const std::string &relation_type,
				   const std::vector<std::string> &ids, const std::vector<std::vector<double>> &distances,
				   const std::string &metadata) -> std::string
{
	return json_object({{"id", visual::quote(id)},
						{"dataset_id", visual::quote(kDatasetId)},
						{"name", visual::quote(name)},
						{"relation_type", visual::quote(relation_type)},
						{"value_type", visual::quote("scalar")},
						{"record_ids", visual::string_array(ids)},
						{"storage", visual::quote("dense_matrix")},
						{"values", relation_values_json(ids, distances)},
						{"metadata", metadata}});
}

auto space_json(const std::string &id, const std::string &primary_relation_id, const std::string &space_type,
				const std::vector<std::string> &ids, const std::string &metadata) -> std::string
{
	return json_object({{"id", visual::quote(id)},
						{"dataset_id", visual::quote(kDatasetId)},
						{"record_ids", visual::string_array(ids)},
						{"primary_relation_id", visual::quote(primary_relation_id)},
						{"space_type", visual::quote(space_type)},
						{"metadata", metadata}});
}

auto categorical_property_json(const std::string &id, const std::string &name, const std::vector<std::string> &ids,
							   const std::vector<std::string> &values, const std::string &metadata = "{}")
	-> std::string
{
	std::vector<std::string> entries;
	entries.reserve(ids.size());
	for (std::size_t index = 0; index < ids.size(); ++index) {
		entries.push_back(json_object({{"record_id", visual::quote(ids[index])},
									   {"value", visual::quote(values[index])}}));
	}
	return json_object({{"id", visual::quote(id)},
						{"dataset_id", visual::quote(kDatasetId)},
						{"name", visual::quote(name)},
						{"target_type", visual::quote("record")},
						{"value_type", visual::quote("categorical")},
						{"values", visual::array_of(entries)},
						{"metadata", metadata}});
}

auto scalar_property_json(const std::string &id, const std::string &name, const std::vector<std::string> &ids,
						  const std::vector<double> &values, const std::string &metadata = "{}") -> std::string
{
	std::vector<std::string> entries;
	entries.reserve(ids.size());
	for (std::size_t index = 0; index < ids.size(); ++index) {
		entries.push_back(json_object({{"record_id", visual::quote(ids[index])},
									   {"value", visual::num(values[index])}}));
	}
	return json_object({{"id", visual::quote(id)},
						{"dataset_id", visual::quote(kDatasetId)},
						{"name", visual::quote(name)},
						{"target_type", visual::quote("record")},
						{"value_type", visual::quote("scalar")},
						{"values", visual::array_of(entries)},
						{"metadata", metadata}});
}

auto vector_norm(const std::vector<double> &values) -> double
{
	double total = 0.0;
	for (const auto value : values) {
		total += value * value;
	}
	return std::sqrt(total);
}

auto curve_mean(const ProcessCurve &record) -> double
{
	double total = 0.0;
	for (const auto value : record.values) {
		total += value;
	}
	return record.values.empty() ? 0.0 : total / static_cast<double>(record.values.size());
}

auto curve_total_variation(const ProcessCurve &record) -> double
{
	double total = 0.0;
	for (std::size_t index = 1; index < record.values.size(); ++index) {
		total += std::abs(record.values[index] - record.values[index - 1]);
	}
	return total;
}

auto source_feature_positions(const std::vector<ProcessCurve> &records) -> std::vector<std::vector<double>>
{
	CurveFeatureEncoder encoder{kFeatureCount};
	std::vector<std::vector<double>> positions;
	positions.reserve(records.size());
	for (const auto &record : records) {
		const auto features = encoder(record);
		positions.push_back({features.front(), features[features.size() / 2], features.back()});
	}
	return positions;
}

auto coordinate_json(const std::string &id, const std::string &space_id, const std::string &name,
					 const std::vector<std::string> &ids, const std::vector<std::vector<double>> &positions,
					 const std::string &metadata) -> std::string
{
	if (positions.empty()) {
		throw std::invalid_argument("coordinate export requires at least one position");
	}
	const auto dimension = positions.front().size();
	std::vector<std::string> entries;
	entries.reserve(ids.size());
	for (std::size_t index = 0; index < ids.size(); ++index) {
		entries.push_back(json_object({{"record_id", visual::quote(ids[index])},
									   {"position", json_number_array(positions[index])}}));
	}
	return json_object({{"id", visual::quote(id)},
						{"dataset_id", visual::quote(kDatasetId)},
						{"space_id", visual::quote(space_id)},
						{"name", visual::quote(name)},
						{"dimension", json_size(dimension)},
						{"record_positions", visual::array_of(entries)},
						{"metadata", metadata}});
}

auto nearest_indices(const std::vector<std::vector<double>> &distances, std::size_t row, std::size_t count)
	-> std::vector<std::size_t>
{
	std::vector<std::pair<double, std::size_t>> candidates;
	candidates.reserve(distances.size() - 1);
	for (std::size_t column = 0; column < distances.size(); ++column) {
		if (row == column) {
			continue;
		}
		candidates.push_back({distances[row][column], column});
	}
	std::sort(candidates.begin(), candidates.end());
	if (candidates.size() > count) {
		candidates.resize(count);
	}

	std::vector<std::size_t> out;
	out.reserve(candidates.size());
	for (const auto &candidate : candidates) {
		out.push_back(candidate.second);
	}
	return out;
}

auto knn_graph_json(const std::vector<std::string> &ids, const std::vector<std::vector<double>> &distances)
	-> std::string
{
	std::vector<std::string> edges;
	edges.reserve(ids.size() * kNeighborCount);
	for (std::size_t row = 0; row < ids.size(); ++row) {
		for (const auto column : nearest_indices(distances, row, kNeighborCount)) {
			edges.push_back(json_object({{"source_id", visual::quote(ids[row])},
										 {"target_id", visual::quote(ids[column])},
										 {"weight", visual::num(distances[row][column])}}));
		}
	}
	return json_object({{"id", visual::quote(kGraphId)},
						{"dataset_id", visual::quote(kDatasetId)},
						{"node_record_ids", visual::string_array(ids)},
						{"edge_relation_id", visual::quote(kSourceRelationId)},
						{"graph_type", visual::quote("k-nearest")},
						{"edges", visual::array_of(edges)},
						{"metadata", json_object({{"neighbors_per_record", json_size(kNeighborCount)},
												   {"edge_source", visual::quote("native aligned curve metric")}})}});
}

auto row_mean_distance(const std::vector<std::vector<double>> &distances, std::size_t row) -> double
{
	if (distances.size() < 2) {
		return 0.0;
	}
	double total = 0.0;
	for (std::size_t column = 0; column < distances.size(); ++column) {
		if (row != column) {
			total += distances[row][column];
		}
	}
	return total / static_cast<double>(distances.size() - 1);
}

auto local_distortion(const std::vector<std::vector<double>> &source, const std::vector<std::vector<double>> &mapped)
	-> std::vector<double>
{
	const auto source_law = diagnose_metric_law(source);
	const auto mapped_law = diagnose_metric_law(mapped);
	const auto source_scale = std::max(source_law.mean_distance, kEps);
	const auto mapped_scale = std::max(mapped_law.mean_distance, kEps);
	std::vector<double> values(source.size(), 0.0);
	for (std::size_t row = 0; row < source.size(); ++row) {
		double total = 0.0;
		std::size_t count = 0;
		for (std::size_t column = 0; column < source.size(); ++column) {
			if (row == column) {
				continue;
			}
			total += std::abs((source[row][column] / source_scale) - (mapped[row][column] / mapped_scale));
			++count;
		}
		values[row] = count == 0 ? 0.0 : total / static_cast<double>(count);
	}
	return values;
}

auto target_residuals(const std::vector<std::vector<double>> &target_positions,
					  const std::vector<std::vector<double>> &latent_positions) -> std::vector<double>
{
	std::vector<double> residuals;
	residuals.reserve(target_positions.size());
	for (std::size_t row = 0; row < target_positions.size(); ++row) {
		std::vector<double> delta;
		delta.reserve(target_positions[row].size());
		for (std::size_t axis = 0; axis < target_positions[row].size(); ++axis) {
			delta.push_back(latent_positions[row][axis] - target_positions[row][axis]);
		}
		residuals.push_back(vector_norm(delta));
	}
	return residuals;
}

auto query_diagnostics_json(const std::vector<ProcessCurve> &records, const std::vector<ProcessCurve> &queries,
							const std::vector<std::string> &ids, const std::vector<std::vector<double>> &source,
							const std::vector<std::vector<double>> &latent,
							const std::vector<std::vector<double>> &query_latent_positions) -> std::string
{
	std::vector<std::string> entries;
	entries.reserve(queries.size());
	AlignedCurveDistance metric;
	for (std::size_t query_index = 0; query_index < queries.size(); ++query_index) {
		std::size_t best_source = 0;
		double best_source_distance = std::numeric_limits<double>::infinity();
		for (std::size_t record_index = 0; record_index < records.size(); ++record_index) {
			const auto distance = metric(queries[query_index], records[record_index]);
			if (distance < best_source_distance) {
				best_source_distance = distance;
				best_source = record_index;
			}
		}

		std::size_t best_latent = 0;
		double best_latent_distance = std::numeric_limits<double>::infinity();
		for (std::size_t record_index = 0; record_index < records.size(); ++record_index) {
			double squared = 0.0;
			for (std::size_t axis = 0; axis < query_latent_positions[query_index].size(); ++axis) {
				const auto delta = query_latent_positions[query_index][axis] - latent[record_index][axis];
				squared += delta * delta;
			}
			const auto distance = std::sqrt(squared);
			if (distance < best_latent_distance) {
				best_latent_distance = distance;
				best_latent = record_index;
			}
		}

		entries.push_back(json_object({
			{"query_id", visual::quote(queries[query_index].id)},
			{"query_family", visual::quote(queries[query_index].family)},
			{"source_metric_nearest_record_id", visual::quote(ids[best_source])},
			{"source_metric_nearest_family", visual::quote(records[best_source].family)},
			{"source_metric_distance", visual::num(best_source_distance)},
			{"latent_nearest_record_id", visual::quote(ids[best_latent])},
			{"latent_nearest_family", visual::quote(records[best_latent].family)},
			{"latent_distance", visual::num(best_latent_distance)},
			{"native_source_metric_row_mean", visual::num(row_mean_distance(source, best_source))},
		}));
	}
	return visual::array_of(entries);
}

template <typename LatentResult>
auto latent_positions(const LatentResult &latent) -> std::vector<std::vector<double>>
{
	std::vector<std::vector<double>> positions;
	positions.reserve(latent.space.size());
	for (std::size_t row = 0; row < latent.space.size(); ++row) {
		positions.push_back(latent.space.record(latent.space.id(row)));
	}
	return positions;
}

template <typename Space, typename Targets>
auto target_positions_in_space_order(const Space &space, const Targets &targets) -> std::vector<std::vector<double>>
{
	std::vector<std::vector<double>> positions;
	positions.reserve(space.size());
	for (std::size_t row = 0; row < space.size(); ++row) {
		positions.push_back(targets.coordinates.at(space.id(row)));
	}
	return positions;
}

auto build_visual_document() -> std::string
{
	const auto records = process_records();
	const auto queries = held_out_queries();
	const auto ids = record_ids(records);

	auto space = mtrc::make_space(records, AlignedCurveDistance{});

	mtrc::modify::map::PhateGeometrySpec<double> geometry;
	geometry.dimensions = 2;
	geometry.diffusion_steps = 2;
	geometry.kernel_scale = 1.0;

	auto codec = mtrc::solve::parametric::dnn::make_feature_record_codec<ProcessCurve, double>(
		kFeatureCount, CurveFeatureEncoder{kFeatureCount}, "process_curve_gallery_feature_codec");
	assert(codec.feature_count() == kFeatureCount);
	assert(!codec.inverse_supported());

	mtrc::solve::parametric::dnn::TrainingSpec<double> training;
	training.epochs = 220;
	training.batch_size = records.size();
	training.shuffle = false;
	training.seed = 41;
	training.gradient_clip_norm = 20.0;

	const auto pipeline_plan = mtrc::modify::compose::native_phate_autoencoder_pipeline_builder(0.02, 1.0)
								   .use_feature_record_codec()
								   .use_distance_table_pairwise_distances()
								   .use_exponential_affinity_kernel()
								   .use_lazy_row_normalized_diffusion_operator()
								   .plan();
	const auto hooks = mtrc::modify::compose::resolve_native_phate_autoencoder_executable_hooks<double>(pipeline_plan);
	if (!hooks.executable()) {
		throw std::runtime_error("native PHATE autoencoder hooks are not executable");
	}

	const auto targets = mtrc::modify::map::phate_geometry_targets<decltype(space), double>(
		space, geometry, hooks.pairwise_distances, hooks.affinity_kernel, hooks.diffusion_operator);
	assert(targets.record_count == records.size());
	assert(targets.coordinates.size() == records.size());

	auto mapping = mtrc::modify::map::native_phate_autoencoder_with_codec(
		mtrc::solve::parametric::dnn::AutoencoderModel<double>(make_autoencoder_network(kFeatureCount)), geometry,
		training, codec, pipeline_plan);
	auto model = mtrc::modify::map::fit(mapping, space);
	const auto latent = mtrc::modify::map::transform(model, space);
	assert(latent.mapping == "native_phate_autoencoder");
	assert(latent.strategy == "native_dnn_phate_ae");
	assert(latent.space.size() == records.size());

	auto query_space = mtrc::make_space(queries, AlignedCurveDistance{});
	const auto query_latent = mtrc::modify::map::transform(model, query_space);
	const auto preservation = mtrc::modify::map::neighbor_preservation(space, latent, kNeighborCount);
	const auto oos_stability = mtrc::modify::map::out_of_sample_neighbor_stability(model, space, query_space, kNeighborCount);

	const auto source_distances = dense_distance_matrix(space);
	const auto latent_distances = dense_distance_matrix(latent.space);
	const auto source_law = diagnose_metric_law(source_distances);
	const auto latent_law = diagnose_metric_law(latent_distances);
	const auto target_coords = target_positions_in_space_order(space, targets);
	const auto latent_coords = latent_positions(latent);
	const auto query_latent_coords = latent_positions(query_latent);

	const auto &report = model.training_report();
	if (report.epochs.empty()) {
		throw std::runtime_error("native training report is empty");
	}
	const auto initial_bottleneck = find_epoch_term(report.epochs.front(), "bottleneck_coordinate_mse");
	const auto final_bottleneck = find_epoch_term(report.epochs.back(), "bottleneck_coordinate_mse");

	std::vector<std::string> datasets;
	datasets.push_back(json_object({
		{"id", visual::quote(kDatasetId)},
		{"title", visual::quote("Mapping And Dimensionality Process Curves")},
		{"description", visual::quote("Native C++ PHATE autoencoder mapping evidence over process curves.")},
		{"source", visual::quote("examples/engine/process_curve_phate_gallery.cpp")},
		{"license", visual::quote("MPL-2.0")},
	}));

	std::vector<std::string> record_entries;
	record_entries.reserve(records.size());
	for (const auto &record : records) {
		const auto payload = json_object({{"kind", visual::quote("time_series")},
										  {"series", json_number_array(record.values)},
										  {"sample_rate_hz", visual::num(1.0)},
										  {"family", visual::quote(record.family)},
										  {"native_source", visual::quote("process_curve_phate_gallery.cpp")}});
		record_entries.push_back(json_object({{"id", visual::quote(record.id)},
											  {"dataset_id", visual::quote(kDatasetId)},
											  {"record_type", visual::quote("process_curve")},
											  {"label", visual::quote(record.id + " / " + record.family)},
											  {"payload", payload}}));
	}

	std::vector<std::string> relations;
	relations.push_back(relation_json(
		kSourceRelationId, "aligned process-curve metric", "metric", ids, source_distances,
		json_object({{"symmetric", "true"},
					 {"complete", "true"},
					 {"source", visual::quote("AlignedCurveDistance from process_curve_phate_gallery.cpp")},
					 {"law_check", metric_law_json(source_law, "AlignedCurveDistance")}})));
	relations.push_back(relation_json(
		kLatentRelationId, "latent Euclidean metric after native PHATE autoencoder", "metric", ids, latent_distances,
		json_object({{"symmetric", "true"},
					 {"complete", "true"},
					 {"source", visual::quote("Euclidean metric on native mapped coordinates")},
					 {"mapping", visual::quote(latent.mapping)},
					 {"strategy", visual::quote(latent.strategy)},
					 {"law_check", metric_law_json(latent_law, "mtrc::Euclidean<double>")}})));

	std::vector<std::string> spaces;
	spaces.push_back(space_json(kSourceSpaceId, kSourceRelationId, "finite_metric_space", ids,
								json_object({{"record_count", json_size(records.size())},
											 {"metric_operator", visual::quote("AlignedCurveDistance")},
											 {"native_mapping_input", "true"}})));
	spaces.push_back(space_json(kLatentSpaceId, kLatentRelationId, "derived_coordinate_space", ids,
								json_object({{"source_space_id", visual::quote(kSourceSpaceId)},
											 {"mapping", visual::quote(latent.mapping)},
											 {"strategy", visual::quote(latent.strategy)},
											 {"inverse_supported", json_bool(latent.inverse_supported)},
											 {"out_of_sample_supported", json_bool(latent.out_of_sample_supported)},
											 {"source_record_count", json_size(latent.source_record_count)}})));

	std::vector<std::string> family_values;
	std::vector<double> lengths;
	std::vector<double> means;
	std::vector<double> variations;
	std::vector<double> row_mean_source;
	family_values.reserve(records.size());
	lengths.reserve(records.size());
	means.reserve(records.size());
	variations.reserve(records.size());
	row_mean_source.reserve(records.size());
	for (std::size_t row = 0; row < records.size(); ++row) {
		family_values.push_back(records[row].family);
		lengths.push_back(static_cast<double>(records[row].values.size()));
		means.push_back(curve_mean(records[row]));
		variations.push_back(curve_total_variation(records[row]));
		row_mean_source.push_back(row_mean_distance(source_distances, row));
	}

	const auto distortion = local_distortion(source_distances, latent_distances);
	const auto target_residual = target_residuals(target_coords, latent_coords);
	std::vector<std::string> properties;
	properties.push_back(categorical_property_json(
		"process-family", "process family", ids, family_values,
		json_object({{"source", visual::quote("native ProcessCurve.family")}})));
	properties.push_back(scalar_property_json("curve-length", "curve sample count", ids, lengths));
	properties.push_back(scalar_property_json("curve-mean", "curve mean", ids, means));
	properties.push_back(scalar_property_json("curve-total-variation", "curve total variation", ids, variations));
	properties.push_back(scalar_property_json(
		"source-row-mean-distance", "mean aligned-curve distance", ids, row_mean_source,
		json_object({{"relation_id", visual::quote(kSourceRelationId)}})));
	properties.push_back(scalar_property_json(
		"local-mapping-distortion", "local normalized distance distortion", ids, distortion,
		json_object({{"source_relation_id", visual::quote(kSourceRelationId)},
					 {"mapped_relation_id", visual::quote(kLatentRelationId)},
					 {"calculation", visual::quote("mean absolute normalized pairwise distance difference per row")}})));
	properties.push_back(scalar_property_json(
		"phate-target-residual", "PHATE target fit residual", ids, target_residual,
		json_object({{"target_generator", visual::quote(targets.method)},
					 {"calculation", visual::quote("Euclidean distance between target coordinate and mapped latent coordinate")}})));

	std::vector<std::string> coordinates;
	coordinates.push_back(coordinate_json(
		"source-feature-layout-3d", kSourceSpaceId, "native feature-codec layout", ids, source_feature_positions(records),
		json_object({{"source", visual::quote("CurveFeatureEncoder used by native feature_record_codec")},
					 {"metric_embedding", "false"},
					 {"purpose", visual::quote("original/native coordinate state for inspecting source records")}})));
	coordinates.push_back(coordinate_json(
		"phate-target-2d", kLatentSpaceId, "native PHATE diffusion target", ids, target_coords,
		json_object({{"target_generator", visual::quote(targets.method)},
					 {"pairwise_distances", visual::quote(targets.pairwise_distances)},
					 {"affinity_kernel", visual::quote(targets.affinity_kernel)},
					 {"diffusion_operator", visual::quote(targets.diffusion_operator)}})));
	coordinates.push_back(coordinate_json(
		"native-autoencoder-latent-2d", kLatentSpaceId, "native PHATE autoencoder latent map", ids, latent_coords,
		json_object({{"mapping", visual::quote(latent.mapping)},
					 {"strategy", visual::quote(latent.strategy)},
					 {"solver", visual::quote("native_dnn_autoencoder_trainer")},
					 {"training_seed", json_size(training.seed)}})));

	std::vector<std::string> graphs;
	graphs.push_back(knn_graph_json(ids, source_distances));

	std::vector<std::string> diagnostics;
	diagnostics.push_back(json_object({
		{"id", visual::quote("native-pipeline-path")},
		{"dataset_id", visual::quote(kDatasetId)},
		{"diagnostic_type", visual::quote("native-mapping-pipeline")},
		{"payload", json_object({{"source_examples", visual::string_array({"examples/engine/process_curve_phate_gallery.cpp",
																			"examples/engine/metric_space_mapping_pipeline.cpp",
																			"examples/engine/phate_pipeline_builder.cpp"})},
								 {"pipeline", visual::quote(model.pipeline_plan().name())},
								 {"component_count", json_size(model.pipeline_plan().component_count())},
								 {"codec", visual::quote(hooks.codec)},
								 {"pairwise_distances", visual::quote(hooks.pairwise_distances)},
								 {"affinity_kernel", visual::quote(hooks.affinity_kernel)},
								 {"diffusion_operator", visual::quote(hooks.diffusion_operator)},
								 {"target_method", visual::quote(targets.method)},
								 {"target_dimensions", json_size(targets.dimensions)},
								 {"dense_distance_evaluations", json_size(targets.dense_distance_evaluations)}})}
	}));
	diagnostics.push_back(json_object({
		{"id", visual::quote("native-training-report")},
		{"dataset_id", visual::quote(kDatasetId)},
		{"diagnostic_type", visual::quote("native-dnn-training")},
		{"payload", json_object({{"epochs", json_size(report.epochs.size())},
								 {"seed", json_size(training.seed)},
								 {"batch_size", json_size(training.batch_size)},
								 {"initial_bottleneck_coordinate_mse", visual::num(initial_bottleneck)},
								 {"final_bottleneck_coordinate_mse", visual::num(final_bottleneck)},
								 {"loss_source", visual::quote("mtrc::solve::parametric::dnn::TrainingReport")}})}
	}));
	diagnostics.push_back(json_object({
		{"id", visual::quote("neighbor-preservation")},
		{"dataset_id", visual::quote(kDatasetId)},
		{"diagnostic_type", visual::quote(preservation.diagnostic)},
		{"payload", json_object({{"source_record_count", json_size(preservation.source_record_count)},
								 {"mapped_record_count", json_size(preservation.mapped_record_count)},
								 {"requested_neighbor_count", json_size(preservation.requested_neighbor_count)},
								 {"evaluated_neighbor_count", json_size(preservation.evaluated_neighbor_count)},
								 {"matched_neighbors", json_size(preservation.matched_neighbors)},
								 {"possible_neighbors", json_size(preservation.possible_neighbors)},
								 {"recall", visual::num(preservation.recall)},
								 {"average_row_recall", visual::num(preservation.average_row_recall)},
								 {"exact", json_bool(preservation.exact)}})}
	}));
	diagnostics.push_back(json_object({
		{"id", visual::quote("out-of-sample-stability")},
		{"dataset_id", visual::quote(kDatasetId)},
		{"diagnostic_type", visual::quote(oos_stability.diagnostic)},
		{"payload", json_object({{"source_record_count", json_size(oos_stability.source_record_count)},
								 {"query_record_count", json_size(oos_stability.query_record_count)},
								 {"evaluated_neighbor_count", json_size(oos_stability.evaluated_neighbor_count)},
								 {"anchor_recall", visual::num(oos_stability.anchor_recall)},
								 {"first_anchor_match_rate", visual::num(oos_stability.first_anchor_match_rate)},
								 {"average_mapped_best_source_rank", visual::num(oos_stability.average_mapped_best_source_rank)},
								 {"maximum_mapped_best_source_rank", json_size(oos_stability.maximum_mapped_best_source_rank)},
								 {"average_best_distance_penalty", visual::num(oos_stability.average_best_distance_penalty)},
								 {"maximum_best_distance_penalty", visual::num(oos_stability.maximum_best_distance_penalty)},
								 {"transform_supported", json_bool(oos_stability.transform_supported)},
								 {"query_nearest_records", query_diagnostics_json(records, queries, ids, source_distances,
																				  latent_coords, query_latent_coords)}})}
	}));

	std::vector<std::string> views;
	views.push_back(json_object({{"id", visual::quote("mapping-dimensionality-source-view")},
								 {"kind", visual::quote("metric-space")},
								 {"name", visual::quote("Process-curve source metric space")},
								 {"spaceId", visual::quote(kSourceSpaceId)},
								 {"coordinateId", visual::quote("source-feature-layout-3d")},
								 {"propertyId", visual::quote("process-family")}}));
	views.push_back(json_object({{"id", visual::quote("mapping-dimensionality-latent-view")},
								 {"kind", visual::quote("metric-space")},
								 {"name", visual::quote("Native PHATE autoencoder latent map")},
								 {"spaceId", visual::quote(kLatentSpaceId)},
								 {"coordinateId", visual::quote("native-autoencoder-latent-2d")},
								 {"propertyId", visual::quote("local-mapping-distortion")}}));
	views.push_back(json_object({{"id", visual::quote("mapping-dimensionality-relation-view")},
								 {"kind", visual::quote("relation-matrix")},
								 {"name", visual::quote("Aligned process-curve distance matrix")},
								 {"relationId", visual::quote(kSourceRelationId)}}));

	std::ostringstream out;
	out << "{\n";
	out << "  \"schema\": \"metric.visual.v1\",\n";
	out << "  \"provenance\": "
		<< json_object({{"generator", visual::quote("examples/engine/mapping_dimensionality_visual_export.cpp")},
						{"runtime", visual::quote("native C++")},
						{"native_export", "true"},
						{"algorithm_source", visual::quote("native PHATE autoencoder mapping pipeline")},
						{"source_examples", visual::string_array({"examples/engine/process_curve_phate_gallery.cpp",
																  "examples/engine/metric_space_mapping_pipeline.cpp",
																  "examples/engine/phate_pipeline_builder.cpp"})},
						{"synthetic_js", "false"}})
		<< ",\n";
	out << "  \"datasets\": " << visual::array_of(datasets) << ",\n";
	out << "  \"records\": " << visual::array_of(record_entries) << ",\n";
	out << "  \"relations\": " << visual::array_of(relations) << ",\n";
	out << "  \"spaces\": " << visual::array_of(spaces) << ",\n";
	out << "  \"properties\": " << visual::array_of(properties) << ",\n";
	out << "  \"graphs\": " << visual::array_of(graphs) << ",\n";
	out << "  \"coordinates\": " << visual::array_of(coordinates) << ",\n";
	out << "  \"timelines\": [],\n";
	out << "  \"events\": [],\n";
	out << "  \"views\": " << visual::array_of(views) << ",\n";
	out << "  \"diagnostics\": " << visual::array_of(diagnostics) << "\n";
	out << "}\n";
	return out.str();
}

auto usage(const char *program) -> std::string
{
	std::ostringstream out;
	out << "usage: " << program << " [--export-dir <dir>]\n";
	out << "  without --export-dir, writes metric.visual.v1 JSON to stdout\n";
	return out.str();
}

} // namespace

int main(int argc, char **argv)
{
	try {
		std::filesystem::path export_dir;
		for (int index = 1; index < argc; ++index) {
			const std::string arg = argv[index];
			if (arg == "--help" || arg == "-h") {
				std::cout << usage(argv[0]);
				return 0;
			}
			if (arg == "--export-dir") {
				if (index + 1 >= argc) {
					throw std::invalid_argument("--export-dir requires a directory argument");
				}
				export_dir = argv[++index];
				continue;
			}
			throw std::invalid_argument("unknown argument: " + arg);
		}

		const auto json = build_visual_document();
		if (export_dir.empty()) {
			std::cout << json;
			return 0;
		}

		std::filesystem::create_directories(export_dir);
		const auto output_path = export_dir / "metric.visual.json";
		std::ofstream out(output_path);
		if (!out) {
			throw std::runtime_error("failed to open output file: " + output_path.string());
		}
		out << json;
		if (!out) {
			throw std::runtime_error("failed to write output file: " + output_path.string());
		}
		std::cerr << "wrote " << output_path << "\n";
		return 0;
	} catch (const std::exception &error) {
		std::cerr << "mapping_dimensionality_visual_export: " << error.what() << "\n";
		std::cerr << usage(argv[0]);
		return 1;
	}
}
