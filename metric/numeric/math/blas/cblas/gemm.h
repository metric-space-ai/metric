// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_BLAS_CBLAS_GEMM_H
#define METRIC_NUMERIC_MATH_BLAS_CBLAS_GEMM_H
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
//  BLAS GENERAL MATRIX MULTIPLICATION FUNCTIONS (GEMM)
//
//=================================================================================================

//*************************************************************************************************
/*!\name BLAS general matrix multiplication functions (gemm) */
//@{
#if METRIC_NUMERIC_BLAS_MODE

void gemm(CBLAS_ORDER order, CBLAS_TRANSPOSE transA, CBLAS_TRANSPOSE transB, blas_int_t m, blas_int_t n, blas_int_t k,
		  float alpha, const float *A, blas_int_t lda, const float *B, blas_int_t ldb, float beta, float *C,
		  blas_int_t ldc);

void gemm(CBLAS_ORDER order, CBLAS_TRANSPOSE transA, CBLAS_TRANSPOSE transB, blas_int_t m, blas_int_t n, blas_int_t k,
		  double alpha, const double *A, blas_int_t lda, const double *B, blas_int_t ldb, double beta, double *C,
		  blas_int_t ldc);

void gemm(CBLAS_ORDER order, CBLAS_TRANSPOSE transA, CBLAS_TRANSPOSE transB, blas_int_t m, blas_int_t n, blas_int_t k,
		  complex<float> alpha, const complex<float> *A, blas_int_t lda, const complex<float> *B, blas_int_t ldb,
		  complex<float> beta, complex<float> *C, blas_int_t ldc);

void gemm(CBLAS_ORDER order, CBLAS_TRANSPOSE transA, CBLAS_TRANSPOSE transB, blas_int_t m, blas_int_t n, blas_int_t k,
		  complex<double> alpha, const complex<double> *A, blas_int_t lda, const complex<double> *B, blas_int_t ldb,
		  complex<double> beta, complex<double> *C, blas_int_t ldc);

#endif
//@}
//*************************************************************************************************

//*************************************************************************************************
#if METRIC_NUMERIC_BLAS_MODE
/*!\brief BLAS kernel for a dense matrix/dense matrix multiplication with single precision
//        matrices (\f$ C=\alpha*A*B+\beta*C \f$).
// \ingroup blas
//
// \param order Specifies the storage order of matrix \a A (\a CblasColMajor or \a CblasColMajor).
// \param transA Specifies whether to transpose matrix \a A (\a CblasNoTrans or \a CblasTrans).
// \param transB Specifies whether to transpose matrix \a B (\a CblasNoTrans or \a CblasTrans).
// \param m The number of rows of matrix \a A and \a C \f$[0..\infty)\f$.
// \param n The number of columns of matrix \a B and \a C \f$[0..\infty)\f$.
// \param k The number of columns of matrix \a A and rows in matrix \a B \f$[0..\infty)\f$.
// \param alpha The scaling factor for \f$ A*B \f$.
// \param A Pointer to the first element of matrix \a A.
// \param lda The total number of elements between two rows/columns of matrix \a A \f$[0..\infty)\f$.
// \param B Pointer to the first element of matrix \a B.
// \param ldb The total number of elements between two rows/columns of matrix \a B \f$[0..\infty)\f$.
// \param beta The scaling factor for \f$ C \f$.
// \param C Pointer to the first element of matrix \a C.
// \param ldc The total number of elements between two rows/columns of matrix \a C \f$[0..\infty)\f$.
// \return void
//
// This function performs the dense matrix/dense matrix multiplication for general single
// precision matrices based on the cblas_sgemm() function (\f$ C=\alpha*A*B+\beta*C \f$).
//
// \note This function can only be used if a fitting BLAS library, which supports this function,
// is available and linked to the executable. Otherwise a call to this function will result in a
// linker error.
*/
inline void gemm(CBLAS_ORDER order, CBLAS_TRANSPOSE transA, CBLAS_TRANSPOSE transB, blas_int_t m, blas_int_t n,
				 blas_int_t k, float alpha, const float *A, blas_int_t lda, const float *B, blas_int_t ldb, float beta,
				 float *C, blas_int_t ldc)
{
	cblas_sgemm(order, transA, transB, m, n, k, alpha, A, lda, B, ldb, beta, C, ldc);
}
#endif
//*************************************************************************************************

//*************************************************************************************************
#if METRIC_NUMERIC_BLAS_MODE
/*!\brief BLAS kernel for a dense matrix/dense matrix multiplication with double precision
//        matrices (\f$ C=\alpha*A*B+\beta*C \f$).
// \ingroup blas
//
// \param order Specifies the storage order of matrix \a A (\a CblasColMajor or \a CblasColMajor).
// \param transA Specifies whether to transpose matrix \a A (\a CblasNoTrans or \a CblasTrans).
// \param transB Specifies whether to transpose matrix \a B (\a CblasNoTrans or \a CblasTrans).
// \param m The number of rows of matrix \a A and \a C \f$[0..\infty)\f$.
// \param n The number of columns of matrix \a B and \a C \f$[0..\infty)\f$.
// \param k The number of columns of matrix \a A and rows in matrix \a B \f$[0..\infty)\f$.
// \param alpha The scaling factor for \f$ A*B \f$.
// \param A Pointer to the first element of matrix \a A.
// \param lda The total number of elements between two rows/columns of matrix \a A \f$[0..\infty)\f$.
// \param B Pointer to the first element of matrix \a B.
// \param ldb The total number of elements between two rows/columns of matrix \a B \f$[0..\infty)\f$.
// \param beta The scaling factor for \f$ C \f$.
// \param C Pointer to the first element of matrix \a C.
// \param ldc The total number of elements between two rows/columns of matrix \a C \f$[0..\infty)\f$.
// \return void
//
// This function performs the dense matrix/dense matrix multiplication for double precision
// matrices based on the cblas_dgemm() function.
//
// \note This function can only be used if a fitting BLAS library, which supports this function,
// is available and linked to the executable. Otherwise a call to this function will result in a
// linker error.
*/
inline void gemm(CBLAS_ORDER order, CBLAS_TRANSPOSE transA, CBLAS_TRANSPOSE transB, blas_int_t m, blas_int_t n,
				 blas_int_t k, double alpha, const double *A, blas_int_t lda, const double *B, blas_int_t ldb,
				 double beta, double *C, blas_int_t ldc)
{
	cblas_dgemm(order, transA, transB, m, n, k, alpha, A, lda, B, ldb, beta, C, ldc);
}
#endif
//*************************************************************************************************

//*************************************************************************************************
#if METRIC_NUMERIC_BLAS_MODE
/*!\brief BLAS kernel for a dense matrix/dense matrix multiplication with single precision
//        matrices (\f$ C=\alpha*A*B+\beta*C \f$).
// \ingroup blas
//
// \param order Specifies the storage order of matrix \a A (\a CblasColMajor or \a CblasColMajor).
// \param transA Specifies whether to transpose matrix \a A (\a CblasNoTrans or \a CblasTrans).
// \param transB Specifies whether to transpose matrix \a B (\a CblasNoTrans or \a CblasTrans).
// \param m The number of rows of matrix \a A and \a C \f$[0..\infty)\f$.
// \param n The number of columns of matrix \a B and \a C \f$[0..\infty)\f$.
// \param k The number of columns of matrix \a A and rows in matrix \a B \f$[0..\infty)\f$.
// \param alpha The scaling factor for \f$ A*B \f$.
// \param A Pointer to the first element of matrix \a A.
// \param lda The total number of elements between two rows/columns of matrix \a A \f$[0..\infty)\f$.
// \param B Pointer to the first element of matrix \a B.
// \param ldb The total number of elements between two rows/columns of matrix \a B \f$[0..\infty)\f$.
// \param beta The scaling factor for \f$ C \f$.
// \param C Pointer to the first element of matrix \a C.
// \param ldc The total number of elements between two rows/columns of matrix \a C \f$[0..\infty)\f$.
// \return void
//
// This function performs the dense matrix/dense matrix multiplication for single precision
// complex matrices based on the cblas_cgemm() function.
//
// \note This function can only be used if a fitting BLAS library, which supports this function,
// is available and linked to the executable. Otherwise a call to this function will result in a
// linker error.
*/
inline void gemm(CBLAS_ORDER order, CBLAS_TRANSPOSE transA, CBLAS_TRANSPOSE transB, blas_int_t m, blas_int_t n,
				 blas_int_t k, complex<float> alpha, const complex<float> *A, blas_int_t lda, const complex<float> *B,
				 blas_int_t ldb, complex<float> beta, complex<float> *C, blas_int_t ldc)
{
	METRIC_NUMERIC_STATIC_ASSERT(sizeof(complex<float>) == 2UL * sizeof(float));

	cblas_cgemm(order, transA, transB, m, n, k, reinterpret_cast<const float *>(&alpha),
				reinterpret_cast<const float *>(A), lda, reinterpret_cast<const float *>(B), ldb,
				reinterpret_cast<const float *>(&beta), reinterpret_cast<float *>(C), ldc);
}
#endif
//*************************************************************************************************

//*************************************************************************************************
#if METRIC_NUMERIC_BLAS_MODE
/*!\brief BLAS kernel for a dense matrix/dense matrix multiplication with double precision
//        matrices (\f$ C=\alpha*A*B+\beta*C \f$).
// \ingroup blas
//
// \param order Specifies the storage order of matrix \a A (\a CblasColMajor or \a CblasColMajor).
// \param transA Specifies whether to transpose matrix \a A (\a CblasNoTrans or \a CblasTrans).
// \param transB Specifies whether to transpose matrix \a B (\a CblasNoTrans or \a CblasTrans).
// \param m The number of rows of matrix \a A and \a C \f$[0..\infty)\f$.
// \param n The number of columns of matrix \a B and \a C \f$[0..\infty)\f$.
// \param k The number of columns of matrix \a A and rows in matrix \a B \f$[0..\infty)\f$.
// \param alpha The scaling factor for \f$ A*B \f$.
// \param A Pointer to the first element of matrix \a A.
// \param lda The total number of elements between two rows/columns of matrix \a A \f$[0..\infty)\f$.
// \param B Pointer to the first element of matrix \a B.
// \param ldb The total number of elements between two rows/columns of matrix \a B \f$[0..\infty)\f$.
// \param beta The scaling factor for \f$ C \f$.
// \param C Pointer to the first element of matrix \a C.
// \param ldc The total number of elements between two rows/columns of matrix \a C \f$[0..\infty)\f$.
// \return void
//
// This function performs the dense matrix/dense matrix multiplication for double precision
// complex matrices based on the cblas_zgemm() function.
//
// \note This function can only be used if a fitting BLAS library, which supports this function,
// is available and linked to the executable. Otherwise a call to this function will result in a
// linker error.
*/
inline void gemm(CBLAS_ORDER order, CBLAS_TRANSPOSE transA, CBLAS_TRANSPOSE transB, blas_int_t m, blas_int_t n,
				 blas_int_t k, complex<double> alpha, const complex<double> *A, blas_int_t lda,
				 const complex<double> *B, blas_int_t ldb, complex<double> beta, complex<double> *C, blas_int_t ldc)
{
	METRIC_NUMERIC_STATIC_ASSERT(sizeof(complex<double>) == 2UL * sizeof(double));

	cblas_zgemm(order, transA, transB, m, n, k, reinterpret_cast<const double *>(&alpha),
				reinterpret_cast<const double *>(A), lda, reinterpret_cast<const double *>(B), ldb,
				reinterpret_cast<const double *>(&beta), reinterpret_cast<double *>(C), ldc);
}
#endif
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
