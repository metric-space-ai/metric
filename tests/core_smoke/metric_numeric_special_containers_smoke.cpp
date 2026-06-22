#include <cassert>
#include <cmath>
#include <cstddef>

#include <metric/numeric/Math.h>

#include "numeric_test_helpers.hpp"

// Exercises the promoted special-purpose vector containers ZeroVector (immutable all-zero vector)
// and InitializerVector (a dense view over an initializer list) that had no smoke coverage.
int main()
{
	using mtrc::test::close_to;

	// ZeroVector: a sized, immutable zero vector that composes with dense vectors.
	mtrc::numeric::ZeroVector<double> zeros(4);
	assert(zeros.size() == 4);
	assert(zeros.nonZeros() == 0);
	assert(close_to(zeros[0], 0.0));
	assert(close_to(zeros[3], 0.0));
	assert(close_to(mtrc::numeric::sum(zeros), 0.0));

	mtrc::numeric::DynamicVector<double> dense(4);
	dense[0] = 1.0;
	dense[1] = 2.0;
	dense[2] = 3.0;
	dense[3] = 4.0;

	const mtrc::numeric::DynamicVector<double> combined = dense + zeros;
	assert(combined.size() == 4);
	assert(close_to(combined[1], 2.0));
	assert(close_to(combined[3], 4.0));
	assert(close_to(mtrc::numeric::dot(dense, zeros), 0.0));

	// InitializerVector: binds to an initializer list and reads as a dense column vector.
	const auto list = {2.0, 6.0, -1.0, 3.0, 5.0};
	mtrc::numeric::InitializerVector<double> init(list);
	assert(init.size() == 5);
	assert(close_to(init[0], 2.0));
	assert(close_to(init[2], -1.0));
	assert(close_to(mtrc::numeric::sum(init), 15.0));

	// It materialises into an owning dense vector with the same contents.
	const mtrc::numeric::DynamicVector<double> materialized = init;
	assert(materialized.size() == 5);
	assert(close_to(materialized[3], 3.0));
	assert(close_to(materialized[4], 5.0));

	// The extended form pads the represented list up to the requested size with zeros.
	mtrc::numeric::InitializerVector<double> padded({1.0, 2.0}, 4);
	assert(padded.size() == 4);
	assert(close_to(padded[0], 1.0));
	assert(close_to(padded[1], 2.0));
	assert(close_to(padded[2], 0.0));
	assert(close_to(padded[3], 0.0));

	return 0;
}
