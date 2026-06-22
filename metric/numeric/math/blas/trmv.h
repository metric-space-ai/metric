// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_BLAS_TRMV_H
#define METRIC_NUMERIC_MATH_BLAS_TRMV_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/Aliases.h>
#include <metric/numeric/math/blas/cblas/trmv.h>
#include <metric/numeric/math/constraints/BLASCompatible.h>
#include <metric/numeric/math/constraints/Computation.h>
#include <metric/numeric/math/constraints/ConstDataAccess.h>
#include <metric/numeric/math/constraints/MutableDataAccess.h>
#include <metric/numeric/math/expressions/DenseMatrix.h>
#include <metric/numeric/math/expressions/DenseVector.h>
#include <metric/numeric/math/typetraits/IsRowMajorMatrix.h>
#include <metric/numeric/system/BLAS.h>
#include <metric/numeric/util/Assert.h>
#include <metric/numeric/util/NumericCast.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  BLAS TRIANGULAR MATRIX/VECTOR MULTIPLICATION FUNCTIONS (TRMV)
//
//=================================================================================================

//*************************************************************************************************
/*!\name BLAS triangular matrix/vector multiplication functions (trmv) */
//@{
#if METRIC_NUMERIC_BLAS_MODE

template <typename VT, typename MT, bool SO>
void trmv(DenseVector<VT, false> &x, const DenseMatrix<MT, SO> &A, CBLAS_UPLO uplo);

template <typename VT, typename MT, bool SO>
void trmv(DenseVector<VT, true> &x, const DenseMatrix<MT, SO> &A, CBLAS_UPLO uplo);

#endif
//@}
//*************************************************************************************************

//*************************************************************************************************
#if METRIC_NUMERIC_BLAS_MODE
/*!\brief BLAS kernel for a triangular dense matrix/dense vector multiplication
//        (\f$ \vec{x}=A*\vec{x} \f$).
// \ingroup blas
//
// \param x The target left-hand side dense vector.
// \param A The dense matrix operand.
// \param uplo \a CblasLower to use the lower triangle from \a A, \a CblasUpper to use the upper triangle.
// \return void
//
// This function performs the multiplication of a triangular matrix by a vector based on the BLAS
// trmv() functions. Note that the function only works for vectors and matrices with \c float,
// \c double, \c complex<float>, or \c complex<double> element type. The attempt to call the
// function with vectors and matrices of any other element type results in a compile time error.
//
// \note This function can only be used if a fitting BLAS library, which supports this function,
// is available and linked to the executable. Otherwise a call to this function will result in a
// linker error.
*/
template <typename VT // Type of the target vector
		  ,
		  typename MT // Type of the matrix operand
		  ,
		  bool SO> // Storage order of the matrix operand
inline void trmv(DenseVector<VT, false> &x, const DenseMatrix<MT, SO> &A, CBLAS_UPLO uplo)
{
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_COMPUTATION_TYPE(VT);
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_COMPUTATION_TYPE(MT);

	METRIC_NUMERIC_CONSTRAINT_MUST_HAVE_MUTABLE_DATA_ACCESS(VT);
	METRIC_NUMERIC_CONSTRAINT_MUST_HAVE_CONST_DATA_ACCESS(MT);

	METRIC_NUMERIC_CONSTRAINT_MUST_BE_BLAS_COMPATIBLE_TYPE(ElementType_t<VT>);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_BLAS_COMPATIBLE_TYPE(ElementType_t<MT>);

	METRIC_NUMERIC_INTERNAL_ASSERT((*A).rows() == (*A).columns(), "Non-square triangular matrix detected");
	METRIC_NUMERIC_INTERNAL_ASSERT(uplo == CblasLower || uplo == CblasUpper, "Invalid uplo argument detected");

	const blas_int_t n(numeric_cast<blas_int_t>((*A).rows()));
	const blas_int_t lda(numeric_cast<blas_int_t>((*A).spacing()));

	trmv((IsRowMajorMatrix_v<MT>) ? (CblasRowMajor) : (CblasColMajor), uplo, CblasNoTrans, CblasNonUnit, n, (*A).data(),
		 lda, (*x).data(), 1);
}
#endif
//*************************************************************************************************

//*************************************************************************************************
#if METRIC_NUMERIC_BLAS_MODE
/*!\brief BLAS kernel for a transpose dense vector/triangular dense matrix multiplication
//        (\f$ \vec{x}^T=\vec{x}^T*A \f$).
// \ingroup blas
//
// \param x The target left-hand side dense vector.
// \param A The dense matrix operand.
// \param uplo \a CblasLower to use the lower triangle from \a A, \a CblasUpper to use the upper triangle.
// \return void
//
// This function performs the multiplication of a vector and a triangular matrix based on the BLAS
// trmv() functions. Note that the function only works for vectors and matrices with \c float,
// \c double, \c complex<float>, or \c complex<double> element type. The attempt to call the
// function with vectors and matrices of any other element type results in a compile time error.
//
// \note This function can only be used if a fitting BLAS library, which supports this function,
// is available and linked to the executable. Otherwise a call to this function will result in a
// linker error.
*/
template <typename VT // Type of the target vector
		  ,
		  typename MT // Type of the matrix operand
		  ,
		  bool SO> // Storage order of the matrix operand
inline void trmv(DenseVector<VT, true> &x, const DenseMatrix<MT, SO> &A, CBLAS_UPLO uplo)
{
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_COMPUTATION_TYPE(VT);
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_COMPUTATION_TYPE(MT);

	METRIC_NUMERIC_CONSTRAINT_MUST_HAVE_MUTABLE_DATA_ACCESS(VT);
	METRIC_NUMERIC_CONSTRAINT_MUST_HAVE_CONST_DATA_ACCESS(MT);

	METRIC_NUMERIC_CONSTRAINT_MUST_BE_BLAS_COMPATIBLE_TYPE(ElementType_t<VT>);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_BLAS_COMPATIBLE_TYPE(ElementType_t<MT>);

	METRIC_NUMERIC_INTERNAL_ASSERT((*A).rows() == (*A).columns(), "Non-square triangular matrix detected");
	METRIC_NUMERIC_INTERNAL_ASSERT(uplo == CblasLower || uplo == CblasUpper, "Invalid uplo argument detected");

	const blas_int_t n(numeric_cast<blas_int_t>((*A).rows()));
	const blas_int_t lda(numeric_cast<blas_int_t>((*A).spacing()));

	trmv((IsRowMajorMatrix_v<MT>) ? (CblasRowMajor) : (CblasColMajor), uplo, CblasTrans, CblasNonUnit, n, (*A).data(),
		 lda, (*x).data(), 1);
}
#endif
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
