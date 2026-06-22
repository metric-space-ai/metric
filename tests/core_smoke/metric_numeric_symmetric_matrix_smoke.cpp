#include <cassert>
#include <cmath>

#include <metric/numeric/Math.h>

#include "numeric_test_helpers.hpp"

int main()
{
	using Matrix = mtrc::numeric::DynamicMatrix<double>;
	using SymmetricMatrix = mtrc::numeric::SymmetricMatrix<Matrix>;
	using mtrc::test::close_to;

	SymmetricMatrix distances(4);
	assert(distances.rows() == 4);
	assert(distances.columns() == 4);

	distances(0, 0) = 0.0;
	distances(0, 1) = 2.0;
	distances(0, 2) = 5.0;
	distances(0, 3) = 9.0;
	distances(1, 2) = 3.0;
	distances(1, 3) = 7.0;
	distances(2, 3) = 4.0;

	assert(close_to(distances(1, 0), 2.0));
	assert(close_to(distances(2, 0), 5.0));
	assert(close_to(distances(3, 0), 9.0));
	assert(close_to(distances(2, 1), 3.0));
	assert(close_to(distances(3, 1), 7.0));
	assert(close_to(distances(3, 2), 4.0));

	distances(3, 1) = 8.0;
	assert(close_to(distances(1, 3), 8.0));

	const auto thresholded = mtrc::numeric::map(distances, [](double value) { return value > 4.0 ? 4.0 : value; });
	assert(close_to(thresholded(0, 3), 4.0));
	assert(close_to(thresholded(3, 0), 4.0));
	assert(close_to(thresholded(1, 2), 3.0));

	const SymmetricMatrix weighted = (2.0 * thresholded) + distances;
	assert(close_to(weighted(0, 1), 6.0));
	assert(close_to(weighted(1, 0), 6.0));
	assert(close_to(weighted(0, 3), 17.0));
	assert(close_to(weighted(3, 0), 17.0));

	mtrc::numeric::DynamicVector<double> weights(4);
	weights[0] = 1.0;
	weights[1] = 0.5;
	weights[2] = -1.0;
	weights[3] = 2.0;

	const mtrc::numeric::DynamicVector<double> product = distances * weights;
	assert(product.size() == 4);
	assert(close_to(product[0], 14.0));
	assert(close_to(product[1], 15.0));
	assert(close_to(product[2], 14.5));
	assert(close_to(product[3], 9.0));

	return 0;
}
