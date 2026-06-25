#include <cassert>
#include <stdexcept>
#include <vector>

#include <metric/numeric/Math.h>

#include "numeric_test_helpers.hpp"

namespace {

using mtrc::test::assert_matrix_close;
using mtrc::test::assert_vector_close;
using mtrc::test::close_to;

template <typename Callable> auto assert_invalid_argument(Callable &&call) -> void
{
	bool rejected = false;
	try {
		call();
	} catch (const std::invalid_argument &) {
		rejected = true;
	}
	assert(rejected);
}

} // namespace

auto main() -> int
{
	using Matrix = mtrc::numeric::DynamicMatrix<double>;
	using Vector = mtrc::numeric::DynamicVector<double>;

	Matrix weights(3, 3);
	weights(0, 0) = 1.0;
	weights(0, 1) = 1.0;
	weights(0, 2) = 2.0;
	weights(1, 0) = 2.0;
	weights(1, 1) = 0.0;
	weights(1, 2) = 2.0;
	weights(2, 0) = 3.0;
	weights(2, 1) = 3.0;
	weights(2, 2) = 0.0;

	Vector expected_sums(3);
	expected_sums[0] = 4.0;
	expected_sums[1] = 4.0;
	expected_sums[2] = 6.0;
	assert_vector_close(mtrc::numeric::row_sums(weights), expected_sums);
	assert(close_to(mtrc::numeric::positive_mean_or(weights, 99.0), 2.0));
	assert(close_to(mtrc::numeric::mean_row_sqr_norm(weights), 32.0 / 3.0));

	Matrix signed_columns(3, 4);
	signed_columns(0, 0) = 1.0;
	signed_columns(0, 1) = -2.0;
	signed_columns(0, 2) = 3.0;
	signed_columns(0, 3) = 4.0;
	signed_columns(1, 0) = -5.0;
	signed_columns(1, 1) = 6.0;
	signed_columns(1, 2) = 7.0;
	signed_columns(1, 3) = -8.0;
	signed_columns(2, 0) = 9.0;
	signed_columns(2, 1) = 10.0;
	signed_columns(2, 2) = 11.0;
	signed_columns(2, 3) = 12.0;
	Vector expected_positive_columns(4);
	expected_positive_columns[0] = 10.0;
	expected_positive_columns[1] = 16.0;
	expected_positive_columns[2] = 21.0;
	expected_positive_columns[3] = 16.0;
	assert_vector_close(mtrc::numeric::positive_column_sums(signed_columns), expected_positive_columns);
	Vector expected_positive_columns_without_diagonal(4);
	expected_positive_columns_without_diagonal[0] = 9.0;
	expected_positive_columns_without_diagonal[1] = 10.0;
	expected_positive_columns_without_diagonal[2] = 10.0;
	expected_positive_columns_without_diagonal[3] = 16.0;
	assert_vector_close(mtrc::numeric::positive_column_sums_excluding_diagonal(signed_columns),
						expected_positive_columns_without_diagonal);
	Matrix no_positive_rows(0, 3);
	Vector expected_empty_positive_columns(3);
	expected_empty_positive_columns[0] = 0.0;
	expected_empty_positive_columns[1] = 0.0;
	expected_empty_positive_columns[2] = 0.0;
	assert_vector_close(mtrc::numeric::positive_column_sums(no_positive_rows), expected_empty_positive_columns);

	Matrix diagonal_scores(4, 3);
	diagonal_scores(0, 0) = -1.0;
	diagonal_scores(0, 1) = 99.0;
	diagonal_scores(0, 2) = 99.0;
	diagonal_scores(1, 0) = 99.0;
	diagonal_scores(1, 1) = 2.0;
	diagonal_scores(1, 2) = 99.0;
	diagonal_scores(2, 0) = 99.0;
	diagonal_scores(2, 1) = 99.0;
	diagonal_scores(2, 2) = 0.0;
	diagonal_scores(3, 0) = 99.0;
	diagonal_scores(3, 1) = 99.0;
	diagonal_scores(3, 2) = 99.0;
	const std::vector<std::size_t> expected_positive_diagonal{1};
	assert(mtrc::numeric::positive_diagonal_indices(diagonal_scores) == expected_positive_diagonal);
	const auto callable_positive_diagonal = mtrc::numeric::positive_diagonal_indices<double>(
		5, [](std::size_t index) { return index == 0 || index == 3 ? 1.0 : -1.0; });
	const std::vector<std::size_t> expected_callable_positive_diagonal{0, 3};
	assert(callable_positive_diagonal == expected_callable_positive_diagonal);

	Matrix column_argmax_source(3, 4);
	column_argmax_source(0, 0) = 1.0;
	column_argmax_source(0, 1) = 5.0;
	column_argmax_source(0, 2) = 3.0;
	column_argmax_source(0, 3) = 4.0;
	column_argmax_source(1, 0) = 2.0;
	column_argmax_source(1, 1) = 4.0;
	column_argmax_source(1, 2) = 3.0;
	column_argmax_source(1, 3) = 7.0;
	column_argmax_source(2, 0) = 0.0;
	column_argmax_source(2, 1) = 6.0;
	column_argmax_source(2, 2) = 3.0;
	column_argmax_source(2, 3) = 1.0;
	const std::vector<std::size_t> expected_column_argmax{1, 2, 0, 1};
	assert(mtrc::numeric::column_argmax_indices(column_argmax_source) == expected_column_argmax);
	const auto callable_column_argmax = mtrc::numeric::column_argmax_indices<double>(
		3, 4, [](std::size_t row, std::size_t column) {
			return column == 2 ? static_cast<double>(2 - row) : static_cast<double>(row + column);
		});
	const std::vector<std::size_t> expected_callable_column_argmax{2, 2, 0, 2};
	assert(callable_column_argmax == expected_callable_column_argmax);
	assert_invalid_argument([&no_positive_rows]() { (void)mtrc::numeric::column_argmax_indices(no_positive_rows); });
	Matrix column_argmin_source(3, 4);
	column_argmin_source(0, 0) = 3.0;
	column_argmin_source(0, 1) = 5.0;
	column_argmin_source(0, 2) = 3.0;
	column_argmin_source(0, 3) = 4.0;
	column_argmin_source(1, 0) = 2.0;
	column_argmin_source(1, 1) = 4.0;
	column_argmin_source(1, 2) = 3.0;
	column_argmin_source(1, 3) = 1.0;
	column_argmin_source(2, 0) = 0.0;
	column_argmin_source(2, 1) = 6.0;
	column_argmin_source(2, 2) = 3.0;
	column_argmin_source(2, 3) = 7.0;
	const std::vector<std::size_t> expected_column_argmin{2, 1, 0, 1};
	assert(mtrc::numeric::column_argmin_indices(column_argmin_source) == expected_column_argmin);
	const auto callable_column_argmin = mtrc::numeric::column_argmin_indices<double>(
		3, 4, [](std::size_t row, std::size_t column) {
			return column == 2 ? static_cast<double>(row) : static_cast<double>(2 - row + column);
		});
	const std::vector<std::size_t> expected_callable_column_argmin{2, 2, 0, 2};
	assert(callable_column_argmin == expected_callable_column_argmin);
	assert_invalid_argument([&no_positive_rows]() { (void)mtrc::numeric::column_argmin_indices(no_positive_rows); });
	const std::vector<std::size_t> index_values{2, 0, 2, 1, 2, 0};
	const std::vector<std::size_t> expected_index_counts{2, 1, 3, 0};
	assert(mtrc::numeric::index_counts(index_values, 4) == expected_index_counts);
	const std::vector<std::size_t> no_indices;
	assert(mtrc::numeric::index_counts(no_indices, 0).empty());
	const std::vector<std::size_t> out_of_range_indices{0, 3};
	assert_invalid_argument([&out_of_range_indices]() { (void)mtrc::numeric::index_counts(out_of_range_indices, 3); });
	const std::vector<std::size_t> expected_index_counts_excluding_label{2, 1, 0};
	assert(mtrc::numeric::index_counts_excluding(std::vector<std::size_t>{0, 99, 1, 0, 99}, 3, 99) ==
		   expected_index_counts_excluding_label);
	assert(mtrc::numeric::index_counts_excluding(std::vector<std::size_t>{99, 99}, 0, 99).empty());
	const std::vector<std::size_t> invalid_counts_excluding_label{0, 3, 99};
	assert_invalid_argument([&invalid_counts_excluding_label]() {
		(void)mtrc::numeric::index_counts_excluding(invalid_counts_excluding_label, 3, 99);
	});
	assert((mtrc::numeric::positions_excluding_value(std::vector<std::size_t>{2, 0, 99, 1}, 99) ==
			std::vector<std::size_t>{0, 1, 3}));
	assert((mtrc::numeric::positions_equal_to_value(std::vector<std::size_t>{2, 0, 99, 1, 99}, 99) ==
			std::vector<std::size_t>{2, 4}));
	assert((mtrc::numeric::positions_equal_to_value(std::vector<bool>{true, false, true}, true) ==
			std::vector<std::size_t>{0, 2}));
	assert((mtrc::numeric::values_at_positions(std::vector<std::size_t>{2, 0, 99, 1},
												 std::vector<std::size_t>{0, 3}) ==
			std::vector<std::size_t>{2, 1}));
	Vector values_source(4);
	values_source[0] = 0.5;
	values_source[1] = 1.5;
	values_source[2] = 2.5;
	values_source[3] = 3.5;
	assert((mtrc::numeric::values_at_positions(values_source, std::vector<std::size_t>{1, 3}) ==
			std::vector<double>{1.5, 3.5}));
	assert_invalid_argument([]() {
		(void)mtrc::numeric::values_at_positions(std::vector<std::size_t>{0}, std::vector<std::size_t>{1});
	});
	const auto bucketed_indices = mtrc::numeric::index_buckets(index_values, 4);
	assert(bucketed_indices.size() == 4);
	assert((bucketed_indices[0] == std::vector<std::size_t>{1, 5}));
	assert((bucketed_indices[1] == std::vector<std::size_t>{3}));
	assert((bucketed_indices[2] == std::vector<std::size_t>{0, 2, 4}));
	assert(bucketed_indices[3].empty());
	assert(mtrc::numeric::index_buckets(no_indices, 0).empty());
	assert_invalid_argument([&out_of_range_indices]() { (void)mtrc::numeric::index_buckets(out_of_range_indices, 3); });
	const auto bucketed_excluding_label =
		mtrc::numeric::index_buckets_excluding(std::vector<std::size_t>{0, 3, 1, 3, 0}, 2, 3);
	assert(bucketed_excluding_label.size() == 2);
	assert((bucketed_excluding_label[0] == std::vector<std::size_t>{0, 4}));
	assert((bucketed_excluding_label[1] == std::vector<std::size_t>{2}));
	const std::vector<std::size_t> invalid_bucket_excluding_label{0, 2, 3};
	assert_invalid_argument([&invalid_bucket_excluding_label]() {
		(void)mtrc::numeric::index_buckets_excluding(invalid_bucket_excluding_label, 2, 3);
	});

	Matrix top_two_source(3, 4);
	top_two_source(0, 0) = 1.0;
	top_two_source(0, 1) = 5.0;
	top_two_source(0, 2) = 3.0;
	top_two_source(0, 3) = 2.0;
	top_two_source(1, 0) = -1.0;
	top_two_source(1, 1) = -4.0;
	top_two_source(1, 2) = -2.0;
	top_two_source(1, 3) = -3.0;
	top_two_source(2, 0) = 7.0;
	top_two_source(2, 1) = 6.0;
	top_two_source(2, 2) = 5.0;
	top_two_source(2, 3) = 8.0;
	const auto top_two_rows = mtrc::numeric::row_top_two_values(top_two_source);
	assert(top_two_rows.size() == 3);
	assert(top_two_rows[0].first_index == 1);
	assert(close_to(top_two_rows[0].first_value, 5.0));
	assert(close_to(top_two_rows[0].second_value, 3.0));
	assert(top_two_rows[1].first_index == 0);
	assert(close_to(top_two_rows[1].first_value, -1.0));
	assert(close_to(top_two_rows[1].second_value, -2.0));
	assert(top_two_rows[2].first_index == 3);
	assert(close_to(top_two_rows[2].first_value, 8.0));
	assert(close_to(top_two_rows[2].second_value, 7.0));
	const auto callable_top_two = mtrc::numeric::row_top_two_values<double>(
		2, 3, [](std::size_t row, std::size_t column) { return static_cast<double>(row + column); });
	assert(callable_top_two[0].first_index == 2);
	assert(close_to(callable_top_two[0].first_value, 2.0));
	assert(close_to(callable_top_two[0].second_value, 1.0));
	assert(callable_top_two[1].first_index == 2);
	assert(close_to(callable_top_two[1].first_value, 3.0));
	assert(close_to(callable_top_two[1].second_value, 2.0));
	Matrix one_column(2, 1, 0.0);
	assert_invalid_argument([&one_column]() { (void)mtrc::numeric::row_top_two_values(one_column); });

	const Matrix normalized_copy = mtrc::numeric::row_normalized(weights);
	assert(close_to(weights(0, 0), 1.0));
	assert(close_to(normalized_copy(0, 0), 0.25));
	assert(close_to(normalized_copy(0, 1), 0.25));
	assert(close_to(normalized_copy(0, 2), 0.5));
	assert(close_to(normalized_copy(1, 0), 0.5));
	assert(close_to(normalized_copy(1, 1), 0.0));
	assert(close_to(normalized_copy(1, 2), 0.5));
	assert(close_to(normalized_copy(2, 0), 0.5));
	assert(close_to(normalized_copy(2, 1), 0.5));
	assert(close_to(normalized_copy(2, 2), 0.0));

	mtrc::numeric::normalize_rows(weights);
	assert_matrix_close(weights, normalized_copy);
	const auto normalized_sums = mtrc::numeric::row_sums(weights);
	assert(close_to(normalized_sums[0], 1.0));
	assert(close_to(normalized_sums[1], 1.0));
	assert(close_to(normalized_sums[2], 1.0));

	Matrix coordinates(3, 2);
	coordinates(0, 0) = 1.0;
	coordinates(0, 1) = 10.0;
	coordinates(1, 0) = 3.0;
	coordinates(1, 1) = 14.0;
	coordinates(2, 0) = 5.0;
	coordinates(2, 1) = 16.0;

	const auto coordinate_rows = mtrc::numeric::matrix_to_row_vectors(coordinates);
	assert(coordinate_rows.size() == 3);
	assert(coordinate_rows[0].size() == 2);
	assert(close_to(coordinate_rows[0][0], 1.0));
	assert(close_to(coordinate_rows[1][1], 14.0));
	assert_matrix_close(mtrc::numeric::row_vectors_to_matrix<double>(coordinate_rows), coordinates);

	const auto coordinate_rows_as_float = mtrc::numeric::matrix_to_row_vectors_as<std::vector<float>>(coordinates);
	assert(coordinate_rows_as_float.size() == 3);
	assert(coordinate_rows_as_float[2].size() == 2);
	assert(close_to(static_cast<double>(coordinate_rows_as_float[2][0]), 5.0));
	assert(close_to(static_cast<double>(coordinate_rows_as_float[2][1]), 16.0));

	const std::vector<std::vector<int>> integer_rows{{1, 2}, {3, 4}};
	Matrix integer_matrix_expected(2, 2);
	integer_matrix_expected(0, 0) = 1.0;
	integer_matrix_expected(0, 1) = 2.0;
	integer_matrix_expected(1, 0) = 3.0;
	integer_matrix_expected(1, 1) = 4.0;
	assert_matrix_close(mtrc::numeric::row_vectors_to_matrix<double>(integer_rows, 2), integer_matrix_expected);

	const std::vector<std::vector<double>> no_rows;
	const Matrix empty_matrix = mtrc::numeric::row_vectors_to_matrix<double>(no_rows);
	assert(empty_matrix.rows() == 0);
	assert(empty_matrix.columns() == 0);
	assert_invalid_argument([&empty_matrix]() { (void)mtrc::numeric::mean_row_sqr_norm(empty_matrix); });
	const Matrix empty_matrix_with_columns = mtrc::numeric::row_vectors_to_matrix<double>(no_rows, 2);
	assert(empty_matrix_with_columns.rows() == 0);
	assert(empty_matrix_with_columns.columns() == 2);

	const std::vector<std::vector<double>> empty_feature_rows{{}, {}};
	const Matrix zero_column_matrix = mtrc::numeric::row_vectors_to_matrix<double>(empty_feature_rows);
	assert(zero_column_matrix.rows() == 2);
	assert(zero_column_matrix.columns() == 0);
	const auto zero_column_rows = mtrc::numeric::matrix_to_row_vectors(zero_column_matrix);
	assert(zero_column_rows.size() == 2);
	assert(zero_column_rows[0].empty());
	assert(zero_column_rows[1].empty());

	const std::vector<std::vector<double>> ragged_rows{{1.0, 2.0}, {3.0}};
	assert_invalid_argument([&ragged_rows]() { (void)mtrc::numeric::row_vectors_to_matrix<double>(ragged_rows); });

	const std::vector<int> flat_values{1, 2, 3, 4, 5, 6};
	Matrix flat_matrix_expected(2, 3);
	flat_matrix_expected(0, 0) = 1.0;
	flat_matrix_expected(0, 1) = 2.0;
	flat_matrix_expected(0, 2) = 3.0;
	flat_matrix_expected(1, 0) = 4.0;
	flat_matrix_expected(1, 1) = 5.0;
	flat_matrix_expected(1, 2) = 6.0;
	const Matrix flat_matrix = mtrc::numeric::flat_values_to_matrix<double>(flat_values, 3);
	assert_matrix_close(flat_matrix, flat_matrix_expected);
	assert((mtrc::numeric::matrix_to_flat_values_as<int>(flat_matrix) == flat_values));
	const auto flat_values_copy = mtrc::numeric::matrix_to_flat_values(flat_matrix);
	assert(flat_values_copy.size() == 6);
	assert(close_to(flat_values_copy[0], 1.0));
	assert(close_to(flat_values_copy[5], 6.0));

	const std::vector<int> no_flat_values;
	const Matrix empty_flat_matrix = mtrc::numeric::flat_values_to_matrix<double>(no_flat_values, 3);
	assert(empty_flat_matrix.rows() == 0);
	assert(empty_flat_matrix.columns() == 3);
	assert(mtrc::numeric::matrix_to_flat_values(empty_flat_matrix).empty());
	assert_invalid_argument([&flat_values]() { (void)mtrc::numeric::flat_values_to_matrix<double>(flat_values, 0); });
	assert_invalid_argument([&flat_values]() { (void)mtrc::numeric::flat_values_to_matrix<double>(flat_values, 4); });

	const std::vector<std::size_t> selected_order{2, 0};
	Matrix expected_selected(2, 2);
	expected_selected(0, 0) = 5.0;
	expected_selected(0, 1) = 16.0;
	expected_selected(1, 0) = 1.0;
	expected_selected(1, 1) = 10.0;
	assert_matrix_close(mtrc::numeric::select_rows(coordinates, selected_order), expected_selected);
	const auto selected_as_float =
		mtrc::numeric::select_rows_as<mtrc::numeric::DynamicMatrix<float>>(coordinates, selected_order);
	assert(selected_as_float.rows() == 2);
	assert(selected_as_float.columns() == 2);
	assert(close_to(static_cast<double>(selected_as_float(0, 0)), 5.0));
	assert(close_to(static_cast<double>(selected_as_float(1, 1)), 10.0));

	const std::vector<std::size_t> slice_order{2, 1, 0};
	Matrix expected_selected_slice(2, 2);
	expected_selected_slice(0, 0) = 3.0;
	expected_selected_slice(0, 1) = 14.0;
	expected_selected_slice(1, 0) = 1.0;
	expected_selected_slice(1, 1) = 10.0;
	assert_matrix_close(mtrc::numeric::select_rows(coordinates, slice_order, 1, 2), expected_selected_slice);

	const std::vector<std::size_t> no_selected_rows;
	const Matrix empty_selected = mtrc::numeric::select_rows(coordinates, no_selected_rows);
	assert(empty_selected.rows() == 0);
	assert(empty_selected.columns() == coordinates.columns());

	const std::vector<std::size_t> bad_selected_row{3};
	assert_invalid_argument(
		[&coordinates, &bad_selected_row]() { (void)mtrc::numeric::select_rows(coordinates, bad_selected_row); });
	assert_invalid_argument(
		[&coordinates, &slice_order]() { (void)mtrc::numeric::select_rows(coordinates, slice_order, 2, 2); });

	const std::vector<std::size_t> selected_columns{1, 0, 1};
	Matrix expected_selected_columns(3, 3);
	expected_selected_columns(0, 0) = 10.0;
	expected_selected_columns(0, 1) = 1.0;
	expected_selected_columns(0, 2) = 10.0;
	expected_selected_columns(1, 0) = 14.0;
	expected_selected_columns(1, 1) = 3.0;
	expected_selected_columns(1, 2) = 14.0;
	expected_selected_columns(2, 0) = 16.0;
	expected_selected_columns(2, 1) = 5.0;
	expected_selected_columns(2, 2) = 16.0;
	assert_matrix_close(mtrc::numeric::select_columns(coordinates, selected_columns), expected_selected_columns);
	const auto selected_columns_as_float =
		mtrc::numeric::select_columns_as<mtrc::numeric::DynamicMatrix<float>>(coordinates, selected_columns);
	assert(selected_columns_as_float.rows() == 3);
	assert(selected_columns_as_float.columns() == 3);
	assert(close_to(static_cast<double>(selected_columns_as_float(0, 0)), 10.0));
	assert(close_to(static_cast<double>(selected_columns_as_float(2, 1)), 5.0));

	const std::vector<std::size_t> column_slice_order{0, 1, 0};
	Matrix expected_selected_column_slice(3, 2);
	expected_selected_column_slice(0, 0) = 10.0;
	expected_selected_column_slice(0, 1) = 1.0;
	expected_selected_column_slice(1, 0) = 14.0;
	expected_selected_column_slice(1, 1) = 3.0;
	expected_selected_column_slice(2, 0) = 16.0;
	expected_selected_column_slice(2, 1) = 5.0;
	assert_matrix_close(mtrc::numeric::select_columns(coordinates, column_slice_order, 1, 2),
						expected_selected_column_slice);

	const std::vector<std::size_t> no_selected_columns;
	const Matrix empty_selected_columns = mtrc::numeric::select_columns(coordinates, no_selected_columns);
	assert(empty_selected_columns.rows() == coordinates.rows());
	assert(empty_selected_columns.columns() == 0);

	const std::vector<std::size_t> bad_selected_column{2};
	assert_invalid_argument([&coordinates, &bad_selected_column]() {
		(void)mtrc::numeric::select_columns(coordinates, bad_selected_column);
	});
	assert_invalid_argument([&coordinates, &column_slice_order]() {
		(void)mtrc::numeric::select_columns(coordinates, column_slice_order, 2, 2);
	});

	Vector expected_means(2);
	expected_means[0] = 3.0;
	expected_means[1] = 40.0 / 3.0;
	assert_vector_close(mtrc::numeric::column_means(coordinates), expected_means);

	const Matrix scaled_copy = mtrc::numeric::columns_centered_to_unit_max_abs(coordinates);
	assert(close_to(coordinates(0, 0), 1.0));
	Matrix expected_scaled(3, 2);
	expected_scaled(0, 0) = -1.0;
	expected_scaled(0, 1) = -1.0;
	expected_scaled(1, 0) = 0.0;
	expected_scaled(1, 1) = 0.2;
	expected_scaled(2, 0) = 1.0;
	expected_scaled(2, 1) = 0.8;
	assert_matrix_close(scaled_copy, expected_scaled);

	mtrc::numeric::center_columns_to_unit_max_abs(coordinates);
	assert_matrix_close(coordinates, expected_scaled);
	const auto scaled_means = mtrc::numeric::column_means(coordinates);
	assert(close_to(scaled_means[0], 0.0));
	assert(close_to(scaled_means[1], 0.0));

	Matrix constant_column(2, 2);
	constant_column(0, 0) = 2.0;
	constant_column(0, 1) = 4.0;
	constant_column(1, 0) = 2.0;
	constant_column(1, 1) = 8.0;
	mtrc::numeric::center_columns_to_unit_max_abs(constant_column);
	Matrix expected_constant_column(2, 2);
	expected_constant_column(0, 0) = 0.0;
	expected_constant_column(0, 1) = -1.0;
	expected_constant_column(1, 0) = 0.0;
	expected_constant_column(1, 1) = 1.0;
	assert_matrix_close(constant_column, expected_constant_column);

	Matrix diagonal_matrix(2, 3, 0.0);
	mtrc::numeric::set_diagonal(diagonal_matrix, 4.0);
	Matrix expected_diagonal_matrix(2, 3, 0.0);
	expected_diagonal_matrix(0, 0) = 4.0;
	expected_diagonal_matrix(1, 1) = 4.0;
	assert_matrix_close(diagonal_matrix, expected_diagonal_matrix);
	mtrc::numeric::add_to_diagonal(diagonal_matrix, -1.5);
	expected_diagonal_matrix(0, 0) = 2.5;
	expected_diagonal_matrix(1, 1) = 2.5;
	assert_matrix_close(diagonal_matrix, expected_diagonal_matrix);

	Matrix stochastic(2, 2);
	stochastic(0, 0) = 0.25;
	stochastic(0, 1) = 0.75;
	stochastic(1, 0) = 0.5;
	stochastic(1, 1) = 0.5;
	const Matrix lazy_copy = mtrc::numeric::identity_blended(stochastic, 0.5);
	assert(close_to(stochastic(0, 0), 0.25));
	Matrix expected_lazy(2, 2);
	expected_lazy(0, 0) = 0.625;
	expected_lazy(0, 1) = 0.375;
	expected_lazy(1, 0) = 0.25;
	expected_lazy(1, 1) = 0.75;
	assert_matrix_close(lazy_copy, expected_lazy);
	mtrc::numeric::blend_with_identity(stochastic, 0.5);
	assert_matrix_close(stochastic, expected_lazy);
	const auto lazy_sums = mtrc::numeric::row_sums(stochastic);
	assert(close_to(lazy_sums[0], 1.0));
	assert(close_to(lazy_sums[1], 1.0));

	Matrix transition(2, 2);
	transition(0, 0) = 0.5;
	transition(0, 1) = 0.5;
	transition(1, 0) = 0.25;
	transition(1, 1) = 0.75;

	Matrix identity(2, 2, 0.0);
	identity(0, 0) = 1.0;
	identity(1, 1) = 1.0;
	assert_matrix_close(mtrc::numeric::matrix_power(transition, 0), identity);
	assert_matrix_close(mtrc::numeric::matrix_power(transition, 1), transition);

	Matrix transition_squared(2, 2);
	transition_squared(0, 0) = 0.375;
	transition_squared(0, 1) = 0.625;
	transition_squared(1, 0) = 0.3125;
	transition_squared(1, 1) = 0.6875;
	assert_matrix_close(mtrc::numeric::matrix_power(transition, 2), transition_squared);

	Matrix transition_cubed(2, 2);
	transition_cubed(0, 0) = 0.34375;
	transition_cubed(0, 1) = 0.65625;
	transition_cubed(1, 0) = 0.328125;
	transition_cubed(1, 1) = 0.671875;
	assert_matrix_close(mtrc::numeric::matrix_power(transition, 3), transition_cubed);

	Matrix zero_row(2, 2, 0.0);
	zero_row(0, 0) = 1.0;
	zero_row(0, 1) = 1.0;
	assert_invalid_argument([&zero_row]() { mtrc::numeric::normalize_rows(zero_row); });

	Matrix positive_with_zeros(2, 2, 0.0);
	positive_with_zeros(0, 0) = 1.0;
	positive_with_zeros(0, 1) = 1.0;
	assert(close_to(mtrc::numeric::positive_mean_or(positive_with_zeros, 7.0), 1.0));

	Matrix rectangular(2, 3, 1.0);
	assert_invalid_argument([&rectangular]() { (void)mtrc::numeric::matrix_power(rectangular, 2); });
	assert_invalid_argument([&rectangular]() { mtrc::numeric::blend_with_identity(rectangular, 0.5); });

	Matrix non_positive(2, 2);
	non_positive(0, 0) = 0.0;
	non_positive(0, 1) = -1.0;
	non_positive(1, 0) = -2.0;
	non_positive(1, 1) = 0.0;
	assert(close_to(mtrc::numeric::positive_mean_or(non_positive, 3.5), 3.5));

	return 0;
}
