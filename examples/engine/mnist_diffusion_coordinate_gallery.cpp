#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <metric/engine.hpp>
#include <metric/metric/catalog.hpp>

#include "assets/mnist/mnist_reader.hpp"

#ifndef METRIC_MNIST_DATA_DIR
#define METRIC_MNIST_DATA_DIR "examples/engine/assets/mnist"
#endif

namespace {

using record_type = std::vector<double>;
constexpr int kSkipMissingMnistData = 77;

struct MnistRecord {
	std::string id;
	std::uint8_t label{0};
	record_type pixels;
};

struct Options {
	std::filesystem::path mnist_dir{METRIC_MNIST_DATA_DIR};
	std::filesystem::path export_dir{"docs/examples/assets/mnist-diffusion-coordinate"};
	std::size_t per_class{60};
	std::size_t calibration_steps{24};
	bool export_files{false};
};

struct Bounds {
	double min_x{std::numeric_limits<double>::infinity()};
	double max_x{-std::numeric_limits<double>::infinity()};
	double min_y{std::numeric_limits<double>::infinity()};
	double max_y{-std::numeric_limits<double>::infinity()};
};

auto parse_size(const std::string &text, const std::string &name) -> std::size_t
{
	std::size_t parsed = 0;
	std::size_t end = 0;
	try {
		parsed = static_cast<std::size_t>(std::stoull(text, &end));
	} catch (const std::exception &) {
		throw std::invalid_argument("invalid " + name + ": " + text);
	}
	if (end != text.size()) {
		throw std::invalid_argument("invalid " + name + ": " + text);
	}
	return parsed;
}

auto parse_options(int argc, char **argv) -> Options
{
	Options options;
	for (int index = 1; index < argc; ++index) {
		const std::string arg = argv[index];
		if (arg == "--mnist-dir" && index + 1 < argc) {
			options.mnist_dir = argv[++index];
		} else if (arg == "--export-dir" && index + 1 < argc) {
			options.export_dir = argv[++index];
			options.export_files = true;
		} else if (arg == "--per-class" && index + 1 < argc) {
			options.per_class = parse_size(argv[++index], "per-class");
		} else if (arg == "--calibration-steps" && index + 1 < argc) {
			options.calibration_steps = parse_size(argv[++index], "calibration steps");
		} else {
			throw std::invalid_argument("unknown or incomplete argument: " + arg);
		}
	}
	if (options.per_class == 0) {
		throw std::invalid_argument("per-class must be positive");
	}
	if (options.calibration_steps == 0) {
		throw std::invalid_argument("calibration steps must be positive");
	}
	return options;
}

auto has_mnist_calibration_files(const std::filesystem::path &folder) -> bool
{
	return std::filesystem::exists(folder / "train-images-idx3-ubyte") &&
		   std::filesystem::exists(folder / "train-labels-idx1-ubyte");
}

auto load_balanced_mnist(const std::filesystem::path &folder, std::size_t per_class) -> std::vector<MnistRecord>
{
	auto images = mnist::read_calibration_images<std::vector, std::vector<double>>(
		folder.string(), 0, [] { return std::vector<double>(28 * 28, 0.0); });
	auto labels = mnist::read_calibration_labels<std::vector, std::uint8_t>(folder.string(), 0);
	if (images.size() != labels.size()) {
		throw std::runtime_error("MNIST image/label count mismatch");
	}

	std::array<std::size_t, 10> selected{};
	std::vector<MnistRecord> records;
	records.reserve(per_class * selected.size());
	for (std::size_t index = 0; index < images.size(); ++index) {
		const auto label = labels[index];
		if (label >= selected.size() || selected[label] >= per_class) {
			continue;
		}
		for (double &pixel : images[index]) {
			pixel /= 255.0;
		}
		std::ostringstream id;
		id << "mnist-" << static_cast<int>(label) << "-" << std::setw(5) << std::setfill('0') << selected[label];
		records.push_back(MnistRecord{id.str(), label, std::move(images[index])});
		++selected[label];
		if (records.size() == per_class * selected.size()) {
			break;
		}
	}
	for (std::size_t label = 0; label < selected.size(); ++label) {
		if (selected[label] != per_class) {
			throw std::runtime_error("not enough MNIST records for digit " + std::to_string(label));
		}
	}
	return records;
}

auto flatten_records(const std::vector<MnistRecord> &records) -> std::vector<record_type>
{
	std::vector<record_type> result;
	result.reserve(records.size());
	for (const auto &record : records) {
		result.push_back(record.pixels);
	}
	return result;
}

auto labels_text(const std::vector<MnistRecord> &records) -> std::string
{
	std::string result;
	result.reserve(records.size());
	for (const auto &record : records) {
		result.push_back(static_cast<char>('0' + record.label));
	}
	return result;
}

auto label_counts(const std::vector<MnistRecord> &records) -> std::array<std::size_t, 10>
{
	std::array<std::size_t, 10> counts{};
	for (const auto &record : records) {
		++counts[record.label];
	}
	return counts;
}

auto update_bounds(Bounds &bounds, double x, double y) -> void
{
	bounds.min_x = std::min(bounds.min_x, x);
	bounds.max_x = std::max(bounds.max_x, x);
	bounds.min_y = std::min(bounds.min_y, y);
	bounds.max_y = std::max(bounds.max_y, y);
}

auto scale_x(const Bounds &bounds, double x, double left, double width) -> double
{
	const auto span = std::max(1.0e-12, bounds.max_x - bounds.min_x);
	return left + ((x - bounds.min_x) / span) * width;
}

auto scale_y(const Bounds &bounds, double y, double top, double height) -> double
{
	const auto span = std::max(1.0e-12, bounds.max_y - bounds.min_y);
	return top + height - ((y - bounds.min_y) / span) * height;
}

auto color_for_label(std::uint8_t label) -> const char *
{
	static constexpr std::array<const char *, 10> colors{
		"#3b82f6", "#ef4444", "#14b8a6", "#f59e0b", "#8b5cf6",
		"#ec4899", "#22c55e", "#a3a33a", "#64748b", "#7c9cff"};
	return colors.at(label);
}

template <typename LatentSpace>
auto write_coordinates_csv(const std::filesystem::path &path, const std::vector<MnistRecord> &records,
						   const LatentSpace &latent_space,
						   const mtrc::modify::map::DiffusionCoordinateTargets<double> &targets) -> void
{
	std::ofstream out(path);
	if (!out) {
		throw std::runtime_error("unable to write " + path.string());
	}
	out << "id,label,latent_x,latent_y,latent_z,target_x,target_y,target_z\n";
	out << std::setprecision(10);
	for (std::size_t index = 0; index < records.size(); ++index) {
		const auto source_id = latent_space.id(index);
		const auto &latent = latent_space.record(source_id);
		const auto target_it = targets.coordinates.find(source_id);
		if (target_it == targets.coordinates.end()) {
			throw std::runtime_error("missing diffusion-coordinate target coordinate");
		}
		const auto &target = target_it->second;
		out << records[index].id << "," << static_cast<int>(records[index].label) << "," << latent[0] << ","
			<< latent[1] << "," << latent[2] << "," << target[0] << "," << target[1] << "," << target[2] << "\n";
	}
}

auto write_summary_csv(const std::filesystem::path &path, const Options &options, std::size_t record_count,
					   std::size_t dense_evaluations, double initial_coordinate_error, double final_coordinate_error) -> void
{
	std::ofstream out(path);
	if (!out) {
		throw std::runtime_error("unable to write " + path.string());
	}
	out << "key,value\n";
	out << "dataset,MNIST train IDX\n";
	out << "records," << record_count << "\n";
	out << "per_class," << options.per_class << "\n";
	out << "metric,Euclidean over normalized 28x28 digit records\n";
	out << "mapping,native parametric diffusion coordinate\n";
	out << "calibration_steps," << options.calibration_steps << "\n";
	out << "dense_distance_evaluations," << dense_evaluations << "\n";
	out << "initial_coordinate_target_mse," << initial_coordinate_error << "\n";
	out << "final_coordinate_target_mse," << final_coordinate_error << "\n";
}

template <typename LatentSpace>
auto write_visual_json(const std::filesystem::path &path, const std::vector<MnistRecord> &records,
					   const LatentSpace &latent_space) -> void
{
	std::ofstream out(path);
	if (!out) {
		throw std::runtime_error("unable to write " + path.string());
	}
	const auto counts = label_counts(records);
	out << std::setprecision(10);
	out << "{\n";
	out << "  \"schema\": \"metric.visual.mnist_diffusion_coordinate.v1\",\n";
	out << "  \"dataset\": \"MNIST train IDX\",\n";
	out << "  \"recordCount\": " << records.size() << ",\n";
	out << "  \"labels\": \"" << labels_text(records) << "\",\n";
	out << "  \"counts\": [";
	for (std::size_t i = 0; i < counts.size(); ++i) {
		if (i != 0) {
			out << ", ";
		}
		out << counts[i];
	}
	out << "],\n";
	out << "  \"p3\": [";
	for (std::size_t index = 0; index < records.size(); ++index) {
		const auto &point = latent_space.record(latent_space.id(index));
		if (index != 0) {
			out << ", ";
		}
		out << point[0] << ", " << point[1] << ", " << point[2];
	}
	out << "]\n";
	out << "}\n";
}

template <typename LatentSpace>
auto write_svg(const std::filesystem::path &path, const std::vector<MnistRecord> &records,
			   const LatentSpace &latent_space) -> void
{
	Bounds bounds;
	for (std::size_t index = 0; index < records.size(); ++index) {
		const auto &point = latent_space.record(latent_space.id(index));
		update_bounds(bounds, point[0], point[1]);
	}

	std::ofstream out(path);
	if (!out) {
		throw std::runtime_error("unable to write " + path.string());
	}
	constexpr double width = 960.0;
	constexpr double height = 620.0;
	constexpr double left = 84.0;
	constexpr double top = 72.0;
	constexpr double plot_w = 792.0;
	constexpr double plot_h = 456.0;
	out << std::fixed << std::setprecision(2);
	out << "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"960\" height=\"620\" viewBox=\"0 0 960 620\" role=\"img\">\n";
	out << "<rect width=\"960\" height=\"620\" fill=\"#f8fafc\"/>\n";
	out << "<text x=\"54\" y=\"46\" font-family=\"Arial, sans-serif\" font-size=\"24\" font-weight=\"700\" fill=\"#172033\">MNIST parametric diffusion coordinate</text>\n";
	out << "<rect x=\"" << left << "\" y=\"" << top << "\" width=\"" << plot_w << "\" height=\"" << plot_h
		<< "\" fill=\"#ffffff\" stroke=\"#d8dee8\"/>\n";
	for (std::size_t index = 0; index < records.size(); ++index) {
		const auto &point = latent_space.record(latent_space.id(index));
		const auto x = scale_x(bounds, point[0], left + 18.0, plot_w - 36.0);
		const auto y = scale_y(bounds, point[1], top + 18.0, plot_h - 36.0);
		out << "<circle cx=\"" << x << "\" cy=\"" << y << "\" r=\"3.2\" fill=\""
			<< color_for_label(records[index].label) << "\" fill-opacity=\"0.72\"/>\n";
	}
	for (std::size_t label = 0; label < 10; ++label) {
		const auto x = 96.0 + static_cast<double>(label) * 78.0;
		out << "<circle cx=\"" << x << "\" cy=\"566\" r=\"5\" fill=\"" << color_for_label(static_cast<std::uint8_t>(label))
			<< "\"/>\n";
		out << "<text x=\"" << (x + 10.0) << "\" y=\"570\" font-family=\"Arial, sans-serif\" font-size=\"13\" fill=\"#334155\">"
			<< label << "</text>\n";
	}
	out << "</svg>\n";
}

} // namespace

int main(int argc, char **argv)
{
	const auto options = parse_options(argc, argv);
	if (!has_mnist_calibration_files(options.mnist_dir)) {
		std::cout << "mnist diffusion coordinate gallery skipped: MNIST train IDX files not found in "
				  << options.mnist_dir.string() << "\n";
		return kSkipMissingMnistData;
	}
	auto records_with_labels = load_balanced_mnist(options.mnist_dir, options.per_class);
	auto records = flatten_records(records_with_labels);
	auto space = mtrc::make_space(records, mtrc::Euclidean<double>{});

	mtrc::modify::map::DiffusionCoordinateSpec<double> geometry;
	geometry.dimensions = 3;
	geometry.diffusion_steps = 2;
	geometry.kernel_scale = 0.0;
	geometry.max_dense_records = records.size();

	const auto targets = mtrc::modify::map::diffusion_coordinate_targets<decltype(space), double>(
		space, geometry, "distance_table_pairwise_distances", "exponential_affinity_kernel",
		"lazy_row_normalized_diffusion_operator");
	assert(targets.record_count == records.size());
	assert(targets.coordinates.size() == records.size());
	assert(targets.dense_distance_evaluations == records.size() * records.size());

	mtrc::modify::map::CoordinateCalibrationSpec<double> calibration;
	calibration.steps = options.calibration_steps;
	calibration.batch_size = std::min<std::size_t>(128, records.size());
	calibration.seed = 71;
	calibration.shuffle = true;
	calibration.gradient_clip_norm = 20.0;

	const auto pipeline_plan = mtrc::modify::compose::parametric_diffusion_coordinate_pipeline_builder(0.05, 1.0)
								   .use_distance_table_pairwise_distances()
								   .use_exponential_affinity_kernel()
								   .use_lazy_row_normalized_diffusion_operator()
								   .plan();

	mtrc::solve::parametric::LinearCoordinateSolverSpec<double> solver_spec;
	solver_spec.input_dimensions = records.front().size();
	solver_spec.coordinate_dimensions = geometry.dimensions;
	solver_spec.initial_weight_scale = 0.015;
	solver_spec.learning_rate = 0.0025;
	solver_spec.random_initialization = true;
	solver_spec.seed = 17;

	auto pipeline = mtrc::modify::map::parametric_diffusion_coordinates(
		mtrc::solve::parametric::make_linear_coordinate_solver(solver_spec), geometry, calibration, pipeline_plan);
	auto mapping_artifact = mtrc::modify::map::derive_from(pipeline, space);
	auto latent = mtrc::modify::map::transform(mapping_artifact, space);
	assert(latent.space.size() == records.size());
	assert(latent.space.record(latent.space.id(0)).size() == geometry.dimensions);

	const auto &report = mapping_artifact.calibration_report();
	const double initial_coordinate_error = mtrc::solve::parametric::coordinate_calibration_target_error(
		report, mtrc::solve::parametric::CoordinateCalibrationPoint::first);
	const double final_coordinate_error = mtrc::solve::parametric::coordinate_calibration_target_error(
		report, mtrc::solve::parametric::CoordinateCalibrationPoint::last);
	assert(std::isfinite(initial_coordinate_error));
	assert(std::isfinite(final_coordinate_error));

	if (options.export_files) {
		std::filesystem::create_directories(options.export_dir);
		write_coordinates_csv(options.export_dir / "coordinates.csv", records_with_labels, latent.space, targets);
		write_summary_csv(options.export_dir / "summary.csv", options, records.size(), targets.dense_distance_evaluations,
						  initial_coordinate_error, final_coordinate_error);
		write_visual_json(options.export_dir / "visual.json", records_with_labels, latent.space);
		write_svg(options.export_dir / "hero-overview.svg", records_with_labels, latent.space);
	}

	std::cout << "mnist diffusion coordinate dataset = MNIST train IDX\n";
	std::cout << "mnist diffusion coordinate records = " << records.size() << "\n";
	std::cout << "mnist diffusion coordinate per class = " << options.per_class << "\n";
	std::cout << "mnist diffusion coordinate metric = Euclidean(normalized_pixels)\n";
	std::cout << "mnist diffusion coordinate dense evaluations = " << targets.dense_distance_evaluations << "\n";
	std::cout << "mnist diffusion coordinate calibration steps = " << options.calibration_steps << "\n";
	std::cout << "mnist diffusion coordinate initial coordinate target mse = " << initial_coordinate_error << "\n";
	std::cout << "mnist diffusion coordinate final coordinate target mse = " << final_coordinate_error << "\n";
	if (options.export_files) {
		std::cout << "mnist diffusion coordinate export dir = " << options.export_dir.string() << "\n";
	}
}
