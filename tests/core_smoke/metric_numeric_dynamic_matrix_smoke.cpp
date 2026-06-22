#include <cassert>

#include <metric/numeric/Math.h>

int main()
{
	mtrc::numeric::DynamicMatrix<double> lhs(2, 3);
	lhs(0, 0) = 1.0;
	lhs(0, 1) = 2.0;
	lhs(0, 2) = 3.0;
	lhs(1, 0) = 4.0;
	lhs(1, 1) = 5.0;
	lhs(1, 2) = 6.0;

	mtrc::numeric::DynamicMatrix<double> rhs(3, 2);
	rhs(0, 0) = 7.0;
	rhs(0, 1) = 8.0;
	rhs(1, 0) = 9.0;
	rhs(1, 1) = 10.0;
	rhs(2, 0) = 11.0;
	rhs(2, 1) = 12.0;

	const auto product = lhs * rhs;
	assert(product.rows() == 2);
	assert(product.columns() == 2);
	assert(product(0, 0) == 58.0);
	assert(product(0, 1) == 64.0);
	assert(product(1, 0) == 139.0);
	assert(product(1, 1) == 154.0);

	mtrc::numeric::DynamicMatrix<double> filled(2, 2, 3.5);
	assert(filled(0, 0) == 3.5);
	assert(filled(1, 1) == 3.5);

	return 0;
}
