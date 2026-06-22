// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_BLAS_CBLAS_GEMV_H
#define METRIC_NUMERIC_MATH_BLAS_CBLAS_GEMV_H
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
//  BLAS GENERAL MATRIX/VECTOR MULTIPLICATION FUNCTIONS (GEMV)
//
//=================================================================================================

//*************************************************************************************************
/*!\name BLAS general matrix/vector multiplication functions (gemv) */
//@{
#if METRIC_NUMERIC_BLAS_MODE

void gemv(CBLAS_ORDER layout, CBLAS_TRANSPOSE transA, blas_int_t m, blas_int_t n, float alpha, const float *A,
		  blas_int_t lda, const float *x, blas_int_t incX, float beta, float *y, blas_int_t incY);

void gemv(CBLAS_ORDER layout, CBLAS_TRANSPOSE transA, blas_int_t m, blas_int_t n, double alpha, const double *A,
		  blas_int_t lda, const double *x, blas_int_t incX, double beta, double *y, blas_int_t incY);

void gemv(CBLAS_ORDER layout, CBLAS_TRANSPOSE transA, blas_int_t m, blas_int_t n, complex<float> alpha,
		  const complex<float> *A, blas_int_t lda, const complex<float> *x, blas_int_t incX, complex<float> beta,
		  complex<float> *y, blas_int_t incY);

void gemv(CBLAS_ORDER layout, CBLAS_TRANSPOSE transA, blas_int_t m, blas_int_t n, complex<double> alpha,
		  const complex<double> *A, blas_int_t lda, const complex<double> *x, blas_int_t incX, complex<double> beta,
		  complex<double> *y, blas_int_t incY);

#endif
//@}
//*************************************************************************************************

//*************************************************************************************************
#if METRIC_NUMERIC_BLAS_MODE
/*!\brief BLAS kernel for a dense matrix/dense vector multiplication for single precision operands
//        (\f$ \vec{y}=\alpha*A*\vec{x}+\beta*\vec{y} \f$).
// \ingroup blas
//
// \param order Specifies the storage order of matrix \a A (\a CblasColMajor or \a CblasColMajor).
// \param transA Specifies whether to transpose matrix \a A (\a CblasNoTrans or \a CblasTrans).
// \param m The number of rows of matrix \a A \f$[0..\infty)\f$.
// \param n The number of columns of matrix \a A \f$[0..\infty)\f$.
// \param alpha The scaling factor for \f$ A*\vec{x} \f$.
// \param A Pointer to the first element of matrix \a A.
// \param lda The total number of elements between two rows/columns of matrix \a A \f$[0..\infty)\f$.
// \param x Pointer to the first element of vector \a x.
// \param incX The stride within vector \a x.
// \param beta The scaling factor for \f$ \vec{y} \f$.
// \param y Pointer to the first element of vector \a y.
// \param incY The stride within vector \a y.
// \return void
//
// This function performs the dense matrix/dense vector multiplication for single precision
// operands based on the cblas_sgemv() function.
//
// \note This function can only be used if a fitting BLAS library, which supports this function,
// is available and linked to the executable. Otherwise a call to this function will result in a
// linker error.
*/
inline void gemv(CBLAS_ORDER order, CBLAS_TRANSPOSE transA, blas_int_t m, blas_int_t n, float alpha, const float *A,
				 blas_int_t lda, const float *x, blas_int_t incX, float beta, float *y, blas_int_t incY)
{
	cblas_sgemv(order, transA, m, n, alpha, A, lda, x, incX, beta, y, incY);
}
#endif
//*************************************************************************************************

//*************************************************************************************************
#if METRIC_NUMERIC_BLAS_MODE
/*!\brief BLAS kernel for a dense matrix/dense vector multiplication for double precision operands
//        (\f$ \vec{y}=\alpha*A*\vec{x}+\beta*\vec{y} \f$).
// \ingroup blas
//
// \param order Specifies the storage order of matrix \a A (\a CblasColMajor or \a CblasColMajor).
// \param transA Specifies whether to transpose matrix \a A (\a CblasNoTrans or \a CblasTrans).
// \param m The number of rows of matrix \a A \f$[0..\infty)\f$.
// \param n The number of columns of matrix \a A \f$[0..\infty)\f$.
// \param alpha The scaling factor for \f$ A*\vec{x} \f$.
// \param A Pointer to the first element of matrix \a A.
// \param lda The total number of elements between two rows/columns of matrix \a A \f$[0..\infty)\f$.
// \param x Pointer to the first element of vector \a x.
// \param incX The stride within vector \a x.
// \param beta The scaling factor for \f$ \vec{y} \f$.
// \param y Pointer to the first element of vector \a y.
// \param incY The stride within vector \a y.
// \return void
//
// This function performs the dense matrix/dense vector multiplication for double precision
// operands based on the cblas_dgemv() function.
//
// \note This function can only be used if a fitting BLAS library, which supports this function,
// is available and linked to the executable. Otherwise a call to this function will result in a
// linker error.
*/
inline void gemv(CBLAS_ORDER order, CBLAS_TRANSPOSE transA, blas_int_t m, blas_int_t n, double alpha, const double *A,
				 blas_int_t lda, const double *x, blas_int_t incX, double beta, double *y, blas_int_t incY)
{
	cblas_dgemv(order, transA, m, n, alpha, A, lda, x, incX, beta, y, incY);
}
#endif
//*************************************************************************************************

//*************************************************************************************************
#if METRIC_NUMERIC_BLAS_MODE
/*!\brief BLAS kernel for a dense matrix/dense vector multiplication for single precision complex
//        operands (\f$ \vec{y}=\alpha*A*\vec{x}+\beta*\vec{y} \f$).
// \ingroup blas
//
// \param order Specifies the storage order of matrix \a A (\a CblasColMajor or \a CblasColMajor).
// \param transA Specifies whether to transpose matrix \a A (\a CblasNoTrans or \a CblasTrans).
// \param m The number of rows of matrix \a A \f$[0..\infty)\f$.
// \param n The number of columns of matrix \a A \f$[0..\infty)\f$.
// \param alpha The scaling factor for \f$ A*\vec{x} \f$.
// \param A Pointer to the first element of matrix \a A.
// \param lda The total number of elements between two rows/columns of matrix \a A \f$[0..\infty)\f$.
// \param x Pointer to the first element of vector \a x.
// \param incX The stride within vector \a x.
// \param beta The scaling factor for \f$ \vec{y} \f$.
// \param y Pointer to the first element of vector \a y.
// \param incY The stride within vector \a y.
// \return void
//
// This function performs the dense matrix/dense vector multiplication for single precision
// complex operands based on the cblas_cgemv() function.
//
// \note This function can only be used if a fitting BLAS library, which supports this function,
// is available and linked to the executable. Otherwise a call to this function will result in a
// linker error.
*/
inline void gemv(CBLAS_ORDER order, CBLAS_TRANSPOSE transA, blas_int_t m, blas_int_t n, complex<float> alpha,
				 const complex<float> *A, blas_int_t lda, const complex<float> *x, blas_int_t incX, complex<float> beta,
				 complex<float> *y, blas_int_t incY)
{
	METRIC_NUMERIC_STATIC_ASSERT(sizeof(complex<float>) == 2UL * sizeof(float));

	cblas_cgemv(order, transA, m, n, reinterpret_cast<const float *>(&alpha), reinterpret_cast<const float *>(A), lda,
				reinterpret_cast<const float *>(x), incX, reinterpret_cast<const float *>(&beta),
				reinterpret_cast<float *>(y), incY);
}
#endif
//*************************************************************************************************

//*************************************************************************************************
#if METRIC_NUMERIC_BLAS_MODE
/*!\brief BLAS kernel for a dense matrix/dense vector multiplication for double precision complex
//        operands (\f$ \vec{y}=\alpha*A*\vec{x}+\beta*\vec{y} \f$).
// \ingroup blas
//
// \param order Specifies the storage order of matrix \a A (\a CblasColMajor or \a CblasColMajor).
// \param transA Specifies whether to transpose matrix \a A (\a CblasNoTrans or \a CblasTrans).
// \param m The number of rows of matrix \a A \f$[0..\infty)\f$.
// \param n The number of columns of matrix \a A \f$[0..\infty)\f$.
// \param alpha The scaling factor for \f$ A*\vec{x} \f$.
// \param A Pointer to the first element of matrix \a A.
// \param lda The total number of elements between two rows/columns of matrix \a A \f$[0..\infty)\f$.
// \param x Pointer to the first element of vector \a x.
// \param incX The stride within vector \a x.
// \param beta The scaling factor for \f$ \vec{y} \f$.
// \param y Pointer to the first element of vector \a y.
// \param incY The stride within vector \a y.
// \return void
//
// This function performs the dense matrix/dense vector multiplication for double precision
// complex operands based on the cblas_zgemv() function.
//
// \note This function can only be used if a fitting BLAS library, which supports this function,
// is available and linked to the executable. Otherwise a call to this function will result in a
// linker error.
*/
inline void gemv(CBLAS_ORDER order, CBLAS_TRANSPOSE transA, blas_int_t m, blas_int_t n, complex<double> alpha,
				 const complex<double> *A, blas_int_t lda, const complex<double> *x, blas_int_t incX,
				 complex<double> beta, complex<double> *y, blas_int_t incY)
{
	METRIC_NUMERIC_STATIC_ASSERT(sizeof(complex<double>) == 2UL * sizeof(double));

	cblas_zgemv(order, transA, m, n, reinterpret_cast<const double *>(&alpha), reinterpret_cast<const double *>(A), lda,
				reinterpret_cast<const double *>(x), incX, reinterpret_cast<const double *>(&beta),
				reinterpret_cast<double *>(y), incY);
}
#endif
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
