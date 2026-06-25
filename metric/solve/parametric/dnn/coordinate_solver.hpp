// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef METRIC_SOLVE_PARAMETRIC_DNN_COORDINATE_SOLVER_HPP
#define METRIC_SOLVE_PARAMETRIC_DNN_COORDINATE_SOLVER_HPP

#include <cstddef>
#include <cstdint>
#include <memory>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <metric/core/metadata.hpp>
#include <metric/solve/parametric/dnn.hpp>

namespace mtrc::solve::parametric {

template <class Scalar> using CoordinateSolver = dnn::AutoencoderModel<Scalar>;
template <class Scalar> using CoordinateObjective = dnn::CompositeLoss<Scalar>;
template <class Scalar> using CoordinateCalibrationConfig = dnn::TrainingSpec<Scalar>;
template <class Scalar> using CoordinateCalibrationReport = dnn::TrainingReport<Scalar>;
template <class Scalar> using CoordinateDataset = dnn::EncodedDataset<Scalar>;
template <class Scalar> using CoordinateSolverArtifact = dnn::NativeAutoencoderArtifact<Scalar>;
template <class Record, class Scalar> using CoordinateRecordCodec = dnn::VectorRecordCodec<Record, Scalar>;
template <class Record, class Scalar, class Encoder>
using EncodedCoordinateRecordCodec = dnn::RecordCoordinateCodec<Record, Scalar, Encoder>;
template <class Scalar>
using CoordinateTargetTable = typename dnn::BottleneckCoordinateMSELoss<Scalar>::target_table_type;

enum class CoordinateCalibrationPoint { first, last };

template <class Scalar> struct LinearCoordinateSolverSpec {
	std::size_t input_dimensions{0};
	std::size_t coordinate_dimensions{0};
	Scalar initial_weight_scale{Scalar(0.01)};
	Scalar learning_rate{Scalar(0.001)};
	Scalar optimizer_epsilon{Scalar(1.0e-8)};
	Scalar optimizer_momentum{Scalar(0)};
	bool random_initialization{false};
	std::uint64_t seed{17};
	std::vector<Scalar> encoder_weights;
	std::vector<Scalar> encoder_bias;
	std::vector<Scalar> decoder_weights;
	std::vector<Scalar> decoder_bias;
};

template <class Scalar>
auto coordinate_calibration_config(std::size_t steps, std::size_t batch_size, bool shuffle, std::uint64_t seed,
								   Scalar gradient_clip_norm) -> CoordinateCalibrationConfig<Scalar>
{
	CoordinateCalibrationConfig<Scalar> config;
	config.epochs = steps;
	config.batch_size = batch_size;
	config.shuffle = shuffle;
	config.seed = seed;
	config.gradient_clip_norm = gradient_clip_norm;
	return config;
}

template <class Scalar>
auto coordinate_calibration_report(const CoordinateSolver<Scalar> &solver) -> const CoordinateCalibrationReport<Scalar> &
{
	return solver.training_report();
}

template <class Scalar>
auto coordinate_calibration_step_count(const CoordinateCalibrationReport<Scalar> &report) -> std::size_t
{
	return report.epochs.size();
}

template <class Scalar>
auto coordinate_calibration_term(const CoordinateCalibrationReport<Scalar> &report, const std::string &name,
								 CoordinateCalibrationPoint point) -> Scalar
{
	if (report.epochs.empty()) {
		throw std::invalid_argument("coordinate calibration report is empty");
	}
	const auto &step = (point == CoordinateCalibrationPoint::first) ? report.epochs.front() : report.epochs.back();
	for (const auto &term : step.terms) {
		if (term.name == name) {
			return term.value;
		}
	}
	throw std::invalid_argument("coordinate calibration report is missing term: " + name);
}

template <class Scalar>
auto coordinate_calibration_target_error(const CoordinateCalibrationReport<Scalar> &report,
										 CoordinateCalibrationPoint point) -> Scalar
{
	return coordinate_calibration_term(report, "bottleneck_coordinate_mse", point);
}

template <class Scalar> auto coordinate_solver_input_dimension(const CoordinateSolver<Scalar> &solver) -> std::size_t
{
	return solver.network().layers.at(solver.topology().input_layer)->getInputSize();
}

template <class Scalar>
auto make_linear_coordinate_solver(const LinearCoordinateSolverSpec<Scalar> &spec) -> CoordinateSolver<Scalar>
{
	if (spec.input_dimensions == 0) {
		throw std::invalid_argument("linear coordinate solver input dimensions must be positive");
	}
	if (spec.coordinate_dimensions == 0) {
		throw std::invalid_argument("linear coordinate solver coordinate dimensions must be positive");
	}
	if (spec.learning_rate <= Scalar(0)) {
		throw std::invalid_argument("linear coordinate solver learning rate must be positive");
	}
	if (spec.optimizer_epsilon <= Scalar(0)) {
		throw std::invalid_argument("linear coordinate solver optimizer epsilon must be positive");
	}

	dnn::Network<Scalar> network;
	dnn::FullyConnected<Scalar, dnn::Identity<Scalar>> encoder(spec.input_dimensions, spec.coordinate_dimensions);
	dnn::FullyConnected<Scalar, dnn::Identity<Scalar>> decoder(spec.coordinate_dimensions, spec.input_dimensions);

	if (spec.random_initialization) {
		std::mt19937 rng(static_cast<std::mt19937::result_type>(spec.seed));
		encoder.init(Scalar(0), spec.initial_weight_scale, rng);
		decoder.init(Scalar(0), spec.initial_weight_scale, rng);
	} else {
		encoder.initConstant(spec.initial_weight_scale, Scalar(0));
		decoder.initConstant(spec.initial_weight_scale, Scalar(0));
	}

	if (!spec.encoder_weights.empty() || !spec.encoder_bias.empty()) {
		if (spec.encoder_weights.size() != spec.input_dimensions * spec.coordinate_dimensions) {
			throw std::invalid_argument("linear coordinate solver encoder weight count does not match dimensions");
		}
		if (spec.encoder_bias.size() != spec.coordinate_dimensions) {
			throw std::invalid_argument("linear coordinate solver encoder bias count does not match dimensions");
		}
		encoder.setParameters({spec.encoder_weights, spec.encoder_bias});
	}
	if (!spec.decoder_weights.empty() || !spec.decoder_bias.empty()) {
		if (spec.decoder_weights.size() != spec.coordinate_dimensions * spec.input_dimensions) {
			throw std::invalid_argument("linear coordinate solver decoder weight count does not match dimensions");
		}
		if (spec.decoder_bias.size() != spec.input_dimensions) {
			throw std::invalid_argument("linear coordinate solver decoder bias count does not match dimensions");
		}
		decoder.setParameters({spec.decoder_weights, spec.decoder_bias});
	}

	network.addLayer(encoder);
	network.addLayer(decoder);
	network.setOutput(dnn::RegressionMSE<Scalar>());
	network.setOptimizer(dnn::RMSProp<Scalar>(spec.learning_rate, spec.optimizer_epsilon, spec.optimizer_momentum));
	return CoordinateSolver<Scalar>(std::move(network));
}

template <class Scalar>
auto coordinate_solver_objective(const CoordinateSolver<Scalar> &solver, const CoordinateTargetTable<Scalar> &targets,
								 std::size_t target_dimensions, Scalar reconstruction_weight, Scalar geometry_weight)
	-> CoordinateObjective<Scalar>
{
	if (reconstruction_weight < Scalar(0) || geometry_weight < Scalar(0)) {
		throw std::invalid_argument("parametric coordinate objective weights must be non-negative");
	}
	if (reconstruction_weight == Scalar(0) && geometry_weight == Scalar(0)) {
		throw std::invalid_argument("parametric coordinate objective requires at least one positive weight");
	}
	if (target_dimensions != solver.latent_dimension()) {
		throw std::invalid_argument("coordinate target dimensions must match the coordinate dimension");
	}

	CoordinateObjective<Scalar> objective;
	if (reconstruction_weight > Scalar(0)) {
		objective.add(std::make_shared<dnn::ReconstructionMSELoss<Scalar>>(), reconstruction_weight);
	}
	if (geometry_weight > Scalar(0)) {
		objective.add(std::make_shared<dnn::BottleneckCoordinateMSELoss<Scalar>>(solver.topology().bottleneck_layer,
																				 targets),
					  geometry_weight);
	}
	return objective;
}

template <class Scalar>
auto calibrate_coordinate_solver(CoordinateSolver<Scalar> &solver, const CoordinateDataset<Scalar> &dataset,
								 const CoordinateObjective<Scalar> &objective,
								 const CoordinateCalibrationConfig<Scalar> &config)
	-> CoordinateCalibrationReport<Scalar>
{
	const dnn::NativeDnnTrainer<Scalar> calibrator;
	return calibrator.fit(solver, dataset, objective, config);
}

template <class Scalar, class Codec>
auto make_coordinate_solver_artifact(const CoordinateSolver<Scalar> &solver, const Codec &codec,
									 const CoordinateObjective<Scalar> &objective,
									 const CoordinateCalibrationConfig<Scalar> &config,
									 std::size_t source_record_count, std::uint64_t source_space_version = 0,
									 std::string artifact_format = "metric.coordinate_solver_artifact",
									 mtrc::core::Metadata additional_manifest = mtrc::core::Metadata::object())
	-> CoordinateSolverArtifact<Scalar>
{
	return dnn::make_native_autoencoder_artifact(solver, codec, objective, config, source_record_count,
												source_space_version, std::move(artifact_format),
												std::move(additional_manifest));
}

template <class Scalar>
auto load_coordinate_solver_model(const CoordinateSolverArtifact<Scalar> &artifact) -> CoordinateSolver<Scalar>
{
	return dnn::load_native_autoencoder_model(artifact);
}

template <class Record, class Scalar, class Encoder>
auto make_coordinate_record_codec(std::size_t coordinate_count, Encoder encoder,
								  std::string codec_name = "record_coordinate_codec")
	-> EncodedCoordinateRecordCodec<Record, Scalar, Encoder>
{
	return dnn::make_record_coordinate_codec<Record, Scalar, Encoder>(
		coordinate_count, std::move(encoder), std::move(codec_name));
}

} // namespace mtrc::solve::parametric

#endif // METRIC_SOLVE_PARAMETRIC_DNN_COORDINATE_SOLVER_HPP
