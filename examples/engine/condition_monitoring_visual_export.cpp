// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// Native metric.visual.v1 exporter for the condition-monitoring engine example.
//
// All condition-monitoring evidence is computed here in C++: TWED distances,
// nearest-reference scores, DBSCAN outliers, metric-law diagnostics, and
// trajectory evidence. JavaScript is used only by the acceptance validator.

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <metric/engine.hpp>
#include <metric/metric/catalog.hpp>

#include "../../visual/cpp/mtrc_visual.hpp"

namespace {

namespace visual = mtrc::visual;

using Curve = std::vector<double>;

enum class Regime { Normal, Drift, Fault, Recovery, Signature };

struct Catalog {
	std::vector<Curve> curves;
	std::vector<Regime> families;
};

struct Observation {
	Curve curve;
	Regime truth{};
	std::string phase;
	double condition_index{};
};

struct MetricLawDiagnostics {
	double diagonal_max_abs{};
	double symmetry_max_abs{};
	double triangle_max_violation{};
	double minimum_nonzero_distance{std::numeric_limits<double>::infinity()};
	double maximum_distance{};
	double average_distance{};
	std::size_t pair_count{};
	std::size_t triangle_triplets{};
	bool finite{true};
};

struct ObservationEvidence {
	std::string id;
	double nearest_healthy_distance{};
	double nearest_catalog_distance{};
	double severity{};
	double local_density{};
	bool dbscan_density_outlier{};
	Regime nearest_catalog_regime{};
	std::string diagnosis_state;
};

auto hash_unit(std::uint64_t seed) -> double
{
	std::uint64_t z = seed + 0x9E3779B97F4A7C15ULL;
	z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
	z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
	z = z ^ (z >> 31);
	return static_cast<double>(z >> 11) / 9007199254740992.0 * 2.0 - 1.0;
}

auto jitter(std::uint64_t stream, int position, double scale) -> double
{
	return scale * hash_unit(stream * 1000003ULL + static_cast<std::uint64_t>(position + 1));
}

auto regime_name(Regime regime) -> std::string
{
	switch (regime) {
	case Regime::Normal:
		return "normal";
	case Regime::Drift:
		return "drift";
	case Regime::Fault:
		return "fault";
	case Regime::Recovery:
		return "recovery";
	case Regime::Signature:
		return "signature";
	}
	return "unknown";
}

auto trapezoid(int rise_start, int ramp, int plateau, int tail, double amplitude, double baseline,
			   std::uint64_t seed) -> Curve
{
	Curve curve;
	int pos = 0;
	for (int i = 0; i < rise_start; ++i, ++pos) {
		curve.push_back(baseline + jitter(seed, pos, 0.02));
	}
	for (int i = 1; i <= ramp; ++i, ++pos) {
		curve.push_back(baseline + amplitude * static_cast<double>(i) / ramp + jitter(seed, pos, 0.02));
	}
	for (int i = 0; i < plateau; ++i, ++pos) {
		curve.push_back(baseline + amplitude + jitter(seed, pos, 0.03));
	}
	for (int i = ramp - 1; i >= 1; --i, ++pos) {
		curve.push_back(baseline + amplitude * static_cast<double>(i) / ramp + jitter(seed, pos, 0.02));
	}
	for (int i = 0; i < tail; ++i, ++pos) {
		curve.push_back(baseline + jitter(seed, pos, 0.02));
	}
	return curve;
}

auto healthy_cycle(int rise_start, int ramp, int plateau, int tail, std::uint64_t seed) -> Curve
{
	return trapezoid(rise_start, ramp, plateau, tail, 1.0, 0.0, seed);
}

auto drift_cycle(double step, std::uint64_t seed) -> Curve
{
	const double creep = 0.16 * static_cast<double>(step);
	const double amplitude = 1.0 + 0.14 * static_cast<double>(step);
	return trapezoid(3, 3, 3, 3, amplitude, creep, seed);
}

auto fault_cycle(std::uint64_t seed) -> Curve
{
	Curve curve;
	int pos = 0;
	for (int i = 0; i < 2; ++i, ++pos) {
		curve.push_back(jitter(seed, pos, 0.02));
	}
	const double peaks[] = {1.8, 3.6, 2.0, 3.2, 1.5};
	for (double value : peaks) {
		curve.push_back(value + jitter(seed, pos++, 0.04));
	}
	for (int i = 0; i < 3; ++i, ++pos) {
		curve.push_back(jitter(seed, pos, 0.02));
	}
	return curve;
}

auto signature_cycle(int rise_start, int notch_offset, std::uint64_t seed) -> Curve
{
	const int ramp = 3;
	const int plateau = 6;
	Curve curve = trapezoid(rise_start, ramp, plateau, 3, 1.0, 0.0, seed);
	const int notch_start = rise_start + ramp + notch_offset;
	for (int i = 0; i < 3; ++i) {
		const int index = notch_start + i;
		if (index >= 0 && index < static_cast<int>(curve.size())) {
			curve[index] = jitter(seed, 500 + index, 0.03);
		}
	}
	return curve;
}

auto process_metric() -> mtrc::TWED<double> { return mtrc::TWED<double>(/*penalty=*/0.5, /*elastic=*/0.001); }

auto build_reference_catalog() -> Catalog
{
	Catalog catalog;
	auto add = [&](Curve curve, Regime regime) {
		catalog.curves.push_back(std::move(curve));
		catalog.families.push_back(regime);
	};

	add(healthy_cycle(2, 3, 3, 3, 11), Regime::Normal);
	add(healthy_cycle(3, 3, 2, 3, 12), Regime::Normal);
	add(healthy_cycle(1, 4, 3, 2, 13), Regime::Normal);
	add(healthy_cycle(4, 3, 2, 2, 14), Regime::Normal);
	add(healthy_cycle(2, 4, 2, 2, 15), Regime::Normal);
	add(healthy_cycle(3, 2, 4, 3, 16), Regime::Normal);
	add(drift_cycle(4, 21), Regime::Drift);
	add(drift_cycle(5, 22), Regime::Drift);
	add(drift_cycle(6, 23), Regime::Drift);
	add(fault_cycle(31), Regime::Fault);
	add(fault_cycle(32), Regime::Fault);
	add(signature_cycle(2, 0, 41), Regime::Signature);
	add(signature_cycle(2, 3, 42), Regime::Signature);
	add(signature_cycle(3, 1, 43), Regime::Signature);
	return catalog;
}

auto healthy_cycle_variant(std::size_t variant, std::uint64_t seed) -> Curve
{
	switch (variant % 6) {
	case 0:
		return healthy_cycle(2, 3, 3, 3, seed);
	case 1:
		return healthy_cycle(3, 3, 2, 3, seed);
	case 2:
		return healthy_cycle(1, 4, 3, 2, seed);
	case 3:
		return healthy_cycle(4, 3, 2, 2, seed);
	case 4:
		return healthy_cycle(2, 4, 2, 2, seed);
	default:
		return healthy_cycle(3, 2, 4, 3, seed);
	}
}

auto clamp01(double value) -> double
{
	return std::max(0.0, std::min(1.0, value));
}

auto build_observations() -> std::vector<Observation>
{
	std::vector<Observation> observations;
	auto add = [&](Curve curve, Regime truth, std::string phase, double condition_index) {
		observations.push_back({std::move(curve), truth, std::move(phase), clamp01(condition_index)});
	};

	constexpr std::size_t stable_count = 128;
	constexpr std::size_t drift_count = 180;
	constexpr std::size_t fault_count = 4;
	constexpr std::size_t recovery_count = 112;
	constexpr std::size_t signature_count = 104;

	for (std::size_t index = 0; index < stable_count; ++index) {
		add(healthy_cycle_variant(index, 7000 + index), Regime::Normal, "stable-reference",
			0.05 + jitter(9000 + index, 0, 0.012));
	}

	for (std::size_t index = 0; index < drift_count; ++index) {
		const double fraction = static_cast<double>(index) / static_cast<double>(drift_count - 1);
		const double step = 0.2 + 6.6 * fraction;
		add(drift_cycle(step, 7100 + index), Regime::Drift, "slow-drift", 0.12 + 0.82 * fraction);
	}

	for (std::size_t index = 0; index < fault_count; ++index) {
		add(fault_cycle(7200 + index), Regime::Fault, "abrupt-change",
			0.94 + jitter(9300 + index, 0, 0.015));
	}

	for (std::size_t index = 0; index < recovery_count; ++index) {
		const double fraction = static_cast<double>(index) / static_cast<double>(recovery_count - 1);
		const double step = 5.8 * (1.0 - fraction);
		add(drift_cycle(step, 7300 + index), Regime::Recovery, "recovery", 0.10 + 0.78 * (1.0 - fraction));
	}

	for (std::size_t index = 0; index < signature_count; ++index) {
		const int rise_start = 2 + static_cast<int>(index % 5);
		const int notch_offset = static_cast<int>((index / 5) % 4);
		const double recurrence_wave =
			0.5 + 0.5 * std::sin(static_cast<double>(index) * 6.2831853071795864769 / 26.0);
		add(signature_cycle(rise_start, notch_offset, 7400 + index), Regime::Signature, "recurring-signature",
			0.46 + 0.12 * recurrence_wave + jitter(9400 + index, 0, 0.015));
	}

	return observations;
}

template <typename Space>
auto healthy_radius(const Space &space, const std::vector<Regime> &families) -> double
{
	double radius = 0.0;
	for (std::size_t i = 0; i < space.size(); ++i) {
		if (families[i] != Regime::Normal) {
			continue;
		}
		double best = std::numeric_limits<double>::infinity();
		for (std::size_t j = 0; j < space.size(); ++j) {
			if (i == j || families[j] != Regime::Normal) {
				continue;
			}
			best = std::min(best, space.metric()(space[space.id(i)], space[space.id(j)]));
		}
		if (std::isfinite(best)) {
			radius = std::max(radius, best);
		}
	}
	return radius;
}

template <typename Space>
auto distance_to_healthy(const Space &space, const std::vector<Regime> &families, const Curve &query) -> double
{
	double best = std::numeric_limits<double>::infinity();
	for (std::size_t i = 0; i < space.size(); ++i) {
		if (families[i] != Regime::Normal) {
			continue;
		}
		best = std::min(best, space.metric()(query, space[space.id(i)]));
	}
	return best;
}

template <typename Space>
auto classify(const Space &space, const std::vector<Regime> &families, const Curve &query)
	-> std::pair<Regime, double>
{
	const auto neighbors = mtrc::find_neighbors(space, query, 1);
	if (neighbors.empty()) {
		throw std::runtime_error("nearest-neighbor classification returned no neighbors");
	}
	return {families[neighbors[0].id.index()], neighbors[0].distance};
}

auto record_id(std::size_t index) -> std::string
{
	std::ostringstream out;
	out << "window-" << std::setw(4) << std::setfill('0') << index;
	return out.str();
}

auto make_record_ids(std::size_t count) -> std::vector<std::string>
{
	std::vector<std::string> ids;
	ids.reserve(count);
	for (std::size_t index = 0; index < count; ++index) {
		ids.push_back(record_id(index));
	}
	return ids;
}

auto metric_law_diagnostics(const std::vector<std::vector<double>> &matrix) -> MetricLawDiagnostics
{
	const std::size_t count = matrix.size();
	MetricLawDiagnostics diagnostics;

	for (std::size_t row = 0; row < count; ++row) {
		const double diagonal = matrix[row][row];
		diagnostics.diagonal_max_abs = std::max(diagnostics.diagonal_max_abs, std::abs(diagonal));
		diagnostics.finite = diagnostics.finite && std::isfinite(diagonal);
		for (std::size_t column = 0; column < count; ++column) {
			const double lhs = matrix[row][column];
			const double rhs = matrix[column][row];
			diagnostics.symmetry_max_abs = std::max(diagnostics.symmetry_max_abs, std::abs(lhs - rhs));
			diagnostics.finite = diagnostics.finite && std::isfinite(lhs);
		}
	}

	for (std::size_t row = 0; row < count; ++row) {
		for (std::size_t column = row + 1; column < count; ++column) {
			const double distance = matrix[row][column];
			diagnostics.maximum_distance = std::max(diagnostics.maximum_distance, distance);
			diagnostics.average_distance += distance;
			if (distance > 0.0) {
				diagnostics.minimum_nonzero_distance = std::min(diagnostics.minimum_nonzero_distance, distance);
			}
			++diagnostics.pair_count;
		}
	}
	if (diagnostics.pair_count != 0) {
		diagnostics.average_distance /= static_cast<double>(diagnostics.pair_count);
	}
	if (!std::isfinite(diagnostics.minimum_nonzero_distance)) {
		diagnostics.minimum_nonzero_distance = 0.0;
	}

	for (std::size_t i = 0; i < count; ++i) {
		for (std::size_t j = 0; j < count; ++j) {
			for (std::size_t k = 0; k < count; ++k) {
				const double violation = matrix[i][k] - (matrix[i][j] + matrix[j][k]);
				diagnostics.triangle_max_violation = std::max(diagnostics.triangle_max_violation, violation);
				++diagnostics.triangle_triplets;
			}
		}
	}

	return diagnostics;
}

auto select_landmarks(const std::vector<std::vector<double>> &matrix) -> std::array<std::size_t, 3>
{
	const std::size_t count = matrix.size();
	std::array<std::size_t, 3> landmarks{0, 0, 0};
	if (count == 0) {
		return landmarks;
	}

	double best = -1.0;
	for (std::size_t index = 0; index < count; ++index) {
		if (matrix[landmarks[0]][index] > best) {
			best = matrix[landmarks[0]][index];
			landmarks[1] = index;
		}
	}

	best = -1.0;
	for (std::size_t index = 0; index < count; ++index) {
		const double nearest_landmark = std::min(matrix[index][landmarks[0]], matrix[index][landmarks[1]]);
		if (nearest_landmark > best) {
			best = nearest_landmark;
			landmarks[2] = index;
		}
	}
	return landmarks;
}

auto landmark_coordinates(const std::vector<std::vector<double>> &matrix) -> std::vector<std::array<double, 3>>
{
	const std::size_t count = matrix.size();
	const auto landmarks = select_landmarks(matrix);
	std::vector<std::array<double, 3>> coordinates(count);

	for (std::size_t row = 0; row < count; ++row) {
		for (std::size_t axis = 0; axis < landmarks.size(); ++axis) {
			coordinates[row][axis] = matrix[row][landmarks[axis]];
		}
	}

	for (std::size_t axis = 0; axis < landmarks.size(); ++axis) {
		double mean = 0.0;
		for (const auto &position : coordinates) {
			mean += position[axis];
		}
		mean /= static_cast<double>(std::max<std::size_t>(count, 1));
		double max_abs = 0.0;
		for (auto &position : coordinates) {
			position[axis] -= mean;
			max_abs = std::max(max_abs, std::abs(position[axis]));
		}
		if (max_abs > 0.0) {
			for (auto &position : coordinates) {
				position[axis] /= max_abs;
			}
		}
	}

	return coordinates;
}

auto local_density(const std::vector<std::vector<double>> &matrix, std::size_t row) -> double
{
	std::vector<double> distances;
	distances.reserve(matrix.size());
	for (std::size_t column = 0; column < matrix.size(); ++column) {
		if (row != column) {
			distances.push_back(matrix[row][column]);
		}
	}
	std::sort(distances.begin(), distances.end());
	const std::size_t k = std::min<std::size_t>(8, distances.size());
	double mean = 0.0;
	for (std::size_t index = 0; index < k; ++index) {
		mean += distances[index];
	}
	if (k != 0) {
		mean /= static_cast<double>(k);
	}
	return 1.0 / (1.0 + mean);
}

auto diagnosis_state(double score, double warn_threshold, double alarm_threshold) -> std::string
{
	if (score <= warn_threshold) {
		return "normal-band";
	}
	if (score <= alarm_threshold) {
		return "drift-warning";
	}
	return "alarm-band";
}

auto scalar_values(const std::vector<std::string> &record_ids, const std::vector<double> &values)
	-> std::vector<visual::ScalarValue>
{
	std::vector<visual::ScalarValue> entries;
	entries.reserve(values.size());
	for (std::size_t index = 0; index < values.size(); ++index) {
		entries.push_back({record_ids[index], values[index]});
	}
	return entries;
}

auto categorical_values(const std::vector<std::string> &record_ids, const std::vector<std::string> &values)
	-> std::vector<visual::CategoricalValue>
{
	std::vector<visual::CategoricalValue> entries;
	entries.reserve(values.size());
	for (std::size_t index = 0; index < values.size(); ++index) {
		entries.push_back({record_ids[index], values[index]});
	}
	return entries;
}

auto law_check_metadata(const MetricLawDiagnostics &law) -> std::string
{
	return visual::object({visual::field("law_check",
						   visual::object({visual::number_field("diagonal_max_abs", law.diagonal_max_abs),
										   visual::number_field("symmetry_max_abs", law.symmetry_max_abs),
										   visual::number_field("triangle_max_violation", law.triangle_max_violation),
										   visual::bool_field("finite", law.finite)}))});
}

auto build_condition_monitoring_document(const std::vector<Observation> &observations,
										 const std::vector<ObservationEvidence> &evidence,
										 const std::vector<std::string> &record_ids,
										 const std::vector<std::vector<double>> &matrix,
										 const std::vector<std::array<double, 3>> &metric_positions,
										 double healthy_radius_value, double warn_threshold, double alarm_threshold,
										 double healthy_average_distance, double healthy_maximum_distance,
										 double healthy_intrinsic_dimension, std::size_t healthy_record_count,
										 std::size_t healthy_unassigned_count, const MetricLawDiagnostics &law)
	-> visual::Document
{
	const std::string dataset_id = "condition-monitoring";
	const std::string metric_relation_id = "condition-monitoring-twed";
	const std::string transition_relation_id = "condition-monitoring-transition";
	const std::string space_id = "condition-monitoring-space";

	visual::Document doc;
	doc.provenance_json(visual::object({visual::string_field("writer", "examples/engine/condition_monitoring_visual_export.cpp"),
										visual::string_field("runtime", "native-c++"),
										visual::bool_field("native_export", true),
										visual::bool_field("synthetic_js", false),
										visual::string_field("source_example", "examples/engine/condition_monitoring.cpp")}))
		.dataset(dataset_id, "Condition monitoring process windows",
				 "Native C++ finite-metric evidence for process-window condition monitoring.",
				 "METRIC engine condition_monitoring.cpp native exporter", "MPL-2.0");

	for (std::size_t index = 0; index < observations.size(); ++index) {
		std::ostringstream label;
		label << "process window " << std::setw(4) << std::setfill('0') << index;
		doc.record(record_ids[index], dataset_id, "process_window", label.str(),
				   visual::object({visual::string_field("kind", "time_series"),
								   visual::number_array_field("series", observations[index].curve),
								   visual::number_field("sample_rate_hz", 1.0),
								   visual::string_field("truth", regime_name(observations[index].truth)),
								   visual::string_field("run_phase", observations[index].phase)}));
	}

	std::vector<visual::Edge> metric_edges;
	metric_edges.reserve(matrix.size() * matrix.size());
	for (std::size_t row = 0; row < matrix.size(); ++row) {
		for (std::size_t column = 0; column < matrix.size(); ++column) {
			metric_edges.push_back({record_ids[row], record_ids[column], matrix[row][column]});
		}
	}
	doc.metric_relation(metric_relation_id, dataset_id, "TWED process-window metric", record_ids, metric_edges,
						"sparse_edge_list", law_check_metadata(law));

	std::vector<std::string> transition_values;
	transition_values.reserve(record_ids.size() > 0 ? record_ids.size() - 1 : 0);
	for (std::size_t index = 1; index < record_ids.size(); ++index) {
		transition_values.push_back(visual::object({visual::string_field("row_id", record_ids[index - 1]),
													visual::string_field("column_id", record_ids[index]),
													visual::number_field("value", 1.0)}));
	}
	doc.relation_json(visual::object({visual::string_field("id", transition_relation_id),
									  visual::string_field("dataset_id", dataset_id),
									  visual::string_field("name", "process-window trajectory"),
									  visual::string_field("relation_type", "transition"),
									  visual::string_field("value_type", "scalar"),
									  visual::string_array_field("record_ids", record_ids),
									  visual::string_field("storage", "sparse_edge_list"),
									  visual::field("values", visual::array_of(transition_values))}));

	doc.space(space_id, dataset_id, record_ids, metric_relation_id, "finite_metric_space",
			  visual::object({visual::string_field("metric", "TWED"),
							  visual::number_field("healthy_radius", healthy_radius_value),
							  visual::number_field("warn_threshold", warn_threshold),
							  visual::number_field("alarm_threshold", alarm_threshold)}));

	std::vector<double> nearest_healthy;
	std::vector<double> nearest_catalog;
	std::vector<double> severity;
	std::vector<double> density;
	std::vector<double> condition_index;
	std::vector<double> density_outlier_flag;
	std::vector<std::string> truth;
	std::vector<std::string> phase;
	std::vector<std::string> nearest_regime;
	std::vector<std::string> diagnosis;
	for (std::size_t index = 0; index < observations.size(); ++index) {
		nearest_healthy.push_back(evidence[index].nearest_healthy_distance);
		nearest_catalog.push_back(evidence[index].nearest_catalog_distance);
		severity.push_back(evidence[index].severity);
		density.push_back(evidence[index].local_density);
		condition_index.push_back(observations[index].condition_index);
		density_outlier_flag.push_back(evidence[index].dbscan_density_outlier ? 1.0 : 0.0);
		truth.push_back(regime_name(observations[index].truth));
		phase.push_back(observations[index].phase);
		nearest_regime.push_back(regime_name(evidence[index].nearest_catalog_regime));
		diagnosis.push_back(evidence[index].diagnosis_state);
	}

	doc.scalar_property("nearest-healthy-distance", dataset_id, "TWED distance to healthy reference",
						scalar_values(record_ids, nearest_healthy))
		.scalar_property("condition-severity", dataset_id, "condition severity index",
						 scalar_values(record_ids, condition_index))
		.scalar_property("metric-anomaly-severity", dataset_id, "metric anomaly severity", scalar_values(record_ids, severity))
		.scalar_property("local-density", dataset_id, "local metric density", scalar_values(record_ids, density))
		.scalar_property("dbscan-density-outlier-flag", dataset_id, "DBSCAN density-unassigned record flag",
						 scalar_values(record_ids, density_outlier_flag))
		.scalar_property("nearest-catalog-distance", dataset_id, "nearest catalog distance",
						 scalar_values(record_ids, nearest_catalog))
		.categorical_property("truth-regime", dataset_id, "truth regime", categorical_values(record_ids, truth))
		.categorical_property("run-phase", dataset_id, "run phase", categorical_values(record_ids, phase))
		.categorical_property("nearest-catalog-regime", dataset_id, "nearest catalog regime",
							  categorical_values(record_ids, nearest_regime))
		.categorical_property("diagnosis-state", dataset_id, "diagnosis state", categorical_values(record_ids, diagnosis));

	std::vector<visual::GraphEdge> graph_edges;
	graph_edges.reserve(record_ids.size() > 0 ? record_ids.size() - 1 : 0);
	for (std::size_t index = 1; index < record_ids.size(); ++index) {
		graph_edges.push_back({record_ids[index - 1], record_ids[index], 1.0});
	}
	doc.graph("process-window-trajectory", dataset_id, record_ids, transition_relation_id, "transition", graph_edges);

	std::vector<visual::Position> landmark_positions;
	landmark_positions.reserve(metric_positions.size());
	for (std::size_t index = 0; index < metric_positions.size(); ++index) {
		landmark_positions.push_back(
			{record_ids[index], {metric_positions[index][0], metric_positions[index][1], metric_positions[index][2]}});
	}
	doc.coordinates3("landmark-3d", dataset_id, space_id, "TWED landmark distances", landmark_positions);

	std::vector<visual::Position> trajectory_positions;
	trajectory_positions.reserve(record_ids.size());
	const double denom = static_cast<double>(std::max<std::size_t>(record_ids.size() - 1, 1));
	for (std::size_t index = 0; index < record_ids.size(); ++index) {
		const double x = 2.0 * static_cast<double>(index) / denom - 1.0;
		const double y = 2.0 * evidence[index].severity - 1.0;
		const double z = observations[index].truth == Regime::Fault		  ? 0.9
						 : observations[index].truth == Regime::Signature ? 0.35
						 : observations[index].truth == Regime::Recovery  ? -0.45
						 : observations[index].truth == Regime::Drift	  ? -0.15
																		  : -0.7;
		trajectory_positions.push_back({record_ids[index], {x, y, z}});
	}
	doc.coordinates3("process-state-trajectory-3d", dataset_id, space_id, "process state trajectory",
					 trajectory_positions);

	std::vector<std::string> timeline_steps;
	timeline_steps.reserve(record_ids.size());
	for (std::size_t index = 0; index < record_ids.size(); ++index) {
		timeline_steps.push_back(visual::object({
			visual::string_field("id", "condition-step-" + record_ids[index]),
			visual::size_field("index", index),
			visual::number_field("t", static_cast<double>(index) / denom),
			visual::string_field("name", observations[index].phase + " " + record_ids[index]),
			visual::string_field("record_id", record_ids[index]),
			visual::string_field("coordinate_id", "process-state-trajectory-3d"),
			visual::string_field("property_id", "metric-anomaly-severity"),
			visual::string_field("relation_id", transition_relation_id),
			visual::string_field("graph_id", "process-window-trajectory"),
			visual::string_field("truth", regime_name(observations[index].truth)),
			visual::string_field("run_phase", observations[index].phase),
			visual::string_field("diagnosis_state", evidence[index].diagnosis_state),
			visual::number_field("nearest_healthy_distance", evidence[index].nearest_healthy_distance),
			visual::number_field("metric_anomaly_severity", evidence[index].severity),
			visual::number_field("local_density", evidence[index].local_density),
		}));
	}
	doc.timeline_json(visual::object({
		visual::string_field("id", "condition-evolution"),
		visual::string_field("dataset_id", dataset_id),
		visual::string_field("name", "healthy to drift to fault to recovery to recurring signature"),
		visual::string_field("space_id", space_id),
		visual::string_field("timeline_type", "condition_evolution"),
		visual::field("steps", visual::array_of(timeline_steps)),
		visual::field("metadata",
					  visual::object({visual::size_field("record_count", record_ids.size()),
									  visual::string_array_field("phase_order",
																 {"stable-reference", "slow-drift", "abrupt-change",
																  "recovery", "recurring-signature"}),
									  visual::string_field("coordinate_id", "process-state-trajectory-3d"),
									  visual::string_field("property_id", "metric-anomaly-severity"),
									  visual::string_field("graph_id", "process-window-trajectory")})),
	}));

	doc.view_json(visual::object({visual::string_field("kind", "metric-space"),
								  visual::string_field("spaceId", "condition-monitoring-space"),
								  visual::string_field("coordinateId", "process-state-trajectory-3d"),
								  visual::string_field("propertyId", "metric-anomaly-severity")}));

	doc.diagnostic("native-reference-checks", dataset_id, "condition_monitoring_reference",
				   visual::object({visual::size_field("healthy_record_count", healthy_record_count),
								   visual::number_field("healthy_average_distance", healthy_average_distance),
								   visual::number_field("healthy_maximum_distance", healthy_maximum_distance),
								   visual::number_field("healthy_intrinsic_dimension", healthy_intrinsic_dimension),
								   visual::number_field("healthy_radius", healthy_radius_value),
								   visual::number_field("warn_threshold", warn_threshold),
								   visual::number_field("alarm_threshold", alarm_threshold),
								   visual::size_field("healthy_dbscan_density_outlier_count", healthy_unassigned_count)}))
		.diagnostic("metric-law-check", dataset_id, "metric_law_check",
					visual::object({visual::number_field("diagonal_max_abs", law.diagonal_max_abs),
									visual::number_field("symmetry_max_abs", law.symmetry_max_abs),
									visual::number_field("triangle_max_violation", law.triangle_max_violation),
									visual::number_field("minimum_nonzero_distance", law.minimum_nonzero_distance),
									visual::number_field("maximum_distance", law.maximum_distance),
									visual::number_field("average_distance", law.average_distance),
									visual::size_field("pair_count", law.pair_count),
									visual::size_field("triangle_triplets", law.triangle_triplets),
									visual::bool_field("finite", law.finite)}));

	return doc;
}

auto build_document() -> std::string
{
	const Catalog catalog = build_reference_catalog();
	auto process_space = mtrc::make_space(catalog.curves, process_metric());

	std::vector<Curve> healthy_only;
	for (std::size_t index = 0; index < catalog.curves.size(); ++index) {
		if (catalog.families[index] == Regime::Normal) {
			healthy_only.push_back(catalog.curves[index]);
		}
	}
	auto healthy_space = mtrc::make_space(healthy_only, process_metric());
	const auto structure = mtrc::describe_structure(healthy_space);
	const double radius = healthy_radius(process_space, catalog.families);
	const double warn_threshold = 2.0 * radius;
	const double alarm_threshold = 4.0 * radius;
	const auto healthy_outliers = mtrc::find_outliers(healthy_space, warn_threshold, 2);

	if (structure.record_count != healthy_only.size() || !structure.has_nonzero_distances || radius <= 0.0 ||
		healthy_outliers.unassigned_count != 0) {
		throw std::runtime_error("native condition-monitoring reference checks failed");
	}

	const std::vector<Observation> observations = build_observations();
	std::vector<Curve> curves;
	curves.reserve(observations.size());
	for (const auto &observation : observations) {
		curves.push_back(observation.curve);
	}
	auto monitored_space = mtrc::make_space(curves, process_metric());
	const auto monitored_outliers = mtrc::find_outliers(monitored_space, warn_threshold, 6);

	std::vector<std::vector<double>> matrix(curves.size(), std::vector<double>(curves.size(), 0.0));
	const auto metric = process_metric();
	for (std::size_t row = 0; row < curves.size(); ++row) {
		for (std::size_t column = 0; column < curves.size(); ++column) {
			matrix[row][column] = metric(curves[row], curves[column]);
		}
	}

	std::vector<ObservationEvidence> evidence;
	evidence.reserve(observations.size());
	for (std::size_t index = 0; index < observations.size(); ++index) {
		const auto [nearest_regime, nearest_distance] = classify(process_space, catalog.families, observations[index].curve);
		const double healthy_distance = distance_to_healthy(process_space, catalog.families, observations[index].curve);
		const double severity = std::min(1.0, healthy_distance / alarm_threshold);
		evidence.push_back({record_id(index), healthy_distance, nearest_distance, severity, local_density(matrix, index),
							false, nearest_regime,
							diagnosis_state(healthy_distance, warn_threshold, alarm_threshold)});
	}
	for (const auto &outlier : monitored_outliers) {
		const std::size_t index = outlier.id.index();
		if (index < evidence.size()) {
			evidence[index].dbscan_density_outlier = true;
		}
	}

	std::vector<double> drift_scores;
	std::vector<double> recovery_scores;
	bool fault_flagged = false;
	for (std::size_t index = 0; index < observations.size(); ++index) {
		if (observations[index].truth == Regime::Drift) {
			drift_scores.push_back(evidence[index].nearest_healthy_distance);
		}
		if (observations[index].truth == Regime::Recovery) {
			recovery_scores.push_back(evidence[index].nearest_healthy_distance);
		}
		if (observations[index].truth == Regime::Fault) {
			fault_flagged = fault_flagged ||
							(evidence[index].dbscan_density_outlier &&
							 evidence[index].nearest_healthy_distance > alarm_threshold &&
							 evidence[index].nearest_catalog_regime == Regime::Fault);
		}
	}
	const bool drift_progression =
		drift_scores.size() >= 3 && drift_scores.front() <= warn_threshold &&
		drift_scores[drift_scores.size() / 2] > drift_scores.front() && drift_scores.back() > alarm_threshold;
	const bool recovery_progression =
		recovery_scores.size() >= 3 && recovery_scores.front() > alarm_threshold &&
		recovery_scores[recovery_scores.size() / 2] < recovery_scores.front() &&
		recovery_scores.back() <= warn_threshold;
	if (!drift_progression || !recovery_progression || !fault_flagged) {
		throw std::runtime_error("native condition-monitoring scenario checks failed");
	}

	const auto law = metric_law_diagnostics(matrix);
	const auto metric_positions = landmark_coordinates(matrix);
	const auto record_ids = make_record_ids(observations.size());

	auto doc = build_condition_monitoring_document(observations, evidence, record_ids, matrix, metric_positions, radius,
												   warn_threshold, alarm_threshold, structure.average_distance,
												   structure.maximum_distance, structure.intrinsic_dimension,
												   structure.record_count, healthy_outliers.unassigned_count, law);
	return doc.to_json() + '\n';
}

} // namespace

int main(int argc, char **argv)
{
	try {
		std::filesystem::path export_dir;
		bool has_export_dir = false;
		for (int index = 1; index < argc; ++index) {
			const std::string arg = argv[index];
			if (arg == "--export-dir") {
				if (index + 1 >= argc) {
					throw std::runtime_error("--export-dir requires a directory argument");
				}
				export_dir = argv[++index];
				has_export_dir = true;
			} else {
				throw std::runtime_error("unknown argument: " + arg);
			}
		}

		const std::string document = build_document();
		if (has_export_dir) {
			if (!visual::write_metric_visual_file(export_dir, document)) {
				throw std::runtime_error("failed to write export file: " + (export_dir / "metric.visual.json").string());
			}
		} else {
			std::cout << document;
		}
		return 0;
	} catch (const std::exception &error) {
		std::cerr << "condition_monitoring_visual_export: " << error.what() << '\n';
		return 1;
	}
}
