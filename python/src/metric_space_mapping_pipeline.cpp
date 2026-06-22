// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// Fresh adapter-only binding for the Metric-Space Mapping Pipeline.
//
// This binding marshals Python row data into the native C++ finite metric-space
// mapping pipeline and exposes its interchangeable components, lineage, and
// diagnostics. It contains NO mapping math: PHATE geometry, diffusion, neural
// training, and neighbor-preservation scoring all run in the native engine. It
// is deliberately registered under the metric.* engine namespace (the Python
// wrapper lives at metric.mapping_pipeline), NOT a neural-network product
// namespace -- the pipeline is a metric-space transform, not a DNN product.

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
using native_model_type = mtrc::modify::map::NativeAutoencoderModel<record_type, double>;

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
	const auto feature_count = records.front().size();
	if (feature_count == 0) {
		throw std::invalid_argument("mapping pipeline vector records must have at least one feature");
	}
	for (const auto &record : records) {
		if (record.size() != feature_count) {
			throw std::invalid_argument("mapping pipeline vector records must have consistent feature counts");
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
	auto builder = mtrc::modify::compose::native_phate_autoencoder_pipeline_builder(reconstruction_weight,
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
	py::list epochs;
	for (const auto &epoch : report.epochs) {
		py::dict epoch_result;
		epoch_result["epoch"] = epoch.epoch;
		epoch_result["total_loss"] = epoch.total_loss;
		epochs.append(std::move(epoch_result));
	}
	py::dict result;
	result["epoch_count"] = report.epochs.size();
	result["stopped_early"] = report.stopped_early;
	result["epochs"] = std::move(epochs);
	return result;
}

// Fitted-pipeline handle: an adapter over the native mapping model. It keeps the
// source records only so the native neighbor-preservation diagnostic can rebuild
// the source finite metric space; it computes nothing in Python.
class PyMetricSpaceMappingModel {
  public:
	PyMetricSpaceMappingModel(native_model_type model, records_type source_records)
		: model_(std::move(model)), source_records_(std::move(source_records))
	{
	}

	auto components() const -> py::list { return plan_components_to_python(model_.pipeline_plan()); }

	auto mapping_name() const -> std::string { return model_.mapping_name(); }
	auto strategy_name() const -> std::string { return model_.strategy_name(); }
	auto source_record_count() const -> std::size_t { return model_.source_record_count(); }
	auto latent_dimension() const -> std::size_t { return model_.latent_dimension(); }
	auto inverse_supported() const -> bool { return model_.codec().inverse_supported(); }

	auto lineage() const -> py::dict
	{
		py::dict result;
		result["scheme"] = "one_to_one_source_records";
		result["source_record_count"] = model_.source_record_count();
		result["derived_record_count"] = source_records_.size();
		return result;
	}

	auto transform(const py::object &records) const -> records_type
	{
		const auto rows = records_from_python(records);
		auto space = mtrc::make_space(rows, mtrc::Euclidean<double>{});
		return mtrc::modify::map::transform(model_, space).space.records();
	}

	auto inverse_transform(const py::object &latent_records) const -> records_type
	{
		return model_.inverse_transform(records_from_python(latent_records));
	}

	// Native neighbor-preservation recall of the fitted derived space.
	auto neighbor_recall(std::size_t neighbor_count) const -> double
	{
		auto space = mtrc::make_space(source_records_, mtrc::Euclidean<double>{});
		const auto latent = mtrc::modify::map::transform(model_, space);
		return mtrc::modify::map::neighbor_preservation(space, latent, neighbor_count).recall;
	}

	auto training_report() const -> py::dict { return training_report_to_python(model_.training_report()); }

  private:
	native_model_type model_;
	records_type source_records_;
};

auto pipeline_plan_components(double reconstruction_weight, double geometry_weight, std::string distance_provider,
							  std::string affinity_kernel, std::string diffusion_operator) -> py::list
{
	return plan_components_to_python(
		build_pipeline_plan(reconstruction_weight, geometry_weight, distance_provider, affinity_kernel,
							diffusion_operator));
}

auto fit_metric_space_mapping_pipeline(const py::object &records, std::size_t dimensions, std::size_t epochs,
									   double learning_rate, std::size_t diffusion_steps, double kernel_scale,
									   double reconstruction_weight, double geometry_weight, std::uint64_t seed,
									   std::string distance_provider, std::string affinity_kernel,
									   std::string diffusion_operator) -> PyMetricSpaceMappingModel
{
	const auto rows = records_from_python(records);
	const auto feature_count = rows.front().size();
	auto space = mtrc::make_space(rows, mtrc::Euclidean<double>{});
	const auto plan = build_pipeline_plan(reconstruction_weight, geometry_weight, distance_provider, affinity_kernel,
										  diffusion_operator);

	mtrc::modify::map::PhateGeometrySpec<double> geometry;
	geometry.dimensions = dimensions;
	geometry.diffusion_steps = diffusion_steps;
	geometry.kernel_scale = kernel_scale;
	geometry.max_dense_records = rows.size();

	mtrc::solve::parametric::dnn::TrainingSpec<double> training;
	training.epochs = epochs;
	training.batch_size = rows.size();
	training.shuffle = false;
	training.seed = seed;
	training.gradient_clip_norm = 20.0;

	auto pipeline = mtrc::modify::compose::native_phate_autoencoder(
		plan, mtrc::solve::parametric::dnn::AutoencoderModel<double>(make_autoencoder_network(feature_count, dimensions, learning_rate)),
		geometry, training);
	return PyMetricSpaceMappingModel(mtrc::modify::map::fit(pipeline, space), rows);
}

} // namespace

void export_metric_space_mapping_pipeline(py::module &m)
{
	py::class_<PyMetricSpaceMappingModel>(m, "_MetricSpaceMappingModel")
		.def_property_readonly("mapping", &PyMetricSpaceMappingModel::mapping_name)
		.def_property_readonly("strategy", &PyMetricSpaceMappingModel::strategy_name)
		.def_property_readonly("source_record_count", &PyMetricSpaceMappingModel::source_record_count)
		.def_property_readonly("latent_dimension", &PyMetricSpaceMappingModel::latent_dimension)
		.def_property_readonly("inverse_supported", &PyMetricSpaceMappingModel::inverse_supported)
		.def_property_readonly("components", &PyMetricSpaceMappingModel::components)
		.def("lineage", &PyMetricSpaceMappingModel::lineage)
		.def("transform", &PyMetricSpaceMappingModel::transform, py::arg("records"))
		.def("inverse_transform", &PyMetricSpaceMappingModel::inverse_transform, py::arg("latent_records"))
		.def("neighbor_recall", &PyMetricSpaceMappingModel::neighbor_recall, py::arg("neighbor_count") = 3)
		.def("training_report", &PyMetricSpaceMappingModel::training_report);

	m.def("_metric_space_mapping_pipeline_plan", &pipeline_plan_components, py::arg("reconstruction_weight") = 0.05,
		  py::arg("geometry_weight") = 1.0, py::arg("distance_provider") = "exact_metric_space_distance_provider",
		  py::arg("affinity_kernel") = "gaussian_affinity_kernel",
		  py::arg("diffusion_operator") = "row_normalized_diffusion_operator");

	m.def("_metric_space_mapping_pipeline_fit", &fit_metric_space_mapping_pipeline, py::arg("records"),
		  py::arg("dimensions") = 1, py::arg("epochs") = 100, py::arg("learning_rate") = 0.01,
		  py::arg("diffusion_steps") = 3, py::arg("kernel_scale") = 1.0, py::arg("reconstruction_weight") = 0.05,
		  py::arg("geometry_weight") = 1.0, py::arg("seed") = 29,
		  py::arg("distance_provider") = "exact_metric_space_distance_provider",
		  py::arg("affinity_kernel") = "gaussian_affinity_kernel",
		  py::arg("diffusion_operator") = "row_normalized_diffusion_operator");
}
