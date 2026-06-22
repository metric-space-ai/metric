#include <cassert>
#include <cmath>

#include <metric/numeric/Math.h>

#include "numeric_test_helpers.hpp"

int main()
{
	using mtrc::test::close_to;

	mtrc::numeric::DynamicMatrix<double> nodes(3, 2);
	nodes(0, 0) = 3.0;
	nodes(0, 1) = 7.0;
	nodes(1, 0) = 5.0;
	nodes(1, 1) = 9.0;
	nodes(2, 0) = 1.0;
	nodes(2, 1) = 5.0;

	mtrc::numeric::DynamicVector<double> mean(2);
	mean[0] = 3.0;
	mean[1] = 7.0;

	const mtrc::numeric::DynamicMatrix<double> centered =
		nodes - mtrc::numeric::expand(mtrc::numeric::trans(mean), nodes.rows());
	assert(centered.rows() == 3);
	assert(centered.columns() == 2);
	assert(close_to(centered(0, 0), 0.0));
	assert(close_to(centered(0, 1), 0.0));
	assert(close_to(centered(1, 0), 2.0));
	assert(close_to(centered(1, 1), 2.0));
	assert(close_to(centered(2, 0), -2.0));
	assert(close_to(centered(2, 1), -2.0));

	mtrc::numeric::DynamicVector<double> column_values(3);
	column_values[0] = 1.0;
	column_values[1] = -2.0;
	column_values[2] = 4.0;

	const mtrc::numeric::DynamicMatrix<double> column_expanded = mtrc::numeric::expand(column_values, 2UL);
	assert(column_expanded.rows() == 3);
	assert(column_expanded.columns() == 2);
	assert(close_to(column_expanded(0, 0), 1.0));
	assert(close_to(column_expanded(0, 1), 1.0));
	assert(close_to(column_expanded(1, 0), -2.0));
	assert(close_to(column_expanded(1, 1), -2.0));
	assert(close_to(column_expanded(2, 0), 4.0));
	assert(close_to(column_expanded(2, 1), 4.0));

	using Complex = mtrc::numeric::complex<double>;
	mtrc::numeric::DynamicMatrix<Complex> complex_matrix(2, 2);
	complex_matrix(0, 0) = Complex(1.0, 2.0);
	complex_matrix(0, 1) = Complex(-3.0, 4.0);
	complex_matrix(1, 0) = Complex(5.0, -6.0);
	complex_matrix(1, 1) = Complex(-7.0, -8.0);

	const mtrc::numeric::DynamicMatrix<Complex> hermitian = mtrc::numeric::ctrans(complex_matrix);
	assert(close_to(mtrc::numeric::real(hermitian(0, 0)), 1.0));
	assert(close_to(mtrc::numeric::imag(hermitian(0, 0)), -2.0));
	assert(close_to(mtrc::numeric::real(hermitian(0, 1)), 5.0));
	assert(close_to(mtrc::numeric::imag(hermitian(0, 1)), 6.0));
	assert(close_to(mtrc::numeric::real(hermitian(1, 0)), -3.0));
	assert(close_to(mtrc::numeric::imag(hermitian(1, 0)), -4.0));
	assert(close_to(mtrc::numeric::real(hermitian(1, 1)), -7.0));
	assert(close_to(mtrc::numeric::imag(hermitian(1, 1)), 8.0));

	mtrc::numeric::CompressedMatrix<double, mtrc::numeric::columnMajor> sparse(3, 2);
	sparse.reserve(3);
	sparse.set(0, 0, 2.0);
	sparse.set(2, 0, -1.0);
	sparse.set(1, 1, 4.0);

	const mtrc::numeric::DynamicMatrix<double> sparse_adjoint = mtrc::numeric::ctrans(sparse);
	assert(sparse_adjoint.rows() == 2);
	assert(sparse_adjoint.columns() == 3);
	assert(close_to(sparse_adjoint(0, 0), 2.0));
	assert(close_to(sparse_adjoint(0, 1), 0.0));
	assert(close_to(sparse_adjoint(0, 2), -1.0));
	assert(close_to(sparse_adjoint(1, 0), 0.0));
	assert(close_to(sparse_adjoint(1, 1), 4.0));
	assert(close_to(sparse_adjoint(1, 2), 0.0));

	return 0;
}
