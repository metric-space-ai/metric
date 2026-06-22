#include <cassert>
#include <cmath>

#include <metric/numeric/Math.h>

#include "numeric_test_helpers.hpp"

// Exercises the triangular and Hermitian matrix adaptors that callers use heavily
// (Lower/Upper/UniLower/UniUpper/StrictlyLower/StrictlyUpper/Hermitian) but that had no
// smoke coverage. The contract under test is the structural invariant each adaptor enforces
// on element access and how it composes with the dense numeric operations.
int main()
{
	using mtrc::test::close_to;
	using Dense = mtrc::numeric::DynamicMatrix<double>;

	// LowerMatrix: writes land in the lower triangle, the strict upper triangle reads as zero.
	mtrc::numeric::LowerMatrix<Dense> lower(3);
	lower(0, 0) = 1.0;
	lower(1, 0) = 2.0;
	lower(1, 1) = 3.0;
	lower(2, 0) = 4.0;
	lower(2, 1) = 5.0;
	lower(2, 2) = 6.0;
	assert(mtrc::numeric::isIntact(lower));
	assert(close_to(lower(0, 1), 0.0));
	assert(close_to(lower(0, 2), 0.0));
	assert(close_to(lower(1, 2), 0.0));
	assert(close_to(lower(2, 0), 4.0));

	// UpperMatrix mirrors the contract on the opposite triangle.
	mtrc::numeric::UpperMatrix<Dense> upper(3);
	upper(0, 2) = 7.0;
	upper(1, 1) = 8.0;
	assert(mtrc::numeric::isIntact(upper));
	assert(close_to(upper(0, 2), 7.0));
	assert(close_to(upper(2, 0), 0.0));
	assert(close_to(upper(1, 0), 0.0));

	// Unitriangular adaptors pin the diagonal to one.
	mtrc::numeric::UniLowerMatrix<Dense> uni_lower(3);
	uni_lower(1, 0) = -2.0;
	uni_lower(2, 0) = 0.5;
	assert(mtrc::numeric::isIntact(uni_lower));
	assert(close_to(uni_lower(0, 0), 1.0));
	assert(close_to(uni_lower(1, 1), 1.0));
	assert(close_to(uni_lower(2, 2), 1.0));
	assert(close_to(uni_lower(1, 0), -2.0));
	assert(close_to(uni_lower(0, 1), 0.0));

	mtrc::numeric::UniUpperMatrix<Dense> uni_upper(3);
	uni_upper(0, 1) = 3.0;
	assert(mtrc::numeric::isIntact(uni_upper));
	assert(close_to(uni_upper(2, 2), 1.0));
	assert(close_to(uni_upper(0, 1), 3.0));
	assert(close_to(uni_upper(1, 0), 0.0));

	// Strictly-triangular adaptors pin the whole diagonal (and the other triangle) to zero.
	mtrc::numeric::StrictlyLowerMatrix<Dense> strict_lower(3);
	strict_lower(2, 0) = 9.0;
	assert(mtrc::numeric::isIntact(strict_lower));
	assert(close_to(strict_lower(0, 0), 0.0));
	assert(close_to(strict_lower(1, 1), 0.0));
	assert(close_to(strict_lower(2, 0), 9.0));
	assert(close_to(strict_lower(0, 2), 0.0));

	mtrc::numeric::StrictlyUpperMatrix<Dense> strict_upper(3);
	strict_upper(0, 2) = -4.0;
	assert(mtrc::numeric::isIntact(strict_upper));
	assert(close_to(strict_upper(2, 2), 0.0));
	assert(close_to(strict_upper(0, 2), -4.0));
	assert(close_to(strict_upper(2, 0), 0.0));

	// The lower-triangular factor still participates in dense linear algebra.
	mtrc::numeric::DynamicVector<double> rhs(3);
	rhs[0] = 1.0;
	rhs[1] = 2.0;
	rhs[2] = 3.0;
	const mtrc::numeric::DynamicVector<double> product = lower * rhs;
	assert(product.size() == 3);
	assert(close_to(product[0], 1.0));         // 1*1
	assert(close_to(product[1], 2.0 + 6.0));   // 2*1 + 3*2
	assert(close_to(product[2], 4.0 + 10.0 + 18.0));

	// randomize() on an adaptor honours its structural invariant (container Rand specialization).
	mtrc::numeric::LowerMatrix<Dense> random_lower(4);
	mtrc::numeric::randomize(random_lower);
	assert(mtrc::numeric::isIntact(random_lower));
	for (std::size_t row = 0; row < random_lower.rows(); ++row) {
		for (std::size_t column = row + 1; column < random_lower.columns(); ++column) {
			assert(close_to(random_lower(row, column), 0.0));
		}
	}

	// HermitianMatrix over a real element type behaves as a symmetric adaptor.
	mtrc::numeric::HermitianMatrix<Dense> hermitian_real(3);
	hermitian_real(0, 1) = 2.5;
	hermitian_real(0, 2) = -1.0;
	assert(mtrc::numeric::isIntact(hermitian_real));
	assert(close_to(hermitian_real(1, 0), 2.5));
	assert(close_to(hermitian_real(2, 0), -1.0));

	// HermitianMatrix over a complex element type enforces conjugate symmetry.
	using Complex = mtrc::numeric::complex<double>;
	mtrc::numeric::HermitianMatrix<mtrc::numeric::DynamicMatrix<Complex>> hermitian(2);
	hermitian(0, 1) = Complex(1.0, 2.0);
	assert(mtrc::numeric::isIntact(hermitian));
	const Complex mirrored = hermitian(1, 0);
	assert(close_to(mirrored.real(), 1.0));
	assert(close_to(mirrored.imag(), -2.0));

	return 0;
}
