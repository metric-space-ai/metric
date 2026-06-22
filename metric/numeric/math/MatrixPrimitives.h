// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_MATRIXPRIMITIVES_H
#define METRIC_NUMERIC_MATH_MATRIXPRIMITIVES_H
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

#include <metric/numeric/math/DynamicMatrix.h>
#include <metric/numeric/math/DynamicVector.h>

namespace mtrc::numeric {

namespace detail {

template <typename Values, typename = void> struct IndexedSequenceValue {
	using type = typename Values::value_type;
};

template <typename Values> struct IndexedSequenceValue<Values, std::void_t<typename Values::ElementType>> {
	using type = typename Values::ElementType;
};

template <typename Values> using indexed_sequence_value_t = typename IndexedSequenceValue<Values>::type;

} // namespace detail

template <typename Value> struct RowTopTwoValues {
	std::size_t first_index{0};
	Value first_value{};
	Value second_value{};
};

template <typename Matrix> auto row_sums(const Matrix &matrix) -> DynamicVector<typename Matrix::ElementType>
{
	using value_type = typename Matrix::ElementType;

	DynamicVector<value_type> sums(matrix.rows(), value_type{0});
	for (std::size_t row = 0; row < matrix.rows(); ++row) {
		for (std::size_t column = 0; column < matrix.columns(); ++column) {
			sums[row] += matrix(row, column);
		}
	}
	return sums;
}

template <typename Matrix> auto positive_column_sums(const Matrix &matrix) -> DynamicVector<typename Matrix::ElementType>
{
	using value_type = typename Matrix::ElementType;

	DynamicVector<value_type> sums(matrix.columns(), value_type{0});
	for (std::size_t column = 0; column < matrix.columns(); ++column) {
		for (std::size_t row = 0; row < matrix.rows(); ++row) {
			const auto value = matrix(row, column);
			if (value > value_type{0}) {
				sums[column] += value;
			}
		}
	}
	return sums;
}

template <typename Matrix>
auto positive_column_sums_excluding_diagonal(const Matrix &matrix) -> DynamicVector<typename Matrix::ElementType>
{
	using value_type = typename Matrix::ElementType;

	DynamicVector<value_type> sums(matrix.columns(), value_type{0});
	for (std::size_t column = 0; column < matrix.columns(); ++column) {
		for (std::size_t row = 0; row < matrix.rows(); ++row) {
			if (row == column) {
				continue;
			}
			const auto value = matrix(row, column);
			if (value > value_type{0}) {
				sums[column] += value;
			}
		}
	}
	return sums;
}

template <typename Value, typename ValueAt>
auto positive_diagonal_indices(std::size_t diagonal_count, ValueAt value_at) -> std::vector<std::size_t>
{
	std::vector<std::size_t> indices;
	for (std::size_t index = 0; index < diagonal_count; ++index) {
		if (static_cast<Value>(value_at(index)) > Value{0}) {
			indices.push_back(index);
		}
	}
	return indices;
}

template <typename Matrix> auto positive_diagonal_indices(const Matrix &matrix) -> std::vector<std::size_t>
{
	using value_type = typename Matrix::ElementType;
	return positive_diagonal_indices<value_type>(std::min(matrix.rows(), matrix.columns()), [&matrix](std::size_t index) {
		return matrix(index, index);
	});
}

template <typename Value, typename ValueAt>
auto column_argmax_indices(std::size_t row_count, std::size_t column_count, ValueAt value_at)
	-> std::vector<std::size_t>
{
	if (row_count == 0) {
		throw std::invalid_argument("column_argmax_indices requires at least one row");
	}

	std::vector<std::size_t> indices(column_count, 0);
	for (std::size_t column = 0; column < column_count; ++column) {
		auto best_value = static_cast<Value>(value_at(0, column));
		for (std::size_t row = 1; row < row_count; ++row) {
			const auto value = static_cast<Value>(value_at(row, column));
			if (value > best_value) {
				best_value = value;
				indices[column] = row;
			}
		}
	}
	return indices;
}

template <typename Matrix> auto column_argmax_indices(const Matrix &matrix) -> std::vector<std::size_t>
{
	using value_type = typename Matrix::ElementType;
	return column_argmax_indices<value_type>(matrix.rows(), matrix.columns(), [&matrix](std::size_t row,
																					   std::size_t column) {
		return matrix(row, column);
	});
}

template <typename Value, typename ValueAt>
auto column_argmin_indices(std::size_t row_count, std::size_t column_count, ValueAt value_at)
	-> std::vector<std::size_t>
{
	if (row_count == 0) {
		throw std::invalid_argument("column_argmin_indices requires at least one row");
	}

	std::vector<std::size_t> indices(column_count, 0);
	for (std::size_t column = 0; column < column_count; ++column) {
		auto best_value = static_cast<Value>(value_at(0, column));
		for (std::size_t row = 1; row < row_count; ++row) {
			const auto value = static_cast<Value>(value_at(row, column));
			if (value < best_value) {
				best_value = value;
				indices[column] = row;
			}
		}
	}
	return indices;
}

template <typename Matrix> auto column_argmin_indices(const Matrix &matrix) -> std::vector<std::size_t>
{
	using value_type = typename Matrix::ElementType;
	return column_argmin_indices<value_type>(matrix.rows(), matrix.columns(), [&matrix](std::size_t row,
																					   std::size_t column) {
		return matrix(row, column);
	});
}

template <typename Indices> auto index_counts(const Indices &indices, std::size_t bucket_count) -> std::vector<std::size_t>
{
	std::vector<std::size_t> counts(bucket_count, 0);
	for (const auto index_value : indices) {
		const auto index = static_cast<std::size_t>(index_value);
		if (index >= bucket_count) {
			throw std::invalid_argument("index_counts index is out of range");
		}
		++counts[index];
	}
	return counts;
}

template <typename Indices>
auto index_counts_excluding(const Indices &indices, std::size_t bucket_count, std::size_t excluded_index,
							const char *out_of_range_message = "index_counts_excluding index is out of range")
	-> std::vector<std::size_t>
{
	std::vector<std::size_t> counts(bucket_count, 0);
	for (const auto index_value : indices) {
		const auto index = static_cast<std::size_t>(index_value);
		if (index == excluded_index) {
			continue;
		}
		if (index >= bucket_count) {
			throw std::invalid_argument(out_of_range_message);
		}
		++counts[index];
	}
	return counts;
}

template <typename Values>
auto positions_excluding_value(const Values &values, std::size_t excluded_value) -> std::vector<std::size_t>
{
	std::vector<std::size_t> positions;
	for (std::size_t position = 0; position < values.size(); ++position) {
		if (static_cast<std::size_t>(values[position]) != excluded_value) {
			positions.push_back(position);
		}
	}
	return positions;
}

template <typename Values, typename Value>
auto positions_equal_to_value(const Values &values, const Value &selected_value) -> std::vector<std::size_t>
{
	std::vector<std::size_t> positions;
	for (std::size_t position = 0; position < values.size(); ++position) {
		if (values[position] == selected_value) {
			positions.push_back(position);
		}
	}
	return positions;
}

template <typename Values>
auto values_at_positions(const Values &values, const std::vector<std::size_t> &positions)
	-> std::vector<detail::indexed_sequence_value_t<Values>>
{
	std::vector<detail::indexed_sequence_value_t<Values>> selected;
	selected.reserve(positions.size());
	for (const auto position : positions) {
		if (position >= values.size()) {
			throw std::invalid_argument("values_at_positions position is out of range");
		}
		selected.push_back(values[position]);
	}
	return selected;
}

template <typename Indices>
auto index_buckets(const Indices &indices, std::size_t bucket_count,
				   const char *out_of_range_message = "index_buckets index is out of range")
	-> std::vector<std::vector<std::size_t>>
{
	std::vector<std::vector<std::size_t>> buckets(bucket_count);
	for (std::size_t position = 0; position < indices.size(); ++position) {
		const auto index = static_cast<std::size_t>(indices[position]);
		if (index >= bucket_count) {
			throw std::invalid_argument(out_of_range_message);
		}
		buckets[index].push_back(position);
	}
	return buckets;
}

template <typename Indices>
auto index_buckets_excluding(const Indices &indices, std::size_t bucket_count, std::size_t excluded_index,
							 const char *out_of_range_message = "index_buckets_excluding index is out of range")
	-> std::vector<std::vector<std::size_t>>
{
	std::vector<std::vector<std::size_t>> buckets(bucket_count);
	for (std::size_t position = 0; position < indices.size(); ++position) {
		const auto index = static_cast<std::size_t>(indices[position]);
		if (index == excluded_index) {
			continue;
		}
		if (index >= bucket_count) {
			throw std::invalid_argument(out_of_range_message);
		}
		buckets[index].push_back(position);
	}
	return buckets;
}

template <typename Matrix> auto column_means(const Matrix &matrix) -> DynamicVector<typename Matrix::ElementType>
{
	using value_type = typename Matrix::ElementType;

	DynamicVector<value_type> means(matrix.columns(), value_type{0});
	if (matrix.rows() == 0) {
		return means;
	}

	for (std::size_t column = 0; column < matrix.columns(); ++column) {
		for (std::size_t row = 0; row < matrix.rows(); ++row) {
			means[column] += matrix(row, column);
		}
		means[column] /= static_cast<value_type>(matrix.rows());
	}
	return means;
}

template <typename Matrix>
auto positive_mean_or(const Matrix &matrix, typename Matrix::ElementType fallback) -> typename Matrix::ElementType
{
	using value_type = typename Matrix::ElementType;

	value_type total{0};
	std::size_t count{0};
	for (std::size_t row = 0; row < matrix.rows(); ++row) {
		for (std::size_t column = 0; column < matrix.columns(); ++column) {
			if (matrix(row, column) > value_type(0)) {
				total += matrix(row, column);
				++count;
			}
		}
	}
	return count == 0 ? fallback : total / static_cast<value_type>(count);
}

template <typename Value, typename ValueAt>
auto row_top_two_values(std::size_t row_count, std::size_t column_count, ValueAt value_at)
	-> std::vector<RowTopTwoValues<Value>>
{
	if (column_count < 2) {
		throw std::invalid_argument("row_top_two_values requires at least two columns");
	}

	std::vector<RowTopTwoValues<Value>> row_values;
	row_values.reserve(row_count);
	for (std::size_t row = 0; row < row_count; ++row) {
		const auto first_candidate = static_cast<Value>(value_at(row, 0));
		const auto second_candidate = static_cast<Value>(value_at(row, 1));

		RowTopTwoValues<Value> current;
		if (first_candidate > second_candidate) {
			current.first_index = 0;
			current.first_value = first_candidate;
			current.second_value = second_candidate;
		} else {
			current.first_index = 1;
			current.first_value = second_candidate;
			current.second_value = first_candidate;
		}

		for (std::size_t column = 2; column < column_count; ++column) {
			const auto value = static_cast<Value>(value_at(row, column));
			if (value > current.second_value) {
				if (value > current.first_value) {
					current.second_value = current.first_value;
					current.first_index = column;
					current.first_value = value;
				} else {
					current.second_value = value;
				}
			}
		}

		row_values.push_back(current);
	}
	return row_values;
}

template <typename Matrix>
auto row_top_two_values(const Matrix &matrix) -> std::vector<RowTopTwoValues<typename Matrix::ElementType>>
{
	using value_type = typename Matrix::ElementType;
	return row_top_two_values<value_type>(matrix.rows(), matrix.columns(), [&matrix](std::size_t row,
																					 std::size_t column) {
		return matrix(row, column);
	});
}

template <typename Matrix> auto mean_row_sqr_norm(const Matrix &matrix)
{
	if (matrix.rows() == 0) {
		throw std::invalid_argument("mean_row_sqr_norm requires at least one row");
	}

	const auto squared_norm = sqrNorm(matrix);
	using norm_type = decltype(squared_norm);
	return squared_norm / static_cast<norm_type>(matrix.rows());
}

template <typename Row, typename Matrix> auto matrix_to_row_vectors_as(const Matrix &matrix) -> std::vector<Row>
{
	using value_type = typename Row::value_type;

	std::vector<Row> rows;
	rows.reserve(matrix.rows());
	for (std::size_t row = 0; row < matrix.rows(); ++row) {
		Row values;
		values.reserve(matrix.columns());
		for (std::size_t column = 0; column < matrix.columns(); ++column) {
			values.push_back(static_cast<value_type>(matrix(row, column)));
		}
		rows.push_back(std::move(values));
	}
	return rows;
}

template <typename Matrix>
auto matrix_to_row_vectors(const Matrix &matrix) -> std::vector<std::vector<typename Matrix::ElementType>>
{
	return matrix_to_row_vectors_as<std::vector<typename Matrix::ElementType>>(matrix);
}

template <typename Scalar, typename Rows>
auto row_vectors_to_matrix(const Rows &rows, std::size_t columns) -> DynamicMatrix<Scalar>
{
	DynamicMatrix<Scalar> matrix(rows.size(), columns);
	for (std::size_t row = 0; row < rows.size(); ++row) {
		if (rows[row].size() != columns) {
			throw std::invalid_argument("row vectors must have a consistent column count");
		}
		for (std::size_t column = 0; column < columns; ++column) {
			matrix(row, column) = static_cast<Scalar>(rows[row][column]);
		}
	}
	return matrix;
}

template <typename Scalar, typename Rows> auto row_vectors_to_matrix(const Rows &rows) -> DynamicMatrix<Scalar>
{
	if (rows.empty()) {
		return DynamicMatrix<Scalar>(0, 0);
	}

	return row_vectors_to_matrix<Scalar>(rows, rows.front().size());
}

template <typename Value, typename Matrix> auto matrix_to_flat_values_as(const Matrix &matrix) -> std::vector<Value>
{
	std::vector<Value> values;
	values.reserve(matrix.rows() * matrix.columns());
	for (std::size_t row = 0; row < matrix.rows(); ++row) {
		for (std::size_t column = 0; column < matrix.columns(); ++column) {
			values.push_back(static_cast<Value>(matrix(row, column)));
		}
	}
	return values;
}

template <typename Matrix> auto matrix_to_flat_values(const Matrix &matrix) -> std::vector<typename Matrix::ElementType>
{
	return matrix_to_flat_values_as<typename Matrix::ElementType>(matrix);
}

template <typename OutputMatrix, typename Matrix, typename Indices>
auto select_rows_as(const Matrix &matrix, const Indices &indices, std::size_t offset, std::size_t count) -> OutputMatrix
{
	using value_type = typename OutputMatrix::ElementType;

	if (offset > indices.size() || count > indices.size() - offset) {
		throw std::invalid_argument("select_rows range exceeds index count");
	}

	OutputMatrix selected(count, matrix.columns());
	for (std::size_t row = 0; row < count; ++row) {
		const auto source_row = static_cast<std::size_t>(indices[offset + row]);
		if (source_row >= matrix.rows()) {
			throw std::invalid_argument("select_rows source row index is out of range");
		}
		for (std::size_t column = 0; column < matrix.columns(); ++column) {
			selected(row, column) = static_cast<value_type>(matrix(source_row, column));
		}
	}
	return selected;
}

template <typename OutputMatrix, typename Matrix, typename Indices>
auto select_rows_as(const Matrix &matrix, const Indices &indices) -> OutputMatrix
{
	return select_rows_as<OutputMatrix>(matrix, indices, 0, indices.size());
}

template <typename Matrix, typename Indices>
auto select_rows(const Matrix &matrix, const Indices &indices, std::size_t offset, std::size_t count) -> Matrix
{
	return select_rows_as<Matrix>(matrix, indices, offset, count);
}

template <typename Matrix, typename Indices> auto select_rows(const Matrix &matrix, const Indices &indices) -> Matrix
{
	return select_rows(matrix, indices, 0, indices.size());
}

template <typename OutputMatrix, typename Matrix, typename Indices>
auto select_columns_as(const Matrix &matrix, const Indices &indices, std::size_t offset, std::size_t count)
	-> OutputMatrix
{
	using value_type = typename OutputMatrix::ElementType;

	if (offset > indices.size() || count > indices.size() - offset) {
		throw std::invalid_argument("select_columns range exceeds index count");
	}

	OutputMatrix selected(matrix.rows(), count);
	for (std::size_t column = 0; column < count; ++column) {
		const auto source_column = static_cast<std::size_t>(indices[offset + column]);
		if (source_column >= matrix.columns()) {
			throw std::invalid_argument("select_columns source column index is out of range");
		}
		for (std::size_t row = 0; row < matrix.rows(); ++row) {
			selected(row, column) = static_cast<value_type>(matrix(row, source_column));
		}
	}
	return selected;
}

template <typename OutputMatrix, typename Matrix, typename Indices>
auto select_columns_as(const Matrix &matrix, const Indices &indices) -> OutputMatrix
{
	return select_columns_as<OutputMatrix>(matrix, indices, 0, indices.size());
}

template <typename Matrix, typename Indices>
auto select_columns(const Matrix &matrix, const Indices &indices, std::size_t offset, std::size_t count) -> Matrix
{
	return select_columns_as<Matrix>(matrix, indices, offset, count);
}

template <typename Matrix, typename Indices> auto select_columns(const Matrix &matrix, const Indices &indices) -> Matrix
{
	return select_columns(matrix, indices, 0, indices.size());
}

template <typename Scalar, typename Values>
auto flat_values_to_matrix(const Values &values, std::size_t columns) -> DynamicMatrix<Scalar>
{
	if (columns == 0) {
		throw std::invalid_argument("flat_values_to_matrix requires a positive column count");
	}
	if (values.size() % columns != 0) {
		throw std::invalid_argument("flat values size must be divisible by column count");
	}

	DynamicMatrix<Scalar> matrix(values.size() / columns, columns);
	for (std::size_t row = 0; row < matrix.rows(); ++row) {
		for (std::size_t column = 0; column < columns; ++column) {
			matrix(row, column) = static_cast<Scalar>(values[row * columns + column]);
		}
	}
	return matrix;
}

template <typename Matrix> auto normalize_rows(Matrix &matrix) -> void
{
	using value_type = typename Matrix::ElementType;

	for (std::size_t row = 0; row < matrix.rows(); ++row) {
		value_type total{0};
		for (std::size_t column = 0; column < matrix.columns(); ++column) {
			total += matrix(row, column);
		}
		if (total == value_type(0)) {
			throw std::invalid_argument("matrix row has zero sum");
		}
		for (std::size_t column = 0; column < matrix.columns(); ++column) {
			matrix(row, column) /= total;
		}
	}
}

template <typename Matrix> auto row_normalized(Matrix matrix) -> Matrix
{
	normalize_rows(matrix);
	return matrix;
}

template <typename Matrix> auto center_columns(Matrix &matrix) -> void
{
	const auto means = column_means(matrix);
	for (std::size_t row = 0; row < matrix.rows(); ++row) {
		for (std::size_t column = 0; column < matrix.columns(); ++column) {
			matrix(row, column) -= means[column];
		}
	}
}

template <typename Matrix> auto scale_columns_to_unit_max_abs(Matrix &matrix) -> void
{
	using value_type = typename Matrix::ElementType;

	for (std::size_t column = 0; column < matrix.columns(); ++column) {
		value_type max_abs{0};
		for (std::size_t row = 0; row < matrix.rows(); ++row) {
			max_abs = std::max(max_abs, static_cast<value_type>(std::abs(matrix(row, column))));
		}
		if (max_abs > value_type(0)) {
			for (std::size_t row = 0; row < matrix.rows(); ++row) {
				matrix(row, column) /= max_abs;
			}
		}
	}
}

template <typename Matrix> auto center_columns_to_unit_max_abs(Matrix &matrix) -> void
{
	center_columns(matrix);
	scale_columns_to_unit_max_abs(matrix);
}

template <typename Matrix> auto columns_centered_to_unit_max_abs(Matrix matrix) -> Matrix
{
	center_columns_to_unit_max_abs(matrix);
	return matrix;
}

template <typename Matrix> auto set_diagonal(Matrix &matrix, typename Matrix::ElementType value) -> void
{
	const auto diagonal_size = std::min(matrix.rows(), matrix.columns());
	for (std::size_t index = 0; index < diagonal_size; ++index) {
		matrix(index, index) = value;
	}
}

template <typename Matrix> auto add_to_diagonal(Matrix &matrix, typename Matrix::ElementType value) -> void
{
	const auto diagonal_size = std::min(matrix.rows(), matrix.columns());
	for (std::size_t index = 0; index < diagonal_size; ++index) {
		matrix(index, index) += value;
	}
}

template <typename Matrix>
auto blend_with_identity(Matrix &matrix, typename Matrix::ElementType identity_weight) -> void
{
	using value_type = typename Matrix::ElementType;

	if (matrix.rows() != matrix.columns()) {
		throw std::invalid_argument("blend_with_identity requires a square matrix");
	}

	const auto matrix_weight = value_type(1) - identity_weight;
	for (std::size_t row = 0; row < matrix.rows(); ++row) {
		for (std::size_t column = 0; column < matrix.columns(); ++column) {
			matrix(row, column) *= matrix_weight;
		}
	}
	add_to_diagonal(matrix, identity_weight);
}

template <typename Matrix> auto identity_blended(Matrix matrix, typename Matrix::ElementType identity_weight) -> Matrix
{
	blend_with_identity(matrix, identity_weight);
	return matrix;
}

template <typename Matrix> auto matrix_power(Matrix matrix, std::size_t exponent) -> Matrix
{
	using value_type = typename Matrix::ElementType;

	if (matrix.rows() != matrix.columns()) {
		throw std::invalid_argument("matrix_power requires a square matrix");
	}

	Matrix result(matrix.rows(), matrix.columns(), value_type{0});
	set_diagonal(result, value_type(1));

	while (exponent > 0) {
		if (exponent % 2 == 1) {
			result = result * matrix;
		}
		exponent /= 2;
		if (exponent > 0) {
			matrix = matrix * matrix;
		}
	}

	return result;
}

} // namespace mtrc::numeric

#endif
