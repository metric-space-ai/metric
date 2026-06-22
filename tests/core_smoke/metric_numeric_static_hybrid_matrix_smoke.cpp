#include <cassert>
#include <cmath>

#include <metric/numeric/Math.h>

#include "numeric_test_helpers.hpp"

// Exercises the fixed-capacity dense matrix containers StaticMatrix (compile-time shape) and
// HybridMatrix (compile-time capacity, runtime shape). Both are in the promoted API but had no
// dedicated smoke coverage; the vector counterparts are covered by fixed_vector_storage.
int main()
{
	using mtrc::test::close_to;

	// StaticMatrix: nested initializer-list construction and 2D element access.
	mtrc::numeric::StaticMatrix<double, 2UL, 3UL, mtrc::numeric::rowMajor> a{{1.0, 2.0, 3.0}, {4.0, 5.0, 6.0}};
	assert(a.rows() == 2);
	assert(a.columns() == 3);
	assert(close_to(a(0, 0), 1.0));
	assert(close_to(a(1, 2), 6.0));
	assert(close_to(mtrc::numeric::sum(a), 21.0));

	// Transpose yields a 3x2 dynamic result that round-trips the entries.
	const mtrc::numeric::DynamicMatrix<double> transposed = mtrc::numeric::trans(a);
	assert(transposed.rows() == 3);
	assert(transposed.columns() == 2);
	assert(close_to(transposed(2, 1), 6.0));

	// StaticMatrix participates in matrix/vector products with the right fixed shape.
	mtrc::numeric::StaticVector<double, 3UL> x{1.0, 0.0, -1.0};
	const mtrc::numeric::StaticVector<double, 2UL> y = a * x;
	assert(close_to(y[0], 1.0 - 3.0));
	assert(close_to(y[1], 4.0 - 6.0));

	// Column-major StaticMatrix stores the same logical values.
	mtrc::numeric::StaticMatrix<double, 2UL, 2UL, mtrc::numeric::columnMajor> cm{{1.0, 2.0}, {3.0, 4.0}};
	assert(close_to(cm(0, 1), 2.0));
	assert(close_to(cm(1, 0), 3.0));

	// HybridMatrix: capacity 4x4, used at a smaller runtime shape, then resized.
	mtrc::numeric::HybridMatrix<double, 4UL, 4UL> h(2, 2);
	h(0, 0) = 1.0;
	h(0, 1) = 2.0;
	h(1, 0) = 3.0;
	h(1, 1) = 4.0;
	assert(h.rows() == 2);
	assert(h.columns() == 2);
	assert(close_to(mtrc::numeric::sum(h), 10.0));

	h.resize(3, 3);
	assert(h.rows() == 3);
	assert(h.columns() == 3);
	h(2, 2) = 9.0;
	assert(close_to(h(2, 2), 9.0));

	// HybridMatrix interoperates with the dense expression layer.
	const mtrc::numeric::HybridMatrix<double, 4UL, 4UL> scaled = 2.0 * h;
	assert(close_to(scaled(0, 0), 2.0));
	assert(close_to(scaled(2, 2), 18.0));

	return 0;
}
