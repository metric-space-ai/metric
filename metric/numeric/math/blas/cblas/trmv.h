// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_BLAS_CBLAS_TRMV_H
#define METRIC_NUMERIC_MATH_BLAS_CBLAS_TRMV_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/blas/Types.h>
#include <metric/numeric/system/BLAS.h>
#include <metric/numeric/util/Complex.h>
#include <metric/numeric/util/StaticAssert.h>

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

void trmv(CBLAS_ORDER order, CBLAS_UPLO uplo, CBLAS_TRANSPOSE transA, CBLAS_DIAG diag, blas_int_t n, const float *A,
		  blas_int_t lda, float *x, blas_int_t incX);

void trmv(CBLAS_ORDER order, CBLAS_UPLO uplo, CBLAS_TRANSPOSE transA, CBLAS_DIAG diag, blas_int_t n, const double *A,
		  blas_int_t lda, double *x, blas_int_t incX);

void trmv(CBLAS_ORDER order, CBLAS_UPLO uplo, CBLAS_TRANSPOSE transA, CBLAS_DIAG diag, blas_int_t n,
		  const complex<float> *A, blas_int_t lda, complex<float> *x, blas_int_t incX);

void trmv(CBLAS_ORDER order, CBLAS_UPLO uplo, CBLAS_TRANSPOSE transA, CBLAS_DIAG diag, blas_int_t n,
		  const complex<double> *A, blas_int_t lda, complex<double> *x, blas_int_t incX);

#endif
//@}
//*************************************************************************************************

//*************************************************************************************************
#if METRIC_NUMERIC_BLAS_MODE
/*!\brief BLAS kernel for a triangular dense matrix/dense vector multiplication for single
//        precision operands (\f$ \vec{x}=A*\vec{x} \f$).
// \ingroup blas
//
// \param order Specifies the storage order of matrix \a A (\a CblasColMajor or \a CblasColMajor).
// \param uplo \a CblasLower to use the lower triangle from \a A, \a CblasUpper to use the upper triangle.
// \param transA Specifies whether to transpose matrix \a A (\a CblasNoTrans or \a CblasTrans).
// \param diag Specifies whether \a A is unitriangular (\a CblasNonUnit or \a CblasUnit).
// \param n The number of rows/columns of matrix \a A \f$[0..\infty)\f$.
// \param A Pointer to the first element of matrix \a A.
// \param lda The total number of elements between two rows/columns of matrix \a A \f$[0..\infty)\f$.
// \param x Pointer to the first element of vector \a x.
// \param incX The stride within vector \a x.
// \return void
//
// This function performs the multiplication of a single precision triangular matrix by a vector
// based on the cblas_strmv() function.
//
// \note This function can only be used if a fitting BLAS library, which supports this function,
// is available and linked to the executable. Otherwise a call to this function will result in a
// linker error.
*/
inline void trmv(CBLAS_ORDER order, CBLAS_UPLO uplo, CBLAS_TRANSPOSE transA, CBLAS_DIAG diag, blas_int_t n,
				 const float *A, blas_int_t lda, float *x, blas_int_t incX)
{
	cblas_strmv(order, uplo, transA, diag, n, A, lda, x, incX);
}
#endif
//*************************************************************************************************

//*************************************************************************************************
#if METRIC_NUMERIC_BLAS_MODE
/*!\brief BLAS kernel for a triangular dense matrix/dense vector multiplication for double
//        precision operands (\f$ \vec{x}=A*\vec{x} \f$).
// \ingroup blas
//
// \param order Specifies the storage order of matrix \a A (\a CblasColMajor or \a CblasColMajor).
// \param uplo \a CblasLower to use the lower triangle from \a A, \a CblasUpper to use the upper triangle.
// \param transA Specifies whether to transpose matrix \a A (\a CblasNoTrans or \a CblasTrans).
// \param diag Specifies whether \a A is unitriangular (\a CblasNonUnit or \a CblasUnit).
// \param n The number of rows/columns of matrix \a A \f$[0..\infty)\f$.
// \param A Pointer to the first element of matrix \a A.
// \param lda The total number of elements between two rows/columns of matrix \a A \f$[0..\infty)\f$.
// \param x Pointer to the first element of vector \a x.
// \param incX The stride within vector \a x.
// \return void
//
// This function performs the multiplication of a double precision triangular matrix by a vector
// based on the cblas_dtrmv() function.
//
// \note This function can only be used if a fitting BLAS library, which supports this function,
// is available and linked to the executable. Otherwise a call to this function will result in a
// linker error.
*/
inline void trmv(CBLAS_ORDER order, CBLAS_UPLO uplo, CBLAS_TRANSPOSE transA, CBLAS_DIAG diag, blas_int_t n,
				 const double *A, blas_int_t lda, double *x, blas_int_t incX)
{
	cblas_dtrmv(order, uplo, transA, diag, n, A, lda, x, incX);
}
#endif
//*************************************************************************************************

//*************************************************************************************************
#if METRIC_NUMERIC_BLAS_MODE
/*!\brief BLAS kernel for a triangular dense matrix/dense vector multiplication for single
//        precision complex operands (\f$ \vec{x}=A*\vec{x} \f$).
// \ingroup blas
//
// \param order Specifies the storage order of matrix \a A (\a CblasColMajor or \a CblasColMajor).
// \param uplo \a CblasLower to use the lower triangle from \a A, \a CblasUpper to use the upper triangle.
// \param transA Specifies whether to transpose matrix \a A (\a CblasNoTrans or \a CblasTrans).
// \param diag Specifies whether \a A is unitriangular (\a CblasNonUnit or \a CblasUnit).
// \param n The number of rows/columns of matrix \a A \f$[0..\infty)\f$.
// \param A Pointer to the first element of matrix \a A.
// \param lda The total number of elements between two rows/columns of matrix \a A \f$[0..\infty)\f$.
// \param x Pointer to the first element of vector \a x.
// \param incX The stride within vector \a x.
// \return void
//
// This function performs the multiplication of a single precision complex triangular matrix by a
// vector based on the cblas_ctrmv() function.
//
// \note This function can only be used if a fitting BLAS library, which supports this function,
// is available and linked to the executable. Otherwise a call to this function will result in a
// linker error.
*/
inline void trmv(CBLAS_ORDER order, CBLAS_UPLO uplo, CBLAS_TRANSPOSE transA, CBLAS_DIAG diag, blas_int_t n,
				 const complex<float> *A, blas_int_t lda, complex<float> *x, blas_int_t incX)
{
	METRIC_NUMERIC_STATIC_ASSERT(sizeof(complex<float>) == 2UL * sizeof(float));

	cblas_ctrmv(order, uplo, transA, diag, n, reinterpret_cast<const float *>(A), lda, reinterpret_cast<float *>(x),
				incX);
}
#endif
//*************************************************************************************************

//*************************************************************************************************
#if METRIC_NUMERIC_BLAS_MODE
/*!\brief BLAS kernel for a triangular dense matrix/dense vector multiplication for double
//        precision complex operands (\f$ \vec{x}=A*\vec{x} \f$).
// \ingroup blas
//
// \param order Specifies the storage order of matrix \a A (\a CblasColMajor or \a CblasColMajor).
// \param uplo \a CblasLower to use the lower triangle from \a A, \a CblasUpper to use the upper triangle.
// \param transA Specifies whether to transpose matrix \a A (\a CblasNoTrans or \a CblasTrans).
// \param diag Specifies whether \a A is unitriangular (\a CblasNonUnit or \a CblasUnit).
// \param n The number of rows/columns of matrix \a A \f$[0..\infty)\f$.
// \param A Pointer to the first element of matrix \a A.
// \param lda The total number of elements between two rows/columns of matrix \a A \f$[0..\infty)\f$.
// \param x Pointer to the first element of vector \a x.
// \param incX The stride within vector \a x.
// \return void
//
// This function performs the multiplication of a double precision complex triangular matrix by a
// vector based on the cblas_ztrmv() function.
//
// \note This function can only be used if a fitting BLAS library, which supports this function,
// is available and linked to the executable. Otherwise a call to this function will result in a
// linker error.
*/
inline void trmv(CBLAS_ORDER order, CBLAS_UPLO uplo, CBLAS_TRANSPOSE transA, CBLAS_DIAG diag, blas_int_t n,
				 const complex<double> *A, blas_int_t lda, complex<double> *x, blas_int_t incX)
{
	METRIC_NUMERIC_STATIC_ASSERT(sizeof(complex<double>) == 2UL * sizeof(double));

	cblas_ztrmv(order, uplo, transA, diag, n, reinterpret_cast<const double *>(A), lda, reinterpret_cast<double *>(x),
				incX);
}
#endif
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
