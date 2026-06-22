#include <cassert>
#include <cmath>

#include <metric/numeric/Math.h>

#include "numeric_test_helpers.hpp"

auto main() -> int
{
	using Matrix = mtrc::numeric::DynamicMatrix<double>;
	using Vector = mtrc::numeric::DynamicVector<double>;
	using mtrc::test::assert_vector_close;
	using mtrc::test::close_to;

	Matrix sigma(3, 3);
	sigma(0, 0) = 4.0;
	sigma(0, 1) = 0.5;
	sigma(0, 2) = 0.25;
	sigma(1, 0) = 0.5;
	sigma(1, 1) = 9.0;
	sigma(1, 2) = 0.75;
	sigma(2, 0) = 0.25;
	sigma(2, 1) = 0.75;
	sigma(2, 2) = 16.0;

	auto diagonal = mtrc::numeric::diagonal(sigma);
	assert(diagonal.size() == 3);
	assert(close_to(diagonal[0], 4.0));
	assert(close_to(diagonal[1], 9.0));
	assert(close_to(diagonal[2], 16.0));

	diagonal[1] = 12.0;
	assert(close_to(sigma(1, 1), 12.0));
	assert(close_to(sigma(1, 2), 0.75));

	Vector replacement(3);
	replacement[0] = 8.0;
	replacement[1] = 18.0;
	replacement[2] = 32.0;
	mtrc::numeric::diagonal(sigma) = replacement;

	assert(close_to(sigma(0, 0), 8.0));
	assert(close_to(sigma(1, 1), 18.0));
	assert(close_to(sigma(2, 2), 32.0));
	assert(close_to(sigma(0, 1), 0.5));
	assert(close_to(sigma(2, 1), 0.75));

	const Matrix fixed_sigma = sigma;
	const Vector copied_diagonal = mtrc::numeric::diagonal(fixed_sigma);
	assert_vector_close(copied_diagonal, replacement);

	Vector tau_site(3);
	tau_site[0] = 0.125;
	tau_site[1] = 0.25;
	tau_site[2] = 0.03125;

	Vector mu(3);
	mu[0] = 2.0;
	mu[1] = 9.0;
	mu[2] = 16.0;

	Vector nu_site(3);
	nu_site[0] = 0.05;
	nu_site[1] = 0.125;
	nu_site[2] = 0.25;

	const Vector tau_cavity = 1.0 / mtrc::numeric::diagonal(sigma) - tau_site;
	assert(close_to(tau_cavity[0], 0.0));
	assert(close_to(tau_cavity[1], 1.0 / 18.0 - 0.25));
	assert(close_to(tau_cavity[2], 0.0));

	const Vector nu_cavity = mu / mtrc::numeric::diagonal(sigma) - nu_site;
	assert(close_to(nu_cavity[0], 0.20));
	assert(close_to(nu_cavity[1], 0.375));
	assert(close_to(nu_cavity[2], 0.25));

	mtrc::numeric::CompressedMatrix<double, mtrc::numeric::rowMajor> sparse(3, 3);
	sparse.reserve(4);
	sparse.append(0, 0, 3.0);
	sparse.append(0, 2, -1.0);
	sparse.finalize(0);
	sparse.append(1, 1, 5.0);
	sparse.finalize(1);
	sparse.append(2, 2, 7.0);
	sparse.finalize(2);

	const Vector sparse_diagonal = mtrc::numeric::diagonal(sparse);
	assert(close_to(sparse_diagonal[0], 3.0));
	assert(close_to(sparse_diagonal[1], 5.0));
	assert(close_to(sparse_diagonal[2], 7.0));

	return 0;
}
