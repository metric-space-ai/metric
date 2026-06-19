// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_MAPPINGS_NATIVE_PHATE_AUTOENCODER_HPP
#define _METRIC_MAPPINGS_NATIVE_PHATE_AUTOENCODER_HPP

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <blaze/Math.h>

#include "../core/concepts.hpp"
#include "../core/record_id.hpp"
#include "../utils/dnn.hpp"
#include "native_autoencoder.hpp"

namespace metric::mappings {

template <typename Scalar> struct PhateGeometrySpec {
	std::size_t dimensions{1};
	std::size_t diffusion_steps{1};
	Scalar kernel_scale{0};
	Scalar epsilon{Scalar(1.0e-12)};
};

template <typename Scalar> struct PhateGeometryTargets {
	using target_table_type = typename dnn::BottleneckCoordinateMSELoss<Scalar>::target_table_type;

	target_table_type coordinates;
	std::size_t dimensions{0};
	std::size_t diffusion_steps{0};
	Scalar kernel_scale{0};
	std::string method{"diffusion_potential_anchor_coordinates"};
};

namespace detail {

template <typename Scalar> auto normalize_rows(blaze::DynamicMatrix<Scalar> &matrix) -> void
{
	for (std::size_t row = 0; row < matrix.rows(); ++row) {
		Scalar total{0};
		for (std::size_t column = 0; column < matrix.columns(); ++column) {
			total += matrix(row, column);
		}
		if (total == Scalar(0)) {
			throw std::invalid_argument("PHATE affinity row has zero mass");
		}
		for (std::size_t column = 0; column < matrix.columns(); ++column) {
			matrix(row, column) /= total;
		}
	}
}

template <typename Scalar> auto diffusion_power(blaze::DynamicMatrix<Scalar> transition, std::size_t steps)
	-> blaze::DynamicMatrix<Scalar>
{
	if (steps == 0) {
		return transition;
	}

	auto diffused = transition;
	for (std::size_t step = 1; step < steps; ++step) {
		diffused = diffused * transition;
	}
	return diffused;
}

template <typename Scalar> auto scale_coordinates(std::vector<std::vector<Scalar>> &coordinates) -> void
{
	if (coordinates.empty()) {
		return;
	}

	const auto dimensions = coordinates.front().size();
	for (std::size_t dimension = 0; dimension < dimensions; ++dimension) {
		Scalar mean{0};
		for (const auto &record : coordinates) {
			mean += record[dimension];
		}
		mean /= static_cast<Scalar>(coordinates.size());

		Scalar max_abs{0};
		for (auto &record : coordinates) {
			record[dimension] -= mean;
			max_abs = std::max(max_abs, static_cast<Scalar>(std::abs(record[dimension])));
		}
		if (max_abs > Scalar(0)) {
			for (auto &record : coordinates) {
				record[dimension] /= max_abs;
			}
		}
	}
}

} // namespace detail

template <typename Space, typename Scalar = double, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto phate_geometry_targets(const Space &space, PhateGeometrySpec<Scalar> spec = {}) -> PhateGeometryTargets<Scalar>
{
	if (space.empty()) {
		throw std::invalid_argument("PHATE geometry targets require a non-empty space");
	}
	if (spec.dimensions == 0) {
		throw std::invalid_argument("PHATE geometry target dimensions must be positive");
	}
	if (spec.diffusion_steps == 0) {
		throw std::invalid_argument("PHATE geometry diffusion steps must be positive");
	}
	if (spec.epsilon <= Scalar(0)) {
		throw std::invalid_argument("PHATE geometry epsilon must be positive");
	}

	const auto record_count = space.size();
	blaze::DynamicMatrix<Scalar> distances(record_count, record_count);
	Scalar positive_distance_sum{0};
	std::size_t positive_distance_count{0};

	for (std::size_t row = 0; row < record_count; ++row) {
		for (std::size_t column = 0; column < record_count; ++column) {
			const auto distance = static_cast<Scalar>(space.distance(space.id(row), space.id(column)));
			distances(row, column) = distance;
			if (distance > Scalar(0)) {
				positive_distance_sum += distance;
				++positive_distance_count;
			}
		}
	}

	const Scalar scale = spec.kernel_scale > Scalar(0)
							 ? spec.kernel_scale
							 : (positive_distance_count == 0
									? Scalar(1)
									: positive_distance_sum / static_cast<Scalar>(positive_distance_count));

	blaze::DynamicMatrix<Scalar> affinity(record_count, record_count);
	for (std::size_t row = 0; row < record_count; ++row) {
		for (std::size_t column = 0; column < record_count; ++column) {
			const auto scaled = distances(row, column) / scale;
			affinity(row, column) = std::exp(-(scaled * scaled));
		}
	}
	detail::normalize_rows(affinity);

	const auto diffused = detail::diffusion_power(affinity, spec.diffusion_steps);
	blaze::DynamicMatrix<Scalar> potential(record_count, record_count);
	for (std::size_t row = 0; row < record_count; ++row) {
		for (std::size_t column = 0; column < record_count; ++column) {
			potential(row, column) = -std::log(std::max(diffused(row, column), spec.epsilon));
		}
	}

	std::vector<std::vector<Scalar>> coordinate_rows(record_count, std::vector<Scalar>(spec.dimensions, Scalar(0)));
	for (std::size_t dimension = 0; dimension < spec.dimensions; ++dimension) {
		const auto anchor = std::min(dimension, record_count - 1);
		for (std::size_t row = 0; row < record_count; ++row) {
			coordinate_rows[row][dimension] = potential(row, anchor);
		}
	}
	detail::scale_coordinates(coordinate_rows);

	PhateGeometryTargets<Scalar> targets;
	targets.dimensions = spec.dimensions;
	targets.diffusion_steps = spec.diffusion_steps;
	targets.kernel_scale = scale;
	for (std::size_t row = 0; row < record_count; ++row) {
		targets.coordinates.emplace(space.id(row), coordinate_rows[row]);
	}
	return targets;
}

template <typename Scalar> class NativePhateAutoencoderMapping {
  public:
	using scalar_type = Scalar;

	NativePhateAutoencoderMapping(dnn::AutoencoderModel<scalar_type> prototype,
								  PhateGeometrySpec<scalar_type> geometry_spec,
								  dnn::TrainingSpec<scalar_type> training_spec = {},
								  scalar_type reconstruction_weight = scalar_type(1),
								  scalar_type geometry_weight = scalar_type(1))
		: prototype_(std::move(prototype))
		, geometry_spec_(std::move(geometry_spec))
		, training_spec_(std::move(training_spec))
		, reconstruction_weight_(reconstruction_weight)
		, geometry_weight_(geometry_weight)
	{
		if (reconstruction_weight_ < scalar_type(0) || geometry_weight_ < scalar_type(0)) {
			throw std::invalid_argument("native PHATE autoencoder loss weights must be non-negative");
		}
		if (reconstruction_weight_ == scalar_type(0) && geometry_weight_ == scalar_type(0)) {
			throw std::invalid_argument("native PHATE autoencoder requires at least one positive loss weight");
		}
	}

	template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
	auto fit(const Space &space) const -> NativeAutoencoderModel<typename Space::record_type, scalar_type>
	{
		if (space.empty()) {
			throw std::invalid_argument("cannot fit native PHATE autoencoder mapping on an empty space");
		}

		using record_type = typename Space::record_type;
		using model_type = NativeAutoencoderModel<record_type, scalar_type>;
		using codec_type = typename model_type::codec_type;

		codec_type codec(space.records().front().size());
		auto features = codec.encode_batch(space.records());
		std::vector<RecordId> ids;
		ids.reserve(space.size());
		for (std::size_t index = 0; index < space.size(); ++index) {
			ids.push_back(space.id(index));
		}

		auto model = prototype_.clone();
		const auto targets = phate_geometry_targets<Space, scalar_type>(space, geometry_spec_);
		if (targets.dimensions != model.latent_dimension()) {
			throw std::invalid_argument("PHATE target dimensions must match the autoencoder bottleneck dimension");
		}

		dnn::CompositeLoss<scalar_type> objective;
		if (reconstruction_weight_ > scalar_type(0)) {
			objective.add(std::make_shared<dnn::ReconstructionMSELoss<scalar_type>>(), reconstruction_weight_);
		}
		if (geometry_weight_ > scalar_type(0)) {
			objective.add(std::make_shared<dnn::BottleneckCoordinateMSELoss<scalar_type>>(
							  model.topology().bottleneck_layer, targets.coordinates),
						  geometry_weight_);
		}

		dnn::EncodedDataset<scalar_type> dataset(std::move(ids), std::move(features), space.version());
		const dnn::NativeDnnTrainer<scalar_type> trainer;
		trainer.fit(model, dataset, objective, training_spec_);
		return model_type(std::move(model), std::move(codec), space.size());
	}

  private:
	dnn::AutoencoderModel<scalar_type> prototype_;
	PhateGeometrySpec<scalar_type> geometry_spec_;
	dnn::TrainingSpec<scalar_type> training_spec_;
	scalar_type reconstruction_weight_{1};
	scalar_type geometry_weight_{1};
};

template <typename Scalar>
auto native_phate_autoencoder(dnn::AutoencoderModel<Scalar> model, PhateGeometrySpec<Scalar> geometry_spec,
							  dnn::TrainingSpec<Scalar> training_spec = {},
							  Scalar reconstruction_weight = Scalar(1), Scalar geometry_weight = Scalar(1))
	-> NativePhateAutoencoderMapping<Scalar>
{
	return NativePhateAutoencoderMapping<Scalar>(std::move(model), std::move(geometry_spec),
												 std::move(training_spec), reconstruction_weight, geometry_weight);
}

} // namespace metric::mappings

#endif
