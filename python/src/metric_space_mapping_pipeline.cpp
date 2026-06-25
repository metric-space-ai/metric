// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// Fresh adapter-only binding for the Metric-Space Mapping Pipeline.
//
// This binding marshals Python row data into the native C++ finite metric-space
// coordinate pipeline and exposes its interchangeable components, lineage, and
// diagnostics. It contains NO coordinate math: diffusion, coordinate calibration,
// and neighbor-preservation scoring all run in the native engine.

#include "metric/metric/catalog.hpp"
#include "metric/engine.hpp"
#include "metric/solve/parametric/dnn.hpp"

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
using source_space_type = decltype(mtrc::make_space(std::declval<records_type>(), mtrc::Euclidean<double>{}));
using coordinate_artifact_type = mtrc::modify::map::ParametricCoordinateMapArtifact<record_type, double>;

auto records_from_array(py::array_t<double, py::array::c_style | py::array::forcecast> array) -> records_type
{
	const auto info = array.request();
	if (info.ndim != 2) {
		throw std::invalid_argument("mapping pipeline vector records must be a 2D numeric array");
	}
	const auto rows = static_cast<std::size_t>(info.shape[0]);
	const auto columns = static_cast<std::size_t>(info.shape[1]);
	if (rows == 0 || columns == 0) {
		throw std::invalid_argument("mapping pipeline vector records must be non-empty");
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
		throw std::invalid_argument("mapping pipeline vector records must be non-empty");
	}
	const auto source_dimension = records.front().size();
	if (source_dimension == 0) {
		throw std::invalid_argument("mapping pipeline vector records must have at least one coordinate");
	}
	for (const auto &record : records) {
		if (record.size() != source_dimension) {
			throw std::invalid_argument("mapping pipeline vector records must have consistent dimensions");
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

auto normalize_distance_provider(const std::string &distance_provider) -> std::string
{
	if (distance_provider == "exact_metric_space_distance_provider" || distance_provider == "exact_space_distances") {
		return "exact_space_distances";
	}
	if (distance_provider == "matrix_cache_distance_provider" ||
		distance_provider == "distance_table_pairwise_distances") {
		return "distance_table_pairwise_distances";
	}
	throw std::invalid_argument("unsupported mapping pipeline distance provider");
}

auto build_pipeline_plan(double reconstruction_weight, double geometry_weight, const std::string &distance_provider,
						 const std::string &affinity_kernel, const std::string &diffusion_operator)
	-> mtrc::modify::compose::PipelinePlan
{
	const auto pairwise_distances = normalize_distance_provider(distance_provider);
	auto builder = mtrc::modify::compose::parametric_diffusion_coordinate_pipeline_builder(reconstruction_weight,
																					 geometry_weight);
	if (pairwise_distances == "distance_table_pairwise_distances") {
		builder.use_distance_table_pairwise_distances();
	}
	if (affinity_kernel == "exponential_affinity_kernel") {
		builder.use_exponential_affinity_kernel();
	} else if (affinity_kernel != "gaussian_affinity_kernel") {
		throw std::invalid_argument("unsupported mapping pipeline affinity kernel");
	}
	if (diffusion_operator == "lazy_row_normalized_diffusion_operator") {
		builder.use_lazy_row_normalized_diffusion_operator();
	} else if (diffusion_operator != "row_normalized_diffusion_operator") {
		throw std::invalid_argument("unsupported mapping pipeline diffusion operator");
	}
	return builder.plan();
}

auto plan_components_to_python(const mtrc::modify::compose::PipelinePlan &plan) -> py::list
{
	py::list components;
	for (const auto &component : plan.components()) {
		py::dict entry;
		entry["role"] = component.role;
		entry["name"] = component.name;
		entry["contributes_to_artifact"] = component.contributes_to_artifact;
		components.append(std::move(entry));
	}
	return components;
}

auto make_dense_layer(std::size_t input_size, std::size_t output_size, double scale)
	-> mtrc::solve::parametric::dnn::FullyConnected<double, mtrc::solve::parametric::dnn::Identity<double>>
{
	mtrc::solve::parametric::dnn::FullyConnected<double, mtrc::solve::parametric::dnn::Identity<double>> layer(
		input_size, output_size);
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
		throw std::invalid_argument("mapping pipeline dimensions must be positive");
	}
	mtrc::solve::parametric::dnn::Network<double> network;
	network.addLayer(make_dense_layer(input_dimension, latent_dimension, 0.10));
	network.addLayer(make_dense_layer(latent_dimension, input_dimension, 0.09));
	network.setOutput(mtrc::solve::parametric::dnn::RegressionMSE<double>());
	network.setOptimizer(mtrc::solve::parametric::dnn::RMSProp<double>(learning_rate, 1.0e-8, 0.0));
	return network;
}

auto training_report_to_python(const mtrc::solve::parametric::dnn::TrainingReport<double> &report) -> py::dict
{
	py::list steps;
	for (const auto &step : report.epochs) {
		py::dict step_result;
		step_result["step"] = step.epoch;
		step_result["objective_value"] = step.total_loss;
		steps.append(std::move(step_result));
	}
	py::dict result;
	result["step_count"] = report.epochs.size();
	result["stopped_early"] = report.stopped_early;
	result["steps"] = std::move(steps);
	return result;
}

// Pipeline artifact handle: an adapter over the native mapping artifact. It keeps the
// source records only so the native neighbor-preservation diagnostic can rebuild
// the source finite metric space; it computes nothing in Python.
class PyMetricSpaceMappingArtifact {
  public:
	PyMetricSpaceMappingArtifact(coordinate_artifact_type artifact, records_type source_records)
		: artifact_(std::move(artifact)), source_records_(std::move(source_records))
	{
	}

	auto components() const -> py::list { return plan_components_to_python(artifact_.pipeline_plan()); }

	auto mapping_name() const -> std::string { return artifact_.mapping_name(); }
	auto strategy_name() const -> std::string { return artifact_.strategy_name(); }
	auto source_record_count() const -> std::size_t { return artifact_.source_record_count(); }
	auto latent_dimension() const -> std::size_t { return artifact_.latent_dimension(); }
	auto inverse_supported() const -> bool { return artifact_.codec().inverse_supported(); }

	auto lineage() const -> py::dict
	{
		py::dict result;
		result["scheme"] = "one_to_one_source_records";
		result["source_record_count"] = artifact_.source_record_count();
		result["derived_record_count"] = source_records_.size();
		return result;
	}

	auto transform(const py::object &records) const -> records_type
	{
		const auto rows = records_from_python(records);
		auto space = mtrc::make_space(rows, mtrc::Euclidean<double>{});
		return mtrc::modify::map::transform(artifact_, space).space.records();
	}

	auto inverse_transform(const py::object &latent_records) const -> records_type
	{
		return artifact_.inverse_transform(records_from_python(latent_records));
	}

	// Native neighbor-preservation recall of the derived coordinate space.
	auto neighbor_recall(std::size_t neighbor_count) const -> double
	{
		auto space = mtrc::make_space(source_records_, mtrc::Euclidean<double>{});
		const auto latent = mtrc::modify::map::transform(artifact_, space);
		return mtrc::modify::map::neighbor_preservation(space, latent, neighbor_count).recall;
	}

	auto calibration_report() const -> py::dict { return training_report_to_python(artifact_.calibration_report()); }

  private:
	coordinate_artifact_type artifact_;
	records_type source_records_;
};

auto pipeline_plan_components(double reconstruction_weight, double geometry_weight, std::string distance_provider,
							  std::string affinity_kernel, std::string diffusion_operator) -> py::list
{
	return plan_components_to_python(
		build_pipeline_plan(reconstruction_weight, geometry_weight, distance_provider, affinity_kernel,
							diffusion_operator));
}

auto derive_metric_space_mapping_pipeline(const py::object &records, std::size_t dimensions,
										  std::size_t calibration_steps, double step_size,
										  std::size_t diffusion_steps, double kernel_scale,
										  double reconstruction_weight, double geometry_weight, std::uint64_t seed,
										  std::string distance_provider, std::string affinity_kernel,
										  std::string diffusion_operator) -> PyMetricSpaceMappingArtifact
{
	const auto rows = records_from_python(records);
	const auto source_dimension = rows.front().size();
	auto space = mtrc::make_space(rows, mtrc::Euclidean<double>{});
	const auto plan = build_pipeline_plan(reconstruction_weight, geometry_weight, distance_provider, affinity_kernel,
										  diffusion_operator);

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

	auto pipeline = mtrc::modify::compose::parametric_diffusion_coordinates(
		plan, mtrc::solve::parametric::dnn::AutoencoderModel<double>(
				  make_autoencoder_network(source_dimension, dimensions, step_size)),
		geometry, calibration);
	return PyMetricSpaceMappingArtifact(mtrc::modify::map::derive_from(pipeline, space), rows);
}

} // namespace

void export_metric_space_mapping_pipeline(py::module &m)
{
	py::class_<PyMetricSpaceMappingArtifact>(m, "_MetricSpaceMappingArtifact")
		.def_property_readonly("mapping", &PyMetricSpaceMappingArtifact::mapping_name)
		.def_property_readonly("strategy", &PyMetricSpaceMappingArtifact::strategy_name)
		.def_property_readonly("source_record_count", &PyMetricSpaceMappingArtifact::source_record_count)
		.def_property_readonly("latent_dimension", &PyMetricSpaceMappingArtifact::latent_dimension)
		.def_property_readonly("inverse_supported", &PyMetricSpaceMappingArtifact::inverse_supported)
		.def_property_readonly("components", &PyMetricSpaceMappingArtifact::components)
		.def("lineage", &PyMetricSpaceMappingArtifact::lineage)
		.def("transform", &PyMetricSpaceMappingArtifact::transform, py::arg("records"))
		.def("inverse_transform", &PyMetricSpaceMappingArtifact::inverse_transform, py::arg("latent_records"))
		.def("neighbor_recall", &PyMetricSpaceMappingArtifact::neighbor_recall, py::arg("neighbor_count") = 3)
		.def("calibration_report", &PyMetricSpaceMappingArtifact::calibration_report);

	m.def("_metric_space_mapping_pipeline_plan", &pipeline_plan_components, py::arg("reconstruction_weight") = 0.05,
		  py::arg("geometry_weight") = 1.0, py::arg("distance_provider") = "exact_metric_space_distance_provider",
		  py::arg("affinity_kernel") = "gaussian_affinity_kernel",
		  py::arg("diffusion_operator") = "row_normalized_diffusion_operator");

	m.def("_metric_space_mapping_pipeline_derive", &derive_metric_space_mapping_pipeline, py::arg("records"),
		  py::arg("dimensions") = 1, py::arg("calibration_steps") = 100, py::arg("step_size") = 0.01,
		  py::arg("diffusion_steps") = 3, py::arg("kernel_scale") = 1.0, py::arg("reconstruction_weight") = 0.05,
		  py::arg("geometry_weight") = 1.0, py::arg("seed") = 29,
		  py::arg("distance_provider") = "exact_metric_space_distance_provider",
		  py::arg("affinity_kernel") = "gaussian_affinity_kernel",
		  py::arg("diffusion_operator") = "row_normalized_diffusion_operator");
}
