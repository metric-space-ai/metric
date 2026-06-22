// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_BLAS_TRMM_H
#define METRIC_NUMERIC_MATH_BLAS_TRMM_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/Aliases.h>
#include <metric/numeric/math/blas/cblas/trmm.h>
#include <metric/numeric/math/constraints/BLASCompatible.h>
#include <metric/numeric/math/constraints/Computation.h>
#include <metric/numeric/math/constraints/ConstDataAccess.h>
#include <metric/numeric/math/constraints/MutableDataAccess.h>
#include <metric/numeric/math/expressions/DenseMatrix.h>
#include <metric/numeric/math/typetraits/IsRowMajorMatrix.h>
#include <metric/numeric/system/BLAS.h>
#include <metric/numeric/util/Assert.h>
#include <metric/numeric/util/NumericCast.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  BLAS TRIANGULAR MATRIX MULTIPLICATION FUNCTIONS (TRMM)
//
//=================================================================================================

//*************************************************************************************************
/*!\name BLAS triangular matrix multiplication functions (trmm) */
//@{
#if METRIC_NUMERIC_BLAS_MODE

template <typename MT1, bool SO1, typename MT2, bool SO2, typename ST>
void trmm(DenseMatrix<MT1, SO1> &B, const DenseMatrix<MT2, SO2> &A, CBLAS_SIDE side, CBLAS_UPLO uplo, ST alpha);

#endif
//@}
//*************************************************************************************************

//*************************************************************************************************
#if METRIC_NUMERIC_BLAS_MODE
/*!\brief BLAS kernel for a triangular dense matrix/dense matrix multiplication
//        (\f$ B=\alpha*A*B \f$ or \f$ B=\alpha*B*A \f$).
// \ingroup blas
//
// \param B The target dense matrix.
// \param A The dense matrix multiplication operand.
// \param side \a CblasLeft to compute \f$ B=\alpha*A*B \f$, \a CblasRight to compute \f$ B=\alpha*B*A \f$.
// \param uplo \a CblasLower to use the lower triangle from \a A, \a CblasUpper to use the upper triangle.
// \param alpha The scaling factor for \f$ A*B \f$ or \f$ B*A \f$.
// \return void
//
// This function performs the scaling and multiplication of a triangular matrix by a matrix
// based on the BLAS trmm() functions. Note that the function only works for matrices with
// \c float, \c double, \c complex<float>, and \c complex<double> element type. The attempt to
// call the function with matrices of any other element type results in a compile time error.
// Also note that matrix \a A is expected to be a square matrix.
//
// \note This function can only be used if a fitting BLAS library, which supports this function,
// is available and linked to the executable. Otherwise a call to this function will result in a
// linker error.
*/
template <typename MT1 // Type of the target matrix
		  ,
		  bool SO1 // Storage order of the target matrix
		  ,
		  typename MT2 // Type of the left-hand side matrix operand
		  ,
		  bool SO2 // Storage order of the left-hand side matrix operand
		  ,
		  typename ST> // Type of the scalar factor
inline void trmm(DenseMatrix<MT1, SO1> &B, const DenseMatrix<MT2, SO2> &A, CBLAS_SIDE side, CBLAS_UPLO uplo, ST alpha)
{
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_COMPUTATION_TYPE(MT1);
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_COMPUTATION_TYPE(MT2);

	METRIC_NUMERIC_CONSTRAINT_MUST_HAVE_MUTABLE_DATA_ACCESS(MT1);
	METRIC_NUMERIC_CONSTRAINT_MUST_HAVE_CONST_DATA_ACCESS(MT2);

	METRIC_NUMERIC_CONSTRAINT_MUST_BE_BLAS_COMPATIBLE_TYPE(ElementType_t<MT1>);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_BLAS_COMPATIBLE_TYPE(ElementType_t<MT2>);

	METRIC_NUMERIC_INTERNAL_ASSERT((*A).rows() == (*A).columns(), "Non-square triangular matrix detected");
	METRIC_NUMERIC_INTERNAL_ASSERT(side == CblasLeft || side == CblasRight, "Invalid side argument detected");
	METRIC_NUMERIC_INTERNAL_ASSERT(uplo == CblasLower || uplo == CblasUpper, "Invalid uplo argument detected");

	const blas_int_t m(numeric_cast<blas_int_t>((*B).rows()));
	const blas_int_t n(numeric_cast<blas_int_t>((*B).columns()));
	const blas_int_t lda(numeric_cast<blas_int_t>((*A).spacing()));
	const blas_int_t ldb(numeric_cast<blas_int_t>((*B).spacing()));

	trmm((IsRowMajorMatrix_v<MT1>) ? (CblasRowMajor) : (CblasColMajor), side,
		 (SO1 == SO2) ? (uplo) : ((uplo == CblasLower) ? (CblasUpper) : (CblasLower)),
		 (SO1 == SO2) ? (CblasNoTrans) : (CblasTrans), CblasNonUnit, m, n, alpha, (*A).data(), lda, (*B).data(), ldb);
}
#endif
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
