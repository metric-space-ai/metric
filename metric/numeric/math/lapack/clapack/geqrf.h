// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_LAPACK_CLAPACK_GEQRF_H
#define METRIC_NUMERIC_MATH_LAPACK_CLAPACK_GEQRF_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/blas/Types.h>
#include <metric/numeric/util/Complex.h>
#include <metric/numeric/util/StaticAssert.h>

//=================================================================================================
//
//  LAPACK FORWARD DECLARATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
#if !defined(INTEL_MKL_VERSION)
extern "C" {

void sgeqrf_(mtrc::numeric::blas_int_t *m, mtrc::numeric::blas_int_t *n, float *A, mtrc::numeric::blas_int_t *lda,
			 float *tau, float *work, mtrc::numeric::blas_int_t *lwork, mtrc::numeric::blas_int_t *info);
void dgeqrf_(mtrc::numeric::blas_int_t *m, mtrc::numeric::blas_int_t *n, double *A,
			 mtrc::numeric::blas_int_t *lda, double *tau, double *work, mtrc::numeric::blas_int_t *lwork,
			 mtrc::numeric::blas_int_t *info);
void cgeqrf_(mtrc::numeric::blas_int_t *m, mtrc::numeric::blas_int_t *n, float *A, mtrc::numeric::blas_int_t *lda,
			 float *tau, float *work, mtrc::numeric::blas_int_t *lwork, mtrc::numeric::blas_int_t *info);
void zgeqrf_(mtrc::numeric::blas_int_t *m, mtrc::numeric::blas_int_t *n, double *A,
			 mtrc::numeric::blas_int_t *lda, double *tau, double *work, mtrc::numeric::blas_int_t *lwork,
			 mtrc::numeric::blas_int_t *info);
}
#endif
/*! \endcond */
//*************************************************************************************************

namespace mtrc::numeric {

//=================================================================================================
//
//  LAPACK QR DECOMPOSITION FUNCTIONS (GEQRF)
//
//=================================================================================================

//*************************************************************************************************
/*!\name LAPACK QR decomposition functions (geqrf) */
//@{
void geqrf(blas_int_t m, blas_int_t n, float *A, blas_int_t lda, float *tau, float *work, blas_int_t lwork,
		   blas_int_t *info);

void geqrf(blas_int_t m, blas_int_t n, double *A, blas_int_t lda, double *tau, double *work, blas_int_t lwork,
		   blas_int_t *info);

void geqrf(blas_int_t m, blas_int_t n, complex<float> *A, blas_int_t lda, complex<float> *tau, complex<float> *work,
		   blas_int_t lwork, blas_int_t *info);

void geqrf(blas_int_t m, blas_int_t n, complex<double> *A, blas_int_t lda, complex<double> *tau, complex<double> *work,
		   blas_int_t lwork, blas_int_t *info);
//@}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief LAPACK kernel for the QR decomposition of the given dense single precision column-major
//        matrix.
// \ingroup lapack_decomposition
//
// \param m The number of rows of the given matrix \f$[0..\infty)\f$.
// \param n The number of columns of the given matrix \f$[0..\infty)\f$.
// \param A Pointer to the first element of the single precision column-major matrix.
// \param lda The total number of elements between two columns of the matrix \f$[0..\infty)\f$.
// \param tau Array for the scalar factors of the elementary reflectors; size >= min( \a m, \a n ).
// \param work Auxiliary array; size >= max( 1, \a lwork ).
// \param lwork The dimension of the array \a work; size >= max( 1, \a n ).
// \param info Return code of the function call.
// \return void
//
// This function performs the dense matrix QR decomposition of a general \a m-by-\a n single
// precision column-major matrix based on the LAPACK sgeqrf() function. The resulting decomposition
// has the form

							  \f[ A = Q \cdot R, \f]

// where the \c Q is represented as a product of elementary reflectors

			   \f[ Q = H(1) H(2) . . . H(k) \texttt{, with k = min(m,n).} \f]

// Each H(i) has the form

					  \f[ H(i) = I - tau \cdot v \cdot v^T, \f]

// where \c tau is a real scalar, and \c v is a real vector with <tt>v(0:i-1) = 0</tt> and
// <tt>v(i) = 1</tt>. <tt>v(i+1:m)</tt> is stored on exit in <tt>A(i+1:m,i)</tt>, and \c tau
// in \c tau(i). Thus on exit the elements on and above the diagonal of the matrix contain the
// min(\a m,\a n)-by-\a n upper trapezoidal matrix \c R (\c R is upper triangular if \a m >= \a n);
// the elements below the diagonal, with the array \c tau, represent the orthogonal matrix \c Q
// as a product of min(\a m,\a n) elementary reflectors.
//
// The \a info argument provides feedback on the success of the function call:
//
//   - = 0: The decomposition finished successfully.
//   - < 0: The i-th argument had an illegal value.
//
// For more information on the sgeqrf() function, see the LAPACK online documentation browser:
//
//        http://www.netlib.org/lapack/explore-html/
//
// \note This function can only be used if a fitting LAPACK library, which supports this function,
// is available and linked to the executable. Otherwise a call to this function will result in a
// linker error.
*/
inline void geqrf(blas_int_t m, blas_int_t n, float *A, blas_int_t lda, float *tau, float *work, blas_int_t lwork,
				  blas_int_t *info)
{
#if defined(INTEL_MKL_VERSION)
	METRIC_NUMERIC_STATIC_ASSERT(sizeof(MKL_INT) == sizeof(blas_int_t));
#endif

	sgeqrf_(&m, &n, A, &lda, tau, work, &lwork, info);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief LAPACK kernel for the QR decomposition of the given dense double precision column-major
//        matrix.
// \ingroup lapack_decomposition
//
// \param m The number of rows of the given matrix \f$[0..\infty)\f$.
// \param n The number of columns of the given matrix \f$[0..\infty)\f$.
// \param A Pointer to the first element of the double precision column-major matrix.
// \param lda The total number of elements between two columns of the matrix \f$[0..\infty)\f$.
// \param tau Array for the scalar factors of the elementary reflectors; size >= min( \a m, \a n ).
// \param work Auxiliary array; size >= max( 1, \a lwork ).
// \param lwork The dimension of the array \a work; size >= max( 1, \a n ).
// \param info Return code of the function call.
// \return void
//
// This function performs the dense matrix QR decomposition of a general \a m-by-\a n double
// precision column-major matrix based on the LAPACK dgeqrf() function. The resulting decomposition
// has the form

							  \f[ A = Q \cdot R, \f]

// where the \c Q is represented as a product of elementary reflectors

			   \f[ Q = H(1) H(2) . . . H(k) \texttt{, with k = min(m,n).} \f]

// Each H(i) has the form

					  \f[ H(i) = I - tau \cdot v \cdot v^T, \f]

// where \c tau is a real scalar, and \c v is a real vector with <tt>v(0:i-1) = 0</tt> and
// <tt>v(i) = 1</tt>. <tt>v(i+1:m)</tt> is stored on exit in <tt>A(i+1:m,i)</tt>, and \c tau
// in \c tau(i). Thus on exit the elements on and above the diagonal of the matrix contain the
// min(\a m,\a n)-by-\a n upper trapezoidal matrix \c R (\c R is upper triangular if \a m >= \a n);
// the elements below the diagonal, with the array \c tau, represent the orthogonal matrix \c Q
// as a product of min(\a m,\a n) elementary reflectors.
//
// The \a info argument provides feedback on the success of the function call:
//
//   - = 0: The decomposition finished successfully.
//   - < 0: The i-th argument had an illegal value.
//
// For more information on the dgeqrf() function, see the LAPACK online documentation browser:
//
//        http://www.netlib.org/lapack/explore-html/
//
// \note This function can only be used if a fitting LAPACK library, which supports this function,
// is available and linked to the executable. Otherwise a call to this function will result in a
// linker error.
*/
inline void geqrf(blas_int_t m, blas_int_t n, double *A, blas_int_t lda, double *tau, double *work, blas_int_t lwork,
				  blas_int_t *info)
{
#if defined(INTEL_MKL_VERSION)
	METRIC_NUMERIC_STATIC_ASSERT(sizeof(MKL_INT) == sizeof(blas_int_t));
#endif

	dgeqrf_(&m, &n, A, &lda, tau, work, &lwork, info);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief LAPACK kernel for the QR decomposition of the given dense single precision complex
//        column-major matrix.
// \ingroup lapack_decomposition
//
// \param m The number of rows of the given matrix \f$[0..\infty)\f$.
// \param n The number of columns of the given matrix \f$[0..\infty)\f$.
// \param A Pointer to the first element of the single precision complex column-major matrix.
// \param lda The total number of elements between two columns of the matrix \f$[0..\infty)\f$.
// \param tau Array for the scalar factors of the elementary reflectors; size >= min( \a m, \a n ).
// \param work Auxiliary array; size >= max( 1, \a lwork ).
// \param lwork The dimension of the array \a work; size >= max( 1, \a n ).
// \param info Return code of the function call.
// \return void
//
// This function performs the dense matrix QR decomposition of a general \a m-by-\a n single
// precision complex column-major matrix based on the LAPACK cgeqrf() function. The resulting
// decomposition has the form

							  \f[ A = Q \cdot R, \f]

// where the \c Q is represented as a product of elementary reflectors

			   \f[ Q = H(1) H(2) . . . H(k) \texttt{, with k = min(m,n).} \f]

// Each H(i) has the form

					  \f[ H(i) = I - tau \cdot v \cdot v^T, \f]

// where \c tau is a real scalar, and \c v is a real vector with <tt>v(0:i-1) = 0</tt> and
// <tt>v(i) = 1</tt>. <tt>v(i+1:m)</tt> is stored on exit in <tt>A(i+1:m,i)</tt>, and \c tau
// in \c tau(i). Thus on exit the elements on and above the diagonal of the matrix contain the
// min(\a m,\a n)-by-\a n upper trapezoidal matrix \c R (\c R is upper triangular if \a m >= \a n);
// the elements below the diagonal, with the array \c tau, represent the orthogonal matrix \c Q
// as a product of min(\a m,\a n) elementary reflectors.
//
// The \a info argument provides feedback on the success of the function call:
//
//   - = 0: The decomposition finished successfully.
//   - < 0: The i-th argument had an illegal value.
//
// For more information on the cgeqrf() function, see the LAPACK online documentation browser:
//
//        http://www.netlib.org/lapack/explore-html/
//
// \note This function can only be used if a fitting LAPACK library, which supports this function,
// is available and linked to the executable. Otherwise a call to this function will result in a
// linker error.
*/
inline void geqrf(blas_int_t m, blas_int_t n, complex<float> *A, blas_int_t lda, complex<float> *tau,
				  complex<float> *work, blas_int_t lwork, blas_int_t *info)
{
	METRIC_NUMERIC_STATIC_ASSERT(sizeof(complex<float>) == 2UL * sizeof(float));

#if defined(INTEL_MKL_VERSION)
	METRIC_NUMERIC_STATIC_ASSERT(sizeof(MKL_INT) == sizeof(blas_int_t));
	METRIC_NUMERIC_STATIC_ASSERT(sizeof(MKL_Complex8) == sizeof(complex<float>));
	using ET = MKL_Complex8;
#else
	using ET = float;
#endif

	cgeqrf_(&m, &n, reinterpret_cast<ET *>(A), &lda, reinterpret_cast<ET *>(tau), reinterpret_cast<ET *>(work), &lwork,
			info);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief LAPACK kernel for the QR decomposition of the given dense double precision complex
//        column-major matrix.
// \ingroup lapack_decomposition
//
// \param m The number of rows of the given matrix \f$[0..\infty)\f$.
// \param n The number of columns of the given matrix \f$[0..\infty)\f$.
// \param A Pointer to the first element of the double precision complex column-major matrix.
// \param lda The total number of elements between two columns of the matrix \f$[0..\infty)\f$.
// \param tau Array for the scalar factors of the elementary reflectors; size >= min( \a m, \a n ).
// \param work Auxiliary array; size >= max( 1, \a lwork ).
// \param lwork The dimension of the array \a work; size >= max( 1, \a n ).
// \param info Return code of the function call.
// \return void
//
// This function performs the dense matrix QR decomposition of a general \a m-by-\a n double
// precision complex column-major matrix based on the LAPACK zgeqrf() function. The resulting
// decomposition has the form

							  \f[ A = Q \cdot R, \f]

// where the \c Q is represented as a product of elementary reflectors

			   \f[ Q = H(1) H(2) . . . H(k) \texttt{, with k = min(m,n).} \f]

// Each H(i) has the form

					  \f[ H(i) = I - tau \cdot v \cdot v^T, \f]

// where \c tau is a real scalar, and \c v is a real vector with <tt>v(0:i-1) = 0</tt> and
// <tt>v(i) = 1</tt>. <tt>v(i+1:m)</tt> is stored on exit in <tt>A(i+1:m,i)</tt>, and \c tau
// in \c tau(i). Thus on exit the elements on and above the diagonal of the matrix contain the
// min(\a m,\a n)-by-\a n upper trapezoidal matrix \c R (\c R is upper triangular if \a m >= \a n);
// the elements below the diagonal, with the array \c tau, represent the orthogonal matrix \c Q
// as a product of min(\a m,\a n) elementary reflectors.
//
// The \a info argument provides feedback on the success of the function call:
//
//   - = 0: The decomposition finished successfully.
//   - < 0: The i-th argument had an illegal value.
//
// For more information on the zgeqrf() function, see the LAPACK online documentation browser:
//
//        http://www.netlib.org/lapack/explore-html/
//
// \note This function can only be used if a fitting LAPACK library, which supports this function,
// is available and linked to the executable. Otherwise a call to this function will result in a
// linker error.
*/
inline void geqrf(blas_int_t m, blas_int_t n, complex<double> *A, blas_int_t lda, complex<double> *tau,
				  complex<double> *work, blas_int_t lwork, blas_int_t *info)
{
	METRIC_NUMERIC_STATIC_ASSERT(sizeof(complex<double>) == 2UL * sizeof(double));

#if defined(INTEL_MKL_VERSION)
	METRIC_NUMERIC_STATIC_ASSERT(sizeof(MKL_INT) == sizeof(blas_int_t));
	METRIC_NUMERIC_STATIC_ASSERT(sizeof(MKL_Complex16) == sizeof(complex<double>));
	using ET = MKL_Complex16;
#else
	using ET = double;
#endif

	zgeqrf_(&m, &n, reinterpret_cast<ET *>(A), &lda, reinterpret_cast<ET *>(tau), reinterpret_cast<ET *>(work), &lwork,
			info);
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
