// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// Native metric.visual.v1 exporter for solver-trace evidence.
//
// The exported residual and objective timeline is computed in this C++ process
// from a small metric-Laplacian smoothing system. JavaScript only renders the
// resulting metric.visual.v1 document.

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <filesystem>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "mtrc_visual.hpp"

namespace {

namespace visual = mtrc::visual;

constexpr const char *kDatasetId = "solver-trace-native";
constexpr const char *kSpaceId = "solver-trace-space";
constexpr const char *kRelationId = "solver-trace-euclidean-metric";
constexpr const char *kGraphId = "solver-trace-metric-knn";
constexpr const char *kCoordinateId = "solver-trace-layout";
constexpr const char *kTimelineId = "native-laplacian-jacobi";
constexpr std::size_t kIterations = 28;
constexpr double kLambda = 0.28;
constexpr double kTolerance = 1.0e-6;

struct Record {
	std::string id;
	double x{};
	double y{};
	double signal{};
};

struct TraceEntry {
	std::size_t iteration{};
	double residual{};
	double objective{};
	double step_delta{};
	double solution_norm{};
	std::vector<double> solution;
};

auto json_object(const std::vector<std::pair<std::string, std::string>> &fields) -> std::string
{
	std::vector<visual::Field> visual_fields;
	visual_fields.reserve(fields.size());
	for (const auto &field : fields) {
		visual_fields.push_back(visual::field(field.first, field.second));
	}
	return visual::object(visual_fields);
}

auto json_bool(bool value) -> std::string { return visual::boolean(value); }

auto json_size(std::size_t value) -> std::string { return visual::size(value); }

auto records() -> std::vector<Record>
{
	std::vector<Record> out;
	out.reserve(12);
	for (std::size_t i = 0; i < 12; ++i) {
		const double t = static_cast<double>(i) / 11.0;
		const double x = 2.0 * t - 1.0;
		const double y = 0.55 * std::sin(2.0 * 3.14159265358979323846 * t);
		const auto centered_bucket = static_cast<int>(i % 3) - 1;
		const double signal = 0.72 * std::sin(3.0 * 3.14159265358979323846 * t) +
							  0.18 * std::cos(9.0 * 3.14159265358979323846 * t) +
							  0.08 * static_cast<double>(centered_bucket);
		std::ostringstream id;
		id << "solver-node-";
		if (i < 10) {
			id << '0';
		}
		id << i;
		out.push_back({id.str(), x, y, signal});
	}
	return out;
}

auto distance_matrix(const std::vector<Record> &items) -> std::vector<std::vector<double>>
{
	std::vector<std::vector<double>> matrix(items.size(), std::vector<double>(items.size(), 0.0));
	for (std::size_t i = 0; i < items.size(); ++i) {
		for (std::size_t j = i + 1; j < items.size(); ++j) {
			const double dx = items[i].x - items[j].x;
			const double dy = items[i].y - items[j].y;
			const double value = std::sqrt(dx * dx + dy * dy);
			matrix[i][j] = value;
			matrix[j][i] = value;
		}
	}
	return matrix;
}

auto affinity_matrix(const std::vector<std::vector<double>> &distances) -> std::vector<std::vector<double>>
{
	const std::size_t n = distances.size();
	std::vector<std::vector<double>> weights(n, std::vector<double>(n, 0.0));
	for (std::size_t i = 0; i < n; ++i) {
		std::vector<std::pair<double, std::size_t>> order;
		order.reserve(n - 1);
		for (std::size_t j = 0; j < n; ++j) {
			if (i != j) {
				order.push_back({distances[i][j], j});
			}
		}
		std::sort(order.begin(), order.end());
		for (std::size_t k = 0; k < std::min<std::size_t>(4, order.size()); ++k) {
			const auto j = order[k].second;
			const double d = order[k].first;
			const double weight = std::exp(-(d * d) / 0.42);
			weights[i][j] = std::max(weights[i][j], weight);
			weights[j][i] = std::max(weights[j][i], weight);
		}
	}
	return weights;
}

auto system_matrix(const std::vector<std::vector<double>> &weights) -> std::vector<std::vector<double>>
{
	const std::size_t n = weights.size();
	std::vector<std::vector<double>> a(n, std::vector<double>(n, 0.0));
	for (std::size_t i = 0; i < n; ++i) {
		double degree = 0.0;
		for (std::size_t j = 0; j < n; ++j) {
			degree += weights[i][j];
		}
		a[i][i] = 1.0 + kLambda * degree;
		for (std::size_t j = 0; j < n; ++j) {
			if (i != j) {
				a[i][j] = -kLambda * weights[i][j];
			}
		}
	}
	return a;
}

auto mat_vec(const std::vector<std::vector<double>> &a, const std::vector<double> &x) -> std::vector<double>
{
	std::vector<double> out(x.size(), 0.0);
	for (std::size_t i = 0; i < a.size(); ++i) {
		for (std::size_t j = 0; j < x.size(); ++j) {
			out[i] += a[i][j] * x[j];
		}
	}
	return out;
}

auto l2_norm(const std::vector<double> &values) -> double
{
	double sum = 0.0;
	for (const auto value : values) {
		sum += value * value;
	}
	return std::sqrt(sum);
}

auto residual_norm(const std::vector<std::vector<double>> &a, const std::vector<double> &x,
				   const std::vector<double> &b) -> double
{
	const auto ax = mat_vec(a, x);
	std::vector<double> residual(ax.size(), 0.0);
	for (std::size_t i = 0; i < ax.size(); ++i) {
		residual[i] = ax[i] - b[i];
	}
	return l2_norm(residual);
}

auto objective_value(const std::vector<std::vector<double>> &a, const std::vector<double> &x,
					 const std::vector<double> &b) -> double
{
	const auto ax = mat_vec(a, x);
	double quadratic = 0.0;
	double linear = 0.0;
	for (std::size_t i = 0; i < x.size(); ++i) {
		quadratic += x[i] * ax[i];
		linear += b[i] * x[i];
	}
	return 0.5 * quadratic - linear;
}

auto jacobi_trace(const std::vector<std::vector<double>> &a, const std::vector<double> &b) -> std::vector<TraceEntry>
{
	std::vector<double> current(b.size(), 0.0);
	std::vector<TraceEntry> trace;
	trace.reserve(kIterations + 1);
	for (std::size_t iteration = 0; iteration <= kIterations; ++iteration) {
		const double residual = residual_norm(a, current, b);
		const double objective = objective_value(a, current, b);
		const double solution_norm = l2_norm(current);
		trace.push_back({iteration, residual, objective, iteration == 0 ? 0.0 : trace.back().step_delta,
						 solution_norm, current});
		if (iteration == kIterations || residual <= kTolerance) {
			break;
		}
		std::vector<double> next(current.size(), 0.0);
		for (std::size_t i = 0; i < current.size(); ++i) {
			double off_diag = 0.0;
			for (std::size_t j = 0; j < current.size(); ++j) {
				if (i != j) {
					off_diag += a[i][j] * current[j];
				}
			}
			next[i] = (b[i] - off_diag) / a[i][i];
		}
		std::vector<double> delta(current.size(), 0.0);
		for (std::size_t i = 0; i < current.size(); ++i) {
			delta[i] = next[i] - current[i];
		}
		trace.back().step_delta = l2_norm(delta);
		current = std::move(next);
	}
	return trace;
}

auto dense_matrix_json(const std::vector<std::vector<double>> &matrix) -> std::string
{
	std::vector<std::string> rows;
	rows.reserve(matrix.size());
	for (const auto &row : matrix) {
		rows.push_back(visual::number_array(row));
	}
	return visual::array_of(rows);
}

auto record_ids(const std::vector<Record> &items) -> std::vector<std::string>
{
	std::vector<std::string> ids;
	ids.reserve(items.size());
	for (const auto &item : items) {
		ids.push_back(item.id);
	}
	return ids;
}

auto timeline_step_id(std::size_t iteration) -> std::string
{
	std::ostringstream id;
	id << "jacobi-step-";
	if (iteration < 10) {
		id << '0';
	}
	id << iteration;
	return id.str();
}

auto trace_property(const std::string &id, const std::string &name, const std::string &field,
					const std::vector<TraceEntry> &trace, const std::string &description) -> std::string
{
	std::vector<std::string> values;
	values.reserve(trace.size());
	for (const auto &entry : trace) {
		double value = entry.residual;
		if (field == "objective") {
			value = entry.objective;
		} else if (field == "step_delta") {
			value = entry.step_delta;
		} else if (field == "solution_norm") {
			value = entry.solution_norm;
		}
		values.push_back(json_object({{"timeline_id", visual::quote(kTimelineId)},
									  {"step_id", visual::quote(timeline_step_id(entry.iteration))},
									  {"index", json_size(entry.iteration)},
									  {field, visual::num(value)},
									  {"value", visual::num(value)}}));
	}
	return json_object({{"id", visual::quote(id)},
						{"dataset_id", visual::quote(kDatasetId)},
						{"name", visual::quote(name)},
						{"target_type", visual::quote("timeline_step")},
						{"value_type", visual::quote("scalar")},
						{"values", visual::array_of(values)},
						{"metadata",
						 json_object({{"description", visual::quote(description)},
									  {"computed_by", visual::quote("native C++ Jacobi iteration")},
									  {"timeline_id", visual::quote(kTimelineId)},
									  {"solver_field", visual::quote(field)}})}});
}

auto build_visual_document() -> std::string
{
	const auto items = records();
	const auto ids = record_ids(items);
	const auto distances = distance_matrix(items);
	const auto weights = affinity_matrix(distances);
	const auto a = system_matrix(weights);
	std::vector<double> b;
	b.reserve(items.size());
	for (const auto &item : items) {
		b.push_back(item.signal);
	}
	const auto trace = jacobi_trace(a, b);
	const auto final_residual = trace.empty() ? 0.0 : trace.back().residual;
	const auto initial_residual = trace.empty() ? 0.0 : trace.front().residual;
	const bool residual_monotone = std::adjacent_find(trace.begin(), trace.end(), [](const auto &lhs, const auto &rhs) {
		return rhs.residual > lhs.residual + 1.0e-12;
	}) == trace.end();
	const bool objective_monotone = std::adjacent_find(trace.begin(), trace.end(), [](const auto &lhs, const auto &rhs) {
		return rhs.objective > lhs.objective + 1.0e-12;
	}) == trace.end();

	std::vector<std::string> datasets;
	datasets.push_back(json_object(
		{{"id", visual::quote(kDatasetId)},
		 {"title", visual::quote("Native solver trace evidence")},
		 {"description",
		  visual::quote("C++ Jacobi solver trace for a metric-Laplacian smoothing system; residuals and objectives are exported as timeline-step properties.")},
		 {"source", visual::quote("examples/engine/solver_trace_visual_export.cpp")},
		 {"license", visual::quote("MPL-2.0")}}));

	std::vector<std::string> records_json;
	records_json.reserve(items.size());
	for (const auto &item : items) {
		records_json.push_back(json_object({{"id", visual::quote(item.id)},
											{"dataset_id", visual::quote(kDatasetId)},
											{"record_type", visual::quote("vector")},
											{"label", visual::quote(item.id)},
											{"payload",
											 json_object({{"kind", visual::quote("vector")},
														  {"values", visual::number_array({item.x, item.y, item.signal})},
														  {"meaning", visual::quote("x, y, observed signal")}})}}));
	}

	const auto relation_metadata =
		json_object({{"computed_by", visual::quote("native C++ Euclidean distance on solver fixture coordinates")},
					 {"used_by", visual::quote("metric-Laplacian smoothing system")},
					 {"system", visual::quote("(I + lambda L) x = b")},
					 {"lambda", visual::num(kLambda)}});
	std::vector<std::string> relations;
	relations.push_back(json_object({{"id", visual::quote(kRelationId)},
									  {"dataset_id", visual::quote(kDatasetId)},
									  {"name", visual::quote("solver fixture Euclidean metric")},
									  {"relation_type", visual::quote("metric")},
									  {"value_type", visual::quote("scalar")},
									  {"record_ids", visual::string_array(ids)},
									  {"storage", visual::quote("dense_matrix")},
									  {"values", dense_matrix_json(distances)},
									  {"metadata", relation_metadata}}));

	std::vector<std::string> spaces;
	spaces.push_back(json_object({{"id", visual::quote(kSpaceId)},
								  {"dataset_id", visual::quote(kDatasetId)},
								  {"record_ids", visual::string_array(ids)},
								  {"primary_relation_id", visual::quote(kRelationId)},
								  {"space_type", visual::quote("finite_metric_space")},
								  {"metadata",
								   json_object({{"native_solver", visual::quote("Jacobi")},
												{"system", visual::quote("(I + lambda L) x = b")},
												{"lambda", visual::num(kLambda)}})}}));

	std::vector<std::string> positions;
	positions.reserve(items.size());
	for (const auto &item : items) {
		positions.push_back(json_object({{"record_id", visual::quote(item.id)},
										 {"position", visual::number_array({item.x, item.y, item.signal})}}));
	}
	std::vector<std::string> coordinates;
	coordinates.push_back(json_object({{"id", visual::quote(kCoordinateId)},
									   {"dataset_id", visual::quote(kDatasetId)},
									   {"space_id", visual::quote(kSpaceId)},
									   {"name", visual::quote("solver fixture coordinate layout")},
									   {"dimension", "3"},
									   {"record_positions", visual::array_of(positions)},
									   {"metadata",
										json_object({{"z_channel", visual::quote("observed signal")},
													 {"computed_by", visual::quote("native fixture coordinates")}})}}));

	std::vector<std::string> graph_edges;
	std::size_t edge_id = 0;
	for (std::size_t i = 0; i < weights.size(); ++i) {
		for (std::size_t j = i + 1; j < weights[i].size(); ++j) {
			if (weights[i][j] <= 0.0) {
				continue;
			}
			graph_edges.push_back(json_object({{"id", visual::quote("solver-edge-" + std::to_string(edge_id++))},
											   {"source", visual::quote(ids[i])},
											   {"target", visual::quote(ids[j])},
											   {"weight", visual::num(weights[i][j])},
											   {"distance", visual::num(distances[i][j])}}));
		}
	}
	std::vector<std::string> graphs;
	graphs.push_back(json_object({{"id", visual::quote(kGraphId)},
								  {"dataset_id", visual::quote(kDatasetId)},
								  {"node_record_ids", visual::string_array(ids)},
								  {"edge_relation_id", visual::quote(kRelationId)},
								  {"graph_type", visual::quote("metric_knn_affinity")},
								  {"edges", visual::array_of(graph_edges)},
								  {"metadata",
								   json_object({{"computed_by", visual::quote("native C++ k-nearest metric affinities")},
												{"edge_count", json_size(graph_edges.size())}})}}));

	std::vector<std::string> properties;
	std::vector<std::string> observed_values;
	std::vector<std::string> solved_values;
	observed_values.reserve(items.size());
	solved_values.reserve(items.size());
	const auto &solution = trace.back().solution;
	for (std::size_t i = 0; i < items.size(); ++i) {
		observed_values.push_back(json_object({{"record_id", visual::quote(ids[i])},
											   {"value", visual::num(b[i])}}));
		solved_values.push_back(json_object({{"record_id", visual::quote(ids[i])},
											 {"value", visual::num(solution[i])}}));
	}
	properties.push_back(json_object({{"id", visual::quote("observed-signal")},
									  {"dataset_id", visual::quote(kDatasetId)},
									  {"name", visual::quote("observed right-hand side signal")},
									  {"target_type", visual::quote("record")},
									  {"value_type", visual::quote("scalar")},
									  {"values", visual::array_of(observed_values)}}));
	properties.push_back(json_object({{"id", visual::quote("final-solver-solution")},
									  {"dataset_id", visual::quote(kDatasetId)},
									  {"name", visual::quote("final native solver solution")},
									  {"target_type", visual::quote("record")},
									  {"value_type", visual::quote("scalar")},
									  {"values", visual::array_of(solved_values)}}));
	properties.push_back(trace_property("jacobi-residual", "Jacobi residual norm", "residual", trace,
										"Native residual norm ||Ax-b||_2 for each exported solver iteration."));
	properties.push_back(trace_property("jacobi-objective", "Jacobi quadratic objective", "objective", trace,
										"Native objective 0.5*x^T*A*x - b^T*x for each exported solver iteration."));
	properties.push_back(trace_property("jacobi-step-delta", "Jacobi step delta", "step_delta", trace,
										"Native ||x_{k+1}-x_k||_2 for hover/preview inspection."));
	properties.push_back(trace_property("jacobi-solution-norm", "Jacobi solution norm", "solution_norm", trace,
										"Native ||x_k||_2 for hover/preview inspection."));

	std::vector<std::string> steps;
	steps.reserve(trace.size());
	for (const auto &entry : trace) {
		steps.push_back(json_object({{"id", visual::quote(timeline_step_id(entry.iteration))},
									 {"index", json_size(entry.iteration)},
									 {"iteration", json_size(entry.iteration)},
									 {"t", visual::num(static_cast<double>(entry.iteration))},
									 {"name", visual::quote("Jacobi iteration " + std::to_string(entry.iteration))},
									 {"coordinate_id", visual::quote(kCoordinateId)},
									 {"graph_id", visual::quote(kGraphId)},
									 {"residual", visual::num(entry.residual)},
									 {"objective", visual::num(entry.objective)},
									 {"step_delta", visual::num(entry.step_delta)},
									 {"solution_norm", visual::num(entry.solution_norm)}}));
	}
	std::vector<std::string> timelines;
	timelines.push_back(json_object({{"id", visual::quote(kTimelineId)},
									 {"dataset_id", visual::quote(kDatasetId)},
									 {"name", visual::quote("Native Jacobi solver trace")},
									 {"space_id", visual::quote(kSpaceId)},
									 {"timeline_type", visual::quote("solver_trace")},
									 {"steps", visual::array_of(steps)},
									 {"metadata",
									  json_object({{"algorithm", visual::quote("Jacobi")},
												   {"computed_by", visual::quote("native C++ metric-Laplacian solve")},
												   {"system", visual::quote("(I + lambda L) x = b")},
												   {"lambda", visual::num(kLambda)},
												   {"iterations", json_size(trace.size() - 1)},
												   {"initial_residual", visual::num(initial_residual)},
												   {"final_residual", visual::num(final_residual)},
												   {"residual_reduction", visual::num(final_residual / initial_residual)},
												   {"objective_initial", visual::num(trace.front().objective)},
												   {"objective_final", visual::num(trace.back().objective)},
												   {"tolerance", visual::num(kTolerance)}})}}));

	std::vector<std::string> events;
	events.push_back(json_object({{"id", visual::quote("event-native-solver-summary")},
								  {"dataset_id", visual::quote(kDatasetId)},
								  {"event_type", visual::quote("solver_summary")},
								  {"payload",
								   json_object({{"algorithm", visual::quote("Jacobi")},
												{"iterations", json_size(trace.size() - 1)},
												{"initial_residual", visual::num(initial_residual)},
												{"final_residual", visual::num(final_residual)},
												{"residual_reduction", visual::num(final_residual / initial_residual)},
												{"objective_initial", visual::num(trace.front().objective)},
												{"objective_final", visual::num(trace.back().objective)},
												{"residual_monotone", json_bool(residual_monotone)},
												{"objective_monotone", json_bool(objective_monotone)}})}}));

	std::vector<std::string> views;
	views.push_back(json_object({{"id", visual::quote("view-native-solver-trace")},
								 {"kind", visual::quote("solver-trace")},
								 {"name", visual::quote("Native solver residual trace")},
								 {"spaceId", visual::quote(kSpaceId)},
								 {"timelineId", visual::quote(kTimelineId)},
								 {"propertyId", visual::quote("jacobi-residual")},
								 {"tracePropertyId", visual::quote("jacobi-residual")}}));

	std::vector<std::string> diagnostics;
	diagnostics.push_back(json_object({{"id", visual::quote("check-residual-reduces")},
									   {"kind", visual::quote("native-invariant")},
									   {"status", visual::quote(final_residual < initial_residual ? "pass" : "fail")},
									   {"native_example_check", visual::quote("residual is reduced by native Jacobi iterations")},
									   {"value", visual::num(final_residual / initial_residual)},
									   {"comparison", visual::quote("<")},
									   {"threshold", visual::num(1.0)}}));
	diagnostics.push_back(json_object({{"id", visual::quote("check-objective-monotone")},
									   {"kind", visual::quote("native-invariant")},
									   {"status", visual::quote(objective_monotone ? "pass" : "fail")},
									   {"native_example_check", visual::quote("quadratic objective is non-increasing")},
									   {"value", json_bool(objective_monotone)},
									   {"expected", "true"}}));

	const auto provenance = json_object(
		{{"writer", visual::quote("examples/engine/solver_trace_visual_export.cpp")},
		 {"generator", visual::quote("native-cpp")},
		 {"computed_by", visual::quote("native C++ metric-Laplacian Jacobi solver")},
		 {"synthetic_js", "false"},
		 {"native_export", "true"},
		 {"public_hero_ready", "false"},
		 {"native_checks_pass", json_bool(final_residual < initial_residual && objective_monotone)},
		 {"native_check_count", json_size(diagnostics.size())},
		 {"schema_note", visual::quote("Native solver-trace export; no JavaScript-computed solver data.")}});

	return json_object({{"schema", visual::quote("metric.visual.v1")},
						{"provenance", provenance},
						{"datasets", visual::array_of(datasets)},
						{"records", visual::array_of(records_json)},
						{"relations", visual::array_of(relations)},
						{"spaces", visual::array_of(spaces)},
						{"properties", visual::array_of(properties)},
						{"graphs", visual::array_of(graphs)},
						{"coordinates", visual::array_of(coordinates)},
						{"timelines", visual::array_of(timelines)},
						{"events", visual::array_of(events)},
						{"views", visual::array_of(views)},
						{"diagnostics", visual::array_of(diagnostics)}});
}

struct CliOptions {
	std::filesystem::path export_dir;
	bool write_file{};
};

auto cli_options_from_args(int argc, char **argv) -> CliOptions
{
	CliOptions options;
	for (int index = 1; index < argc; ++index) {
		const std::string arg = argv[index];
		if (arg == "--export-dir") {
			options.write_file = true;
			if (index + 1 >= argc) {
				return options;
			}
			options.export_dir = argv[++index];
		} else if (arg.rfind("--export-dir=", 0) == 0) {
			options.export_dir = arg.substr(std::string("--export-dir=").size());
			options.write_file = true;
		}
	}
	return options;
}

} // namespace

auto main(int argc, char **argv) -> int
{
	const auto options = cli_options_from_args(argc, argv);
	const auto document = build_visual_document();
	if (!options.write_file) {
		std::cout << document << "\n";
		return 0;
	}

	if (options.export_dir.empty()) {
		std::cerr << "--export-dir requires a directory\n";
		return 1;
	}

	if (!visual::write_metric_visual_file(options.export_dir, document)) {
		std::cerr << "failed to write metric.visual.json to " << options.export_dir << "\n";
		return 1;
	}
	std::cerr << "wrote " << (options.export_dir / "metric.visual.json") << "\n";
	return 0;
}
