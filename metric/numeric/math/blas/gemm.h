// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_BLAS_GEMM_H
#define METRIC_NUMERIC_MATH_BLAS_GEMM_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/Aliases.h>
#include <metric/numeric/math/blas/cblas/gemm.h>
#include <metric/numeric/math/constraints/BLASCompatible.h>
#include <metric/numeric/math/constraints/Computation.h>
#include <metric/numeric/math/constraints/ConstDataAccess.h>
#include <metric/numeric/math/constraints/MutableDataAccess.h>
#include <metric/numeric/math/expressions/DenseMatrix.h>
#include <metric/numeric/math/typetraits/IsRowMajorMatrix.h>
#include <metric/numeric/system/BLAS.h>
#include <metric/numeric/util/NumericCast.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  BLAS GENERAL MATRIX MULTIPLICATION FUNCTIONS (GEMM)
//
//=================================================================================================

//*************************************************************************************************
/*!\name BLAS general matrix multiplication functions (gemm) */
//@{
#if METRIC_NUMERIC_BLAS_MODE

template <typename MT1, bool SO1, typename MT2, bool SO2, typename MT3, bool SO3, typename ST>
void gemm(DenseMatrix<MT1, SO1> &C, const DenseMatrix<MT2, SO2> &A, const DenseMatrix<MT3, SO3> &B, ST alpha, ST beta);

#endif
//@}
//*************************************************************************************************

//*************************************************************************************************
#if METRIC_NUMERIC_BLAS_MODE
/*!\brief BLAS kernel for a dense matrix/dense matrix multiplication (\f$ C=\alpha*A*B+\beta*C \f$).
// \ingroup blas
//
// \param C The target left-hand side dense matrix.
// \param A The left-hand side multiplication operand.
// \param B The right-hand side multiplication operand.
// \param alpha The scaling factor for \f$ A*B \f$.
// \param beta The scaling factor for \f$ C \f$.
// \return void
//
// This function performs the dense matrix/dense matrix multiplication based on the BLAS
// gemm() functions. Note that the function only works for matrices with \c float, \c double,
// \c complex<float>, and \c complex<double> element type. The attempt to call the function
// with matrices of any other element type results in a compile time error.
//
// \note This function can only be used if a fitting BLAS library, which supports this function,
// is available and linked to the executable. Otherwise a call to this function will result in a
// linker error.
*/
template <typename MT1 // Type of the left-hand side target matrix
		  ,
		  bool SO1 // Storage order of the left-hand side target matrix
		  ,
		  typename MT2 // Type of the left-hand side matrix operand
		  ,
		  bool SO2 // Storage order of the left-hand side matrix operand
		  ,
		  typename MT3 // Type of the right-hand side matrix operand
		  ,
		  bool SO3 // Storage order of the right-hand side matrix operand
		  ,
		  typename ST> // Type of the scalar factors
inline void gemm(DenseMatrix<MT1, SO1> &C, const DenseMatrix<MT2, SO2> &A, const DenseMatrix<MT3, SO3> &B, ST alpha,
				 ST beta)
{
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_COMPUTATION_TYPE(MT1);
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_COMPUTATION_TYPE(MT2);
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_COMPUTATION_TYPE(MT3);

	METRIC_NUMERIC_CONSTRAINT_MUST_HAVE_MUTABLE_DATA_ACCESS(MT1);
	METRIC_NUMERIC_CONSTRAINT_MUST_HAVE_CONST_DATA_ACCESS(MT2);
	METRIC_NUMERIC_CONSTRAINT_MUST_HAVE_CONST_DATA_ACCESS(MT3);

	METRIC_NUMERIC_CONSTRAINT_MUST_BE_BLAS_COMPATIBLE_TYPE(ElementType_t<MT1>);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_BLAS_COMPATIBLE_TYPE(ElementType_t<MT2>);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_BLAS_COMPATIBLE_TYPE(ElementType_t<MT3>);

	const blas_int_t m(numeric_cast<blas_int_t>((*A).rows()));
	const blas_int_t n(numeric_cast<blas_int_t>((*B).columns()));
	const blas_int_t k(numeric_cast<blas_int_t>((*A).columns()));
	const blas_int_t lda(numeric_cast<blas_int_t>((*A).spacing()));
	const blas_int_t ldb(numeric_cast<blas_int_t>((*B).spacing()));
	const blas_int_t ldc(numeric_cast<blas_int_t>((*C).spacing()));

	gemm((IsRowMajorMatrix_v<MT1>) ? (CblasRowMajor) : (CblasColMajor), (SO1 == SO2) ? (CblasNoTrans) : (CblasTrans),
		 (SO1 == SO3) ? (CblasNoTrans) : (CblasTrans), m, n, k, alpha, (*A).data(), lda, (*B).data(), ldb, beta,
		 (*C).data(), ldc);
}
#endif
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
