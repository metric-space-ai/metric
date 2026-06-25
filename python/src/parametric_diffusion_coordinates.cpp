#include "metric/metric/catalog.hpp"
#include "metric/engine.hpp"
#include "metric/solve/parametric/dnn.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

namespace {

using record_type = std::vector<double>;
using records_type = std::vector<record_type>;
using coordinate_artifact_type = mtrc::modify::map::ParametricCoordinateMapArtifact<record_type, double>;

auto records_from_array(py::array_t<double, py::array::c_style | py::array::forcecast> array) -> records_type
{
	const auto info = array.request();
	if (info.ndim != 2) {
		throw std::invalid_argument("parametric diffusion coordinate vector records must be a 2D numeric array");
	}
	const auto rows = static_cast<std::size_t>(info.shape[0]);
	const auto columns = static_cast<std::size_t>(info.shape[1]);
	if (rows == 0 || columns == 0) {
		throw std::invalid_argument("parametric diffusion coordinate vector records must be non-empty");
	}

	const auto view = array.unchecked<2>();
	records_type records(rows, record_type(columns, 0.0));
	for (std::size_t row = 0; row < rows; ++row) {
		for (std::size_t column = 0; column < columns; ++column) {
			records[row][column] = view(row, column);
		}
	}
	return records;
}

auto records_from_sequence(const records_type &records) -> records_type
{
	if (records.empty()) {
		throw std::invalid_argument("parametric diffusion coordinate vector records must be non-empty");
	}
	const auto coordinate_count = records.front().size();
	if (coordinate_count == 0) {
		throw std::invalid_argument("parametric diffusion coordinate vector records must have at least one coordinate");
	}
	for (const auto &record : records) {
		if (record.size() != coordinate_count) {
			throw std::invalid_argument("parametric diffusion coordinate vector records must have consistent dimensions");
		}
	}
	return records;
}

auto records_from_python(const py::object &records) -> records_type
{
	if (py::isinstance<py::array>(records)) {
		return records_from_array(py::cast<py::array_t<double, py::array::c_style | py::array::forcecast>>(records));
	}
	return records_from_sequence(py::cast<records_type>(records));
}

auto make_dense_layer(std::size_t input_size, std::size_t output_size, double scale)
	-> mtrc::solve::parametric::dnn::FullyConnected<double, mtrc::solve::parametric::dnn::Identity<double>>
{
	mtrc::solve::parametric::dnn::FullyConnected<double, mtrc::solve::parametric::dnn::Identity<double>> layer(input_size, output_size);
	layer.initConstant(0.0, 0.0);
	std::vector<double> weights(input_size * output_size, 0.0);
	std::vector<double> biases(output_size, 0.0);
	for (std::size_t column = 0; column < output_size; ++column) {
		for (std::size_t row = 0; row < input_size; ++row) {
			weights[column * input_size + row] = scale / static_cast<double>(1 + row + column);
		}
	}
	layer.setParameters({std::move(weights), std::move(biases)});
	return layer;
}

auto make_autoencoder_network(std::size_t input_dimension, std::size_t latent_dimension, double learning_rate)
	-> mtrc::solve::parametric::dnn::Network<double>
{
	if (input_dimension == 0 || latent_dimension == 0) {
		throw std::invalid_argument("parametric diffusion coordinate dimensions must be positive");
	}
	mtrc::solve::parametric::dnn::Network<double> network;
	network.addLayer(make_dense_layer(input_dimension, latent_dimension, 0.10));
	network.addLayer(make_dense_layer(latent_dimension, input_dimension, 0.09));
	network.setOutput(mtrc::solve::parametric::dnn::RegressionMSE<double>());
	network.setOptimizer(mtrc::solve::parametric::dnn::RMSProp<double>(learning_rate, 1.0e-8, 0.0));
	return network;
}

auto normalize_pairwise_distances_name(const std::string &distance_provider) -> std::string
{
	if (distance_provider == "exact_metric_space_distance_provider" || distance_provider == "exact_space_distances") {
		return "exact_space_distances";
	}
	if (distance_provider == "matrix_cache_distance_provider" ||
		distance_provider == "distance_table_pairwise_distances") {
		return "distance_table_pairwise_distances";
	}
	throw std::invalid_argument("unsupported parametric diffusion coordinate distance provider");
}

auto training_report_to_python(const mtrc::solve::parametric::dnn::TrainingReport<double> &report) -> py::dict
{
	py::list steps;
	for (const auto &step : report.epochs) {
		py::list terms;
		for (const auto &term : step.terms) {
			py::dict term_result;
			term_result["name"] = term.name;
			term_result["weight"] = term.weight;
			term_result["value"] = term.value;
			term_result["weighted_value"] = term.weighted_value;
			terms.append(std::move(term_result));
		}
		py::dict step_result;
		step_result["step"] = step.epoch;
		step_result["objective_value"] = step.total_loss;
		step_result["batch_count"] = step.batch_count;
		step_result["terms"] = std::move(terms);
		steps.append(std::move(step_result));
	}
	py::dict result;
	result["step_count"] = report.epochs.size();
	result["stopped_early"] = report.stopped_early;
	result["stop_reason"] = report.stop_reason;
	result["steps"] = std::move(steps);
	return result;
}

class PyParametricDiffusionCoordinateArtifact {
  public:
	explicit PyParametricDiffusionCoordinateArtifact(coordinate_artifact_type artifact) : artifact_(std::move(artifact)) {}

	auto transform(const py::object &records) const -> records_type
	{
		const auto rows = records_from_python(records);
		auto space = mtrc::make_space(rows, mtrc::Euclidean<double>{});
		auto result = mtrc::modify::map::transform(artifact_, space);
		return result.space.records();
	}

	auto inverse_transform(const py::object &latent_records) const -> records_type
	{
		return artifact_.inverse_transform(records_from_python(latent_records));
	}

	auto mapping_name() const -> std::string { return artifact_.mapping_name(); }
	auto strategy_name() const -> std::string { return artifact_.strategy_name(); }
	auto source_record_count() const -> std::size_t { return artifact_.source_record_count(); }
	auto latent_dimension() const -> std::size_t { return artifact_.latent_dimension(); }
	auto inverse_supported() const -> bool { return artifact_.codec().inverse_supported(); }
	auto calibration_report() const -> py::dict { return training_report_to_python(artifact_.calibration_report()); }

  private:
	coordinate_artifact_type artifact_;
};

auto derive_parametric_diffusion_coordinates_vectors(const py::object &records, std::size_t dimensions,
													 std::size_t calibration_steps, double step_size,
													 std::size_t diffusion_steps, double kernel_scale,
													 double reconstruction_weight, double geometry_weight,
													 std::uint64_t seed, std::string distance_provider,
													 std::string affinity_kernel, std::string diffusion_operator)
	-> PyParametricDiffusionCoordinateArtifact
{
	const auto rows = records_from_python(records);
	const auto source_dimension = rows.front().size();
	auto space = mtrc::make_space(rows, mtrc::Euclidean<double>{});
	const auto pairwise_distances = normalize_pairwise_distances_name(distance_provider);

	mtrc::modify::map::DiffusionCoordinateSpec<double> geometry;
	geometry.dimensions = dimensions;
	geometry.diffusion_steps = diffusion_steps;
	geometry.kernel_scale = kernel_scale;
	geometry.max_dense_records = rows.size();

	mtrc::modify::map::CoordinateCalibrationSpec<double> calibration;
	calibration.steps = calibration_steps;
	calibration.batch_size = rows.size();
	calibration.shuffle = false;
	calibration.seed = seed;
	calibration.gradient_clip_norm = 20.0;

	auto builder = mtrc::modify::compose::parametric_diffusion_coordinate_pipeline_builder(reconstruction_weight, geometry_weight);
	if (pairwise_distances == "distance_table_pairwise_distances") {
		builder.use_distance_table_pairwise_distances();
	} else if (pairwise_distances != "exact_space_distances") {
		throw std::invalid_argument("unsupported parametric diffusion coordinate distance provider");
	}
	if (affinity_kernel == "exponential_affinity_kernel") {
		builder.use_exponential_affinity_kernel();
	} else if (affinity_kernel != "gaussian_affinity_kernel") {
		throw std::invalid_argument("unsupported parametric diffusion coordinate affinity kernel");
	}
	if (diffusion_operator == "lazy_row_normalized_diffusion_operator") {
		builder.use_lazy_row_normalized_diffusion_operator();
	} else if (diffusion_operator != "row_normalized_diffusion_operator") {
		throw std::invalid_argument("unsupported parametric diffusion coordinate diffusion operator");
	}

	auto pipeline = mtrc::modify::compose::parametric_diffusion_coordinates(
		builder,
		mtrc::solve::parametric::dnn::AutoencoderModel<double>(
			make_autoencoder_network(source_dimension, dimensions, step_size)),
		geometry, calibration);
	return PyParametricDiffusionCoordinateArtifact(mtrc::modify::map::derive_from(pipeline, space));
}

} // namespace

void export_parametric_diffusion_coordinates(py::module &m)
{
	py::class_<PyParametricDiffusionCoordinateArtifact>(m, "_ParametricDiffusionCoordinateArtifact")
		.def_property_readonly("mapping", &PyParametricDiffusionCoordinateArtifact::mapping_name)
		.def_property_readonly("strategy", &PyParametricDiffusionCoordinateArtifact::strategy_name)
		.def_property_readonly("source_record_count", &PyParametricDiffusionCoordinateArtifact::source_record_count)
		.def_property_readonly("latent_dimension", &PyParametricDiffusionCoordinateArtifact::latent_dimension)
		.def_property_readonly("inverse_supported", &PyParametricDiffusionCoordinateArtifact::inverse_supported)
		.def("transform", &PyParametricDiffusionCoordinateArtifact::transform, py::arg("records"))
		.def("inverse_transform", &PyParametricDiffusionCoordinateArtifact::inverse_transform, py::arg("latent_records"))
		.def("calibration_report", &PyParametricDiffusionCoordinateArtifact::calibration_report);

	m.def("_parametric_diffusion_coordinate_derive_vectors", &derive_parametric_diffusion_coordinates_vectors, py::arg("records"),
		  py::arg("dimensions") = 1, py::arg("calibration_steps") = 100, py::arg("step_size") = 0.001,
		  py::arg("diffusion_steps") = 2, py::arg("kernel_scale") = 1.0, py::arg("reconstruction_weight") = 0.05,
		  py::arg("geometry_weight") = 1.0, py::arg("seed") = 23,
		  py::arg("distance_provider") = "exact_metric_space_distance_provider",
		  py::arg("affinity_kernel") = "gaussian_affinity_kernel",
		  py::arg("diffusion_operator") = "row_normalized_diffusion_operator");
}
