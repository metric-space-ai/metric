// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_MAPPINGS_PCFA_HPP
#define _METRIC_MAPPINGS_PCFA_HPP

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <metric/numeric.hpp>
#include <metric/core/concepts.hpp>
#include <metric/core/metric_space.hpp>
#include <metric/record/id.hpp>
#include <metric/core/result.hpp>
#include <metric/metric/catalog/vector/Standards.hpp>

namespace mtrc::modify::map {

namespace detail {

template <typename Matrix>
auto symmetric_jacobi_eigenvectors_as_rows(Matrix matrix)
	-> std::pair<std::vector<typename Matrix::ElementType>, Matrix>
{
	using value_type = typename Matrix::ElementType;
	const auto n = matrix.rows();
	if (n != matrix.columns()) {
		throw std::invalid_argument("symmetric Jacobi decomposition requires a square matrix");
	}

	Matrix vectors(n, n, value_type{});
	for (std::size_t index = 0; index < n; ++index) {
		vectors(index, index) = value_type(1);
	}

	const auto tolerance = std::numeric_limits<value_type>::epsilon() * value_type(128);
	const auto max_iterations = std::max<std::size_t>(64, n * n * 64);
	for (std::size_t iteration = 0; iteration < max_iterations; ++iteration) {
		std::size_t p = 0;
		std::size_t q = 0;
		value_type largest{};
		for (std::size_t row = 0; row < n; ++row) {
			for (std::size_t column = row + 1; column < n; ++column) {
				const auto magnitude = std::abs(matrix(row, column));
				if (magnitude > largest) {
					largest = magnitude;
					p = row;
					q = column;
				}
			}
		}
		if (largest <= tolerance) {
			break;
		}

		const auto app = matrix(p, p);
		const auto aqq = matrix(q, q);
		const auto apq = matrix(p, q);
		const auto tau = (aqq - app) / (value_type(2) * apq);
		const auto sign = tau < value_type(0) ? value_type(-1) : value_type(1);
		const auto t = sign / (std::abs(tau) + std::sqrt(value_type(1) + tau * tau));
		const auto c = value_type(1) / std::sqrt(value_type(1) + t * t);
		const auto s = t * c;

		for (std::size_t index = 0; index < n; ++index) {
			if (index == p || index == q) {
				continue;
			}
			const auto aip = matrix(index, p);
			const auto aiq = matrix(index, q);
			matrix(index, p) = c * aip - s * aiq;
			matrix(p, index) = matrix(index, p);
			matrix(index, q) = s * aip + c * aiq;
			matrix(q, index) = matrix(index, q);
		}

		matrix(p, p) = c * c * app - value_type(2) * s * c * apq + s * s * aqq;
		matrix(q, q) = s * s * app + value_type(2) * s * c * apq + c * c * aqq;
		matrix(p, q) = value_type{};
		matrix(q, p) = value_type{};

		for (std::size_t row = 0; row < n; ++row) {
			const auto vip = vectors(row, p);
			const auto viq = vectors(row, q);
			vectors(row, p) = c * vip - s * viq;
			vectors(row, q) = s * vip + c * viq;
		}
	}

	std::vector<value_type> values(n, value_type{});
	Matrix eigenvectors(n, n, value_type{});
	for (std::size_t index = 0; index < n; ++index) {
		values[index] = matrix(index, index);
		for (std::size_t column = 0; column < n; ++column) {
			eigenvectors(index, column) = vectors(column, index);
		}
	}
	return {std::move(values), std::move(eigenvectors)};
}

template <typename Record> class LinearCoordinateProjection {
  public:
	using record_type = Record;
	using value_type = typename record_type::value_type;
	using matrix_type = mtrc::numeric::DynamicMatrix<value_type>;
	using average_type = mtrc::numeric::DynamicVector<value_type, mtrc::numeric::rowVector>;

	LinearCoordinateProjection(const std::vector<record_type> &records, std::size_t component_count)
	{
		if (records.empty()) {
			throw std::invalid_argument("cannot derive linear coordinate projection from an empty space");
		}
		if (records.front().empty()) {
			throw std::invalid_argument("linear coordinate projection records must have positive coordinate count");
		}
		if (component_count == 0 || component_count > records.front().size()) {
			throw std::invalid_argument("linear coordinate projection component count is outside the record coordinate range");
		}

		auto data = records_to_matrix(records);
		averages_ = mtrc::numeric::sum<mtrc::numeric::columnwise>(data) / data.rows();
		auto centered = centered_matrix(data);
		auto covariance = mtrc::numeric::evaluate(trans(centered) * centered);
		auto eigen = symmetric_jacobi_eigenvectors_as_rows(covariance);
		auto &eigenvalues = eigen.first;
		auto &eigenvectors = eigen.second;

		std::vector<std::size_t> order(eigenvalues.size());
		for (std::size_t index = 0; index < order.size(); ++index) {
			order[index] = index;
		}
		std::sort(order.begin(), order.end(), [&](std::size_t lhs, std::size_t rhs) {
			return std::abs(eigenvalues[lhs]) > std::abs(eigenvalues[rhs]);
		});

		decoder_ = matrix_type(component_count, data.columns(), value_type{});
		for (std::size_t component = 0; component < component_count; ++component) {
			mtrc::numeric::row(decoder_, component) = mtrc::numeric::row(eigenvectors, order[component]);
		}
		encoder_ = trans(decoder_);
	}

		auto encode(const std::vector<record_type> &records) const -> std::vector<record_type>
		{
			auto data = records_to_matrix(records);
			auto centered = centered_matrix(data);
			auto encoded = mtrc::numeric::evaluate(centered * encoder_);
			return matrix_to_records(encoded);
		}

		auto decode(const std::vector<record_type> &codes, bool unshift = true) const -> std::vector<record_type>
		{
			auto code_matrix = records_to_matrix(codes);
			auto decoded = mtrc::numeric::evaluate(code_matrix * decoder_);
			auto records = matrix_to_records(decoded);
			if (unshift) {
				for (auto &record : records) {
				for (std::size_t column_index = 0; column_index < record.size(); ++column_index) {
					record[column_index] += averages_[column_index];
				}
			}
		}
		return records;
	}

  private:
	auto records_to_matrix(const std::vector<record_type> &records) const -> matrix_type
	{
		if (records.empty()) {
			return matrix_type(0, 0);
		}
		const auto coordinate_count = records.front().size();
		matrix_type result(records.size(), coordinate_count, value_type{});
		for (std::size_t row_index = 0; row_index < records.size(); ++row_index) {
			if (records[row_index].size() != coordinate_count) {
				throw std::invalid_argument("linear coordinate projection records must have consistent coordinate counts");
			}
			for (std::size_t column_index = 0; column_index < coordinate_count; ++column_index) {
				result(row_index, column_index) = records[row_index][column_index];
			}
		}
		return result;
	}

	auto matrix_to_records(const matrix_type &matrix) const -> std::vector<record_type>
	{
		std::vector<record_type> records;
		records.reserve(matrix.rows());
		for (std::size_t row_index = 0; row_index < matrix.rows(); ++row_index) {
			record_type record;
			for (std::size_t column_index = 0; column_index < matrix.columns(); ++column_index) {
				record.push_back(matrix(row_index, column_index));
			}
			records.push_back(std::move(record));
		}
		return records;
	}

	auto centered_matrix(const matrix_type &data) const -> matrix_type
	{
		matrix_type centered(data.rows(), data.columns(), value_type{});
		for (std::size_t row_index = 0; row_index < data.rows(); ++row_index) {
			for (std::size_t column_index = 0; column_index < data.columns(); ++column_index) {
				centered(row_index, column_index) = data(row_index, column_index) - averages_[column_index];
			}
		}
		return centered;
	}

	matrix_type decoder_;
	matrix_type encoder_;
	average_type averages_;
};

} // namespace detail

template <typename Record> using default_pcfa_metric_t = Euclidean<typename Record::value_type>;

template <typename Record, typename CodeMetric = default_pcfa_metric_t<Record>> class PCFACoordinateMap {
  public:
	using record_type = Record;
	using code_metric_type = CodeMetric;
	using backend_projection_type = detail::LinearCoordinateProjection<record_type>;
	using space_type = MetricSpace<record_type, code_metric_type>;
	using result_type = MappingResult<space_type>;

	PCFACoordinateMap(backend_projection_type projection, std::size_t component_count,
					  code_metric_type code_metric = code_metric_type{})
		: projection_(std::move(projection)), component_count_(component_count), code_metric_(std::move(code_metric))
	{
	}

	template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
	auto transform(const Space &space) const -> result_type
	{
		static_assert(std::is_same<typename Space::record_type, record_type>::value,
					  "PCFACoordinateMap can only transform spaces with the derived record type");

		auto encoded_records = projection_.encode(space.records());
		space_type derived_space(encoded_records, code_metric_);
		auto lineage = mtrc::one_to_one_lineage(space);

		// PCFA is coordinate (dimension) reduction: it produces a derived
		// Euclidean coordinate space that approximates the source geometry. It is
		// not record-set compression and not record thinning. The code metric is
		// a true metric; out-of-sample transform is supported by the derived coordinate map.
		return core::make_mapping_result(
			std::move(derived_space), std::move(lineage.source_records),
			std::move(lineage.representative_records), space.size(), true, "pcfa", "pcfa", "metric_space",
			core::metric_traits<code_metric_type>::law, true,
			"coordinate (dimension) reduction; derived Euclidean coordinate space approximating source geometry; "
			"not record compression");
	}

	auto inverse_transform(const space_type &derived_space, bool unshift = true) const -> std::vector<record_type>
	{
		return projection_.decode(derived_space.records(), unshift);
	}

	auto inverse_transform(const result_type &result, bool unshift = true) const -> std::vector<record_type>
	{
		return inverse_transform(result.space, unshift);
	}

	auto component_count() const -> std::size_t { return component_count_; }
	auto backend_projection() const -> const backend_projection_type & { return projection_; }

  private:
	mutable backend_projection_type projection_;
	std::size_t component_count_{};
	code_metric_type code_metric_;
};

class PCFAMapping {
  public:
	explicit PCFAMapping(std::size_t component_count) : component_count_(component_count)
	{
		if (component_count_ == 0) {
			throw std::invalid_argument("PCFA component count must be positive");
		}
	}

	template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
	auto derive_from(const Space &space) const
		-> PCFACoordinateMap<typename Space::record_type, default_pcfa_metric_t<typename Space::record_type>>
	{
		if (space.empty()) {
			throw std::invalid_argument("cannot derive PCFA coordinate map from an empty space");
		}

		using record_type = typename Space::record_type;
		using model_type = PCFACoordinateMap<record_type, default_pcfa_metric_t<record_type>>;
		using backend_projection_type = typename model_type::backend_projection_type;

		backend_projection_type projection(space.records(), component_count_);
		return model_type(std::move(projection), component_count_);
	}

	auto component_count() const -> std::size_t { return component_count_; }

  private:
	std::size_t component_count_{};
};

inline auto pcfa(std::size_t component_count) -> PCFAMapping { return PCFAMapping(component_count); }

template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto pcfa_space(const Space &space, std::size_t component_count) ->
	typename PCFACoordinateMap<typename Space::record_type, default_pcfa_metric_t<typename Space::record_type>>::result_type
{
	auto mapping = pcfa(component_count);
	auto coordinate_map = mapping.derive_from(space);
	return coordinate_map.transform(space);
}

} // namespace mtrc::modify::map

#endif
