#include <cassert>
#include <cmath>

#include <metric/numeric/Math.h>

#include "numeric_test_helpers.hpp"

auto main() -> int
{
	using Matrix = mtrc::numeric::DynamicMatrix<double>;
	using Vector = mtrc::numeric::DynamicVector<double>;
	using mtrc::test::assert_matrix_close;
	using mtrc::test::assert_vector_close;
	using mtrc::test::close_to;

	Matrix system(7, 7, 0.0);
	for (std::size_t index = 0; index < 7; ++index) {
		system(index, index) = 4.0 + static_cast<double>(index);
	}
	for (std::size_t index = 0; index < 6; ++index) {
		system(index, index + 1) = 0.25;
		system(index + 1, index) = -0.125;
	}

	Vector expected(7);
	for (std::size_t index = 0; index < expected.size(); ++index) {
		expected[index] = 1.0 + static_cast<double>(index);
	}

	Vector rhs(7, 0.0);
	for (std::size_t row = 0; row < system.rows(); ++row) {
		for (std::size_t column = 0; column < system.columns(); ++column) {
			rhs[row] += system(row, column) * expected[column];
		}
	}

	Vector solution;
	mtrc::numeric::solve(system, solution, rhs);
	assert_vector_close(solution, expected);
	assert_vector_close(system * solution, rhs);

	Matrix expected_matrix(7, 2);
	for (std::size_t row = 0; row < expected_matrix.rows(); ++row) {
		expected_matrix(row, 0) = 1.0 + static_cast<double>(row);
		expected_matrix(row, 1) = 3.5 - 0.5 * static_cast<double>(row);
	}

	Matrix rhs_matrix(7, 2, 0.0);
	for (std::size_t row = 0; row < system.rows(); ++row) {
		for (std::size_t column = 0; column < system.columns(); ++column) {
			for (std::size_t rhs_column = 0; rhs_column < rhs_matrix.columns(); ++rhs_column) {
				rhs_matrix(row, rhs_column) += system(row, column) * expected_matrix(column, rhs_column);
			}
		}
	}

	Matrix solution_matrix;
	mtrc::numeric::solve(system, solution_matrix, rhs_matrix);
	assert_matrix_close(solution_matrix, expected_matrix);
	assert_matrix_close(system * solution_matrix, rhs_matrix);

	using SymmetricMatrix = mtrc::numeric::SymmetricMatrix<Matrix>;
	SymmetricMatrix covariance(2);
	covariance(0, 0) = 2.0;
	covariance(0, 1) = 1.0;
	covariance(1, 1) = 2.0;

	Vector eigenvalues;
	Matrix eigenvectors;
	mtrc::numeric::eigen(covariance, eigenvalues, eigenvectors);

	assert(eigenvalues.size() == 2);
	assert(close_to(eigenvalues[0], 1.0));
	assert(close_to(eigenvalues[1], 3.0));
	assert_matrix_close(eigenvectors * mtrc::numeric::trans(eigenvectors),
						Matrix(2, 2, 0.0) + mtrc::numeric::IdentityMatrix<double>(2));

	const auto first_eigenvector = mtrc::numeric::row(eigenvectors, 0UL);
	const auto second_eigenvector = mtrc::numeric::row(eigenvectors, 1UL);
	assert_vector_close(first_eigenvector * covariance, eigenvalues[0] * first_eigenvector);
	assert_vector_close(second_eigenvector * covariance, eigenvalues[1] * second_eigenvector);

	Matrix rotation(2, 2);
	rotation(0, 0) = 0.0;
	rotation(0, 1) = -1.0;
	rotation(1, 0) = 1.0;
	rotation(1, 1) = 0.0;

	mtrc::numeric::DynamicVector<mtrc::numeric::complex<double>, mtrc::numeric::columnVector> complex_eigenvalues;
	mtrc::numeric::eigen(rotation, complex_eigenvalues);

	assert(complex_eigenvalues.size() == 2);
	for (std::size_t index = 0; index < complex_eigenvalues.size(); ++index) {
		assert(close_to(mtrc::numeric::real(complex_eigenvalues[index]), 0.0));
		assert(close_to(std::abs(mtrc::numeric::imag(complex_eigenvalues[index])), 1.0));
	}

	return 0;
}
