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
using native_model_type = mtrc::modify::map::NativeAutoencoderModel<record_type, double>;

auto records_from_array(py::array_t<double, py::array::c_style | py::array::forcecast> array) -> records_type
{
	const auto info = array.request();
	if (info.ndim != 2) {
		throw std::invalid_argument("native PHATE-AE vector records must be a 2D numeric array");
	}
	const auto rows = static_cast<std::size_t>(info.shape[0]);
	const auto columns = static_cast<std::size_t>(info.shape[1]);
	if (rows == 0 || columns == 0) {
		throw std::invalid_argument("native PHATE-AE vector records must be non-empty");
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
		throw std::invalid_argument("native PHATE-AE vector records must be non-empty");
	}
	const auto feature_count = records.front().size();
	if (feature_count == 0) {
		throw std::invalid_argument("native PHATE-AE vector records must have at least one feature");
	}
	for (const auto &record : records) {
		if (record.size() != feature_count) {
			throw std::invalid_argument("native PHATE-AE vector records must have consistent feature counts");
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
		throw std::invalid_argument("native PHATE-AE dimensions must be positive");
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
	throw std::invalid_argument("unsupported native PHATE-AE distance provider");
}

auto training_report_to_python(const mtrc::solve::parametric::dnn::TrainingReport<double> &report) -> py::dict
{
	py::list epochs;
	for (const auto &epoch : report.epochs) {
		py::list terms;
		for (const auto &term : epoch.terms) {
			py::dict term_result;
			term_result["name"] = term.name;
			term_result["weight"] = term.weight;
			term_result["value"] = term.value;
			term_result["weighted_value"] = term.weighted_value;
			terms.append(std::move(term_result));
		}
		py::dict epoch_result;
		epoch_result["epoch"] = epoch.epoch;
		epoch_result["total_loss"] = epoch.total_loss;
		epoch_result["batch_count"] = epoch.batch_count;
		epoch_result["terms"] = std::move(terms);
		epochs.append(std::move(epoch_result));
	}
	py::dict result;
	result["epoch_count"] = report.epochs.size();
	result["stopped_early"] = report.stopped_early;
	result["stop_reason"] = report.stop_reason;
	result["epochs"] = std::move(epochs);
	return result;
}

class PyNativePhateAutoencoderModel {
  public:
	explicit PyNativePhateAutoencoderModel(native_model_type model) : model_(std::move(model)) {}

	auto transform(const py::object &records) const -> records_type
	{
		const auto rows = records_from_python(records);
		auto space = mtrc::make_space(rows, mtrc::Euclidean<double>{});
		auto result = mtrc::modify::map::transform(model_, space);
		return result.space.records();
	}

	auto inverse_transform(const py::object &latent_records) const -> records_type
	{
		return model_.inverse_transform(records_from_python(latent_records));
	}

	auto mapping_name() const -> std::string { return model_.mapping_name(); }
	auto strategy_name() const -> std::string { return model_.strategy_name(); }
	auto source_record_count() const -> std::size_t { return model_.source_record_count(); }
	auto latent_dimension() const -> std::size_t { return model_.latent_dimension(); }
	auto inverse_supported() const -> bool { return model_.codec().inverse_supported(); }
	auto training_report() const -> py::dict { return training_report_to_python(model_.training_report()); }

  private:
	native_model_type model_;
};

auto fit_native_phate_autoencoder_vectors(const py::object &records, std::size_t dimensions, std::size_t epochs,
										  double learning_rate, std::size_t diffusion_steps, double kernel_scale,
										  double reconstruction_weight, double geometry_weight, std::uint64_t seed,
										  std::string distance_provider, std::string affinity_kernel,
										  std::string diffusion_operator) -> PyNativePhateAutoencoderModel
{
	const auto rows = records_from_python(records);
	const auto feature_count = rows.front().size();
	auto space = mtrc::make_space(rows, mtrc::Euclidean<double>{});
	const auto pairwise_distances = normalize_pairwise_distances_name(distance_provider);

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

	auto builder = mtrc::modify::compose::native_phate_autoencoder_pipeline_builder(reconstruction_weight, geometry_weight);
	if (pairwise_distances == "distance_table_pairwise_distances") {
		builder.use_distance_table_pairwise_distances();
	} else if (pairwise_distances != "exact_space_distances") {
		throw std::invalid_argument("unsupported native PHATE-AE distance provider");
	}
	if (affinity_kernel == "exponential_affinity_kernel") {
		builder.use_exponential_affinity_kernel();
	} else if (affinity_kernel != "gaussian_affinity_kernel") {
		throw std::invalid_argument("unsupported native PHATE-AE affinity kernel");
	}
	if (diffusion_operator == "lazy_row_normalized_diffusion_operator") {
		builder.use_lazy_row_normalized_diffusion_operator();
	} else if (diffusion_operator != "row_normalized_diffusion_operator") {
		throw std::invalid_argument("unsupported native PHATE-AE diffusion operator");
	}

	auto pipeline = mtrc::modify::compose::native_phate_autoencoder(
		builder,
		mtrc::solve::parametric::dnn::AutoencoderModel<double>(make_autoencoder_network(feature_count, dimensions, learning_rate)),
		geometry, training);
	return PyNativePhateAutoencoderModel(mtrc::modify::map::fit(pipeline, space));
}

} // namespace

void export_native_phate_autoencoder(py::module &m)
{
	py::class_<PyNativePhateAutoencoderModel>(m, "_NativePhateAutoencoderModel")
		.def_property_readonly("mapping", &PyNativePhateAutoencoderModel::mapping_name)
		.def_property_readonly("strategy", &PyNativePhateAutoencoderModel::strategy_name)
		.def_property_readonly("source_record_count", &PyNativePhateAutoencoderModel::source_record_count)
		.def_property_readonly("latent_dimension", &PyNativePhateAutoencoderModel::latent_dimension)
		.def_property_readonly("inverse_supported", &PyNativePhateAutoencoderModel::inverse_supported)
		.def("transform", &PyNativePhateAutoencoderModel::transform, py::arg("records"))
		.def("inverse_transform", &PyNativePhateAutoencoderModel::inverse_transform, py::arg("latent_records"))
		.def("training_report", &PyNativePhateAutoencoderModel::training_report);

	m.def("_native_phate_autoencoder_fit_vectors", &fit_native_phate_autoencoder_vectors, py::arg("records"),
		  py::arg("dimensions") = 1, py::arg("epochs") = 100, py::arg("learning_rate") = 0.001,
		  py::arg("diffusion_steps") = 2, py::arg("kernel_scale") = 1.0, py::arg("reconstruction_weight") = 0.05,
		  py::arg("geometry_weight") = 1.0, py::arg("seed") = 23,
		  py::arg("distance_provider") = "exact_metric_space_distance_provider",
		  py::arg("affinity_kernel") = "gaussian_affinity_kernel",
		  py::arg("diffusion_operator") = "row_normalized_diffusion_operator");
}
