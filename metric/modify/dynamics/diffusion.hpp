// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_MODIFY_DYNAMICS_DIFFUSION_HPP
#define _METRIC_MODIFY_DYNAMICS_DIFFUSION_HPP

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <metric/core/concepts.hpp>
#include <metric/numeric/Math.h>
#include <metric/space/storage/distance_matrix.hpp>

namespace mtrc::modify::dynamics {

inline constexpr std::size_t default_diffusion_max_dense_records = 4096;

template <typename Scalar> struct DiffusionOptions {
	std::size_t diffusion_steps{1};
	std::size_t max_dense_records{default_diffusion_max_dense_records};
	Scalar kernel_scale{0};
	Scalar epsilon{Scalar(1.0e-12)};
	std::string pairwise_distances{"exact_space_distances"};
	std::string affinity_kernel{"gaussian_affinity_kernel"};
	std::string diffusion_operator{"row_normalized_diffusion_operator"};
};

template <typename Scalar> struct DiffusionProcess {
	using scalar_type = Scalar;
	using matrix_type = numeric::DynamicMatrix<scalar_type>;

	matrix_type transition;
	matrix_type diffused;
	matrix_type potential;
	std::size_t diffusion_steps{0};
	std::size_t record_count{0};
	std::size_t dense_distance_evaluations{0};
	std::size_t max_dense_records{0};
	scalar_type kernel_scale{0};
	std::string pairwise_distances{"exact_space_distances"};
	std::string affinity_kernel{"gaussian_affinity_kernel"};
	std::string diffusion_operator{"row_normalized_diffusion_operator"};
};

inline auto is_supported_pairwise_distances(const std::string &name) -> bool
{
	return name == "exact_space_distances" || name == "distance_table_pairwise_distances";
}

inline auto is_supported_affinity_kernel(const std::string &name) -> bool
{
	return name == "gaussian_affinity_kernel" || name == "exponential_affinity_kernel";
}

inline auto is_supported_diffusion_operator(const std::string &name) -> bool
{
	return name == "row_normalized_diffusion_operator" || name == "lazy_row_normalized_diffusion_operator";
}

template <typename Scalar> auto affinity_value(Scalar scaled_distance, const std::string &affinity_kernel) -> Scalar
{
	if (affinity_kernel == "gaussian_affinity_kernel") {
		return std::exp(-(scaled_distance * scaled_distance));
	}
	if (affinity_kernel == "exponential_affinity_kernel") {
		return std::exp(-scaled_distance);
	}
	throw std::invalid_argument("unsupported diffusion affinity kernel");
}

template <typename Matrix> auto apply_diffusion_operator(Matrix &matrix, const std::string &diffusion_operator) -> void
{
	if (diffusion_operator == "row_normalized_diffusion_operator") {
		return;
	}
	if (diffusion_operator != "lazy_row_normalized_diffusion_operator") {
		throw std::invalid_argument("unsupported diffusion operator");
	}
	numeric::blend_with_identity(matrix, typename Matrix::ElementType(0.5));
}

template <typename Scalar, typename Matrix>
auto diffusion_process_from_distances(const Matrix &distances, DiffusionOptions<Scalar> options = {})
	-> DiffusionProcess<Scalar>
{
	if (distances.rows() != distances.columns()) {
		throw std::invalid_argument("diffusion process requires a square distance matrix");
	}
	if (distances.rows() == 0) {
		throw std::invalid_argument("diffusion process requires a non-empty finite space");
	}
	if (!is_supported_pairwise_distances(options.pairwise_distances)) {
		throw std::invalid_argument("unsupported diffusion distance provider");
	}
	if (!is_supported_affinity_kernel(options.affinity_kernel)) {
		throw std::invalid_argument("unsupported diffusion affinity kernel");
	}
	if (!is_supported_diffusion_operator(options.diffusion_operator)) {
		throw std::invalid_argument("unsupported diffusion operator");
	}
	if (options.diffusion_steps == 0) {
		throw std::invalid_argument("diffusion steps must be positive");
	}
	if (options.epsilon <= Scalar(0)) {
		throw std::invalid_argument("diffusion epsilon must be positive");
	}

	const auto record_count = distances.rows();
	if (options.max_dense_records > 0 && record_count > options.max_dense_records) {
		throw std::invalid_argument("diffusion dense construction exceeds max_dense_records");
	}

	const Scalar scale = options.kernel_scale > Scalar(0) ? options.kernel_scale
														 : numeric::positive_mean_or(distances, Scalar(1));

	numeric::DynamicMatrix<Scalar> transition = numeric::map(distances, [&scale, &options](Scalar distance) {
		return affinity_value(distance / scale, options.affinity_kernel);
	});
	numeric::normalize_rows(transition);
	apply_diffusion_operator(transition, options.diffusion_operator);

	const auto diffused = numeric::matrix_power(transition, options.diffusion_steps);
	numeric::DynamicMatrix<Scalar> potential = numeric::map(
		diffused, [&options](Scalar value) { return static_cast<Scalar>(-std::log(std::max(value, options.epsilon))); });

	DiffusionProcess<Scalar> process;
	process.transition = std::move(transition);
	process.diffused = std::move(diffused);
	process.potential = std::move(potential);
	process.diffusion_steps = options.diffusion_steps;
	process.record_count = record_count;
	process.dense_distance_evaluations = record_count * record_count;
	process.max_dense_records = options.max_dense_records;
	process.kernel_scale = scale;
	process.pairwise_distances = std::move(options.pairwise_distances);
	process.affinity_kernel = std::move(options.affinity_kernel);
	process.diffusion_operator = std::move(options.diffusion_operator);
	return process;
}

template <typename Space, typename Scalar = double, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto diffusion_process(const Space &space, DiffusionOptions<Scalar> options = {}) -> DiffusionProcess<Scalar>
{
	if (space.empty()) {
		throw std::invalid_argument("diffusion process requires a non-empty finite space");
	}
	if (options.max_dense_records > 0 && space.size() > options.max_dense_records) {
		throw std::invalid_argument("diffusion dense construction exceeds max_dense_records");
	}
	const auto distances = space::storage::metric_space_dense_distance_matrix<Scalar>(space);
	return diffusion_process_from_distances<Scalar>(distances, std::move(options));
}

template <typename Scalar>
auto diffusion_potential_anchor_coordinates(const DiffusionProcess<Scalar> &process, std::size_t dimensions)
	-> numeric::DynamicMatrix<Scalar>
{
	if (dimensions == 0) {
		throw std::invalid_argument("diffusion coordinate dimensions must be positive");
	}
	if (process.record_count == 0 || process.potential.rows() == 0) {
		throw std::invalid_argument("diffusion coordinates require a non-empty process");
	}

	// One anchor record per requested coordinate. NOTE: when `dimensions` exceeds
	// record_count, every extra coordinate is clamped to the last anchor column, so
	// the result has REPEATED (collinear) columns and is rank-deficient. This is
	// intentional and pinned by native_diffusion_coordinate_targets_smoke (repeated_anchor
	// spec); callers should keep the embedding dimension <= record_count - 1 for a
	// full-rank target.
	std::vector<std::size_t> anchor_columns;
	anchor_columns.reserve(dimensions);
	for (std::size_t dimension = 0; dimension < dimensions; ++dimension) {
		anchor_columns.push_back(std::min(dimension, process.record_count - 1));
	}

	auto coordinates = numeric::select_columns(process.potential, anchor_columns);
	numeric::center_columns_to_unit_max_abs(coordinates);
	return coordinates;
}

template <typename Scalar>
auto diffuse_distribution(const DiffusionProcess<Scalar> &process, const std::vector<Scalar> &distribution)
	-> std::vector<Scalar>
{
	if (distribution.size() != process.record_count) {
		throw std::invalid_argument("diffusion distribution size does not match process record count");
	}

	std::vector<Scalar> evolved(process.record_count, Scalar(0));
	for (std::size_t source = 0; source < process.record_count; ++source) {
		const auto mass = distribution[source];
		if (!std::isfinite(static_cast<double>(mass)) || mass < Scalar(0)) {
			throw std::invalid_argument("diffusion distribution entries must be finite and nonnegative");
		}
		for (std::size_t target = 0; target < process.record_count; ++target) {
			evolved[target] += mass * process.diffused(source, target);
		}
	}
	return evolved;
}

} // namespace mtrc::modify::dynamics

#endif
