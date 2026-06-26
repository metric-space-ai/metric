/*

PANDA presents

███╗   ███╗███████╗████████╗██████╗ ██╗ ██████╗    ███████╗██████╗  █████╗  ██████╗███████╗
████╗ ████║██╔════╝╚══██╔══╝██╔══██╗██║██╔════╝    ██╔════╝██╔══██╗██╔══██╗██╔════╝██╔════╝
██╔████╔██║█████╗     ██║   ██████╔╝██║██║         ███████╗██████╔╝███████║██║     █████╗
██║╚██╔╝██║██╔══╝     ██║   ██╔══██╗██║██║         ╚════██║██╔═══╝ ██╔══██║██║     ██╔══╝
██║ ╚═╝ ██║███████╗   ██║   ██║  ██║██║╚██████╗    ███████║██║     ██║  ██║╚██████╗███████╗
╚═╝     ╚═╝╚══════╝   ╚═╝   ╚═╝  ╚═╝╚═╝ ╚═════╝    ╚══════╝╚═╝     ╚═╝  ╚═╝ ╚═════╝╚══════╝
																		Licensed under MPL 2.0.

a library for metric space data containers and corresponding basic methods

This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) PANDA Team
*/

#ifndef _METRIC_SPACE_HPP
#define _METRIC_SPACE_HPP

#include <cstddef>
#include <limits>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <metric/core/errors.hpp>

#include "space/knn_graph.hpp"
#include "space/matrix.hpp"
#include "space/partition.hpp"
#include "space/tree.hpp"
#include "space/user_api.hpp"

namespace mtrc {

template <typename RecType, typename Metric> using MatrixSpace = Matrix<RecType, Metric>;

template <typename RecType, typename Metric> using TreeSpace = Tree<RecType, Metric>;

template <typename Sample, typename Distance, typename WeightType = bool, bool isDense = false, bool isSymmetric = true>
using GraphSpace = KNNGraph<Sample, Distance, WeightType, isDense, isSymmetric>;

inline constexpr std::size_t default_max_pairwise_matrix_cells = 1'000'000;

struct pairwise_matrix_options {
	// Maximum number of cells that pairwise_distances() may materialize in memory.
	// Set to 0 only when the caller intentionally opts into an unbounded n x n matrix.
	std::size_t max_cells{default_max_pairwise_matrix_cells};
};

namespace finite_space_detail {

inline auto require_pairwise_matrix_budget(std::size_t count, pairwise_matrix_options options) -> std::size_t
{
	if (count != 0 && count > std::numeric_limits<std::size_t>::max() / count) {
		throw RepresentationError("FiniteSpace::pairwise_distances cannot represent record_count * record_count");
	}
	const auto cell_count = count * count;
	if (options.max_cells == 0 || cell_count <= options.max_cells) {
		return cell_count;
	}
	throw RepresentationError(
		std::string("FiniteSpace::pairwise_distances refused to collect a dense matrix before allocation: records=") +
		std::to_string(count) + ", cells=" + std::to_string(cell_count) +
		", max_cells=" + std::to_string(options.max_cells) +
		". Use promoted mtrc::space providers for streaming/lazy access or pass pairwise_matrix_options{0} "
		"to explicitly opt into an unbounded in-memory matrix.");
}

} // namespace finite_space_detail

template <typename RecType, typename Metric> class FiniteSpace {
  public:
	using record_type = RecType;
	using metric_type = Metric;
	using representation_type = MatrixSpace<record_type, metric_type>;
	using distance_type = typename representation_type::distType;
	using neighbor_type = std::pair<std::size_t, distance_type>;

	FiniteSpace(const std::vector<record_type> &records, metric_type metric = metric_type(),
				matrix_space_options matrix_options = {})
		: representation_(records, std::move(metric), matrix_options)
	{
	}

	template <typename Container>
	FiniteSpace(const Container &records, metric_type metric = metric_type(), matrix_space_options matrix_options = {})
		: representation_(records, std::move(metric), matrix_options)
	{
	}

	auto size() const -> std::size_t { return representation_.size(); }

	auto operator[](std::size_t index) const -> record_type { return representation_[index]; }

	auto operator()(std::size_t lhs_index, std::size_t rhs_index) const -> distance_type
	{
		return distance(lhs_index, rhs_index);
	}

	auto distance(std::size_t lhs_index, std::size_t rhs_index) const -> distance_type
	{
		return representation_(lhs_index, rhs_index);
	}

	auto pairwise_distances(pairwise_matrix_options options = {}) const -> std::vector<std::vector<distance_type>>
	{
		(void)finite_space_detail::require_pairwise_matrix_budget(size(), options);
		std::vector<std::vector<distance_type>> distances(size(), std::vector<distance_type>(size()));
		for (std::size_t row = 0; row < size(); ++row) {
			for (std::size_t col = 0; col < size(); ++col) {
				distances[row][col] = distance(row, col);
			}
		}
		return distances;
	}

	auto neighbors(const record_type &query, unsigned k = 10) const -> std::vector<neighbor_type>
	{
		return representation_.knn(query, k);
	}

	auto nearest(const record_type &query) const -> neighbor_type { return neighbors(query, 1).at(0); }

	auto within_radius(const record_type &query, distance_type radius) const -> std::vector<neighbor_type>
	{
		return representation_.rnn(query, radius);
	}

	auto knn(const record_type &query, unsigned k = 10) const -> std::vector<neighbor_type>
	{
		return neighbors(query, k);
	}

	auto nn(const record_type &query) const -> std::size_t { return nearest(query).first; }

	auto rnn(const record_type &query, distance_type radius) const -> std::vector<neighbor_type>
	{
		return within_radius(query, radius);
	}

	auto representation() const -> const representation_type & { return representation_; }

  private:
	representation_type representation_;
};

struct Space {
	template <typename Container, typename Metric>
	static auto from_records(const Container &records, Metric metric, matrix_space_options matrix_options = {})
		-> FiniteSpace<typename std::decay<typename Container::value_type>::type, Metric>
	{
		using record_type = typename std::decay<typename Container::value_type>::type;
		return FiniteSpace<record_type, Metric>(records, std::move(metric), matrix_options);
	}
};

} // namespace mtrc
#endif
