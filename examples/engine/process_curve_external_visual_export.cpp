// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// Native metric.visual.v1 exporter for the external process-curve gallery.
//
// The source records and query outcomes are computed from the existing UCR
// process-curve CSV evidence. JavaScript consumers only validate and render the
// exported document through the METRIC Visual Engine API.

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <metric/engine.hpp>

#include "mtrc_visual.hpp"

#ifndef METRIC_PROCESS_CURVE_POWER_DEMAND_CSV
#define METRIC_PROCESS_CURVE_POWER_DEMAND_CSV "assets/process_curve_power_demand_gallery.csv"
#endif

#ifndef METRIC_PROCESS_CURVE_INTERNAL_BLEEDING_CSV
#define METRIC_PROCESS_CURVE_INTERNAL_BLEEDING_CSV "assets/process_curve_internal_bleeding_gallery.csv"
#endif

namespace {

namespace visual = mtrc::visual;

constexpr const char *kDatasetId = "process-curve-external";
constexpr const char *kRelationId = "process-curve-external-aligned-metric";
constexpr const char *kSpaceId = "process-curve-external-finite-space";
constexpr const char *kCoordinateId = "process-curve-external-landmark-3d";
constexpr const char *kGraphId = "process-curve-external-knn";
constexpr const char *kSource = "UCR_Time_Series_Anomaly_Detection_2021";
constexpr const char *kLicense = "CC BY 4.0";
constexpr std::size_t kNearestNeighbors = 4;
constexpr const char *kPowerDemandAsset = "examples/engine/assets/process_curve_power_demand_gallery.csv";
constexpr const char *kInternalBleedingAsset = "examples/engine/assets/process_curve_internal_bleeding_gallery.csv";

using Matrix = std::vector<std::vector<double>>;

struct ExternalProcessWindow {
	std::string id;
	std::string domain;
	std::string domain_title;
	std::string source_file;
	std::string source_label;
	std::size_t source_start_index{};
	std::size_t source_end_index{};
	std::vector<double> values;
};

struct ProcessCurveQuery {
	std::string id;
	std::string source_record_id;
	std::string domain;
	std::string expected_label;
	std::vector<double> values;
};

struct QueryEvidence {
	ProcessCurveQuery query;
	std::string metric_winner_id;
	std::string metric_winner_label;
	std::size_t metric_winner_index{};
	double metric_distance{};
	std::string vector_winner_id;
	std::string vector_winner_label;
	std::size_t vector_winner_index{};
	double vector_distance{};
	double metric_margin{};
	bool metric_correct{};
	bool vector_mismatch{};
};

struct DomainInput {
	std::string name;
	std::string title;
	std::string csv_path;
	std::size_t expected_records{};
	std::size_t expected_queries{};
	double minimum_average_margin{};
};

struct DomainEvidence {
	std::string name;
	std::string title;
	std::vector<ExternalProcessWindow> records;
	std::vector<QueryEvidence> queries;
	Matrix metric_distances;
	Matrix vector_distances;
	std::size_t metric_correct{};
	std::size_t vector_mismatches{};
	double average_metric_margin{};
};

struct AlignedCurveDistance {
	double gap_cost{18.0};

	auto operator()(const std::vector<double> &lhs, const std::vector<double> &rhs) const -> double
	{
		std::vector<double> previous(rhs.size() + 1, 0.0);
		for (std::size_t index = 0; index < previous.size(); ++index) {
			previous[index] = static_cast<double>(index) * gap_cost;
		}

		for (std::size_t lhs_index = 1; lhs_index <= lhs.size(); ++lhs_index) {
			std::vector<double> current(rhs.size() + 1, 0.0);
			current[0] = static_cast<double>(lhs_index) * gap_cost;
			for (std::size_t rhs_index = 1; rhs_index <= rhs.size(); ++rhs_index) {
				const auto substitution =
					previous[rhs_index - 1] +
					std::min(std::abs(lhs[lhs_index - 1] - rhs[rhs_index - 1]), 2.0 * gap_cost);
				const auto deletion = previous[rhs_index] + gap_cost;
				const auto insertion = current[rhs_index - 1] + gap_cost;
				current[rhs_index] = std::min({substitution, deletion, insertion});
			}
			previous = std::move(current);
		}

		return previous.back();
	}
};

struct PointwisePaddedEuclideanDistance {
	auto operator()(const std::vector<double> &lhs, const std::vector<double> &rhs) const -> double
	{
		const auto size = std::max(lhs.size(), rhs.size());
		double squared_sum = 0.0;
		for (std::size_t index = 0; index < size; ++index) {
			const auto left = index < lhs.size() ? lhs[index] : 0.0;
			const auto right = index < rhs.size() ? rhs[index] : 0.0;
			const auto delta = left - right;
			squared_sum += delta * delta;
		}
		return std::sqrt(squared_sum);
	}
};

auto split_csv_line(const std::string &line) -> std::vector<std::string>
{
	std::vector<std::string> fields;
	std::string field;
	bool in_quotes = false;
	for (const char character : line) {
		if (character == '"') {
			in_quotes = !in_quotes;
			field.push_back(character);
		} else if (character == ',' && !in_quotes) {
			fields.push_back(field);
			field.clear();
		} else {
			field.push_back(character);
		}
	}
	fields.push_back(field);
	return fields;
}

auto parse_values(std::string value_text) -> std::vector<double>
{
	if (value_text.size() >= 2 && value_text.front() == '"' && value_text.back() == '"') {
		value_text = value_text.substr(1, value_text.size() - 2);
	}

	std::istringstream stream(value_text);
	std::vector<double> values;
	double value = 0.0;
	while (stream >> value) {
		values.push_back(value);
	}
	return values;
}

auto parse_index(const std::string &text, const std::string &field_name) -> std::size_t
{
	try {
		return static_cast<std::size_t>(std::stoull(text));
	} catch (const std::exception &) {
		throw std::runtime_error("invalid " + field_name + " in process-curve CSV: " + text);
	}
}

auto load_external_windows(const DomainInput &input) -> std::vector<ExternalProcessWindow>
{
	std::ifstream in(input.csv_path);
	if (!in) {
		throw std::runtime_error("unable to open process-curve CSV: " + input.csv_path);
	}

	std::string line;
	std::getline(in, line);
	std::vector<ExternalProcessWindow> records;
	while (std::getline(in, line)) {
		if (line.empty()) {
			continue;
		}
		const auto fields = split_csv_line(line);
		if (fields.size() != 6) {
			throw std::runtime_error("unexpected process-curve CSV row: " + line);
		}
		auto values = parse_values(fields[5]);
		if (values.size() != 36) {
			throw std::runtime_error("process-curve source record is not a 36-sample snippet: " + fields[0]);
		}
		records.push_back({fields[0],
						   input.name,
						   input.title,
						   fields[1],
						   fields[4],
						   parse_index(fields[2], "source_start_index"),
						   parse_index(fields[3], "source_end_index"),
						   std::move(values)});
	}
	if (records.size() != input.expected_records) {
		throw std::runtime_error("unexpected record count for " + input.name);
	}
	return records;
}

auto downsample_even(const std::vector<double> &values) -> std::vector<double>
{
	std::vector<double> query;
	query.reserve((values.size() + 1) / 2);
	for (std::size_t index = 0; index < values.size(); index += 2) {
		query.push_back(values[index]);
	}
	return query;
}

auto contains(const std::string &text, const std::string &needle) -> bool
{
	return text.find(needle) != std::string::npos;
}

auto dense_distances(const std::vector<std::vector<double>> &records, AlignedCurveDistance metric) -> Matrix
{
	Matrix distances(records.size(), std::vector<double>(records.size(), 0.0));
	for (std::size_t row = 0; row < records.size(); ++row) {
		for (std::size_t column = row + 1; column < records.size(); ++column) {
			const auto value = metric(records[row], records[column]);
			distances[row][column] = value;
			distances[column][row] = value;
		}
	}
	return distances;
}

auto dense_vector_baseline_distances(const std::vector<std::vector<double>> &records,
									 PointwisePaddedEuclideanDistance metric) -> Matrix
{
	Matrix distances(records.size(), std::vector<double>(records.size(), 0.0));
	for (std::size_t row = 0; row < records.size(); ++row) {
		for (std::size_t column = row + 1; column < records.size(); ++column) {
			const auto value = metric(records[row], records[column]);
			distances[row][column] = value;
			distances[column][row] = value;
		}
	}
	return distances;
}

auto run_domain(const DomainInput &input) -> DomainEvidence
{
	DomainEvidence evidence;
	evidence.name = input.name;
	evidence.title = input.title;
	evidence.records = load_external_windows(input);

	std::vector<ProcessCurveQuery> queries;
	std::vector<std::vector<double>> record_values;
	record_values.reserve(evidence.records.size());
	for (const auto &record : evidence.records) {
		record_values.push_back(record.values);
		if (contains(record.id, "anomaly_start") || contains(record.id, "recovery")) {
			queries.push_back({"downsampled_" + record.id, record.id, input.name, record.source_label,
							   downsample_even(record.values)});
		}
	}
	if (queries.size() != input.expected_queries) {
		throw std::runtime_error("unexpected query count for " + input.name);
	}

	evidence.metric_distances = dense_distances(record_values, AlignedCurveDistance{});
	evidence.vector_distances = dense_vector_baseline_distances(record_values, PointwisePaddedEuclideanDistance{});

	const auto alignment_space = mtrc::make_space(record_values, AlignedCurveDistance{});
	const auto vector_baseline = mtrc::make_space(record_values, PointwisePaddedEuclideanDistance{});
	double metric_margin_sum = 0.0;
	for (const auto &query : queries) {
		if (query.values.size() != 18) {
			throw std::runtime_error("query is not an 18-sample downsampled snippet: " + query.id);
		}
		const auto metric_neighbors = mtrc::find_neighbors(alignment_space, query.values, 1);
		const auto vector_neighbors = mtrc::find_neighbors(vector_baseline, query.values, 1);
		if (metric_neighbors.size() != 1 || vector_neighbors.size() != 1) {
			throw std::runtime_error("nearest-neighbor lookup returned no result for query: " + query.id);
		}

		const auto metric_index = metric_neighbors[0].id.index();
		const auto vector_index = vector_neighbors[0].id.index();
		const auto &metric_record = evidence.records[metric_index];
		const auto &vector_record = evidence.records[vector_index];
		const auto baseline_distance_under_metric =
			alignment_space.metric()(query.values, evidence.records[vector_index].values);
		const auto metric_margin = baseline_distance_under_metric - metric_neighbors[0].distance;
		if (!(metric_margin > 0.0)) {
			throw std::runtime_error("metric margin is not positive for query: " + query.id);
		}

		const bool metric_correct = metric_record.source_label == query.expected_label;
		const bool vector_mismatch = vector_record.source_label != query.expected_label;
		evidence.metric_correct += metric_correct ? 1 : 0;
		evidence.vector_mismatches += vector_mismatch ? 1 : 0;
		metric_margin_sum += metric_margin;
		evidence.queries.push_back({query,
									metric_record.id,
									metric_record.source_label,
									metric_index,
									metric_neighbors[0].distance,
									vector_record.id,
									vector_record.source_label,
									vector_index,
									vector_neighbors[0].distance,
									metric_margin,
									metric_correct,
									vector_mismatch});
	}

	evidence.average_metric_margin = metric_margin_sum / static_cast<double>(evidence.queries.size());
	if (evidence.metric_correct != evidence.queries.size() ||
		evidence.vector_mismatches != evidence.queries.size() ||
		evidence.average_metric_margin <= input.minimum_average_margin) {
		throw std::runtime_error("process-curve query evidence did not meet expected native outcomes for " + input.name);
	}
	return evidence;
}

auto all_source_records(const std::vector<DomainEvidence> &domains) -> std::vector<ExternalProcessWindow>
{
	std::vector<ExternalProcessWindow> records;
	for (const auto &domain : domains) {
		records.insert(records.end(), domain.records.begin(), domain.records.end());
	}
	return records;
}

auto source_values(const std::vector<ExternalProcessWindow> &records) -> std::vector<std::vector<double>>
{
	std::vector<std::vector<double>> values;
	values.reserve(records.size());
	for (const auto &record : records) {
		values.push_back(record.values);
	}
	return values;
}

auto ids_for(const std::vector<ExternalProcessWindow> &records) -> std::vector<std::string>
{
	std::vector<std::string> ids;
	ids.reserve(records.size());
	for (const auto &record : records) {
		ids.push_back(record.id);
	}
	return ids;
}

auto object(const std::vector<visual::Field> &fields) -> std::string { return visual::object(fields); }

auto payload_for_record(const ExternalProcessWindow &record) -> std::string
{
	return object({visual::string_field("kind", "time_series"),
				   visual::number_array_field("series", record.values),
				   visual::size_field("sample_count", record.values.size()),
				   visual::string_field("domain", record.domain),
				   visual::string_field("source_file", record.source_file),
				   visual::size_field("source_start_index", record.source_start_index),
				   visual::size_field("source_end_index", record.source_end_index),
				   visual::string_field("source_label", record.source_label)});
}

auto payload_for_query(const QueryEvidence &evidence) -> std::string
{
	return object({visual::string_field("kind", "time_series"),
				   visual::number_array_field("series", evidence.query.values),
				   visual::size_field("sample_count", evidence.query.values.size()),
				   visual::string_field("domain", evidence.query.domain),
				   visual::string_field("source_record_id", evidence.query.source_record_id),
				   visual::string_field("expected_label", evidence.query.expected_label),
				   visual::string_field("sampling", "even-index downsampled source process curve")});
}

auto relation_edges(const std::vector<std::string> &ids, const Matrix &distances) -> std::vector<visual::Edge>
{
	std::vector<visual::Edge> edges;
	edges.reserve(ids.size() * ids.size());
	for (std::size_t row = 0; row < ids.size(); ++row) {
		for (std::size_t column = 0; column < ids.size(); ++column) {
			edges.push_back({ids[row], ids[column], distances[row][column]});
		}
	}
	return edges;
}

auto matrix_min_nonzero(const Matrix &distances) -> double
{
	double value = std::numeric_limits<double>::infinity();
	for (std::size_t row = 0; row < distances.size(); ++row) {
		for (std::size_t column = 0; column < distances.size(); ++column) {
			if (row != column) {
				value = std::min(value, distances[row][column]);
			}
		}
	}
	return std::isfinite(value) ? value : 0.0;
}

auto matrix_max(const Matrix &distances) -> double
{
	double value = 0.0;
	for (const auto &row : distances) {
		for (const auto distance : row) {
			value = std::max(value, distance);
		}
	}
	return value;
}

auto block_ranges_json(const std::vector<DomainEvidence> &domains) -> std::string
{
	std::vector<std::string> ranges;
	std::size_t start = 0;
	for (const auto &domain : domains) {
		const auto end = start + domain.records.size();
		ranges.push_back(object({visual::string_field("id", domain.name),
								 visual::string_field("label", domain.title),
								 visual::size_field("start", start),
								 visual::size_field("end", end)}));
		start = end;
	}
	return visual::array_of(ranges);
}

auto landmark_positions(const std::vector<std::string> &ids, const Matrix &distances) -> std::vector<visual::Position>
{
	const std::size_t count = ids.size();
	const std::size_t anchors[3] = {0, count / 2, count - 1};
	std::vector<visual::Position> positions;
	positions.reserve(count);
	for (std::size_t index = 0; index < count; ++index) {
		std::vector<double> position;
		position.reserve(3);
		for (const auto anchor : anchors) {
			const auto maximum = *std::max_element(distances[anchor].begin(), distances[anchor].end());
			const auto normalized = maximum > 0.0 ? distances[anchor][index] / maximum : 0.0;
			position.push_back(normalized - 0.5);
		}
		positions.push_back({ids[index], position});
	}
	return positions;
}

auto nearest_indices(const Matrix &distances, std::size_t row, std::size_t count) -> std::vector<std::size_t>
{
	std::vector<std::pair<double, std::size_t>> candidates;
	candidates.reserve(distances.size() - 1);
	for (std::size_t column = 0; column < distances.size(); ++column) {
		if (row != column) {
			candidates.push_back({distances[row][column], column});
		}
	}
	std::sort(candidates.begin(), candidates.end());
	if (candidates.size() > count) {
		candidates.resize(count);
	}
	std::vector<std::size_t> indices;
	indices.reserve(candidates.size());
	for (const auto &candidate : candidates) {
		indices.push_back(candidate.second);
	}
	return indices;
}

auto graph_edges(const std::vector<std::string> &ids, const Matrix &distances) -> std::vector<visual::GraphEdge>
{
	std::vector<visual::GraphEdge> edges;
	edges.reserve(ids.size() * kNearestNeighbors);
	for (std::size_t row = 0; row < ids.size(); ++row) {
		for (const auto column : nearest_indices(distances, row, kNearestNeighbors)) {
			edges.push_back({ids[row], ids[column], distances[row][column]});
		}
	}
	return edges;
}

auto record_property_values(const std::vector<ExternalProcessWindow> &records,
							const std::string ExternalProcessWindow::*member) -> std::vector<visual::CategoricalValue>
{
	std::vector<visual::CategoricalValue> values;
	values.reserve(records.size());
	for (const auto &record : records) {
		values.push_back({record.id, record.*member});
	}
	return values;
}

auto source_mean_values(const std::vector<ExternalProcessWindow> &records) -> std::vector<visual::ScalarValue>
{
	std::vector<visual::ScalarValue> values;
	values.reserve(records.size());
	for (const auto &record : records) {
		const auto mean = std::accumulate(record.values.begin(), record.values.end(), 0.0) /
						  static_cast<double>(record.values.size());
		values.push_back({record.id, mean});
	}
	return values;
}

template <typename ValueFn>
auto query_categorical_values(const std::vector<DomainEvidence> &domains, ValueFn value_fn)
	-> std::vector<visual::CategoricalValue>
{
	std::vector<visual::CategoricalValue> values;
	for (const auto &domain : domains) {
		for (const auto &query : domain.queries) {
			values.push_back({query.query.id, value_fn(query)});
		}
	}
	return values;
}

template <typename ValueFn>
auto query_scalar_values(const std::vector<DomainEvidence> &domains, ValueFn value_fn) -> std::vector<visual::ScalarValue>
{
	std::vector<visual::ScalarValue> values;
	for (const auto &domain : domains) {
		for (const auto &query : domain.queries) {
			values.push_back({query.query.id, value_fn(query)});
		}
	}
	return values;
}

auto query_records_count(const std::vector<DomainEvidence> &domains) -> std::size_t
{
	std::size_t count = 0;
	for (const auto &domain : domains) {
		count += domain.queries.size();
	}
	return count;
}

auto diagnostic_payload(const std::vector<DomainEvidence> &domains, const Matrix &combined_distances) -> std::string
{
	std::size_t metric_correct = 0;
	std::size_t vector_mismatches = 0;
	std::vector<std::string> domain_entries;
	for (const auto &domain : domains) {
		metric_correct += domain.metric_correct;
		vector_mismatches += domain.vector_mismatches;
		domain_entries.push_back(object({visual::string_field("domain", domain.name),
										  visual::size_field("records", domain.records.size()),
										  visual::size_field("queries", domain.queries.size()),
										  visual::size_field("metric_correct", domain.metric_correct),
										  visual::size_field("vector_mismatches", domain.vector_mismatches),
										  visual::number_field("average_metric_margin", domain.average_metric_margin)}));
	}
	return object({visual::string_field("generator", "examples/engine/process_curve_external_visual_export.cpp"),
				   visual::size_field("source_record_count", combined_distances.size()),
				   visual::size_field("query_record_count", query_records_count(domains)),
				   visual::size_field("relation_pair_count", combined_distances.size() * combined_distances.size()),
				   visual::size_field("knn_edge_count", combined_distances.size() * kNearestNeighbors),
				   visual::size_field("metric_correct", metric_correct),
				   visual::size_field("vector_baseline_mismatches", vector_mismatches),
				   visual::number_field("aligned_metric_min_nonzero", matrix_min_nonzero(combined_distances)),
				   visual::number_field("aligned_metric_max", matrix_max(combined_distances)),
				   visual::field("domains", visual::array_of(domain_entries))});
}

auto build_document(const std::vector<DomainEvidence> &domains) -> visual::Document
{
	const auto records = all_source_records(domains);
	const auto ids = ids_for(records);
	const auto combined_distances = dense_distances(source_values(records), AlignedCurveDistance{});

	visual::Document doc;
	doc.provenance_json(object({visual::string_field("writer", "examples/engine/process_curve_external_visual_export.cpp"),
								visual::string_field("runtime", "native-c++"),
								visual::bool_field("native_export", true),
								visual::bool_field("synthetic", false),
								visual::string_field("source", kSource),
								visual::string_field("license", kLicense),
								visual::string_field("power_demand_csv", kPowerDemandAsset),
								visual::string_field("internal_bleeding_csv", kInternalBleedingAsset),
								visual::string_field("public_evidence_policy", "no synthetic public evidence")}));
	doc.dataset(kDatasetId,
				"External UCR process curves",
				"Native C++ metric.visual.v1 export for process-curve source windows, downsampled query recovery, "
				"aligned-curve metric relations, and padded-vector baseline outcomes.",
				kSource,
				kLicense);

	for (const auto &record : records) {
		doc.record(record.id, kDatasetId, "process_curve_source", record.source_label, payload_for_record(record),
				   object({visual::string_field("domain", record.domain),
						   visual::string_field("domain_title", record.domain_title),
						   visual::string_field("source_record_id", record.id)}));
	}
	for (const auto &domain : domains) {
		for (const auto &query : domain.queries) {
			doc.record(query.query.id, kDatasetId, "process_curve_query", query.query.expected_label,
					   payload_for_query(query),
					   object({visual::string_field("domain", query.query.domain),
							   visual::string_field("expected_label", query.query.expected_label),
							   visual::string_field("metric_winner_id", query.metric_winner_id),
							   visual::string_field("vector_winner_id", query.vector_winner_id)}));
			doc.event_json(object({visual::string_field("id", "event-" + query.query.id),
								   visual::string_field("dataset_id", kDatasetId),
								   visual::string_field("event_type", "query_recovery"),
								   visual::string_field("record_id", query.query.id),
								   visual::string_field("expected_label", query.query.expected_label),
								   visual::string_field("metric_winner_id", query.metric_winner_id),
								   visual::string_field("vector_winner_id", query.vector_winner_id),
								   visual::number_field("metric_distance", query.metric_distance),
								   visual::number_field("vector_distance", query.vector_distance),
								   visual::number_field("metric_margin", query.metric_margin)}));
		}
	}

	doc.metric_relation(kRelationId,
						kDatasetId,
						"Aligned process-curve metric",
						ids,
						relation_edges(ids, combined_distances),
						"dense_matrix",
						object({visual::string_field("operator", "AlignedCurveDistance"),
								visual::number_field("gap_cost", AlignedCurveDistance{}.gap_cost),
								visual::bool_field("symmetric", true),
								visual::string_field("record_order", "domain CSV order"),
								visual::field("block_ranges", block_ranges_json(domains))}));
	doc.space(kSpaceId,
			  kDatasetId,
			  ids,
			  kRelationId,
			  "finite_metric_space",
			  object({visual::string_field("evidence_source", "native aligned process-curve distances")}));
	doc.coordinates3(kCoordinateId,
					 kDatasetId,
					 kSpaceId,
					 "Landmark coordinates from aligned metric",
					 landmark_positions(ids, combined_distances),
					 object({visual::string_field("layout_source", "C++ landmark distances over exported metric matrix"),
							 visual::bool_field("metric_derived", true)}));
	doc.graph(kGraphId,
			  kDatasetId,
			  ids,
			  kRelationId,
			  "k-nearest",
			  graph_edges(ids, combined_distances),
			  object({visual::size_field("neighbors_per_record", kNearestNeighbors),
					  visual::bool_field("directed", true),
					  visual::string_field("edge_source", "native aligned process-curve metric")}));

	doc.categorical_property("process-role", kDatasetId, "process role", record_property_values(records, &ExternalProcessWindow::source_label));
	doc.categorical_property("process-domain", kDatasetId, "process domain", record_property_values(records, &ExternalProcessWindow::domain));
	doc.scalar_property("source-curve-mean", kDatasetId, "source curve mean", source_mean_values(records));
	doc.categorical_property("query-expected-role",
							 kDatasetId,
							 "query expected role",
							 query_categorical_values(domains, [](const QueryEvidence &query) {
								 return query.query.expected_label;
							 }));
	doc.categorical_property("query-metric-winner-role",
							 kDatasetId,
							 "metric-space winner role",
							 query_categorical_values(domains, [](const QueryEvidence &query) {
								 return query.metric_winner_label;
							 }));
	doc.categorical_property("query-vector-baseline-winner-role",
							 kDatasetId,
							 "padded-vector baseline winner role",
							 query_categorical_values(domains, [](const QueryEvidence &query) {
								 return query.vector_winner_label;
							 }));
	doc.scalar_property("query-metric-distance",
						kDatasetId,
						"metric-space winner distance",
						query_scalar_values(domains, [](const QueryEvidence &query) {
							return query.metric_distance;
						}));
	doc.scalar_property("query-vector-baseline-distance",
						kDatasetId,
						"padded-vector baseline distance",
						query_scalar_values(domains, [](const QueryEvidence &query) {
							return query.vector_distance;
						}));
	doc.scalar_property("query-metric-margin",
						kDatasetId,
						"metric-space margin over baseline winner",
						query_scalar_values(domains, [](const QueryEvidence &query) {
							return query.metric_margin;
						}));
	doc.scalar_property("query-metric-correct",
						kDatasetId,
						"metric-space winner has expected role",
						query_scalar_values(domains, [](const QueryEvidence &query) {
							return query.metric_correct ? 1.0 : 0.0;
						}));
	doc.scalar_property("query-vector-baseline-mismatch",
						kDatasetId,
						"padded-vector baseline winner has wrong role",
						query_scalar_values(domains, [](const QueryEvidence &query) {
							return query.vector_mismatch ? 1.0 : 0.0;
						}));

	doc.view_json(object({visual::string_field("id", "process-curve-external-relation-neighborhood-view"),
						  visual::string_field("kind", "relation-matrix-neighborhood"),
						  visual::string_field("command", "showRelationMatrixNeighborhood"),
						  visual::string_field("name", "External process-curve aligned metric relation"),
						  visual::string_field("spaceId", kSpaceId),
						  visual::string_field("relationId", kRelationId),
						  visual::string_field("graphId", kGraphId),
						  visual::string_field("coordinateId", kCoordinateId),
						  visual::string_field("colorPropertyId", "process-role")}));
	doc.diagnostic("process-curve-external-native-summary",
				   kDatasetId,
				   "native-export-summary",
				   diagnostic_payload(domains, combined_distances));
	return doc;
}

auto parse_flag_value(int argc, char **argv, const std::string &flag) -> std::string
{
	for (int index = 1; index < argc; ++index) {
		const std::string argument = argv[index];
		if (argument == flag) {
			if (index + 1 >= argc) {
				throw std::runtime_error(flag + " requires a value");
			}
			return argv[index + 1];
		}
		if (argument.rfind(flag + "=", 0) == 0) {
			return argument.substr(flag.size() + 1);
		}
	}
	return {};
}

} // namespace

int main(int argc, char **argv)
{
	const auto export_dir = parse_flag_value(argc, argv, "--export-dir");
	const auto export_json = parse_flag_value(argc, argv, "--export-json");

	const std::vector<DomainInput> inputs{
		{"power_demand", "Power Demand", METRIC_PROCESS_CURVE_POWER_DEMAND_CSV, 24, 8, 300.0},
		{"internal_bleeding", "Internal Bleeding", METRIC_PROCESS_CURVE_INTERNAL_BLEEDING_CSV, 24, 8, 150.0},
	};

	std::vector<DomainEvidence> domains;
	for (const auto &input : inputs) {
		domains.push_back(run_domain(input));
	}

	auto doc = build_document(domains);
	if (export_dir.empty() && export_json.empty()) {
		std::cout << doc.to_json() << "\n";
		return 0;
	}

	const auto output = export_json.empty() ? (std::filesystem::path(export_dir) / "metric.visual.json")
											: std::filesystem::path(export_json);
	if (!output.parent_path().empty()) {
		std::filesystem::create_directories(output.parent_path());
	}
	if (!doc.write_file(output)) {
		throw std::runtime_error("unable to write metric.visual.json: " + output.string());
	}

	std::cerr << "process curve external visual export = " << output.string() << "\n";
	std::cerr << "process curve external visual source records = " << all_source_records(domains).size() << "\n";
	std::cerr << "process curve external visual query records = " << query_records_count(domains) << "\n";
	std::cerr << "process curve external visual relation pairs = "
			  << all_source_records(domains).size() * all_source_records(domains).size() << "\n";
	return 0;
}
