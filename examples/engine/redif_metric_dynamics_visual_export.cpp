// Native metric.visual.v1 exporter for Redif metric dynamics.
//
// C++ computes the Redif measure paths and serializes the evidence directly.
// JavaScript validation/rendering consumes this evidence; it does not compute
// Redif dynamics from records.

#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <string>
#include <stdexcept>
#include <utility>
#include <vector>

#include <metric/engine.hpp>

#include "../../visual/cpp/mtrc_visual.hpp"

namespace {

namespace visual = mtrc::visual;

auto record_id(std::size_t index) -> std::string
{
	std::ostringstream out;
	out << "atom-" << index;
	return out.str();
}

auto coordinate_id(std::size_t step) -> std::string
{
	std::ostringstream out;
	out << "redif-measure-step-" << step;
	return out.str();
}

auto entropy(const std::vector<double> &measure) -> double
{
	double value = 0.0;
	for (const auto mass : measure) {
		if (mass > 0.0) {
			value -= mass * std::log(mass);
		}
	}
	return value;
}

auto json_object(const std::vector<visual::Field> &fields) -> std::string { return visual::object(fields); }

auto matrix_json(const std::vector<std::vector<double>> &matrix) -> std::string
{
	std::vector<std::string> rows;
	rows.reserve(matrix.size());
	for (const auto &row : matrix) {
		rows.push_back(visual::number_array(row));
	}
	return visual::array_of(rows);
}

auto step_transport_json(const mtrc::RedifMeasureResult &result) -> std::string
{
	std::vector<std::string> entries;
	entries.reserve(result.paths.size());
	for (const auto &path : result.paths) {
		entries.push_back(json_object({visual::string_field("record_id", record_id(path.id.index())),
									   visual::number_array_field("step_transport", path.step_transport),
									   visual::number_field("transport_path_length", path.transport_path_length)}));
	}
	return visual::array_of(entries);
}

auto measure_paths_json(const mtrc::RedifMeasureResult &result) -> std::string
{
	std::vector<std::string> entries;
	entries.reserve(result.paths.size());
	for (const auto &path : result.paths) {
		entries.push_back(json_object({visual::string_field("record_id", record_id(path.id.index())),
									   visual::field("measures", matrix_json(path.measures))}));
	}
	return visual::array_of(entries);
}

auto scalar_values_from_summaries(const std::vector<mtrc::RedifMeasureSummary> &summaries,
								  double mtrc::RedifMeasureSummary::*member)
	-> std::vector<visual::ScalarValue>
{
	std::vector<visual::ScalarValue> values;
	values.reserve(summaries.size());
	for (const auto &summary : summaries) {
		values.push_back(visual::ScalarValue{record_id(summary.id.index()), summary.*member});
	}
	return values;
}

auto step_positions(const std::vector<double> &source_positions, const mtrc::RedifMeasureResult &result,
					std::size_t step) -> std::vector<visual::Position>
{
	std::vector<visual::Position> positions;
	positions.reserve(result.paths.size());
	for (const auto &path : result.paths) {
		const auto &measure = path.measures[step];
		double barycenter = 0.0;
		for (std::size_t column = 0; column < measure.size(); ++column) {
			barycenter += measure[column] * source_positions[column];
		}
		positions.push_back(visual::Position{record_id(path.id.index()),
											 {barycenter, entropy(measure), static_cast<double>(step)}});
	}
	return positions;
}

auto timeline_json(const mtrc::RedifMeasureResult &result) -> std::string
{
	std::vector<std::string> steps;
	const auto frame_count = result.paths.empty() ? 0 : result.paths.front().measures.size();
	steps.reserve(frame_count);
	for (std::size_t step = 0; step < frame_count; ++step) {
		steps.push_back(json_object({visual::string_field("id", "redif-step-" + std::to_string(step)),
									 visual::size_field("index", step),
									 visual::number_field("t", static_cast<double>(step)),
									 visual::string_field("name", "Redif measure step " + std::to_string(step)),
									 visual::string_field("coordinate_id", coordinate_id(step)),
									 visual::string_field("property_id", "redif-terminal-transport-path")}));
	}
	return json_object({visual::string_field("id", "redif-measure-path"),
						visual::string_field("dataset_id", "redif-dynamics"),
						visual::string_field("name", "Redif inverse measure path"),
						visual::string_field("timeline_type", "redif_measure_path"),
						visual::field("steps", visual::array_of(steps)),
						visual::field("metadata",
									  json_object({visual::string_field("computed_by", "METRIC C++ redif_remove_noise"),
												   visual::bool_field("algorithmic_computation", false)}))});
}

auto view_json(const std::string &kind, const std::string &id, const std::string &name,
			   const std::vector<std::pair<std::string, std::string>> &refs) -> std::string
{
	std::vector<visual::Field> fields{visual::string_field("id", id),
									  visual::string_field("kind", kind),
									  visual::string_field("name", name)};
	for (const auto &ref : refs) {
		fields.push_back(visual::string_field(ref.first, ref.second));
	}
	return json_object(fields);
}

auto usage(const char *program) -> std::string
{
	std::ostringstream out;
	out << "usage: " << program << " [--export-dir <dir>]\n"
		<< "  without --export-dir, writes metric.visual.v1 JSON to stdout\n";
	return out.str();
}

} // namespace

int main(int argc, char **argv)
{
	std::filesystem::path export_dir;
	try {
		for (int index = 1; index < argc; ++index) {
			const std::string arg = argv[index];
			if (arg == "--help" || arg == "-h") {
				std::cout << usage(argv[0]);
				return EXIT_SUCCESS;
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
	} catch (const std::exception &error) {
		std::cerr << "redif_metric_dynamics_visual_export: " << error.what() << "\n";
		std::cerr << usage(argv[0]);
		return EXIT_FAILURE;
	}

	const std::vector<int> records{0, 1, 2, 6, 10, 11, 12};
	auto space = mtrc::make_space(records, [](int lhs, int rhs) {
		const auto difference = lhs - rhs;
		return difference < 0 ? -difference : difference;
	});

	mtrc::redif_options options;
	options.neighbors = 2;
	options.iterations = 4;
	options.euler_step = 0.25;
	options.adaptive_geometry = false;

	const auto removed = mtrc::redif_remove_noise(space, options);
	const auto added = mtrc::redif_add_noise(space, options);
	const auto summaries = removed.summaries();

	std::vector<std::string> ids;
	std::vector<double> source_positions;
	ids.reserve(records.size());
	source_positions.reserve(records.size());
	for (std::size_t index = 0; index < records.size(); ++index) {
		ids.push_back(record_id(index));
		source_positions.push_back(static_cast<double>(records[index]));
	}

	std::vector<visual::Edge> metric_edges;
	for (std::size_t i = 0; i < records.size(); ++i) {
		for (std::size_t j = i + 1; j < records.size(); ++j) {
			metric_edges.push_back(visual::Edge{ids[i], ids[j], static_cast<double>(std::abs(records[i] - records[j]))});
		}
	}

	visual::Document doc;
	doc.provenance_json(json_object({visual::string_field("writer", "engine_redif_metric_dynamics_visual_export"),
									 visual::bool_field("native_export", true),
									 visual::bool_field("synthetic", false),
									 visual::bool_field("synthetic_js", false),
									 visual::string_field("computed_by", "METRIC C++ redif_remove_noise/redif_add_noise")}))
		.dataset("redif-dynamics", "Redif Metric Dynamics",
				 "Native Redif measure paths over a finite metric space", "METRIC C++ example", "CC0-1.0");
	for (std::size_t index = 0; index < records.size(); ++index) {
		doc.record(ids[index], "redif-dynamics", "scalar_atom", ids[index],
				   visual::vector_payload({static_cast<double>(records[index])}));
	}
	doc.metric_relation("redif-source-metric", "redif-dynamics", "absolute source metric", ids, metric_edges,
						"sparse_edge_list",
						json_object({visual::string_field("law_check", "absolute metric on scalar atoms"),
									 visual::bool_field("algorithmic_computation", false)}));
	doc.space("redif-source-space", "redif-dynamics", ids, "redif-source-metric", "finite_metric_space",
			  json_object({visual::string_field("operator", "redif_metric_dynamics")}));
	for (std::size_t step = 0; step < removed.paths.front().measures.size(); ++step) {
		doc.coordinates(coordinate_id(step), "redif-dynamics", "redif-source-space",
						"Redif measure barycenter step " + std::to_string(step), 3,
						step_positions(source_positions, removed, step),
						json_object({visual::string_field("computed_by", "METRIC C++ redif_remove_noise"),
									 visual::size_field("redif_step", step),
									 visual::bool_field("algorithmic_computation", false)}));
	}
	doc.scalar_property("redif-terminal-transport-path", "redif-dynamics", "Redif terminal transport path length",
						scalar_values_from_summaries(summaries, &mtrc::RedifMeasureSummary::transport_path_length),
						json_object({visual::string_field("computed_by", "METRIC C++ redif_remove_noise"),
									 visual::string_field("meaning", "sum of exact Wasserstein movement over the Redif path")}));
	doc.scalar_property("redif-terminal-entropy", "redif-dynamics", "Redif terminal entropy",
						scalar_values_from_summaries(summaries, &mtrc::RedifMeasureSummary::terminal_shannon_entropy),
						json_object({visual::string_field("computed_by", "METRIC C++ redif_remove_noise")}));
	doc.timeline_json(timeline_json(removed));
	doc.view_json(view_json("metric-space", "redif-measure-view", "Redif measure path",
							{{"spaceId", "redif-source-space"},
							 {"coordinateId", coordinate_id(removed.paths.front().measures.size() - 1)},
							 {"propertyId", "redif-terminal-transport-path"}}));
	doc.view_json(view_json("relation-matrix", "redif-source-relation-view", "Redif source distances",
							{{"relationId", "redif-source-metric"}}));
	doc.diagnostic("redif-native-evidence", "redif-dynamics", "redif_measure_paths",
				   json_object({visual::string_field("computed_by", "METRIC C++ redif_remove_noise/redif_add_noise"),
								visual::bool_field("javascript_computation", false),
								visual::string_field("operator", "redif_metric_dynamics"),
								visual::size_field("record_count", records.size()),
								visual::size_field("iterations", options.iterations),
								visual::number_field("euler_step", options.euler_step),
								visual::field("inverse_measure_paths", measure_paths_json(removed)),
								visual::field("inverse_step_transport", step_transport_json(removed)),
								visual::field("forward_measure_paths", measure_paths_json(added)),
								visual::field("operator_diagnostics",
											  json_object({visual::number_field(
															   "spectral_gap_proxy_value",
															   removed.operator_diagnostics.front().spectral_gap_proxy_value),
														   visual::string_field(
															   "spectral_gap_proxy",
															   removed.operator_diagnostics.front().spectral_gap_proxy),
														   visual::size_field(
															   "component_count",
															   removed.operator_diagnostics.front().component_count)}))}));

	const auto json = doc.to_json();
	if (export_dir.empty()) {
		std::cout << json << "\n";
		return EXIT_SUCCESS;
	}

	const auto output_path = export_dir / "metric.visual.json";
	if (!visual::write_metric_visual_file(export_dir, json + "\n")) {
		std::cerr << "failed to write Redif visual export to " << output_path << "\n";
		return EXIT_FAILURE;
	}
	std::cerr << "wrote " << output_path << "\n";
	return EXIT_SUCCESS;
}
