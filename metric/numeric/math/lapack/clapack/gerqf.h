// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_LAPACK_CLAPACK_GERQF_H
#define METRIC_NUMERIC_MATH_LAPACK_CLAPACK_GERQF_H
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

void sgerqf_(mtrc::numeric::blas_int_t *m, mtrc::numeric::blas_int_t *n, float *A, mtrc::numeric::blas_int_t *lda,
			 float *tau, float *work, mtrc::numeric::blas_int_t *lwork, mtrc::numeric::blas_int_t *info);
void dgerqf_(mtrc::numeric::blas_int_t *m, mtrc::numeric::blas_int_t *n, double *A,
			 mtrc::numeric::blas_int_t *lda, double *tau, double *work, mtrc::numeric::blas_int_t *lwork,
			 mtrc::numeric::blas_int_t *info);
void cgerqf_(mtrc::numeric::blas_int_t *m, mtrc::numeric::blas_int_t *n, float *A, mtrc::numeric::blas_int_t *lda,
			 float *tau, float *work, mtrc::numeric::blas_int_t *lwork, mtrc::numeric::blas_int_t *info);
void zgerqf_(mtrc::numeric::blas_int_t *m, mtrc::numeric::blas_int_t *n, double *A,
			 mtrc::numeric::blas_int_t *lda, double *tau, double *work, mtrc::numeric::blas_int_t *lwork,
			 mtrc::numeric::blas_int_t *info);
}
#endif
/*! \endcond */
//*************************************************************************************************

namespace mtrc::numeric {

//=================================================================================================
//
//  LAPACK RQ DECOMPOSITION FUNCTIONS (GERQF)
//
//=================================================================================================

//*************************************************************************************************
/*!\name LAPACK RQ decomposition functions (gerqf) */
//@{
void gerqf(blas_int_t m, blas_int_t n, float *A, blas_int_t lda, float *tau, float *work, blas_int_t lwork,
		   blas_int_t *info);

void gerqf(blas_int_t m, blas_int_t n, double *A, blas_int_t lda, double *tau, double *work, blas_int_t lwork,
		   blas_int_t *info);

void gerqf(blas_int_t m, blas_int_t n, complex<float> *A, blas_int_t lda, complex<float> *tau, complex<float> *work,
		   blas_int_t lwork, blas_int_t *info);

void gerqf(blas_int_t m, blas_int_t n, complex<double> *A, blas_int_t lda, complex<double> *tau, complex<double> *work,
		   blas_int_t lwork, blas_int_t *info);
//@}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief LAPACK kernel for the RQ decomposition of the given dense single precision column-major
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
// This function performs the dense matrix RQ decomposition of a general \a m-by-\a n single
// precision column-major matrix based on the LAPACK sgerqf() function. The resulting decomposition
// has the form

							  \f[ A = R \cdot Q, \f]

// where the \c Q is represented as a product of elementary reflectors

			   \f[ Q = H(1) H(2) . . . H(k) \texttt{, with k = min(m,n).} \f]

// Each H(i) has the form

					  \f[ H(i) = I - tau \cdot v \cdot v^T, \f]

// where \c tau is a real scalar, and \c v is a real vector with <tt>v(n-k+i+1:n) = 0</tt> and
// <tt>v(n-k+i) = 1</tt>. <tt>v(1:n-k+i-1)</tt> is stored on exit in <tt>A(m-k+i,1:n-k+i-1)</tt>,
// and \c tau in \c tau(i). Thus in case \a m <= \a n, the upper triangle of the subarray
// A(1:m,n-m+1:n) contains the \a m-by-\a m upper triangular matrix \c R and in case \a m >= \a n,
// the elements on and above the (\a m-\a n)-th subdiagonal contain the \a m-by-\a n upper
// trapezoidal matrix \c R; the remaining elements in combination with the array \c tau represent
// the orthogonal matrix \c Q as a product of min(\a m,\a n) elementary reflectors.
//
// The \a info argument provides feedback on the success of the function call:
//
//   - = 0: The decomposition finished successfully.
//   - < 0: The i-th argument had an illegal value.
//
// For more information on the sgerqf() function, see the LAPACK online documentation browser:
//
//        http://www.netlib.org/lapack/explore-html/
//
// \note This function can only be used if a fitting LAPACK library, which supports this function,
// is available and linked to the executable. Otherwise a call to this function will result in a
// linker error.
*/
inline void gerqf(blas_int_t m, blas_int_t n, float *A, blas_int_t lda, float *tau, float *work, blas_int_t lwork,
				  blas_int_t *info)
{
#if defined(INTEL_MKL_VERSION)
	METRIC_NUMERIC_STATIC_ASSERT(sizeof(MKL_INT) == sizeof(blas_int_t));
#endif

	sgerqf_(&m, &n, A, &lda, tau, work, &lwork, info);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief LAPACK kernel for the RQ decomposition of the given dense single precision column-major
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
// This function performs the dense matrix RQ decomposition of a general \a m-by-\a n double
// precision column-major matrix based on the LAPACK dgerqf() function. The resulting decomposition
// has the form

							  \f[ A = R \cdot Q, \f]

// where the \c Q is represented as a product of elementary reflectors

			   \f[ Q = H(1) H(2) . . . H(k) \texttt{, with k = min(m,n).} \f]

// Each H(i) has the form

					  \f[ H(i) = I - tau \cdot v \cdot v^T, \f]

// where \c tau is a real scalar, and \c v is a real vector with <tt>v(n-k+i+1:n) = 0</tt> and
// <tt>v(n-k+i) = 1</tt>. <tt>v(1:n-k+i-1)</tt> is stored on exit in <tt>A(m-k+i,1:n-k+i-1)</tt>,
// and \c tau in \c tau(i). Thus in case \a m <= \a n, the upper triangle of the subarray
// A(1:m,n-m+1:n) contains the \a m-by-\a m upper triangular matrix \c R and in case \a m >= \a n,
// the elements on and above the (\a m-\a n)-th subdiagonal contain the \a m-by-\a n upper
// trapezoidal matrix \c R; the remaining elements in combination with the array \c tau represent
// the orthogonal matrix \c Q as a product of min(\a m,\a n) elementary reflectors.
//
// The \a info argument provides feedback on the success of the function call:
//
//   - = 0: The decomposition finished successfully.
//   - < 0: The i-th argument had an illegal value.
//
// For more information on the dgerqf() function, see the LAPACK online documentation browser:
//
//        http://www.netlib.org/lapack/explore-html/
//
// \note This function can only be used if a fitting LAPACK library, which supports this function,
// is available and linked to the executable. Otherwise a call to this function will result in a
// linker error.
*/
inline void gerqf(blas_int_t m, blas_int_t n, double *A, blas_int_t lda, double *tau, double *work, blas_int_t lwork,
				  blas_int_t *info)
{
#if defined(INTEL_MKL_VERSION)
	METRIC_NUMERIC_STATIC_ASSERT(sizeof(MKL_INT) == sizeof(blas_int_t));
#endif

	dgerqf_(&m, &n, A, &lda, tau, work, &lwork, info);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief LAPACK kernel for the RQ decomposition of the given dense single precision complex
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
// This function performs the dense matrix RQ decomposition of a general \a m-by-\a n single
// precision complex column-major matrix based on the LAPACK cgerqf() function. The resulting
// decomposition has the form

							  \f[ A = R \cdot Q, \f]

// where the \c Q is represented as a product of elementary reflectors

			   \f[ Q = H(1) H(2) . . . H(k) \texttt{, with k = min(m,n).} \f]

// Each H(i) has the form

					  \f[ H(i) = I - tau \cdot v \cdot v^T, \f]

// where \c tau is a real scalar, and \c v is a real vector with <tt>v(n-k+i+1:n) = 0</tt> and
// <tt>v(n-k+i) = 1</tt>. <tt>v(1:n-k+i-1)</tt> is stored on exit in <tt>A(m-k+i,1:n-k+i-1)</tt>,
// and \c tau in \c tau(i). Thus in case \a m <= \a n, the upper triangle of the subarray
// A(1:m,n-m+1:n) contains the \a m-by-\a m upper triangular matrix \c R and in case \a m >= \a n,
// the elements on and above the (\a m-\a n)-th subdiagonal contain the \a m-by-\a n upper
// trapezoidal matrix \c R; the remaining elements in combination with the array \c tau represent
// the orthogonal matrix \c Q as a product of min(\a m,\a n) elementary reflectors.
//
// The \a info argument provides feedback on the success of the function call:
//
//   - = 0: The decomposition finished successfully.
//   - < 0: The i-th argument had an illegal value.
//
// For more information on the cgerqf() function, see the LAPACK online documentation browser:
//
//        http://www.netlib.org/lapack/explore-html/
//
// \note This function can only be used if a fitting LAPACK library, which supports this function,
// is available and linked to the executable. Otherwise a call to this function will result in a
// linker error.
*/
inline void gerqf(blas_int_t m, blas_int_t n, complex<float> *A, blas_int_t lda, complex<float> *tau,
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

	cgerqf_(&m, &n, reinterpret_cast<ET *>(A), &lda, reinterpret_cast<ET *>(tau), reinterpret_cast<ET *>(work), &lwork,
			info);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief LAPACK kernel for the RQ decomposition of the given dense double precision complex
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
// This function performs the dense matrix RQ decomposition of a general \a m-by-\a n double
// precision complex column-major matrix based on the LAPACK zgerqf() function. The resulting
// decomposition has the form

							  \f[ A = R \cdot Q, \f]

// where the \c Q is represented as a product of elementary reflectors

			   \f[ Q = H(1) H(2) . . . H(k) \texttt{, with k = min(m,n).} \f]

// Each H(i) has the form

					  \f[ H(i) = I - tau \cdot v \cdot v^T, \f]

// where \c tau is a real scalar, and \c v is a real vector with <tt>v(n-k+i+1:n) = 0</tt> and
// <tt>v(n-k+i) = 1</tt>. <tt>v(1:n-k+i-1)</tt> is stored on exit in <tt>A(m-k+i,1:n-k+i-1)</tt>,
// and \c tau in \c tau(i). Thus in case \a m <= \a n, the upper triangle of the subarray
// A(1:m,n-m+1:n) contains the \a m-by-\a m upper triangular matrix \c R and in case \a m >= \a n,
// the elements on and above the (\a m-\a n)-th subdiagonal contain the \a m-by-\a n upper
// trapezoidal matrix \c R; the remaining elements in combination with the array \c tau represent
// the orthogonal matrix \c Q as a product of min(\a m,\a n) elementary reflectors.
//
// The \a info argument provides feedback on the success of the function call:
//
//   - = 0: The decomposition finished successfully.
//   - < 0: The i-th argument had an illegal value.
//
// For more information on the zgerqf() function, see the LAPACK online documentation browser:
//
//        http://www.netlib.org/lapack/explore-html/
//
// \note This function can only be used if a fitting LAPACK library, which supports this function,
// is available and linked to the executable. Otherwise a call to this function will result in a
// linker error.
*/
inline void gerqf(blas_int_t m, blas_int_t n, complex<double> *A, blas_int_t lda, complex<double> *tau,
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

	zgerqf_(&m, &n, reinterpret_cast<ET *>(A), &lda, reinterpret_cast<ET *>(tau), reinterpret_cast<ET *>(work), &lwork,
			info);
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
