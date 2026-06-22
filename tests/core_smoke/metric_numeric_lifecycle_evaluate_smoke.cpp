#include <cassert>
#include <cmath>

#include <metric/numeric/Math.h>

#include "numeric_test_helpers.hpp"

auto main() -> int
{
	using Matrix = mtrc::numeric::DynamicMatrix<double>;
	using SparseMatrix = mtrc::numeric::CompressedMatrix<double, mtrc::numeric::rowMajor>;
	using Vector = mtrc::numeric::DynamicVector<double>;
	using mtrc::test::close_to;

	Vector mu(3);
	mu[0] = 1.0;
	mu[1] = -2.0;
	mu[2] = 3.0;

	mtrc::numeric::reset(mu);
	assert(mu.size() == 3);
	assert(close_to(mu[0], 0.0));
	assert(close_to(mu[1], 0.0));
	assert(close_to(mu[2], 0.0));

	Matrix nodes(3, 2);
	nodes(0, 0) = 1.0;
	nodes(0, 1) = 2.0;
	nodes(1, 0) = 3.0;
	nodes(1, 1) = 4.0;
	nodes(2, 0) = 5.0;
	nodes(2, 1) = 6.0;

	Matrix covariance_like = mtrc::numeric::evaluate(mtrc::numeric::trans(nodes) * nodes +
													   mtrc::numeric::IdentityMatrix<double>(2) * 0.5);
	assert(covariance_like.rows() == 2);
	assert(covariance_like.columns() == 2);
	assert(close_to(covariance_like(0, 0), 35.5));
	assert(close_to(covariance_like(0, 1), 44.0));
	assert(close_to(covariance_like(1, 0), 44.0));
	assert(close_to(covariance_like(1, 1), 56.5));

	Vector weights(2);
	weights[0] = 0.25;
	weights[1] = -0.5;

	const Vector projected = mtrc::numeric::evaluate(covariance_like * weights);
	assert(projected.size() == 2);
	assert(close_to(projected[0], -13.125));
	assert(close_to(projected[1], -17.25));

	mtrc::numeric::reset(covariance_like, 0UL);
	assert(covariance_like.rows() == 2);
	assert(covariance_like.columns() == 2);
	assert(close_to(covariance_like(0, 0), 0.0));
	assert(close_to(covariance_like(0, 1), 0.0));
	assert(close_to(covariance_like(1, 0), 44.0));
	assert(close_to(covariance_like(1, 1), 56.5));

	mtrc::numeric::reset(covariance_like);
	assert(covariance_like.rows() == 2);
	assert(covariance_like.columns() == 2);
	assert(close_to(covariance_like(0, 0), 0.0));
	assert(close_to(covariance_like(1, 1), 0.0));

	Matrix dense_to_clear(2, 2, 7.0);
	mtrc::numeric::clear(dense_to_clear);
	assert(dense_to_clear.rows() == 0);
	assert(dense_to_clear.columns() == 0);
	dense_to_clear.shrinkToFit();
	assert(dense_to_clear.capacity() == 0);

	Vector vector_to_clear(4, 3.0);
	mtrc::numeric::clear(vector_to_clear);
	assert(vector_to_clear.size() == 0);
	vector_to_clear.shrinkToFit();
	assert(vector_to_clear.capacity() == 0);

	SparseMatrix sparse(3, 3);
	sparse.reserve(3);
	sparse.append(0, 0, 1.0);
	sparse.finalize(0);
	sparse.append(1, 2, -2.0);
	sparse.finalize(1);
	sparse.append(2, 1, 3.0);
	sparse.finalize(2);
	assert(sparse.nonZeros() == 3);

	mtrc::numeric::clear(sparse);
	assert(sparse.rows() == 0);
	assert(sparse.columns() == 0);
	assert(sparse.nonZeros() == 0);
	sparse.shrinkToFit();
	assert(sparse.capacity() == 0);

	double scalar = 3.5;
	mtrc::numeric::reset(scalar);
	assert(close_to(scalar, 0.0));

	return 0;
}
