#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <metric/engine.hpp>

#ifndef METRIC_PROCESS_CURVE_POWER_DEMAND_CSV
#define METRIC_PROCESS_CURVE_POWER_DEMAND_CSV "assets/process_curve_power_demand_gallery.csv"
#endif

#ifndef METRIC_PROCESS_CURVE_INTERNAL_BLEEDING_CSV
#define METRIC_PROCESS_CURVE_INTERNAL_BLEEDING_CSV "assets/process_curve_internal_bleeding_gallery.csv"
#endif

namespace {

constexpr const char *kSource = "UCR_Time_Series_Anomaly_Detection_2021";
constexpr const char *kLicense = "CC BY 4.0";

struct ExternalProcessWindow {
	std::string id;
	std::string source_label;
	std::vector<double> values;
};

struct ProcessCurveQuery {
	std::string id;
	std::string expected_label;
	std::vector<double> values;
};

struct DomainBenchmarkInput {
	std::string name;
	std::string title;
	std::string csv_path;
	std::size_t expected_records;
	std::size_t expected_queries;
	double minimum_average_margin;
};

// One query carries the full recovery evidence: the metric-space winner, the
// padded-vector baseline winner, both record-pair values, the metric margin,
// and the role-correctness flags.
struct QueryEvidence {
	std::string id;
	std::string expected_label;
	std::vector<double> values;
	std::string metric_winner_id;
	std::string metric_winner_label;
	std::size_t metric_winner_index{0};
	double metric_distance{0.0};
	std::string vector_winner_id;
	std::string vector_winner_label;
	std::size_t vector_winner_index{0};
	double vector_distance{0.0};
	double metric_margin{0.0};
	bool metric_correct{false};
	bool vector_mismatch{false};
};

// All computed evidence for one process-curve domain. The summary scalars stay
// identical to the original report; the extra record/query/matrix fields exist
// only so the export mode can render the same C++ evidence as static assets.
struct DomainEvidence {
	std::string name;
	std::string title;
	std::vector<std::string> ids;
	std::vector<std::string> labels;
	std::vector<std::vector<double>> records;
	std::vector<QueryEvidence> queries;
	std::vector<std::vector<double>> metric_distances;
	std::vector<std::vector<double>> vector_distances;
	std::size_t metric_correct{0};
	std::size_t vector_mismatches{0};
	double average_metric_margin{0.0};
	std::size_t dense_evaluations{0};
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
					previous[rhs_index - 1] + std::min(std::abs(lhs[lhs_index - 1] - rhs[rhs_index - 1]), 2 * gap_cost);
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

auto load_external_windows(const std::string &path) -> std::vector<ExternalProcessWindow>
{
	std::ifstream input(path);
	if (!input) {
		throw std::runtime_error("unable to open process-curve gallery CSV: " + path);
	}

	std::string line;
	std::getline(input, line);
	std::vector<ExternalProcessWindow> records;
	while (std::getline(input, line)) {
		if (line.empty()) {
			continue;
		}
		auto fields = split_csv_line(line);
		if (fields.size() != 6) {
			throw std::runtime_error("unexpected process-curve gallery CSV row: " + line);
		}
		records.push_back(ExternalProcessWindow{fields[0], fields[4], parse_values(fields[5])});
	}
	return records;
}

auto downsample_even(const std::vector<double> &values) -> std::vector<double>
{
	std::vector<double> query;
	for (std::size_t index = 0; index < values.size(); index += 2) {
		query.push_back(values[index]);
	}
	return query;
}

auto contains(const std::string &text, const std::string &needle) -> bool
{
	return text.find(needle) != std::string::npos;
}

auto run_domain_benchmark(const DomainBenchmarkInput &input) -> DomainEvidence
{
	const auto windows = load_external_windows(input.csv_path);
	assert(windows.size() == input.expected_records);

	DomainEvidence evidence;
	evidence.name = input.name;
	evidence.title = input.title;

	std::vector<ProcessCurveQuery> queries;
	for (const auto &window : windows) {
		evidence.ids.push_back(window.id);
		evidence.labels.push_back(window.source_label);
		evidence.records.push_back(window.values);
		assert(window.values.size() == 36);
		if (contains(window.id, "anomaly_start") || contains(window.id, "recovery")) {
			queries.push_back(
				ProcessCurveQuery{"downsampled_" + window.id, window.source_label, downsample_even(window.values)});
		}
	}
	assert(queries.size() == input.expected_queries);

	auto alignment_space = mtrc::make_space(evidence.records, AlignedCurveDistance{});
	auto vector_baseline = mtrc::make_space(evidence.records, PointwisePaddedEuclideanDistance{});

	mtrc::space::storage::DistanceTable<decltype(alignment_space)> matrix(alignment_space);
	const auto matrix_diagnostics = matrix.diagnostics();
	assert(matrix_diagnostics.cached_distances == evidence.records.size() * evidence.records.size());

	// Materialize both representations of the finite-space pair values from the
	// same record set so the exported heatmaps are read straight off the metric.
	const auto record_count = evidence.records.size();
	evidence.metric_distances.assign(record_count, std::vector<double>(record_count, 0.0));
	evidence.vector_distances.assign(record_count, std::vector<double>(record_count, 0.0));
	for (std::size_t row = 0; row < record_count; ++row) {
		for (std::size_t column = 0; column < record_count; ++column) {
			evidence.metric_distances[row][column] =
				matrix.distance(alignment_space.id(row), alignment_space.id(column));
			evidence.vector_distances[row][column] =
				vector_baseline.distance(vector_baseline.id(row), vector_baseline.id(column));
		}
	}

	std::size_t metric_correct = 0;
	std::size_t vector_mismatches = 0;
	double metric_margin_sum = 0.0;

	for (std::size_t query_index = 0; query_index < queries.size(); ++query_index) {
		const auto &query = queries[query_index];
		assert(query.values.size() == 18);
		const auto metric_neighbors = mtrc::find_neighbors(alignment_space, query.values, 1);
		const auto baseline_neighbors = mtrc::find_neighbors(vector_baseline, query.values, 1);
		assert(metric_neighbors.size() == 1);
		assert(baseline_neighbors.size() == 1);

		const auto metric_index = metric_neighbors[0].id.index();
		const auto baseline_index = baseline_neighbors[0].id.index();
		const auto metric_label = evidence.labels[metric_index];
		const auto baseline_label = evidence.labels[baseline_index];
		const bool metric_correct_here = metric_label == query.expected_label;
		const bool vector_mismatch_here = baseline_label != query.expected_label;
		if (metric_correct_here) {
			++metric_correct;
		}
		if (vector_mismatch_here) {
			++vector_mismatches;
		}

		const auto baseline_distance_under_metric =
			alignment_space.metric()(query.values, evidence.records[baseline_index]);
		const auto metric_margin = baseline_distance_under_metric - metric_neighbors[0].distance;
		assert(metric_margin > 0.0);
		metric_margin_sum += metric_margin;

		QueryEvidence query_evidence;
		query_evidence.id = query.id;
		query_evidence.expected_label = query.expected_label;
		query_evidence.values = query.values;
		query_evidence.metric_winner_id = evidence.ids[metric_index];
		query_evidence.metric_winner_label = metric_label;
		query_evidence.metric_winner_index = metric_index;
		query_evidence.metric_distance = metric_neighbors[0].distance;
		query_evidence.vector_winner_id = evidence.ids[baseline_index];
		query_evidence.vector_winner_label = baseline_label;
		query_evidence.vector_winner_index = baseline_index;
		query_evidence.vector_distance = baseline_neighbors[0].distance;
		query_evidence.metric_margin = metric_margin;
		query_evidence.metric_correct = metric_correct_here;
		query_evidence.vector_mismatch = vector_mismatch_here;
		evidence.queries.push_back(std::move(query_evidence));
	}
	assert(metric_correct == queries.size());
	assert(vector_mismatches == queries.size());
	assert(metric_margin_sum / static_cast<double>(queries.size()) > input.minimum_average_margin);

	evidence.metric_correct = metric_correct;
	evidence.vector_mismatches = vector_mismatches;
	evidence.average_metric_margin = metric_margin_sum / static_cast<double>(queries.size());
	evidence.dense_evaluations = matrix_diagnostics.cached_distances;
	return evidence;
}

void print_text_report(const std::vector<DomainEvidence> &domains, std::size_t total_records, std::size_t total_queries,
					   std::size_t total_dense_evaluations)
{
	std::cout << "process external source = " << kSource << "\n";
	std::cout << "process external license = " << kLicense << "\n";
	std::cout << "process external domains = " << domains.size() << "\n";
	std::cout << "process external records = " << total_records << "\n";
	std::cout << "process external queries = " << total_queries << "\n";
	for (const auto &summary : domains) {
		const auto &first_query = summary.queries.front();
		std::cout << "process external " << summary.name << " records = " << summary.records.size() << "\n";
		std::cout << "process external " << summary.name << " queries = " << summary.queries.size() << "\n";
		std::cout << "process external " << summary.name << " metric correct = " << summary.metric_correct << "/"
				  << summary.queries.size() << "\n";
		std::cout << "process external " << summary.name << " vector mismatches = " << summary.vector_mismatches << "/"
				  << summary.queries.size() << "\n";
		std::cout << "process external " << summary.name << " average metric margin = " << summary.average_metric_margin
				  << "\n";
		std::cout << "process external " << summary.name << " first query = " << first_query.id << "\n";
		std::cout << "process external " << summary.name << " first metric winner = " << first_query.metric_winner_id
				  << " at " << first_query.metric_distance << "\n";
		std::cout << "process external " << summary.name << " first vector winner = " << first_query.vector_winner_id
				  << " at " << first_query.vector_distance << "\n";
		std::cout << "process external " << summary.name << " dense evaluations = " << summary.dense_evaluations << "\n";
	}
	std::cout << "process external dense evaluations = " << total_dense_evaluations << "\n";
}

// --------------------------------------------------------------------------
// Export mode: render the C++ evidence as CSV tables and static SVG assets.
// Nothing below recomputes a metric value; it only formats already-computed
// evidence held in the DomainEvidence structures.
// --------------------------------------------------------------------------

auto format_number(double value) -> std::string
{
	std::ostringstream out;
	out << std::setprecision(10) << value;
	return out.str();
}

auto format_coordinate(double value) -> std::string
{
	std::ostringstream out;
	out << std::fixed << std::setprecision(1) << value;
	return out.str();
}

// Distances are hundreds of metric units; rounded whole numbers keep the SVG
// labels readable. Full precision stays in the CSV evidence.
auto format_display(double value) -> std::string
{
	std::ostringstream out;
	out << std::fixed << std::setprecision(0) << value;
	return out.str();
}

auto format_values(const std::vector<double> &values) -> std::string
{
	std::ostringstream out;
	for (std::size_t index = 0; index < values.size(); ++index) {
		if (index != 0) {
			out << ' ';
		}
		out << format_number(values[index]);
	}
	return out.str();
}

auto csv_field(const std::string &value) -> std::string
{
	const bool needs_quotes = value.find_first_of(",\"\n ") != std::string::npos;
	if (!needs_quotes) {
		return value;
	}
	std::string escaped = "\"";
	for (const char character : value) {
		if (character == '"') {
			escaped += "\"\"";
		} else {
			escaped.push_back(character);
		}
	}
	escaped.push_back('"');
	return escaped;
}

auto svg_escape(const std::string &value) -> std::string
{
	std::string escaped;
	for (const char character : value) {
		switch (character) {
		case '&':
			escaped += "&amp;";
			break;
		case '<':
			escaped += "&lt;";
			break;
		case '>':
			escaped += "&gt;";
			break;
		case '"':
			escaped += "&quot;";
			break;
		case '\'':
			escaped += "&#39;";
			break;
		default:
			escaped.push_back(character);
		}
	}
	return escaped;
}

auto json_escape(const std::string &value) -> std::string
{
	std::string escaped;
	for (const char character : value) {
		switch (character) {
		case '\\':
			escaped += "\\\\";
			break;
		case '"':
			escaped += "\\\"";
			break;
		case '\b':
			escaped += "\\b";
			break;
		case '\f':
			escaped += "\\f";
			break;
		case '\n':
			escaped += "\\n";
			break;
		case '\r':
			escaped += "\\r";
			break;
		case '\t':
			escaped += "\\t";
			break;
		default:
			escaped.push_back(character);
		}
	}
	return escaped;
}

auto json_string(const std::string &value) -> std::string { return "\"" + json_escape(value) + "\""; }

auto json_bool(bool value) -> const char * { return value ? "true" : "false"; }

auto json_values(const std::vector<double> &values) -> std::string
{
	std::ostringstream out;
	out << '[';
	for (std::size_t index = 0; index < values.size(); ++index) {
		if (index != 0) {
			out << ',';
		}
		out << format_number(values[index]);
	}
	out << ']';
	return out.str();
}

void write_file(const std::filesystem::path &path, const std::string &content)
{
	std::ofstream output(path, std::ios::binary);
	if (!output) {
		throw std::runtime_error("unable to write export file: " + path.string());
	}
	output << content;
}

// Power Demand labels its onset windows "anomaly"; Internal Bleeding splits the
// same phase into "anomaly_start"/"anomaly_mid". Both onset variants share the
// anomaly color so the role legend stays consistent across domains.
auto role_color(const std::string &label) -> std::string
{
	if (label == "normal") {
		return "#2563eb";
	}
	if (label == "normal_mid") {
		return "#3b82f6";
	}
	if (label == "pre_anomaly") {
		return "#16a34a";
	}
	if (label == "anomaly" || label == "anomaly_start") {
		return "#dc2626";
	}
	if (label == "anomaly_mid") {
		return "#ea580c";
	}
	if (label == "recovery") {
		return "#7c3aed";
	}
	return "#64748b";
}

struct PlotBox {
	double x{0.0};
	double y{0.0};
	double width{0.0};
	double height{0.0};
};

auto curve_extent(const std::vector<const std::vector<double> *> &curves) -> std::pair<double, double>
{
	double minimum = 0.0;
	double maximum = 0.0;
	bool first = true;
	for (const auto *curve : curves) {
		for (const double value : *curve) {
			if (first) {
				minimum = maximum = value;
				first = false;
			} else {
				minimum = std::min(minimum, value);
				maximum = std::max(maximum, value);
			}
		}
	}
	if (maximum <= minimum) {
		maximum = minimum + 1.0;
	}
	return {minimum, maximum};
}

auto curve_points(const std::vector<double> &values, const PlotBox &box, double minimum, double maximum) -> std::string
{
	std::ostringstream out;
	const std::size_t count = values.size();
	for (std::size_t index = 0; index < count; ++index) {
		const double fraction = count <= 1 ? 0.0 : static_cast<double>(index) / static_cast<double>(count - 1);
		const double x = box.x + fraction * box.width;
		const double normalized = (values[index] - minimum) / (maximum - minimum);
		const double y = box.y + box.height - normalized * box.height;
		if (index != 0) {
			out << ' ';
		}
		out << format_coordinate(x) << ',' << format_coordinate(y);
	}
	return out.str();
}

auto heat_color(double normalized) -> std::string
{
	normalized = std::max(0.0, std::min(1.0, normalized));
	// Low metric value (close records) reads dark; large value fades to light,
	// so similarity structure stays the visually dense region of each grid.
	const double low[3] = {12.0, 61.0, 84.0};
	const double high[3] = {238.0, 243.0, 247.0};
	int channels[3];
	for (int component = 0; component < 3; ++component) {
		channels[component] =
			static_cast<int>(std::lround(low[component] + (high[component] - low[component]) * normalized));
	}
	std::ostringstream out;
	out << '#' << std::hex << std::setfill('0');
	for (const int channel : channels) {
		out << std::setw(2) << channel;
	}
	return out.str();
}

auto text_element(double x, double y, int size, const std::string &fill, const std::string &content,
				  const std::string &weight = "400") -> std::string
{
	std::ostringstream out;
	out << "  <text x=\"" << format_coordinate(x) << "\" y=\"" << format_coordinate(y)
		<< "\" font-family=\"Arial, sans-serif\" font-size=\"" << size << "\"";
	if (weight != "400") {
		out << " font-weight=\"" << weight << "\"";
	}
	out << " fill=\"" << fill << "\">" << svg_escape(content) << "</text>\n";
	return out.str();
}

auto make_query_comparison_svg(const DomainEvidence &domain) -> std::string
{
	const auto &query = domain.queries.front();
	const auto &metric_curve = domain.records[query.metric_winner_index];
	const auto &vector_curve = domain.records[query.vector_winner_index];
	const auto extent = curve_extent({&query.values, &metric_curve, &vector_curve});

	const PlotBox box{70.0, 104.0, 786.0, 196.0};
	std::ostringstream svg;
	svg << "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"920\" height=\"480\" viewBox=\"0 0 920 480\" role=\"img\" "
		   "aria-labelledby=\"title desc\">\n";
	svg << "  <title id=\"title\">" << svg_escape(domain.title) << " query recovery</title>\n";
	svg << "  <desc id=\"desc\">A downsampled " << svg_escape(domain.title)
		<< " query, the finite metric-space winner record, and the padded-vector baseline winner record, with their "
		   "metric values and recovered roles.</desc>\n";
	svg << "  <rect width=\"920\" height=\"480\" fill=\"#f8fafc\"/>\n";
	svg << text_element(54, 44, 23, "#172033", domain.title + " query recovery", "700");
	svg << text_element(54, 70, 13, "#526070",
						 "Finite metric space over process curves vs padded-vector baseline. UCR 2021, CC BY 4.0.");

	svg << "  <line x1=\"" << format_coordinate(box.x) << "\" y1=\"" << format_coordinate(box.y + box.height) << "\" x2=\""
		<< format_coordinate(box.x + box.width) << "\" y2=\"" << format_coordinate(box.y + box.height)
		<< "\" stroke=\"#cbd5e1\"/>\n";
	svg << "  <line x1=\"" << format_coordinate(box.x) << "\" y1=\"" << format_coordinate(box.y) << "\" x2=\""
		<< format_coordinate(box.x) << "\" y2=\"" << format_coordinate(box.y + box.height) << "\" stroke=\"#cbd5e1\"/>\n";
	svg << text_element(42, box.y + box.height + 4, 10, "#64748b", "low");
	svg << text_element(40, box.y + 6, 10, "#64748b", "high");

	const auto metric_points = curve_points(metric_curve, box, extent.first, extent.second);
	const auto vector_points = curve_points(vector_curve, box, extent.first, extent.second);
	const auto query_points = curve_points(query.values, box, extent.first, extent.second);
	svg << "  <polyline fill=\"none\" stroke=\"#16a34a\" stroke-width=\"2.5\" stroke-linejoin=\"round\" "
		   "stroke-linecap=\"round\" points=\""
		<< metric_points << "\"/>\n";
	svg << "  <polyline fill=\"none\" stroke=\"#dc2626\" stroke-width=\"2.5\" stroke-linejoin=\"round\" "
		   "stroke-linecap=\"round\" points=\""
		<< vector_points << "\"/>\n";
	svg << "  <polyline fill=\"none\" stroke=\"#0f172a\" stroke-width=\"2\" stroke-dasharray=\"5 4\" "
		   "stroke-linejoin=\"round\" stroke-linecap=\"round\" points=\""
		<< query_points << "\"/>\n";

	// Three full-width evidence cells below the plot leave room for every label.
	struct Cell {
		double x;
		std::string marker;
		std::string heading;
		std::string heading_fill;
		std::string role_line;
		std::string value_line;
		std::string note;
	};
	const std::vector<Cell> cells{
		{54.0, "#0f172a", "Query (dashed)", "#172033", query.expected_label + " role, 18 samples",
		 "the record to recover", ""},
		{328.0, "#16a34a", "Metric-space winner", "#166534", query.metric_winner_label + " role",
		 "metric value " + format_display(query.metric_distance), "matches expected role"},
		{602.0, "#dc2626", "Baseline winner", "#991b1b", query.vector_winner_label + " role",
		 "baseline value " + format_display(query.vector_distance), "wrong role"}};
	const double cell_y = 328.0;
	const double cell_w = 264.0;
	for (const auto &cell : cells) {
		svg << "  <rect x=\"" << format_coordinate(cell.x) << "\" y=\"" << format_coordinate(cell_y) << "\" width=\""
			<< format_coordinate(cell_w) << "\" height=\"96\" rx=\"8\" fill=\"#ffffff\" stroke=\"#cbd5e1\"/>\n";
		svg << "  <line x1=\"" << format_coordinate(cell.x + 18) << "\" y1=\"" << format_coordinate(cell_y + 22)
			<< "\" x2=\"" << format_coordinate(cell.x + 44) << "\" y2=\"" << format_coordinate(cell_y + 22)
			<< "\" stroke=\"" << cell.marker << "\" stroke-width=\"3\"/>\n";
		svg << text_element(cell.x + 52, cell_y + 26, 12, cell.heading_fill, cell.heading, "700");
		svg << text_element(cell.x + 18, cell_y + 50, 12, "#172033", cell.role_line);
		svg << text_element(cell.x + 18, cell_y + 70, 11, "#526070", cell.value_line);
		if (!cell.note.empty()) {
			svg << text_element(cell.x + 18, cell_y + 88, 11, cell.heading_fill, cell.note);
		}
	}

	svg << "  <rect x=\"54\" y=\"432\" width=\"812\" height=\"44\" rx=\"6\" fill=\"#ffffff\" stroke=\"#cbd5e1\"/>\n";
	svg << text_element(72, 452, 12, "#334155",
						 "The alignment metric recovers the " + query.expected_label +
							 " role despite the timing shift.");
	svg << text_element(72, 470, 12, "#334155",
						 "The padded-vector baseline compares index by index and locks onto the " +
							 query.vector_winner_label + " level instead.");
	svg << "</svg>\n";
	return svg.str();
}

auto make_distance_heatmap_svg(const DomainEvidence &domain) -> std::string
{
	const std::size_t count = domain.records.size();
	const double cell = 8.0;
	const double grid_size = static_cast<double>(count) * cell;
	const double left_x = 70.0;
	const double right_x = 70.0 + grid_size + 200.0;
	const double grid_y = 132.0;
	const double role_bar_y = grid_y - 12.0;
	const double total_width = right_x + grid_size + 70.0;
	const double width = std::max(860.0, total_width);

	auto extent_of = [count](const std::vector<std::vector<double>> &matrix) -> std::pair<double, double> {
		double minimum = matrix[0][0];
		double maximum = matrix[0][0];
		for (std::size_t row = 0; row < count; ++row) {
			for (std::size_t column = 0; column < count; ++column) {
				minimum = std::min(minimum, matrix[row][column]);
				maximum = std::max(maximum, matrix[row][column]);
			}
		}
		if (maximum <= minimum) {
			maximum = minimum + 1.0;
		}
		return {minimum, maximum};
	};

	const auto metric_extent = extent_of(domain.metric_distances);
	const auto vector_extent = extent_of(domain.vector_distances);

	std::ostringstream svg;
	svg << "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"" << format_coordinate(width)
		<< "\" height=\"460\" viewBox=\"0 0 " << format_coordinate(width)
		<< " 460\" role=\"img\" aria-labelledby=\"title desc\">\n";
	svg << "  <title id=\"title\">" << svg_escape(domain.title) << " pair-value heatmaps</title>\n";
	svg << "  <desc id=\"desc\">Side-by-side pair-value grids over the same " << count
		<< " " << svg_escape(domain.title)
		<< " records: the finite metric space on the left and the padded-vector baseline on the right.</desc>\n";
	svg << "  <rect width=\"" << format_coordinate(width) << "\" height=\"460\" fill=\"#f8fafc\"/>\n";
	svg << text_element(54, 44, 23, "#172033", domain.title + " pair-value heatmaps", "700");
	svg << text_element(54, 70, 13, "#526070",
						 "Record-to-record metric values, normalized within each grid. UCR 2021, CC BY 4.0.");

	auto emit_grid = [&](double origin_x, const std::vector<std::vector<double>> &matrix,
						 const std::pair<double, double> &extent, const std::string &heading) {
		svg << text_element(origin_x, grid_y - 20.0, 13, "#172033", heading, "700");
		for (std::size_t column = 0; column < count; ++column) {
			const double x = origin_x + static_cast<double>(column) * cell;
			svg << "  <rect x=\"" << format_coordinate(x) << "\" y=\"" << format_coordinate(role_bar_y) << "\" width=\""
				<< format_coordinate(cell) << "\" height=\"6\" fill=\"" << role_color(domain.labels[column]) << "\"/>\n";
		}
		for (std::size_t row = 0; row < count; ++row) {
			for (std::size_t column = 0; column < count; ++column) {
				const double x = origin_x + static_cast<double>(column) * cell;
				const double y = grid_y + static_cast<double>(row) * cell;
				const double normalized = (matrix[row][column] - extent.first) / (extent.second - extent.first);
				svg << "  <rect x=\"" << format_coordinate(x) << "\" y=\"" << format_coordinate(y) << "\" width=\""
					<< format_coordinate(cell) << "\" height=\"" << format_coordinate(cell) << "\" fill=\""
					<< heat_color(normalized) << "\"/>\n";
			}
		}
		svg << "  <rect x=\"" << format_coordinate(origin_x) << "\" y=\"" << format_coordinate(grid_y) << "\" width=\""
			<< format_coordinate(grid_size) << "\" height=\"" << format_coordinate(grid_size)
			<< "\" fill=\"none\" stroke=\"#cbd5e1\"/>\n";
		const double legend_y = grid_y + grid_size + 22.0;
		svg << text_element(origin_x, legend_y, 11, "#526070",
							"metric value " + format_display(extent.first) + " to " + format_display(extent.second));
	};

	emit_grid(left_x, domain.metric_distances, metric_extent, "Finite metric space");
	emit_grid(right_x, domain.vector_distances, vector_extent, "Padded-vector baseline");

	const double legend_y = grid_y + grid_size + 22.0;
	svg << text_element(54, legend_y + 26.0, 11, "#475569",
						"Top color strip marks each record role; darker cells are smaller metric values.");
	svg << text_element(54, legend_y + 44.0, 11, "#475569",
						"The baseline grid flattens the role structure the alignment metric keeps.");
	svg << "</svg>\n";
	return svg.str();
}

auto make_hero_overview_svg(const std::vector<DomainEvidence> &domains) -> std::string
{
	std::ostringstream svg;
	svg << "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"920\" height=\"420\" viewBox=\"0 0 920 420\" role=\"img\" "
		   "aria-labelledby=\"title desc\">\n";
	svg << "  <title id=\"title\">Process-curve finite metric space overview</title>\n";
	svg << "  <desc id=\"desc\">Two licensed process-curve domains. Each panel shows source process windows by role as "
		   "solid curves and a downsampled query as a dashed curve.</desc>\n";
	svg << "  <rect width=\"920\" height=\"420\" fill=\"#f8fafc\"/>\n";
	svg << text_element(54, 44, 23, "#172033", "Process curves as a finite metric space", "700");
	svg << text_element(54, 70, 13, "#526070",
						 "Records are 36-sample windows; queries are even-downsampled 18-sample windows.");

	// Preferred roles span both domain vocabularies; only the first match per
	// role that is present in a domain is drawn, so each panel shows four phases.
	const std::vector<std::string> role_order{"normal", "pre_anomaly", "anomaly", "anomaly_start", "recovery"};
	const double panel_width = 396.0;
	const double panel_gap = 28.0;
	const double panel_x0 = 54.0;
	const double panel_y = 96.0;
	const double panel_height = 252.0;

	for (std::size_t domain_index = 0; domain_index < domains.size(); ++domain_index) {
		const auto &domain = domains[domain_index];
		const double panel_x = panel_x0 + static_cast<double>(domain_index) * (panel_width + panel_gap);
		svg << "  <rect x=\"" << format_coordinate(panel_x) << "\" y=\"" << format_coordinate(panel_y) << "\" width=\""
			<< format_coordinate(panel_width) << "\" height=\"" << format_coordinate(panel_height)
			<< "\" rx=\"8\" fill=\"#ffffff\" stroke=\"#cbd5e1\"/>\n";
		svg << text_element(panel_x + 18, panel_y + 28, 15, "#172033", domain.title, "700");

		const PlotBox box{panel_x + 18.0, panel_y + 48.0, panel_width - 36.0, panel_height - 96.0};

		// Collect one source window per role plus the first query for the
		// raw-vs-query contrast, all drawn on a shared vertical scale.
		std::vector<std::pair<std::string, const std::vector<double> *>> selected;
		for (const auto &role : role_order) {
			for (std::size_t index = 0; index < domain.labels.size(); ++index) {
				if (domain.labels[index] == role) {
					selected.emplace_back(role, &domain.records[index]);
					break;
				}
			}
		}
		const auto &query = domain.queries.front();

		std::vector<const std::vector<double> *> extent_inputs;
		for (const auto &entry : selected) {
			extent_inputs.push_back(entry.second);
		}
		extent_inputs.push_back(&query.values);
		const auto extent = curve_extent(extent_inputs);

		for (const auto &entry : selected) {
			svg << "  <polyline fill=\"none\" stroke=\"" << role_color(entry.first)
				<< "\" stroke-width=\"2\" stroke-linejoin=\"round\" stroke-linecap=\"round\" opacity=\"0.92\" points=\""
				<< curve_points(*entry.second, box, extent.first, extent.second) << "\"/>\n";
		}
		svg << "  <polyline fill=\"none\" stroke=\"#0f172a\" stroke-width=\"2\" stroke-dasharray=\"5 4\" "
			   "stroke-linejoin=\"round\" stroke-linecap=\"round\" points=\""
			<< curve_points(query.values, box, extent.first, extent.second) << "\"/>\n";

		const double legend_y = panel_y + panel_height - 18.0;
		svg << text_element(panel_x + 18, legend_y, 11, "#526070", "solid: source roles    dashed: query");
	}

	const std::vector<std::pair<std::string, std::string>> legend{
		{"normal", "#2563eb"}, {"pre_anomaly", "#16a34a"}, {"anomaly", "#dc2626"}, {"recovery", "#7c3aed"}};
	double legend_x = 54.0;
	const double legend_y = 380.0;
	for (const auto &entry : legend) {
		svg << "  <line x1=\"" << format_coordinate(legend_x) << "\" y1=\"" << format_coordinate(legend_y - 4.0)
			<< "\" x2=\"" << format_coordinate(legend_x + 26.0) << "\" y2=\"" << format_coordinate(legend_y - 4.0)
			<< "\" stroke=\"" << entry.second << "\" stroke-width=\"3\"/>\n";
		svg << text_element(legend_x + 34.0, legend_y, 11, "#334155", entry.first);
		legend_x += 34.0 + static_cast<double>(entry.first.size()) * 7.0 + 30.0;
	}
	svg << text_element(54, 406, 11, "#64748b",
						"Source: UCR Time Series Anomaly Detection datasets (2021), Figshare, CC BY 4.0.");
	svg << "</svg>\n";
	return svg.str();
}

auto make_outcome_strip_svg(std::size_t total_records, std::size_t total_queries, std::size_t metric_matches,
							std::size_t vector_misses, std::size_t total_dense_evaluations) -> std::string
{
	std::ostringstream svg;
	svg << "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"920\" height=\"150\" viewBox=\"0 0 920 150\" role=\"img\" "
		   "aria-labelledby=\"title desc\">\n";
	svg << "  <title id=\"title\">Process-curve evidence strip</title>\n";
	svg << "  <desc id=\"desc\">Evidence attached to the licensed dataset: metric-space matches, vector-baseline "
		   "misses, record and query counts, and dense metric evaluations.</desc>\n";
	svg << "  <rect width=\"920\" height=\"150\" fill=\"#f8fafc\"/>\n";
	svg << "  <rect x=\"24\" y=\"24\" width=\"872\" height=\"102\" rx=\"8\" fill=\"#ffffff\" stroke=\"#cbd5e1\"/>\n";
	svg << "  <rect x=\"24\" y=\"24\" width=\"6\" height=\"102\" rx=\"3\" fill=\"#007a9a\"/>\n";
	svg << text_element(48, 52, 12, "#526070", "Dataset evidence");
	svg << text_element(48, 76, 15, "#172033",
						"UCR Time Series Anomaly Detection datasets (2021)", "700");
	svg << text_element(48, 98, 12, "#526070", "Figshare, CC BY 4.0");
	svg << text_element(48, 118, 11, "#64748b",
						"Reproduced by engine_process_curve_external_gallery (native C++ evidence).");

	svg << "  <line x1=\"470\" y1=\"44\" x2=\"470\" y2=\"106\" stroke=\"#e2e8f0\"/>\n";

	const std::string matches = std::to_string(metric_matches) + "/" + std::to_string(total_queries);
	const std::string misses = std::to_string(vector_misses) + "/" + std::to_string(total_queries);

	svg << text_element(498, 60, 12, "#166534", "Metric-space matches");
	svg << text_element(498, 88, 26, "#166534", matches, "700");
	svg << text_element(700, 60, 12, "#991b1b", "Vector-baseline misses");
	svg << text_element(700, 88, 26, "#991b1b", misses, "700");

	const std::string ledger = std::to_string(total_records) + " records  \xc2\xb7  " + std::to_string(total_queries) +
							   " queries  \xc2\xb7  " + std::to_string(total_dense_evaluations) + " dense evaluations";
	svg << text_element(498, 116, 12, "#475569", ledger);
	svg << "</svg>\n";
	return svg.str();
}

void write_summary_csv(const std::filesystem::path &dir, const std::vector<DomainEvidence> &domains)
{
	std::ostringstream out;
	out << "domain,source,license,records,queries,metric_correct,metric_total,vector_mismatches,vector_total,"
		   "average_metric_margin,dense_evaluations\n";
	for (const auto &domain : domains) {
		out << csv_field(domain.name) << ',' << csv_field(kSource) << ',' << csv_field(kLicense) << ','
			<< domain.records.size() << ',' << domain.queries.size() << ',' << domain.metric_correct << ','
			<< domain.queries.size() << ',' << domain.vector_mismatches << ',' << domain.queries.size() << ','
			<< format_number(domain.average_metric_margin) << ',' << domain.dense_evaluations << '\n';
	}
	write_file(dir / "summary.csv", out.str());
}

void write_records_csv(const std::filesystem::path &dir, const std::vector<DomainEvidence> &domains)
{
	std::ostringstream out;
	out << "domain,record_index,record_id,source_label,sample_count,values\n";
	for (const auto &domain : domains) {
		for (std::size_t index = 0; index < domain.records.size(); ++index) {
			out << csv_field(domain.name) << ',' << index << ',' << csv_field(domain.ids[index]) << ','
				<< csv_field(domain.labels[index]) << ',' << domain.records[index].size() << ','
				<< csv_field(format_values(domain.records[index])) << '\n';
		}
	}
	write_file(dir / "records.csv", out.str());
}

void write_queries_csv(const std::filesystem::path &dir, const std::vector<DomainEvidence> &domains)
{
	std::ostringstream out;
	out << "domain,query_index,query_id,expected_label,metric_winner_id,metric_winner_label,metric_distance,"
		   "vector_winner_id,vector_winner_label,vector_distance,metric_margin,metric_correct,vector_mismatch,"
		   "query_values\n";
	for (const auto &domain : domains) {
		for (std::size_t index = 0; index < domain.queries.size(); ++index) {
			const auto &query = domain.queries[index];
			out << csv_field(domain.name) << ',' << index << ',' << csv_field(query.id) << ','
				<< csv_field(query.expected_label) << ',' << csv_field(query.metric_winner_id) << ','
				<< csv_field(query.metric_winner_label) << ',' << format_number(query.metric_distance) << ','
				<< csv_field(query.vector_winner_id) << ',' << csv_field(query.vector_winner_label) << ','
				<< format_number(query.vector_distance) << ',' << format_number(query.metric_margin) << ','
				<< (query.metric_correct ? 1 : 0) << ',' << (query.vector_mismatch ? 1 : 0) << ','
				<< csv_field(format_values(query.values)) << '\n';
		}
	}
	write_file(dir / "queries.csv", out.str());
}

void write_distances_csv(const std::filesystem::path &dir, const std::vector<DomainEvidence> &domains)
{
	std::ostringstream out;
	out << "domain,representation,row_index,row_id,column_index,column_id,distance\n";
	for (const auto &domain : domains) {
		const std::size_t count = domain.records.size();
		const std::vector<std::pair<std::string, const std::vector<std::vector<double>> *>> representations{
			{"metric_space", &domain.metric_distances}, {"padded_vector_baseline", &domain.vector_distances}};
		for (const auto &representation : representations) {
			for (std::size_t row = 0; row < count; ++row) {
				for (std::size_t column = 0; column < count; ++column) {
					out << csv_field(domain.name) << ',' << csv_field(representation.first) << ',' << row << ','
						<< csv_field(domain.ids[row]) << ',' << column << ',' << csv_field(domain.ids[column]) << ','
						<< format_number((*representation.second)[row][column]) << '\n';
				}
			}
		}
	}
	write_file(dir / "distances.csv", out.str());
}

void write_query_winners_csv(const std::filesystem::path &dir, const std::vector<DomainEvidence> &domains)
{
	std::ostringstream out;
	out << "domain,query_id,curve_role,curve_id,curve_label,distance,values\n";
	for (const auto &domain : domains) {
		for (const auto &query : domain.queries) {
			out << csv_field(domain.name) << ',' << csv_field(query.id) << ',' << "query" << ','
				<< csv_field(query.id) << ',' << csv_field(query.expected_label) << ',' << format_number(0.0) << ','
				<< csv_field(format_values(query.values)) << '\n';
			out << csv_field(domain.name) << ',' << csv_field(query.id) << ',' << "metric_winner" << ','
				<< csv_field(query.metric_winner_id) << ',' << csv_field(query.metric_winner_label) << ','
				<< format_number(query.metric_distance) << ','
				<< csv_field(format_values(domain.records[query.metric_winner_index])) << '\n';
			out << csv_field(domain.name) << ',' << csv_field(query.id) << ',' << "vector_baseline_winner" << ','
				<< csv_field(query.vector_winner_id) << ',' << csv_field(query.vector_winner_label) << ','
				<< format_number(query.vector_distance) << ','
				<< csv_field(format_values(domain.records[query.vector_winner_index])) << '\n';
		}
	}
	write_file(dir / "query-winners.csv", out.str());
}

auto landmark3_coordinates(const DomainEvidence &domain, std::size_t index) -> std::vector<double>
{
	const std::size_t count = domain.records.size();
	if (count == 0) {
		return {0.0, 0.0, 0.0};
	}

	const std::size_t anchors[3] = {0, count / 2, count - 1};
	std::vector<double> coordinates;
	coordinates.reserve(3);
	for (const std::size_t anchor : anchors) {
		double maximum = 0.0;
		for (std::size_t record = 0; record < count; ++record) {
			maximum = std::max(maximum, domain.metric_distances[anchor][record]);
		}
		const double normalized = maximum > 0.0 ? domain.metric_distances[anchor][index] / maximum : 0.0;
		coordinates.push_back(normalized - 0.5);
	}
	return coordinates;
}

auto landmark2_coordinates(const DomainEvidence &domain, std::size_t index) -> std::vector<double>
{
	auto coordinates = landmark3_coordinates(domain, index);
	if (coordinates.size() < 3) {
		return {0.0, 0.0, 0.0};
	}
	return {coordinates[0], 0.0, coordinates[1]};
}

void write_evidence_json(const std::filesystem::path &path, const std::vector<DomainEvidence> &domains,
						 std::size_t total_records, std::size_t total_queries, std::size_t total_dense_evaluations)
{
	std::size_t metric_matches = 0;
	std::size_t vector_misses = 0;
	for (const auto &domain : domains) {
		metric_matches += domain.metric_correct;
		vector_misses += domain.vector_mismatches;
	}

	std::ostringstream out;
	out << "{\n";
	out << "  \"schema\": \"metric.evidence.v1\",\n";
	out << "  \"provenance\": {\n";
	out << "    \"source\": " << json_string(kSource) << ",\n";
	out << "    \"license\": " << json_string(kLicense) << ",\n";
	out << "    \"generator\": \"examples/engine/process_curve_external_gallery.cpp\",\n";
	out << "    \"records\": " << total_records << ",\n";
	out << "    \"queries\": " << total_queries << ",\n";
	out << "    \"dense_evaluations\": " << total_dense_evaluations << ",\n";
	out << "    \"metric_matches\": " << metric_matches << ",\n";
	out << "    \"vector_baseline_misses\": " << vector_misses << "\n";
	out << "  },\n";

	out << "  \"spaces\": [\n";
	bool first_space = true;
	for (const auto &domain : domains) {
		const std::pair<const char *, const char *> spaces[] = {
			{"metric_space", "aligned_curve_distance"},
			{"padded_vector_baseline", "pointwise_padded_euclidean"},
		};
		for (const auto &space : spaces) {
			if (!first_space) {
				out << ",\n";
			}
			first_space = false;
			out << "    {\"id\": " << json_string(domain.name + ":" + space.first)
				<< ", \"domain\": " << json_string(domain.name)
				<< ", \"title\": " << json_string(domain.title)
				<< ", \"representation\": " << json_string(space.first)
				<< ", \"metric\": " << json_string(space.second)
				<< ", \"record_count\": " << domain.records.size() << "}";
		}
	}
	out << "\n  ],\n";

	out << "  \"records\": [\n";
	bool first_record = true;
	for (const auto &domain : domains) {
		for (std::size_t index = 0; index < domain.records.size(); ++index) {
			if (!first_record) {
				out << ",\n";
			}
			first_record = false;
			out << "    {\"domain\": " << json_string(domain.name)
				<< ", \"index\": " << index
				<< ", \"id\": " << json_string(domain.ids[index])
				<< ", \"label\": " << json_string(domain.labels[index])
				<< ", \"record_type\": \"process_curve\""
				<< ", \"values\": " << json_values(domain.records[index]) << "}";
		}
	}
	out << "\n  ],\n";

	out << "  \"queries\": [\n";
	bool first_query = true;
	for (const auto &domain : domains) {
		for (std::size_t index = 0; index < domain.queries.size(); ++index) {
			const auto &query = domain.queries[index];
			if (!first_query) {
				out << ",\n";
			}
			first_query = false;
			out << "    {\"domain\": " << json_string(domain.name)
				<< ", \"index\": " << index
				<< ", \"id\": " << json_string(query.id)
				<< ", \"expected_label\": " << json_string(query.expected_label)
				<< ", \"values\": " << json_values(query.values) << "}";
		}
	}
	out << "\n  ],\n";

	out << "  \"winners\": [\n";
	bool first_winner = true;
	for (const auto &domain : domains) {
		for (const auto &query : domain.queries) {
			const std::pair<const char *, const QueryEvidence *> winner_rows[] = {
				{"metric_space", &query},
				{"padded_vector_baseline", &query},
			};
			for (const auto &winner : winner_rows) {
				if (!first_winner) {
					out << ",\n";
				}
				first_winner = false;
				const bool metric_space = std::string(winner.first) == "metric_space";
				out << "    {\"domain\": " << json_string(domain.name)
					<< ", \"query_id\": " << json_string(query.id)
					<< ", \"space\": " << json_string(winner.first)
					<< ", \"winner_index\": " << (metric_space ? query.metric_winner_index : query.vector_winner_index)
					<< ", \"winner_id\": "
					<< json_string(metric_space ? query.metric_winner_id : query.vector_winner_id)
					<< ", \"winner_label\": "
					<< json_string(metric_space ? query.metric_winner_label : query.vector_winner_label)
					<< ", \"value\": " << format_number(metric_space ? query.metric_distance : query.vector_distance)
					<< ", \"correct\": " << json_bool(metric_space ? query.metric_correct : !query.vector_mismatch)
					<< ", \"metric_margin\": " << format_number(query.metric_margin) << "}";
			}
		}
	}
	out << "\n  ],\n";

	out << "  \"pair_values\": [\n";
	bool first_pair = true;
	for (const auto &domain : domains) {
		const std::pair<const char *, const std::vector<std::vector<double>> *> matrices[] = {
			{"metric_space", &domain.metric_distances},
			{"padded_vector_baseline", &domain.vector_distances},
		};
		for (const auto &matrix : matrices) {
			for (std::size_t row = 0; row < domain.records.size(); ++row) {
				for (std::size_t column = 0; column < domain.records.size(); ++column) {
					if (!first_pair) {
						out << ",\n";
					}
					first_pair = false;
					out << "    {\"domain\": " << json_string(domain.name)
						<< ", \"space\": " << json_string(matrix.first)
						<< ", \"row\": " << row
						<< ", \"row_id\": " << json_string(domain.ids[row])
						<< ", \"column\": " << column
						<< ", \"column_id\": " << json_string(domain.ids[column])
						<< ", \"value\": " << format_number((*matrix.second)[row][column]) << "}";
				}
			}
		}
	}
	out << "\n  ],\n";

	out << "  \"representatives\": [],\n";
	out << "  \"graphs\": [],\n";
	out << "  \"coordinates\": [\n";
	bool first_coordinate = true;
	for (const auto &domain : domains) {
		for (std::size_t index = 0; index < domain.records.size(); ++index) {
			const std::pair<const char *, std::vector<double>> coordinate_rows[] = {
				{"metric_space_landmark3", landmark3_coordinates(domain, index)},
				{"metric_space_landmark2", landmark2_coordinates(domain, index)},
			};
			for (const auto &row : coordinate_rows) {
				if (!first_coordinate) {
					out << ",\n";
				}
				first_coordinate = false;
				out << "    {\"domain\": " << json_string(domain.name)
					<< ", \"space\": " << json_string(row.first)
					<< ", \"record_id\": " << json_string(domain.ids[index])
					<< ", \"record_index\": " << index
					<< ", \"label\": " << json_string(domain.labels[index])
					<< ", \"values\": " << json_values(row.second) << "}";
			}
		}
	}
	out << "\n  ],\n";
	out << "  \"diagnostics\": [\n";
	bool first_diagnostic = true;
	for (const auto &domain : domains) {
		if (!first_diagnostic) {
			out << ",\n";
		}
		first_diagnostic = false;
		out << "    {\"domain\": " << json_string(domain.name)
			<< ", \"metric_correct\": " << domain.metric_correct
			<< ", \"metric_total\": " << domain.queries.size()
			<< ", \"vector_mismatches\": " << domain.vector_mismatches
			<< ", \"vector_total\": " << domain.queries.size()
			<< ", \"average_metric_margin\": " << format_number(domain.average_metric_margin)
			<< ", \"dense_evaluations\": " << domain.dense_evaluations << "}";
	}
	out << "\n  ]\n";
	out << "}\n";

	write_file(path, out.str());
}

void export_evidence(const std::string &export_dir, const std::vector<DomainEvidence> &domains,
					 std::size_t total_records, std::size_t total_queries, std::size_t total_dense_evaluations)
{
	const std::filesystem::path dir(export_dir);
	std::filesystem::create_directories(dir);

	write_summary_csv(dir, domains);
	write_records_csv(dir, domains);
	write_queries_csv(dir, domains);
	write_distances_csv(dir, domains);
	write_query_winners_csv(dir, domains);
	write_evidence_json(dir / "evidence.json", domains, total_records, total_queries, total_dense_evaluations);

	std::size_t metric_matches = 0;
	std::size_t vector_misses = 0;
	for (const auto &domain : domains) {
		metric_matches += domain.metric_correct;
		vector_misses += domain.vector_mismatches;
	}

	write_file(dir / "hero-overview.svg", make_hero_overview_svg(domains));
	for (const auto &domain : domains) {
		write_file(dir / (domain.name == "power_demand" ? "power-demand-query-comparison.svg"
														: "internal-bleeding-query-comparison.svg"),
				   make_query_comparison_svg(domain));
		write_file(dir / (domain.name == "power_demand" ? "power-demand-distance-heatmap.svg"
														: "internal-bleeding-distance-heatmap.svg"),
				   make_distance_heatmap_svg(domain));
	}
	write_file(dir / "outcome-strip.svg",
			   make_outcome_strip_svg(total_records, total_queries, metric_matches, vector_misses,
									  total_dense_evaluations));

	std::cout << "process external export dir = " << dir.string() << "\n";
}

auto parse_flag_value(int argc, char **argv, const std::string &flag) -> std::string
{
	for (int index = 1; index < argc; ++index) {
		const std::string argument = argv[index];
		if (argument == flag) {
			if (index + 1 < argc) {
				return argv[index + 1];
			}
			throw std::runtime_error(flag + " requires a path");
		}
		if (argument.rfind(flag + "=", 0) == 0) {
			return argument.substr(flag.size() + 1);
		}
	}
	return std::string{};
}

} // namespace

int main(int argc, char **argv)
{
	const auto export_dir = parse_flag_value(argc, argv, "--export-dir");
	const auto export_json = parse_flag_value(argc, argv, "--export-json");

	const std::vector<DomainBenchmarkInput> domains{
		{"power_demand", "Power Demand", METRIC_PROCESS_CURVE_POWER_DEMAND_CSV, 24, 8, 300.0},
		{"internal_bleeding", "Internal Bleeding", METRIC_PROCESS_CURVE_INTERNAL_BLEEDING_CSV, 24, 8, 150.0},
	};

	std::vector<DomainEvidence> evidence;
	std::size_t total_records = 0;
	std::size_t total_queries = 0;
	std::size_t total_dense_evaluations = 0;
	for (const auto &domain : domains) {
		auto domain_evidence = run_domain_benchmark(domain);
		total_records += domain_evidence.records.size();
		total_queries += domain_evidence.queries.size();
		total_dense_evaluations += domain_evidence.dense_evaluations;
		evidence.push_back(std::move(domain_evidence));
	}

	print_text_report(evidence, total_records, total_queries, total_dense_evaluations);

	if (!export_dir.empty()) {
		export_evidence(export_dir, evidence, total_records, total_queries, total_dense_evaluations);
	}
	if (!export_json.empty()) {
		write_evidence_json(export_json, evidence, total_records, total_queries, total_dense_evaluations);
		std::cout << "process external export json = " << export_json << "\n";
	}

	return 0;
}
