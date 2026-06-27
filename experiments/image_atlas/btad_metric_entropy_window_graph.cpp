// Native finite-metric-space entropy window visual exporter.
//
// This exporter intentionally has no Python/Torch/NumPy preprocessing path. It
// reads BTAD image records in C++, constructs mtrc::MetricSpace records, maps
// them with mtrc::embed(..., 3), and computes entropy with
// mtrc::stats::properties::entropy(...). The extra local terms are exported only
// as visual evidence for the same kpN estimator and are checked against the API
// entropy value frame by frame.

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include <metric/engine.hpp>
#include <metric/metric/catalog.hpp>
#include <metric/numeric.hpp>

namespace {

constexpr const char *kDefaultBtadRoot = "/Volumes/Models/metric-demos/extracted/BTAD/BTech_Dataset_transformed/03";
constexpr const char *kDefaultExportDir = "/Volumes/Models/metric-demos/outputs/visa-entropy-window-graph";
constexpr double kCovarianceOffset = 1.0e-8;

using Vector = std::vector<double>;

struct Options {
	std::filesystem::path btad_root{kDefaultBtadRoot};
	std::filesystem::path export_dir{kDefaultExportDir};
	std::size_t sample_width{24};
	std::size_t sample_height{18};
	std::size_t window_size{1000};
	std::size_t step_size{100};
	std::size_t entropy_neighbors{7};
	std::size_t approximation_order{20};
	std::size_t graph_neighbors{5};
};

struct RecordMeta {
	std::string id;
	std::size_t index{};
	std::string source_id;
	std::string category;
	std::string label;
	std::string split;
	std::filesystem::path path;
	std::size_t width{};
	std::size_t height{};
	Vector source_record;
	Vector embedded;
};

struct LocalTerm {
	std::size_t global_index{};
	double eps{};
	double log_g{};
	double density{};
	double contribution{};
	bool valid{};
	std::vector<std::size_t> entropy_neighbors;
};

struct Edge {
	std::size_t source{};
	std::size_t target{};
	double distance{};
};

struct Frame {
	std::size_t index{};
	std::size_t offset{};
	std::vector<std::size_t> active;
	std::vector<std::size_t> incoming;
	std::vector<std::size_t> outgoing;
	std::vector<LocalTerm> local_terms;
	std::vector<Edge> edges;
	double entropy_api{};
	double entropy_visual_reconstruction{};
	double entropy_delta{};
	std::string entropy_status;
	std::size_t valid_terms{};
	std::size_t effective_k{};
	std::size_t effective_p{};
};

auto usage() -> std::string
{
	return R"(visa_entropy_window_graph

Build a native Metric finite-space entropy window visual from BTAD/03.

Options:
  --btad-root <path>            BTAD object root, default /Volumes/Models/metric-demos/extracted/BTAD/BTech_Dataset_transformed/03
  --export-dir <path>           Output dir, default /Volumes/Models/metric-demos/outputs/visa-entropy-window-graph
  --sample-width <n>            Raw image vector width, default 24
  --sample-height <n>           Raw image vector height, default 18
  --window-size <n>             Active records per frame, default 1000
  --step-size <n>               Records leaving/entering per frame, default 100
  --entropy-neighbors <k>       kpN entropy k, default 7
  --approximation-order <p>     kpN entropy p, default 20
  --graph-neighbors <k>         Metric kNN graph degree, default 5
)";
}

auto parse_options(int argc, char **argv) -> Options
{
	Options options;
	for (int i = 1; i < argc; ++i) {
		const std::string arg = argv[i];
		auto value = [&](const char *name) -> std::string {
			if (i + 1 >= argc) {
				throw std::invalid_argument(std::string(name) + " requires a value");
			}
			return argv[++i];
		};
		if (arg == "--btad-root") {
			options.btad_root = value("--btad-root");
		} else if (arg == "--export-dir") {
			options.export_dir = value("--export-dir");
		} else if (arg == "--sample-width") {
			options.sample_width = static_cast<std::size_t>(std::stoull(value("--sample-width")));
		} else if (arg == "--sample-height") {
			options.sample_height = static_cast<std::size_t>(std::stoull(value("--sample-height")));
		} else if (arg == "--window-size") {
			options.window_size = static_cast<std::size_t>(std::stoull(value("--window-size")));
		} else if (arg == "--step-size") {
			options.step_size = static_cast<std::size_t>(std::stoull(value("--step-size")));
		} else if (arg == "--entropy-neighbors") {
			options.entropy_neighbors = static_cast<std::size_t>(std::stoull(value("--entropy-neighbors")));
		} else if (arg == "--approximation-order") {
			options.approximation_order = static_cast<std::size_t>(std::stoull(value("--approximation-order")));
		} else if (arg == "--graph-neighbors") {
			options.graph_neighbors = static_cast<std::size_t>(std::stoull(value("--graph-neighbors")));
		} else if (arg == "--help" || arg == "-h") {
			std::cout << usage();
			std::exit(0);
		} else {
			throw std::invalid_argument("unknown option: " + arg);
		}
	}
	if (options.sample_width == 0 || options.sample_height == 0) {
		throw std::invalid_argument("image sample dimensions must be positive");
	}
	if (options.window_size < 4 || options.step_size == 0) {
		throw std::invalid_argument("window_size must be >= 4 and step_size must be positive");
	}
	return options;
}

auto lower_ext(const std::filesystem::path &path) -> std::string
{
	auto ext = path.extension().string();
	std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char ch) {
		return static_cast<char>(std::tolower(ch));
	});
	return ext;
}

auto is_image_file(const std::filesystem::path &path) -> bool
{
	const auto ext = lower_ext(path);
	return ext == ".bmp";
}

auto path_has_part(const std::filesystem::path &path, const std::string &part) -> bool
{
	for (const auto &segment : path) {
		if (segment == part) {
			return true;
		}
	}
	return false;
}

auto read_u16(std::ifstream &in) -> std::uint16_t
{
	std::array<unsigned char, 2> bytes{};
	in.read(reinterpret_cast<char *>(bytes.data()), bytes.size());
	if (!in) {
		throw std::runtime_error("unexpected EOF in BMP header");
	}
	return static_cast<std::uint16_t>(bytes[0] | (bytes[1] << 8));
}

auto read_u32(std::ifstream &in) -> std::uint32_t
{
	std::array<unsigned char, 4> bytes{};
	in.read(reinterpret_cast<char *>(bytes.data()), bytes.size());
	if (!in) {
		throw std::runtime_error("unexpected EOF in BMP header");
	}
	return static_cast<std::uint32_t>(bytes[0] | (bytes[1] << 8) | (bytes[2] << 16) | (bytes[3] << 24));
}

auto read_i32(std::ifstream &in) -> std::int32_t
{
	return static_cast<std::int32_t>(read_u32(in));
}

struct LoadedBmp {
	std::size_t width{};
	std::size_t height{};
	std::vector<unsigned char> rgb;
};

auto read_bmp24(const std::filesystem::path &path) -> LoadedBmp
{
	std::ifstream in(path, std::ios::binary);
	if (!in) {
		throw std::runtime_error("failed to open BMP: " + path.string());
	}
	if (read_u16(in) != 0x4D42) {
		throw std::runtime_error("not a BMP file: " + path.string());
	}
	(void)read_u32(in);
	(void)read_u16(in);
	(void)read_u16(in);
	const auto pixel_offset = read_u32(in);
	const auto dib_size = read_u32(in);
	if (dib_size < 40) {
		throw std::runtime_error("unsupported BMP DIB header: " + path.string());
	}
	const auto width_signed = read_i32(in);
	const auto height_signed = read_i32(in);
	const auto planes = read_u16(in);
	const auto bits = read_u16(in);
	const auto compression = read_u32(in);
	if (planes != 1 || bits != 24 || compression != 0 || width_signed <= 0 || height_signed == 0) {
		throw std::runtime_error("only uncompressed 24-bit BMP files are supported: " + path.string());
	}
	const auto width = static_cast<std::size_t>(width_signed);
	const auto height = static_cast<std::size_t>(std::abs(height_signed));
	const bool bottom_up = height_signed > 0;
	const auto row_stride = ((width * 3 + 3) / 4) * 4;
	LoadedBmp image{width, height, std::vector<unsigned char>(width * height * 3, 0)};
	std::vector<unsigned char> row(row_stride);
	in.seekg(static_cast<std::streamoff>(pixel_offset), std::ios::beg);
	for (std::size_t file_y = 0; file_y < height; ++file_y) {
		in.read(reinterpret_cast<char *>(row.data()), static_cast<std::streamsize>(row.size()));
		if (!in) {
			throw std::runtime_error("truncated BMP pixel data: " + path.string());
		}
		const auto y = bottom_up ? height - 1 - file_y : file_y;
		for (std::size_t x = 0; x < width; ++x) {
			const auto src = x * 3;
			const auto dst = (y * width + x) * 3;
			image.rgb[dst + 0] = row[src + 2];
			image.rgb[dst + 1] = row[src + 1];
			image.rgb[dst + 2] = row[src + 0];
		}
	}
	return image;
}

auto downsample_luma(const LoadedBmp &image, std::size_t out_width, std::size_t out_height) -> Vector
{
	Vector out;
	out.reserve(out_width * out_height);
	for (std::size_t y = 0; y < out_height; ++y) {
		const auto src_y = std::min(image.height - 1, static_cast<std::size_t>(
			(static_cast<double>(y) + 0.5) * static_cast<double>(image.height) / static_cast<double>(out_height)));
		for (std::size_t x = 0; x < out_width; ++x) {
			const auto src_x = std::min(image.width - 1, static_cast<std::size_t>(
				(static_cast<double>(x) + 0.5) * static_cast<double>(image.width) / static_cast<double>(out_width)));
			const auto offset = (src_y * image.width + src_x) * 3;
			const auto r = static_cast<double>(image.rgb[offset + 0]);
			const auto g = static_cast<double>(image.rgb[offset + 1]);
			const auto b = static_cast<double>(image.rgb[offset + 2]);
			out.push_back((0.2126 * r + 0.7152 * g + 0.0722 * b) / 255.0);
		}
	}
	return out;
}

auto json_escape(const std::string &value) -> std::string
{
	std::ostringstream out;
	for (const auto ch : value) {
		switch (ch) {
		case '"':
			out << "\\\"";
			break;
		case '\\':
			out << "\\\\";
			break;
		case '\n':
			out << "\\n";
			break;
		case '\r':
			out << "\\r";
			break;
		case '\t':
			out << "\\t";
			break;
		default:
			out << ch;
			break;
		}
	}
	return out.str();
}

auto collect_records(const Options &options) -> std::vector<RecordMeta>
{
	if (!std::filesystem::exists(options.btad_root)) {
		throw std::runtime_error("missing BTAD root: " + options.btad_root.string());
	}
	std::vector<std::filesystem::path> files;
	for (const auto &entry : std::filesystem::recursive_directory_iterator(options.btad_root)) {
		if (!entry.is_regular_file()) {
			continue;
		}
		const auto path = entry.path();
		if (path_has_part(path, "ground_truth") || !is_image_file(path)) {
			continue;
		}
		files.push_back(path);
	}
	std::sort(files.begin(), files.end());
	if (files.empty()) {
		throw std::runtime_error("no BTAD BMP records found in " + options.btad_root.string());
	}

	std::vector<RecordMeta> records;
	records.reserve(files.size());
	for (std::size_t index = 0; index < files.size(); ++index) {
		const auto &path = files[index];
		const auto rel = std::filesystem::relative(path, options.btad_root);
		const auto rel_string = rel.generic_string();
		const auto split = path_has_part(rel, "train") ? std::string("train") : std::string("test");
		const auto label = path_has_part(rel, "ko") ? std::string("ko") : std::string("ok");
		auto image = read_bmp24(path);
		RecordMeta record;
		record.id = "img-" + [&] {
			std::ostringstream id;
			id << std::setw(5) << std::setfill('0') << index;
			return id.str();
		}();
		record.index = index;
		record.source_id = rel_string;
		record.category = "BTAD/03";
		record.label = label;
		record.split = split;
		record.path = path;
		record.width = image.width;
		record.height = image.height;
		record.source_record = downsample_luma(image, options.sample_width, options.sample_height);
		records.push_back(std::move(record));
	}
	return records;
}

auto euclidean(const Vector &lhs, const Vector &rhs) -> double
{
	double sum = 0.0;
	for (std::size_t i = 0; i < lhs.size(); ++i) {
		const auto delta = lhs[i] - rhs[i];
		sum += delta * delta;
	}
	return std::sqrt(sum);
}

auto effective_kp(std::size_t record_count, std::size_t k, std::size_t p) -> std::pair<std::size_t, std::size_t>
{
	std::size_t k_eff = k;
	std::size_t p_eff = p;
	if (p_eff >= record_count) {
		p_eff = record_count - 1;
	}
	if (k_eff >= p_eff) {
		k_eff = p_eff - 1;
	}
	if (p_eff < 3) {
		p_eff = 3;
	}
	if (k_eff < 2) {
		k_eff = 2;
	}
	return {k_eff, p_eff};
}

auto local_entropy_terms(const std::vector<RecordMeta> &records, const std::vector<std::size_t> &active,
						 std::size_t requested_k, std::size_t requested_p) -> std::vector<LocalTerm>
{
	const auto n = active.size();
	const auto dim = records[active.front()].embedded.size();
	const auto [k_eff, p_eff] = effective_kp(n, requested_k, requested_p);
	std::vector<LocalTerm> terms;
	terms.reserve(n);
	std::vector<double> distances(n * n, 0.0);
	for (std::size_t i = 0; i < n; ++i) {
		for (std::size_t j = i + 1; j < n; ++j) {
			const auto d = euclidean(records[active[i]].embedded, records[active[j]].embedded);
			distances[i * n + j] = d;
			distances[j * n + i] = d;
		}
	}

	for (std::size_t i = 0; i < n; ++i) {
		std::vector<std::size_t> order(n);
		std::iota(order.begin(), order.end(), std::size_t{0});
		std::stable_sort(order.begin(), order.end(), [&](std::size_t lhs, std::size_t rhs) {
			return distances[i * n + lhs] < distances[i * n + rhs];
		});

		LocalTerm term;
		term.global_index = active[i];
		term.eps = distances[i * n + order[k_eff - 1]];
		for (std::size_t p_idx = 0; p_idx < p_eff; ++p_idx) {
			term.entropy_neighbors.push_back(active[order[p_idx]]);
		}

		mtrc::numeric::DynamicMatrix<double> nodes(p_eff, dim, 0.0);
		mtrc::numeric::DynamicVector<double> mu(dim, 0.0);
		mtrc::numeric::DynamicVector<double> lb(dim, 0.0);
		mtrc::numeric::DynamicVector<double> ub(dim, 0.0);
		mtrc::numeric::DynamicVector<double> x_vector(dim, 0.0);
		for (std::size_t p_idx = 0; p_idx < p_eff; ++p_idx) {
			const auto &neighbor = records[active[order[p_idx]]].embedded;
			for (std::size_t d_idx = 0; d_idx < dim; ++d_idx) {
				nodes(p_idx, d_idx) = neighbor[d_idx];
				mu[d_idx] += neighbor[d_idx];
			}
		}
		mu = mu / static_cast<double>(p_eff);
		nodes = nodes - mtrc::numeric::expand(mtrc::numeric::trans(mu), nodes.rows());
		auto covariance = mtrc::numeric::evaluate(
			(mtrc::numeric::trans(nodes) * nodes) * static_cast<double>(p_eff) / static_cast<double>(p_eff - 1) +
			mtrc::numeric::IdentityMatrix<double>(dim) * kCovarianceOffset);
		const auto &x = records[active[i]].embedded;
		for (std::size_t d_idx = 0; d_idx < dim; ++d_idx) {
			lb[d_idx] = x[d_idx] - term.eps;
			ub[d_idx] = x[d_idx] + term.eps;
			x_vector[d_idx] = x[d_idx];
		}

		auto g_local = epmgp::local_gaussian_axis_aligned_hyperrectangles<double>(mu, covariance, lb, ub);
		term.log_g = std::get<0>(g_local);
		if (!std::isnan(term.log_g)) {
			term.density = mtrc::entropy_details::mvnpdf(x_vector, mu, covariance);
			if (term.density > 0.0 && std::isfinite(term.density)) {
				term.contribution = term.log_g - std::log(term.density);
				term.valid = std::isfinite(term.contribution);
			}
		}
		terms.push_back(std::move(term));
	}
	return terms;
}

auto visual_entropy_from_terms(const std::vector<LocalTerm> &terms, std::size_t n, std::size_t requested_k) -> std::pair<double, std::size_t>
{
	double h = 0.0;
	std::size_t valid = 0;
	for (const auto &term : terms) {
		if (term.valid) {
			h += term.contribution;
			++valid;
		}
	}
	if (valid == 0) {
		return {std::numeric_limits<double>::quiet_NaN(), 0};
	}
	return {mtrc::entropy_details::digamma(static_cast<double>(n)) -
				mtrc::entropy_details::digamma(static_cast<double>(requested_k)) + h / static_cast<double>(n),
			valid};
}

auto make_frames(const std::vector<RecordMeta> &records, const Options &options) -> std::vector<Frame>
{
	const auto n_all = records.size();
	if (n_all < options.window_size) {
		throw std::runtime_error("record count is smaller than the requested window size");
	}
	const auto frame_count = std::max<std::size_t>(1, n_all / options.step_size);
	std::vector<Frame> frames;
	frames.reserve(frame_count);
	std::vector<std::size_t> previous_active;
	for (std::size_t frame_index = 0; frame_index < frame_count; ++frame_index) {
		Frame frame;
		frame.index = frame_index;
		frame.offset = (frame_index * options.step_size) % n_all;
		frame.active.reserve(options.window_size);
		for (std::size_t i = 0; i < options.window_size; ++i) {
			frame.active.push_back((frame.offset + i) % n_all);
		}

		std::vector<bool> previous_mask(n_all, false);
		std::vector<bool> current_mask(n_all, false);
		for (const auto index : previous_active) {
			previous_mask[index] = true;
		}
		for (const auto index : frame.active) {
			current_mask[index] = true;
			if (!previous_mask[index]) {
				frame.incoming.push_back(index);
			}
		}
		for (const auto index : previous_active) {
			if (!current_mask[index]) {
				frame.outgoing.push_back(index);
			}
		}

		std::vector<Vector> window_records;
		window_records.reserve(frame.active.size());
		for (const auto index : frame.active) {
			window_records.push_back(records[index].embedded);
		}
		auto window_space = mtrc::make_space(window_records, mtrc::Euclidean<double>{});

		mtrc::stats::properties::entropy_options entropy_options;
		entropy_options.neighbor_count = options.entropy_neighbors;
		entropy_options.approximation_order = options.approximation_order;
		entropy_options.max_exact_records = 0;
		entropy_options.max_distance_evaluations = 0;
		const auto entropy_result = mtrc::stats::properties::entropy(window_space, entropy_options);
		frame.entropy_api = entropy_result.value;
		frame.entropy_status = mtrc::core::entropy_status_name(entropy_result.status);
		frame.effective_k = entropy_result.effective_neighbor_count;
		frame.effective_p = entropy_result.effective_approximation_order;

		for (std::size_t local = 0; local < frame.active.size(); ++local) {
			const auto neighbors = mtrc::find_neighbors(window_space, window_space.id(local), options.graph_neighbors);
			for (const auto &neighbor : neighbors.neighbors) {
				frame.edges.push_back({frame.active[local], frame.active[neighbor.id.index()], neighbor.distance});
			}
		}

		frame.local_terms = local_entropy_terms(records, frame.active, options.entropy_neighbors, options.approximation_order);
		auto reconstructed = visual_entropy_from_terms(frame.local_terms, frame.active.size(), options.entropy_neighbors);
		frame.entropy_visual_reconstruction = reconstructed.first;
		frame.valid_terms = reconstructed.second;
		frame.entropy_delta =
			std::isfinite(frame.entropy_api) && std::isfinite(frame.entropy_visual_reconstruction)
				? std::abs(frame.entropy_api - frame.entropy_visual_reconstruction)
				: std::numeric_limits<double>::quiet_NaN();
		frames.push_back(std::move(frame));
		previous_active = frames.back().active;
	}
	return frames;
}

auto write_json(const std::filesystem::path &path, const std::vector<RecordMeta> &records,
				const std::vector<Frame> &frames, const Options &options) -> void
{
	std::ofstream out(path);
	if (!out) {
		throw std::runtime_error("failed to write " + path.string());
	}
	out << std::setprecision(12);
	out << "{\n";
	out << "  \"schema\":\"metric.demo.native_metric_entropy_window.v1\",\n";
	out << "  \"dataset\":{\"id\":\"btad-03\",\"title\":\"BTAD/03\",\"source\":\"" << json_escape(options.btad_root.string())
		<< "\",\"record_count\":" << records.size() << "},\n";
	out << "  \"metric_pipeline\":{\n";
	out << "    \"record_ingestion\":\"native C++ BMP reader; " << options.sample_width << "x" << options.sample_height
		<< " grayscale vectors\",\n";
	out << "    \"source_space\":\"mtrc::make_space(records, mtrc::Euclidean<double>{})\",\n";
	out << "    \"embedding\":\"mtrc::embed(source_space, 3)\",\n";
	out << "    \"entropy\":\"mtrc::stats::properties::entropy(window_space, entropy_options)\",\n";
	out << "    \"knn_graph\":\"mtrc::find_neighbors(window_space, id, graph_neighbors)\",\n";
	out << "    \"window_size\":" << options.window_size << ",\"step_size\":" << options.step_size
		<< ",\"entropy_neighbors\":" << options.entropy_neighbors << ",\"approximation_order\":"
		<< options.approximation_order << ",\"graph_neighbors\":" << options.graph_neighbors << "\n";
	out << "  },\n";
	out << "  \"records\":[\n";
	for (std::size_t i = 0; i < records.size(); ++i) {
		const auto &record = records[i];
		out << "    {\"id\":\"" << json_escape(record.id) << "\",\"index\":" << record.index << ",\"source_id\":\""
			<< json_escape(record.source_id) << "\",\"category\":\"" << json_escape(record.category) << "\",\"label\":\""
			<< json_escape(record.label) << "\",\"split\":\"" << json_escape(record.split) << "\",\"path\":\""
			<< json_escape(record.path.string()) << "\",\"width\":" << record.width << ",\"height\":" << record.height
			<< ",\"coordinates\":[" << record.embedded[0] << "," << record.embedded[1] << "," << record.embedded[2]
			<< "]}";
		out << (i + 1 == records.size() ? "\n" : ",\n");
	}
	out << "  ],\n";
	out << "  \"frames\":[\n";
	for (std::size_t f = 0; f < frames.size(); ++f) {
		const auto &frame = frames[f];
		out << "    {\"index\":" << frame.index << ",\"offset\":" << frame.offset << ",\"entropy_api\":"
			<< frame.entropy_api << ",\"entropy_visual_reconstruction\":" << frame.entropy_visual_reconstruction
			<< ",\"entropy_delta\":" << frame.entropy_delta << ",\"entropy_status\":\""
			<< json_escape(frame.entropy_status) << "\",\"valid_terms\":" << frame.valid_terms << ",\"effective_k\":"
			<< frame.effective_k << ",\"effective_p\":" << frame.effective_p << ",";
		auto write_index_array = [&](const char *name, const std::vector<std::size_t> &values) {
			out << "\"" << name << "\":[";
			for (std::size_t i = 0; i < values.size(); ++i) {
				if (i) {
					out << ",";
				}
				out << values[i];
			}
			out << "]";
		};
		write_index_array("active", frame.active);
		out << ",";
		write_index_array("incoming", frame.incoming);
		out << ",";
		write_index_array("outgoing", frame.outgoing);
		out << ",\"local\":[";
		for (std::size_t i = 0; i < frame.local_terms.size(); ++i) {
			const auto &term = frame.local_terms[i];
			if (i) {
				out << ",";
			}
			out << "{\"record\":" << term.global_index << ",\"eps\":" << term.eps << ",\"logG\":" << term.log_g
				<< ",\"g\":" << term.density << ",\"c\":" << term.contribution << ",\"valid\":"
				<< (term.valid ? "true" : "false") << ",\"neighbors\":[";
			for (std::size_t j = 0; j < term.entropy_neighbors.size(); ++j) {
				if (j) {
					out << ",";
				}
				out << term.entropy_neighbors[j];
			}
			out << "]}";
		}
		out << "],\"edges\":[";
		for (std::size_t i = 0; i < frame.edges.size(); ++i) {
			const auto &edge = frame.edges[i];
			if (i) {
				out << ",";
			}
			out << "[" << edge.source << "," << edge.target << "," << edge.distance << "]";
		}
		out << "]}";
		out << (f + 1 == frames.size() ? "\n" : ",\n");
	}
	out << "  ]\n";
	out << "}\n";
}

auto write_html(const std::filesystem::path &path) -> void
{
	std::ofstream out(path);
	if (!out) {
		throw std::runtime_error("failed to write " + path.string());
	}
	out << R"HTML(<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>BTAD/03 Metric Entropy Space</title>
  <style>
    :root { color-scheme: light; --ink:#172326; --muted:#5a6669; --line:rgba(24,35,37,.18); }
    * { box-sizing:border-box; }
    html, body { margin:0; width:100%; height:100%; overflow:hidden; font-family:ui-sans-serif,-apple-system,BlinkMacSystemFont,"Segoe UI",sans-serif; background:#eef2ee; color:var(--ink); }
    #scene { display:block; width:100vw; height:100vh; }
    .controls {
      position:fixed; left:50%; bottom:18px; transform:translateX(-50%);
      display:grid; grid-template-columns:42px minmax(240px,560px) 82px; grid-template-rows:30px 18px;
      align-items:center; gap:4px 12px;
      width:min(720px, calc(100vw - 28px)); padding:10px 12px;
      border:1px solid rgba(24,35,37,.16); border-radius:14px;
      background:rgba(249,250,246,.78); backdrop-filter:blur(18px) saturate(1.1);
      box-shadow:0 18px 56px rgba(28,42,40,.13);
    }
    .spark { grid-column:2; grid-row:1; width:100%; height:30px; display:block; }
    .readout {
      grid-column:3; grid-row:1 / span 2; align-self:center;
      font:650 12px/1.28 ui-sans-serif,-apple-system,BlinkMacSystemFont,"Segoe UI",sans-serif;
      color:#243236; letter-spacing:0; white-space:pre-line;
    }
    button {
      grid-column:1; grid-row:1 / span 2; width:38px; height:38px; display:grid; place-items:center;
      border:1px solid rgba(24,35,37,.18); border-radius:999px; padding:0;
      background:rgba(255,255,252,.8); color:#172326; font-size:0;
      box-shadow:0 6px 18px rgba(28,42,40,.08);
    }
    button::before {
      content:""; display:block; width:0; height:0; margin-left:2px;
      border-top:7px solid transparent; border-bottom:7px solid transparent; border-left:11px solid currentColor;
    }
    button.active { background:#172326; color:#fff; }
    button.active::before {
      width:12px; height:14px; margin-left:0; border:0;
      background:linear-gradient(90deg,currentColor 0 34%,transparent 34% 66%,currentColor 66% 100%);
    }
    input[type=range] {
      grid-column:2; grid-row:2; width:100%; height:18px; margin:0; appearance:none; background:transparent;
    }
    input[type=range]::-webkit-slider-runnable-track {
      height:3px; border-radius:999px; background:rgba(32,48,52,.22);
    }
    input[type=range]::-webkit-slider-thumb {
      appearance:none; width:14px; height:14px; margin-top:-5.5px; border-radius:999px;
      background:#1b7583; border:2px solid rgba(249,250,246,.95); box-shadow:0 3px 10px rgba(22,48,54,.22);
    }
    input[type=range]::-moz-range-track {
      height:3px; border-radius:999px; background:rgba(32,48,52,.22);
    }
    input[type=range]::-moz-range-thumb {
      width:14px; height:14px; border-radius:999px;
      background:#1b7583; border:2px solid rgba(249,250,246,.95); box-shadow:0 3px 10px rgba(22,48,54,.22);
    }
    body .mtrc-record-preview {
      width:min(390px, calc(100vw - 28px));
      padding:0;
      overflow:hidden auto;
      border-radius:10px;
      background:rgba(251,251,244,.98);
    }
    body .mtrc-record-preview .metric-data-card { display:block; position:relative; }
    .metric-data-card__image {
      width:auto;
      height:auto;
      max-width:100%;
      max-height:min(52vh, 320px);
      display:block;
      margin:0 auto;
      object-fit:contain;
      background:#070a09;
    }
    .metric-data-card__caption {
      display:flex;
      align-items:center;
      justify-content:space-between;
      gap:10px;
      min-width:0;
      padding:7px 9px;
      border-top:1px solid rgba(24,35,37,.12);
      background:rgba(251,251,244,.97);
      color:#172326;
    }
    .metric-data-card__caption strong {
      min-width:0;
      overflow:hidden;
      text-overflow:ellipsis;
      white-space:nowrap;
      font:750 11px/1.2 ui-sans-serif,-apple-system,BlinkMacSystemFont,"Segoe UI",sans-serif;
      letter-spacing:0;
    }
    .metric-data-card__caption span {
      flex:0 0 auto;
      color:#516064;
      font:650 10px/1.2 ui-monospace,SFMono-Regular,Menlo,monospace;
      white-space:nowrap;
    }
    .analysis-tag {
      position:fixed;
      top:18px;
      left:18px;
      z-index:4;
      max-width:min(430px, calc(100vw - 36px));
      padding:10px 12px;
      border:1px solid rgba(24,35,37,.16);
      border-radius:12px;
      background:rgba(249,250,246,.80);
      color:#172326;
      backdrop-filter:blur(18px) saturate(1.08);
      box-shadow:0 18px 48px rgba(28,42,40,.11);
    }
    .analysis-tag strong {
      display:block;
      margin-bottom:4px;
      font:800 13px/1.18 ui-sans-serif,-apple-system,BlinkMacSystemFont,"Segoe UI",sans-serif;
      letter-spacing:0;
    }
    .analysis-tag span,
    .analysis-tag code {
      display:block;
      color:#536164;
      font-size:11px;
      line-height:1.35;
    }
    .analysis-tag code {
      margin-top:3px;
      color:#233236;
      font-family:ui-monospace,SFMono-Regular,Menlo,monospace;
      font-weight:650;
    }
  </style>
</head>
<body>
  <canvas id="scene"></canvas>
  <div class="analysis-tag" aria-label="continuous entropy analysis">
    <strong>Kontinuierliche Entropie-Analyse</strong>
    <span>Sliding Window: 1000 Records, je Schritt 100 raus und 100 rein.</span>
    <code>mtrc::make_space -> mtrc::embed(3) -> stats::properties::entropy</code>
  </div>
  <div class="controls">
    <button id="play" type="button" aria-label="Play"></button>
    <canvas class="spark" id="spark" width="620" height="64" aria-hidden="true"></canvas>
    <input id="frame" type="range" min="0" max="10" step="1" value="0" aria-label="window frame">
    <span class="readout" id="readout">t=0</span>
  </div>
  <script type="module">
    import {
      createMetricVisual,
      MetricSpaceView,
      NeighborhoodGraphView,
      createChannel,
      createStringChannel
    } from '/Users/michaelwelsch/Documents/metric/visual/src/index.js';

    const canvas = document.getElementById("scene");
    const slider = document.getElementById("frame");
    const play = document.getElementById("play");
    const readout = document.getElementById("readout");
    const spark = document.getElementById("spark");
    const sparkCtx = spark.getContext("2d");
    const data = await fetch("entropy_window_data.json", { cache: "no-store" }).then(response => response.json());
    slider.max = String(data.frames.length - 1);

    const datasetId = data.dataset.id;
    const recordById = new Map(data.records.map(record => [record.id, record]));
    const evidence = {
      schema: "metric.visual.v1",
      provenance: {
        generator: "experiments/image_atlas/btad_metric_entropy_window_graph.cpp",
        metric_pipeline: data.metric_pipeline
      },
      datasets: [{
        id: datasetId,
        title: data.dataset.title,
        description: "BTAD/03 records as one finite metric image space; 3D coordinates are produced by mtrc::embed(source_space, 3).",
        source: data.dataset.source,
        license: "local research dataset copy",
        record_count: data.records.length
      }],
      records: data.records.map(record => ({
        id: record.id,
        dataset_id: datasetId,
        record_type: "image",
        label: record.source_id,
        payload: {
          kind: "image-ref",
          href: record.path,
          src: record.path,
          width: record.width,
          height: record.height,
          label: record.source_id,
          alt: `${record.category} ${record.label}`,
          features: {
            category: record.category,
            label: record.label,
            split: record.split,
            source_space: data.metric_pipeline.source_space,
            embedding: data.metric_pipeline.embedding,
            entropy: data.metric_pipeline.entropy
          }
        }
      })),
      relations: [{
        id: "embedded-euclidean-distance",
        dataset_id: datasetId,
        name: "Euclidean metric on mtrc::embed(..., 3) coordinates",
        relation_type: "distance",
        value_type: "number",
        record_ids: data.records.map(record => record.id),
        storage: "dynamic-frame-sparse",
        values: []
      }],
      spaces: [{
        id: "entropy-window-space",
        dataset_id: datasetId,
        record_ids: data.records.map(record => record.id),
        primary_relation_id: "embedded-euclidean-distance",
        space_type: "finite_metric_space",
        metadata: data.metric_pipeline,
        metric: { kind: "euclidean", coordinate_source: "mtrc::embed(source_space, 3)" }
      }],
      properties: [],
      graphs: [],
      coordinates: [],
      timelines: [],
      events: [],
      views: [],
      diagnostics: []
    };

    const visual = await createMetricVisual({
      evidence,
      canvas,
      start: true,
      controls: { enabled: true, rotateSpeed: 0.0065 },
      runtime: {
        camera: {
          position: [3.2, -5.6, 2.7],
          target: [0, 0.05, 0],
          orbit: { autoRotate: true, enableDamping: true, dampingFactor: 0.055, minPitch: -1.18, maxPitch: 1.05 }
        },
        hoverFocus: { enabled: true, thresholdPx: 34, clearOnLeave: false },
        profileOptions: {
          stageOptions: {
            camera: { orbit: { autoRotate: true } },
            background: "#eef2ee"
          }
        }
      }
    });
    visual.setMotion(false);
    startManualCameraOrbit(visual);
    visual.enableRecordPreview({
      mode: "record",
      panel: {
        maxFields: 0,
        resolver: previewForRecord,
        render: renderRecordCard
      }
    });

    let frameIndex = 0;
    let playing = false;
    let playTimer = null;
    let focusedRecordId = null;

    slider.addEventListener("input", event => setFrame(event.target.value));
    play.addEventListener("click", () => {
      playing = !playing;
      play.classList.toggle("active", playing);
      play.setAttribute("aria-label", playing ? "Pause" : "Play");
      if (playTimer) {
        clearInterval(playTimer);
        playTimer = null;
      }
      if (playing) {
        playTimer = setInterval(() => setFrame((frameIndex + 1) % data.frames.length), 1500);
      }
    });
    visual.runtime.on("hoverfocuschange", ({ focusTarget }) => {
      const next = focusTarget?.recordId ? String(focusTarget.recordId) : null;
      if (!next || next === focusedRecordId) return;
      focusedRecordId = next;
      renderFrame();
    });

    function setFrame(next) {
      frameIndex = Math.max(0, Math.min(data.frames.length - 1, Number(next)));
      slider.value = String(frameIndex);
      const frame = data.frames[frameIndex];
      if (!frame.active.some(index => data.records[index].id === focusedRecordId)) {
        focusedRecordId = defaultFocusRecordId(frame);
      }
      renderFrame();
    }

    function frameLocalMap(frame) {
      if (!frame._localMap) {
        frame._localMap = new Map(frame.local.map(item => [data.records[item.record].id, item]));
      }
      return frame._localMap;
    }

    function contributionRange(frame) {
      if (frame._ranges) return frame._ranges;
      const values = frame.local.filter(term => term.valid && Number.isFinite(term.c)).map(term => term.c);
      values.sort((a, b) => a - b);
      const q = p => values[Math.max(0, Math.min(values.length - 1, Math.floor(p * (values.length - 1))))] ?? 0;
      frame._ranges = { min: q(0.03), max: q(0.97) };
      return frame._ranges;
    }

    function localFreedomIndex(term, frame) {
      if (!term?.valid || !Number.isFinite(term.c)) return 0;
      const range = contributionRange(frame);
      return clamp01((term.c - range.min) / Math.max(1e-9, range.max - range.min));
    }

    function renderFrame() {
      const frame = data.frames[frameIndex];
      const local = frameLocalMap(frame);
      const recordIds = frame.active.map(index => data.records[index].id);
      const positions = new Map(recordIds.map(id => [id, recordById.get(id).coordinates]));
      const scalarValues = new Map(recordIds.map(id => [id, localFreedomIndex(local.get(id), frame)]));
      const sizeValues = new Map(recordIds.map(id => [id, 0.72 + localFreedomIndex(local.get(id), frame) * 1.08]));
      const colorValues = new Map(recordIds.map(id => [id, recordById.get(id).label]));
      const space = new MetricSpaceView({
        id: "native-metric-entropy-space",
        records: evidence.records,
        recordIds,
        positions,
        scalarValues,
        colorValues,
        sizeValues,
        recordGlyphs: false,
        labels: false,
        ground: true,
        groundProjection: true,
        groundProjectionAlpha: 0.34,
        targetRadius: 1.72,
        pointMaterial: {
          pointPixelScale: 14,
          minPointSize: 3,
          maxPointSize: 38,
          sphereShade: 0.78,
          gloss: 0.36,
          specular: 0.3,
          alpha: 0.94,
          phasePulse: 0.02
        },
        metadata: {
          source_space: data.metric_pipeline.source_space,
          embedding: data.metric_pipeline.embedding,
          entropy: data.metric_pipeline.entropy
        }
      });
      const fittedPositions = space.positions;
      const fitScale = space.fit?.scale || 1;
      const graph = new NeighborhoodGraphView({
        id: "metric-knn-graph",
        recordIds,
        records: evidence.records,
        positions: fittedPositions,
        fit: false,
        nodes: false,
        relationKind: "distance",
        graph: {
          id: `metric-knn-frame-${frame.index}`,
          record_ids: recordIds,
          nodes: recordIds,
          directed: true,
          edges: frame.edges.map(([source, target, value], index) => ({
            id: `e-${frame.index}-${index}`,
            source: data.records[source].id,
            target: data.records[target].id,
            value
          }))
        },
        metadata: { operator: data.metric_pipeline.knn_graph }
      });
      const focus = focusedRecordId || defaultFocusRecordId(frame);
      focusedRecordId = focus;
      visual.setViews([
        space,
        graph,
        integrationWindowsView(frame, fittedPositions, fitScale, focus),
        focusSupportGraph(frame, fittedPositions, focus)
      ].filter(Boolean), { source: "native-metric-entropy-window", viewKind: "metric-space" });
      updateReadout(frame);
      drawSparkline();
      refreshVisiblePreview();
    }

    function startManualCameraOrbit(visual) {
      const runtime = visual.runtime;
      const cameraState = runtime?.cameraState;
      if (!runtime || !cameraState || typeof cameraState.toJSON !== "function") return;
      const base = cameraState.toJSON();
      const startedAt = performance.now();
      let lastRender = 0;
      const orbit = now => {
        const seconds = (now - startedAt) / 1000;
        if (!runtime.controls?.dragging) {
          cameraState.setOrbit({
            yaw: base.yaw + seconds * 0.105 + Math.sin(seconds * 0.45) * 0.055,
            pitch: base.pitch + Math.sin(seconds * 0.55) * 0.028,
            radius: base.radius * (1 + Math.sin(seconds * 0.35) * 0.014)
          });
          cameraState.applyTo(runtime.camera);
          runtime.renderer?.setCamera?.(runtime.camera);
          runtime.syncFocusTarget?.();
          if (now - lastRender > 16) {
            runtime.renderOnce?.(now);
            lastRender = now;
          }
        }
        requestAnimationFrame(orbit);
      };
      requestAnimationFrame(orbit);
    }

    function integrationWindowsView(frame, fittedPositions, fitScale, focusRecordId) {
      const samples = sampledIntegrationTerms(frame, focusRecordId);
      const boxIds = [];
      const boxPositions = [];
      const boxScales = [];
      const boxColors = [];
      const haloIds = [];
      const haloPositions = [];
      const haloRadii = [];
      const haloScalars = [];
      for (const term of samples) {
        const record = data.records[term.record];
        const recordId = record.id;
        const position = fittedPositions.get(recordId);
        if (!position || !term.valid) continue;
        const isFocus = recordId === focusRecordId;
        const localIndex = localFreedomIndex(term, frame);
        const rawRadius = term.eps * fitScale;
        const radius = isFocus
          ? Math.max(0.045, Math.min(0.55, rawRadius))
          : Math.max(0.025, Math.min(0.18, rawRadius));
        boxIds.push(recordId);
        boxPositions.push(position[0], position[1], position[2]);
        boxScales.push(radius * 2, radius * 2, radius * 2);
        boxColors.push(isFocus ? 0.95 : 0.18, isFocus ? 0.64 : 0.58, isFocus ? 0.23 : 0.72, isFocus ? 0.38 : 0.055);
        haloIds.push(recordId);
        haloPositions.push(position[0], position[1], position[2]);
        haloRadii.push(isFocus ? 28 + radius * 70 : 5 + radius * 24);
        haloScalars.push(localIndex);
      }
      return {
        toLayerDescriptors() {
          return [
            {
              schema: "metric.visual.layer_descriptor.v1",
              id: "entropy-eps-halos",
              kind: "entropy-integration-eps-radius",
              primitive: "HeatFieldLayer",
              order: 6,
              channels: {
                recordId: createStringChannel(haloIds, "record-id"),
                position: createChannel(new Float32Array(haloPositions), 3, "position"),
                scalar: createChannel(new Float32Array(haloScalars), 1, "local-freedom-index"),
                radius: createChannel(new Float32Array(haloRadii), 1, "screen-radius")
              },
              geometry: { mode: "radial" },
              material: { alpha: 0.36, glow: 0.2, contour: 0.3, pointPixelScale: 1, maxPointSize: 92 },
              picking: { mode: "record-id", channel: "recordId" },
              metadata: { role: "visible eps radius B(q,eps) from the kpN entropy estimator" }
            },
            {
              schema: "metric.visual.layer_descriptor.v1",
              id: "entropy-local-boxes",
              kind: "entropy-local-gaussian-boxes",
              primitive: "InstancedBoxLayer",
              order: 7,
              channels: {
                recordId: createStringChannel(boxIds, "record-id"),
                position: createChannel(new Float32Array(boxPositions), 3, "position"),
                scale: createChannel(new Float32Array(boxScales), 3, "scale"),
                color: createChannel(new Float32Array(boxColors), 4, "rgba")
              },
              geometry: { anchor: "center" },
              material: {
                alpha: 0.52,
                alphaMode: "blend",
                ambient: 0.62,
                pointLight: 0.24,
                specular: 0.18,
                roughness: 0.62,
                metalness: 0.08,
                faceContrast: 0.18
              },
              picking: { mode: "record-id", channel: "recordId" },
              metadata: { role: "axis-aligned local integration windows [x_i-eps,x_i+eps]" }
            }
          ];
        }
      };
    }

    function sampledIntegrationTerms(frame, focusRecordId) {
      const valid = frame.local.filter(term => term.valid && Number.isFinite(term.c));
      valid.sort((a, b) => a.c - b.c);
      const out = [];
      const seen = new Set();
      const push = term => {
        if (!term) return;
        const id = data.records[term.record].id;
        if (seen.has(id)) return;
        seen.add(id);
        out.push(term);
      };
      for (let index = 0; index < valid.length; index += Math.max(1, Math.floor(valid.length / 14))) {
        push(valid[index]);
      }
      valid.slice(0, 4).forEach(push);
      valid.slice(-5).forEach(push);
      const focus = focusRecordId ? frameLocalMap(frame).get(focusRecordId) : null;
      push(focus);
      return out;
    }

    function focusSupportGraph(frame, fittedPositions, focusRecordId) {
      const term = focusRecordId ? frameLocalMap(frame).get(focusRecordId) : null;
      if (!term || !Array.isArray(term.neighbors)) return null;
      const source = data.records[term.record].id;
      const ids = [source, ...term.neighbors.map(index => data.records[index].id)];
      return new NeighborhoodGraphView({
        id: "entropy-local-neighbor-support",
        recordIds: ids,
        records: evidence.records,
        positions: fittedPositions,
        fit: false,
        nodes: false,
        relationKind: "distance",
        graph: {
          id: `entropy-support-${frame.index}-${source}`,
          record_ids: ids,
          nodes: ids,
          directed: true,
          edges: term.neighbors
            .filter(index => data.records[index].id !== source)
            .map((target, index) => ({
              id: `support-${index}`,
              source,
              target: data.records[target].id,
              value: index
            }))
        },
        metadata: { role: "p-neighbor support used by the local Gaussian entropy term" }
      });
    }

    function defaultFocusRecordId(frame) {
      const valid = frame.local.filter(term => term.valid && Number.isFinite(term.c));
      valid.sort((a, b) => b.c - a.c);
      return data.records[valid[0]?.record ?? frame.active[0]].id;
    }

    function updateReadout(frame) {
      const h = Number.isFinite(frame.entropy_api) ? frame.entropy_api.toFixed(4) : "NaN";
      readout.textContent = `W${String(frame.index).padStart(2, "0")}\nH ${h}`;
    }

    function drawSparkline() {
      const dpr = Math.min(2, window.devicePixelRatio || 1);
      const width = spark.clientWidth || 520;
      const height = spark.clientHeight || 44;
      spark.width = Math.round(width * dpr);
      spark.height = Math.round(height * dpr);
      sparkCtx.setTransform(dpr, 0, 0, dpr, 0, 0);
      sparkCtx.clearRect(0, 0, width, height);
      const values = data.frames.map(frame => frame.entropy_api).filter(Number.isFinite);
      const min = Math.min(...values);
      const max = Math.max(...values);
      const span = Math.max(1e-9, max - min);
      const yFor = value => height - 7 - ((value - min) / span) * (height - 14);
      sparkCtx.strokeStyle = "rgba(28,107,124,.82)";
      sparkCtx.lineWidth = 1.8;
      sparkCtx.beginPath();
      data.frames.forEach((frame, index) => {
        const x = (index / Math.max(1, data.frames.length - 1)) * width;
        const y = yFor(frame.entropy_api);
        if (index === 0) sparkCtx.moveTo(x, y);
        else sparkCtx.lineTo(x, y);
      });
      sparkCtx.stroke();
      const x = (frameIndex / Math.max(1, data.frames.length - 1)) * width;
      sparkCtx.fillStyle = "rgba(24,117,131,.95)";
      sparkCtx.beginPath();
      sparkCtx.arc(x, yFor(data.frames[frameIndex].entropy_api), 4, 0, Math.PI * 2);
      sparkCtx.fill();
    }

    function previewForRecord(input = {}) {
      const recordId = String(input.recordId || input.focusTarget?.recordId || "");
      const record = recordById.get(recordId);
      if (!record) return null;
      return { title: record.source_id, subtitle: `${record.category} / ${record.split} / ${record.label}` };
    }

    function refreshVisiblePreview() {
      const panel = document.querySelector(".mtrc-record-preview");
      if (!panel || !focusedRecordId) return;
      const card = renderRecordCard(null, { recordId: focusedRecordId });
      if (!card) return;
      panel.replaceChildren(card);
    }

    function renderRecordCard(preview, input = {}) {
      const recordId = String(input.recordId || input.focusTarget?.recordId || "");
      const record = recordById.get(recordId);
      if (!record) return null;
      const card = document.createElement("article");
      card.className = "metric-data-card";
      const image = document.createElement("img");
      image.className = "metric-data-card__image";
      image.src = record.path;
      image.alt = record.source_id;
      const caption = document.createElement("div");
      caption.className = "metric-data-card__caption";
      const title = document.createElement("strong");
      title.textContent = record.source_id;
      const coords = Array.isArray(record.coordinates)
        ? record.coordinates.map(value => Number(value).toFixed(3)).join(", ")
        : "-";
      const embed = document.createElement("span");
      embed.textContent = `embed [${coords}]`;
      caption.append(title, embed);
      card.append(image, caption);
      return card;
    }

    function clamp01(value) {
      return Math.max(0, Math.min(1, Number.isFinite(Number(value)) ? Number(value) : 0));
    }

    addEventListener("resize", drawSparkline);
    setFrame(0);
  </script>
</body>
</html>
)HTML";
}

} // namespace

int main(int argc, char **argv)
{
	try {
		const auto options = parse_options(argc, argv);
		std::filesystem::create_directories(options.export_dir);
		auto records = collect_records(options);
		std::vector<Vector> source_records;
		source_records.reserve(records.size());
		for (const auto &record : records) {
			source_records.push_back(record.source_record);
		}

		auto source_space = mtrc::make_space(source_records, mtrc::Euclidean<double>{});
		auto embedded = mtrc::embed(source_space, std::size_t{3});
		for (std::size_t i = 0; i < records.size(); ++i) {
			records[i].embedded = embedded.space.record(embedded.space.id(i));
		}

		std::cerr << "records=" << records.size() << " source_space=mtrc::make_space embedding=mtrc::embed(...,3)\n";
		auto frames = make_frames(records, options);
		for (const auto &frame : frames) {
			std::cerr << "frame=" << frame.index << " entropy_api=" << frame.entropy_api
					  << " reconstructed=" << frame.entropy_visual_reconstruction << " delta=" << frame.entropy_delta
					  << " status=" << frame.entropy_status << "\n";
			if (std::isfinite(frame.entropy_delta) && frame.entropy_delta > 1.0e-8) {
				throw std::runtime_error("visual entropy reconstruction diverges from metric entropy API");
			}
		}

		write_json(options.export_dir / "entropy_window_data.json", records, frames, options);
		write_html(options.export_dir / "index.html");
		std::cout << "wrote " << (options.export_dir / "index.html") << "\n";
		std::cout << "records " << records.size() << ", frames " << frames.size() << "\n";
		std::cout << "pipeline mtrc::make_space -> mtrc::embed(...,3) -> mtrc::stats::properties::entropy\n";
	} catch (const std::exception &error) {
		std::cerr << "error: " << error.what() << "\n";
		return 1;
	}
	return 0;
}
