// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_LAPACK_CLAPACK_UNGQL_H
#define METRIC_NUMERIC_MATH_LAPACK_CLAPACK_UNGQL_H
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

void cungql_(mtrc::numeric::blas_int_t *m, mtrc::numeric::blas_int_t *n, mtrc::numeric::blas_int_t *k, float *A,
			 mtrc::numeric::blas_int_t *lda, float *tau, float *work, mtrc::numeric::blas_int_t *lwork,
			 mtrc::numeric::blas_int_t *info);
void zungql_(mtrc::numeric::blas_int_t *m, mtrc::numeric::blas_int_t *n, mtrc::numeric::blas_int_t *k, double *A,
			 mtrc::numeric::blas_int_t *lda, double *tau, double *work, mtrc::numeric::blas_int_t *lwork,
			 mtrc::numeric::blas_int_t *info);
}
#endif
/*! \endcond */
//*************************************************************************************************

namespace mtrc::numeric {

//=================================================================================================
//
//  LAPACK FUNCTIONS TO RECONSTRUCT Q FROM A QL DECOMPOSITION (UNGQL)
//
//=================================================================================================

//*************************************************************************************************
/*!\name LAPACK functions to reconstruct Q from a QL decomposition (ungql) */
//@{
void ungql(blas_int_t m, blas_int_t n, blas_int_t k, complex<float> *A, blas_int_t lda, const complex<float> *tau,
		   complex<float> *work, blas_int_t lwork, blas_int_t *info);

void ungql(blas_int_t m, blas_int_t n, blas_int_t k, complex<double> *A, blas_int_t lda, const complex<double> *tau,
		   complex<double> *work, blas_int_t lwork, blas_int_t *info);
//@}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief LAPACK kernel for the reconstruction of the orthogonal matrix Q from a QL decomposition.
// \ingroup lapack_decomposition
//
// \param m The number of rows of the given matrix \f$[0..\infty)\f$.
// \param n The number of columns of the given matrix \f$[0..m)\f$.
// \param k The number of elementary reflectors, whose product defines the matrix \f$[0..n)\f$.
// \param A Pointer to the first element of the single precision column-major matrix.
// \param lda The total number of elements between two columns of the matrix \f$[0..\infty)\f$.
// \param tau Array for the scalar factors of the elementary reflectors; size >= min( \a m, \a n ).
// \param work Auxiliary array; size >= max( 1, \a lwork ).
// \param lwork The dimension of the array \a work; size >= max( 1, \a n ).
// \param info Return code of the function call.
// \return void
//
// This function generates all or part of the orthogonal matrix Q from a QL decomposition based on
// the LAPACK cungql() function for single precision column-major matrices that have already been
// factorized by the sgeqlf() function. The \a info argument provides feedback on the success of
// the function call:
//
//   - = 0: The decomposition finished successfully.
//   - < 0: The i-th argument had an illegal value.
//
// For more information on the cungql() function, see the LAPACK online documentation browser:
//
//        http://www.netlib.org/lapack/explore-html/
//
// \note This function can only be used if a fitting LAPACK library, which supports this function,
// is available and linked to the executable. Otherwise a call to this function will result in a
// linker error.
*/
inline void ungql(blas_int_t m, blas_int_t n, blas_int_t k, complex<float> *A, blas_int_t lda,
				  const complex<float> *tau, complex<float> *work, blas_int_t lwork, blas_int_t *info)
{
	METRIC_NUMERIC_STATIC_ASSERT(sizeof(complex<float>) == 2UL * sizeof(float));

#if defined(INTEL_MKL_VERSION)
	METRIC_NUMERIC_STATIC_ASSERT(sizeof(MKL_INT) == sizeof(blas_int_t));
	METRIC_NUMERIC_STATIC_ASSERT(sizeof(MKL_Complex8) == sizeof(complex<float>));
	using ET = MKL_Complex8;
#else
	using ET = float;
#endif

	cungql_(&m, &n, &k, reinterpret_cast<ET *>(A), &lda, const_cast<ET *>(reinterpret_cast<const ET *>(tau)),
			reinterpret_cast<ET *>(work), &lwork, info);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief LAPACK kernel for the reconstruction of the orthogonal matrix Q from a QL decomposition.
// \ingroup lapack_decomposition
//
// \param m The number of rows of the given matrix \f$[0..\infty)\f$.
// \param n The number of columns of the given matrix \f$[0..m)\f$.
// \param k The number of elementary reflectors, whose product defines the matrix \f$[0..n)\f$.
// \param A Pointer to the first element of the double precision column-major matrix.
// \param lda The total number of elements between two columns of the matrix \f$[0..\infty)\f$.
// \param tau Array for the scalar factors of the elementary reflectors; size >= min( \a m, \a n ).
// \param work Auxiliary array; size >= max( 1, \a lwork ).
// \param lwork The dimension of the array \a work; size >= max( 1, \a n ).
// \param info Return code of the function call.
// \return void
//
// This function generates all or part of the orthogonal matrix Q from a QL decomposition based on
// the LAPACK zungql() function for double precision column-major matrices that have already been
// factorized by the dgeqlf() function. The \a info argument provides feedback on the success of
// the function call:
//
//   - = 0: The decomposition finished successfully.
//   - < 0: The i-th argument had an illegal value.
//
// For more information on the zungql() function, see the LAPACK online documentation browser:
//
//        http://www.netlib.org/lapack/explore-html/
//
// \note This function can only be used if a fitting LAPACK library, which supports this function,
// is available and linked to the executable. Otherwise a call to this function will result in a
// linker error.
*/
inline void ungql(blas_int_t m, blas_int_t n, blas_int_t k, complex<double> *A, blas_int_t lda,
				  const complex<double> *tau, complex<double> *work, blas_int_t lwork, blas_int_t *info)
{
	METRIC_NUMERIC_STATIC_ASSERT(sizeof(complex<double>) == 2UL * sizeof(double));

#if defined(INTEL_MKL_VERSION)
	METRIC_NUMERIC_STATIC_ASSERT(sizeof(MKL_INT) == sizeof(blas_int_t));
	METRIC_NUMERIC_STATIC_ASSERT(sizeof(MKL_Complex16) == sizeof(complex<double>));
	using ET = MKL_Complex16;
#else
	using ET = double;
#endif

	zungql_(&m, &n, &k, reinterpret_cast<ET *>(A), &lda, const_cast<ET *>(reinterpret_cast<const ET *>(tau)),
			reinterpret_cast<ET *>(work), &lwork, info);
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
