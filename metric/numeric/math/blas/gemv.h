// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_BLAS_GEMV_H
#define METRIC_NUMERIC_MATH_BLAS_GEMV_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/Aliases.h>
#include <metric/numeric/math/blas/cblas/gemv.h>
#include <metric/numeric/math/constraints/BLASCompatible.h>
#include <metric/numeric/math/constraints/Computation.h>
#include <metric/numeric/math/constraints/ConstDataAccess.h>
#include <metric/numeric/math/constraints/MutableDataAccess.h>
#include <metric/numeric/math/expressions/DenseMatrix.h>
#include <metric/numeric/math/expressions/DenseVector.h>
#include <metric/numeric/math/typetraits/IsRowMajorMatrix.h>
#include <metric/numeric/system/MacroDisable.h>
#include <metric/numeric/util/NumericCast.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  BLAS GENERAL MATRIX/VECTOR MULTIPLICATION FUNCTIONS (GEMV)
//
//=================================================================================================

//*************************************************************************************************
/*!\name BLAS general matrix/vector multiplication functions (gemv) */
//@{
#if METRIC_NUMERIC_BLAS_MODE

template <typename VT1, typename MT1, bool SO, typename VT2, typename ST>
void gemv(DenseVector<VT1, false> &y, const DenseMatrix<MT1, SO> &A, const DenseVector<VT2, false> &x, ST alpha,
		  ST beta);

template <typename VT1, typename VT2, typename MT1, bool SO, typename ST>
void gemv(DenseVector<VT1, true> &y, const DenseVector<VT2, true> &x, const DenseMatrix<MT1, SO> &A, ST alpha, ST beta);

#endif
//@}
//*************************************************************************************************

//*************************************************************************************************
#if METRIC_NUMERIC_BLAS_MODE
/*!\brief BLAS kernel for a dense matrix/dense vector multiplication
//        (\f$ \vec{y}=\alpha*A*\vec{x}+\beta*\vec{y} \f$).
// \ingroup blas
//
// \param y The target left-hand side dense vector.
// \param A The left-hand side dense matrix operand.
// \param x The right-hand side dense vector operand.
// \param alpha The scaling factor for \f$ A*\vec{x} \f$.
// \param beta The scaling factor for \f$ \vec{y} \f$.
// \return void
//
// This function performs the dense matrix/dense vector multiplication based on the BLAS gemv()
// functions. Note that the function only works for vectors and matrices with \c float, \c double,
// \c complex<float>, or \c complex<double> element type. The attempt to call the function with
// vectors and matrices of any other element type results in a compile time error.
//
// \note This function can only be used if a fitting BLAS library, which supports this function,
// is available and linked to the executable. Otherwise a call to this function will result in a
// linker error.
*/
template <typename VT1 // Type of the left-hand side target vector
		  ,
		  typename MT1 // Type of the left-hand side matrix operand
		  ,
		  bool SO // Storage order of the left-hand side matrix operand
		  ,
		  typename VT2 // Type of the right-hand side vector operand
		  ,
		  typename ST> // Type of the scalar factors
inline void gemv(DenseVector<VT1, false> &y, const DenseMatrix<MT1, SO> &A, const DenseVector<VT2, false> &x, ST alpha,
				 ST beta)
{
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_COMPUTATION_TYPE(VT1);
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_COMPUTATION_TYPE(MT1);
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_COMPUTATION_TYPE(VT2);

	METRIC_NUMERIC_CONSTRAINT_MUST_HAVE_MUTABLE_DATA_ACCESS(VT1);
	METRIC_NUMERIC_CONSTRAINT_MUST_HAVE_CONST_DATA_ACCESS(MT1);
	METRIC_NUMERIC_CONSTRAINT_MUST_HAVE_CONST_DATA_ACCESS(VT2);

	METRIC_NUMERIC_CONSTRAINT_MUST_BE_BLAS_COMPATIBLE_TYPE(ElementType_t<VT1>);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_BLAS_COMPATIBLE_TYPE(ElementType_t<MT1>);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_BLAS_COMPATIBLE_TYPE(ElementType_t<VT2>);

	const blas_int_t m(numeric_cast<blas_int_t>((*A).rows()));
	const blas_int_t n(numeric_cast<blas_int_t>((*A).columns()));
	const blas_int_t lda(numeric_cast<blas_int_t>((*A).spacing()));

	gemv((SO) ? (CblasColMajor) : (CblasRowMajor), CblasNoTrans, m, n, alpha, (*A).data(), lda, (*x).data(), 1, beta,
		 (*y).data(), 1);
}
#endif
//*************************************************************************************************

//*************************************************************************************************
#if METRIC_NUMERIC_BLAS_MODE
/*!\brief BLAS kernel for a transpose dense vector/dense matrix multiplication
//        (\f$ \vec{y}^T=\alpha*\vec{x}^T*A+\beta*\vec{y}^T \f$).
// \ingroup blas
//
// \param y The target left-hand side dense vector.
// \param x The left-hand side dense vector operand.
// \param A The right-hand side dense matrix operand.
// \param alpha The scaling factor for \f$ \vec{x}^T*A \f$.
// \param beta The scaling factor for \f$ \vec{y}^T \f$.
// \return void
//
// This function performs the transpose dense vector/dense matrix multiplication based on the
// BLAS gemv() functions. Note that the function only works for vectors and matrices with \c float,
// \c double, \c complex<float>, or \c complex<double> element type. The attempt to call the
// function with vectors and matrices of any other element type results in a compile time error.
//
// \note This function can only be used if a fitting BLAS library, which supports this function,
// is available and linked to the executable. Otherwise a call to this function will result in a
// linker error.
*/
template <typename VT1 // Type of the left-hand side target vector
		  ,
		  typename VT2 // Type of the left-hand side vector operand
		  ,
		  typename MT1 // Type of the right-hand side matrix operand
		  ,
		  bool SO // Storage order of the right-hand side matrix operand
		  ,
		  typename ST> // Type of the scalar factors
inline void gemv(DenseVector<VT1, true> &y, const DenseVector<VT2, true> &x, const DenseMatrix<MT1, SO> &A, ST alpha,
				 ST beta)
{
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_COMPUTATION_TYPE(VT1);
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_COMPUTATION_TYPE(MT1);
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_COMPUTATION_TYPE(VT2);

	METRIC_NUMERIC_CONSTRAINT_MUST_HAVE_MUTABLE_DATA_ACCESS(VT1);
	METRIC_NUMERIC_CONSTRAINT_MUST_HAVE_CONST_DATA_ACCESS(VT2);
	METRIC_NUMERIC_CONSTRAINT_MUST_HAVE_CONST_DATA_ACCESS(MT1);

	METRIC_NUMERIC_CONSTRAINT_MUST_BE_BLAS_COMPATIBLE_TYPE(ElementType_t<VT1>);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_BLAS_COMPATIBLE_TYPE(ElementType_t<MT1>);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_BLAS_COMPATIBLE_TYPE(ElementType_t<VT2>);

	const blas_int_t m(numeric_cast<blas_int_t>((*A).rows()));
	const blas_int_t n(numeric_cast<blas_int_t>((*A).columns()));
	const blas_int_t lda(numeric_cast<blas_int_t>((*A).spacing()));

	gemv((SO) ? (CblasColMajor) : (CblasRowMajor), CblasTrans, m, n, alpha, (*A).data(), lda, (*x).data(), 1, beta,
		 (*y).data(), 1);
}
#endif
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
