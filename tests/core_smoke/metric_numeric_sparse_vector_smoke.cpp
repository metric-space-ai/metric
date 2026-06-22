#include <cassert>
#include <cmath>
#include <cstddef>
#include <vector>

#include <metric/numeric/Math.h>

#include "numeric_test_helpers.hpp"

int main()
{
	using mtrc::test::close_to;

	mtrc::numeric::CompressedVector<double> sparse(6);
	sparse.reserve(4);
	sparse.append(0, 1.5);
	sparse.append(3, -2.0);
	sparse.set(5, 4.0);

	assert(sparse.size() == 6);
	assert(sparse.nonZeros() == 3);
	assert(close_to(sparse[0], 1.5));
	assert(close_to(sparse[3], -2.0));
	assert(close_to(sparse[5], 4.0));

	const auto &const_sparse = sparse;
	assert(close_to(const_sparse[1], 0.0));
	assert(close_to(const_sparse[4], 0.0));

	std::vector<std::size_t> indices;
	std::vector<double> values;
	for (auto it = sparse.cbegin(); it != sparse.cend(); ++it) {
		indices.push_back(it->index());
		values.push_back(it->value());
	}
	assert((indices == std::vector<std::size_t>{0, 3, 5}));
	assert(close_to(values[0], 1.5));
	assert(close_to(values[1], -2.0));
	assert(close_to(values[2], 4.0));

	mtrc::numeric::DynamicVector<double> dense(6);
	dense[0] = 2.0;
	dense[1] = 3.0;
	dense[2] = 4.0;
	dense[3] = 5.0;
	dense[4] = 6.0;
	dense[5] = 7.0;

	const auto dot = mtrc::numeric::trans(sparse) * dense;
	assert(close_to(dot, 21.0));

	const mtrc::numeric::DynamicVector<double> sum = sparse + dense;
	assert(close_to(sum[0], 3.5));
	assert(close_to(sum[1], 3.0));
	assert(close_to(sum[3], 3.0));
	assert(close_to(sum[5], 11.0));

	const mtrc::numeric::CompressedVector<double> scaled = sparse * 2.0;
	assert(scaled.nonZeros() == 3);
	assert(close_to(scaled[0], 3.0));
	assert(close_to(scaled[3], -4.0));
	assert(close_to(scaled[5], 8.0));

	mtrc::numeric::CompressedVector<double, mtrc::numeric::rowVector> row_sparse(6);
	row_sparse.reserve(2);
	row_sparse.append(1, 3.0);
	row_sparse.append(4, -1.0);

	const auto row_dot = row_sparse * dense;
	assert(close_to(row_dot, 3.0));

	const auto outer = sparse * row_sparse;
	assert(outer.rows() == 6);
	assert(outer.columns() == 6);
	assert(close_to(outer(0, 1), 4.5));
	assert(close_to(outer(3, 4), 2.0));
	assert(close_to(outer(5, 1), 12.0));
	assert(close_to(outer(2, 2), 0.0));

	return 0;
}
