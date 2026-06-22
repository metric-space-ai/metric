// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_LAPACK_CLAPACK_ORGLQ_H
#define METRIC_NUMERIC_MATH_LAPACK_CLAPACK_ORGLQ_H
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

void sorglq_(mtrc::numeric::blas_int_t *m, mtrc::numeric::blas_int_t *n, mtrc::numeric::blas_int_t *k, float *A,
			 mtrc::numeric::blas_int_t *lda, float *tau, float *work, mtrc::numeric::blas_int_t *lwork,
			 mtrc::numeric::blas_int_t *info);
void dorglq_(mtrc::numeric::blas_int_t *m, mtrc::numeric::blas_int_t *n, mtrc::numeric::blas_int_t *k, double *A,
			 mtrc::numeric::blas_int_t *lda, double *tau, double *work, mtrc::numeric::blas_int_t *lwork,
			 mtrc::numeric::blas_int_t *info);
}
#endif
/*! \endcond */
//*************************************************************************************************

namespace mtrc::numeric {

//=================================================================================================
//
//  LAPACK FUNCTIONS TO RECONSTRUCT Q FROM A LQ DECOMPOSITION (ORGLQ)
//
//=================================================================================================

//*************************************************************************************************
/*!\name LAPACK functions to reconstruct Q from a LQ decomposition (orglq) */
//@{
void orglq(blas_int_t m, blas_int_t n, blas_int_t k, float *A, blas_int_t lda, const float *tau, float *work,
		   blas_int_t lwork, blas_int_t *info);

void orglq(blas_int_t m, blas_int_t n, blas_int_t k, double *A, blas_int_t lda, const double *tau, double *work,
		   blas_int_t lwork, blas_int_t *info);
//@}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief LAPACK kernel for the reconstruction of the orthogonal matrix Q from a LQ decomposition.
// \ingroup lapack_decomposition
//
// \param m The number of rows of the given matrix \f$[0..n)\f$.
// \param n The number of columns of the given matrix \f$[0..\infty)\f$.
// \param k The number of elementary reflectors, whose product defines the matrix \f$[0..m)\f$.
// \param A Pointer to the first element of the single precision column-major matrix.
// \param lda The total number of elements between two columns of the matrix \f$[0..\infty)\f$.
// \param tau Array for the scalar factors of the elementary reflectors; size >= min( \a m, \a n ).
// \param work Auxiliary array; size >= max( 1, \a lwork ).
// \param lwork The dimension of the array \a work; size >= max( 1, \a n ).
// \param info Return code of the function call.
// \return void
//
// This function generates all or part of the orthogonal matrix Q from a LQ decomposition based on
// the LAPACK sorglq() function for single precision column-major matrices that have already been
// factorized by the sgelqf() function. The \a info argument provides feedback on the success of
// the function call:
//
//   - = 0: The decomposition finished successfully.
//   - < 0: The i-th argument had an illegal value.
//
// For more information on the sorglq() function, see the LAPACK online documentation browser:
//
//        http://www.netlib.org/lapack/explore-html/
//
// \note This function can only be used if a fitting LAPACK library, which supports this function,
// is available and linked to the executable. Otherwise a call to this function will result in a
// linker error.
*/
inline void orglq(blas_int_t m, blas_int_t n, blas_int_t k, float *A, blas_int_t lda, const float *tau, float *work,
				  blas_int_t lwork, blas_int_t *info)
{
#if defined(INTEL_MKL_VERSION)
	METRIC_NUMERIC_STATIC_ASSERT(sizeof(MKL_INT) == sizeof(blas_int_t));
#endif

	sorglq_(&m, &n, &k, A, &lda, const_cast<float *>(tau), work, &lwork, info);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief LAPACK kernel for the reconstruction of the orthogonal matrix Q from a LQ decomposition.
// \ingroup lapack_decomposition
//
// \param m The number of rows of the given matrix \f$[0..n)\f$.
// \param n The number of columns of the given matrix \f$[0..\infty)\f$.
// \param k The number of elementary reflectors, whose product defines the matrix \f$[0..m)\f$.
// \param A Pointer to the first element of the double precision column-major matrix.
// \param lda The total number of elements between two columns of the matrix \f$[0..\infty)\f$.
// \param tau Array for the scalar factors of the elementary reflectors; size >= min( \a m, \a n ).
// \param work Auxiliary array; size >= max( 1, \a lwork ).
// \param lwork The dimension of the array \a work; size >= max( 1, \a n ).
// \param info Return code of the function call.
// \return void
//
// This function generates all or part of the orthogonal matrix Q from a LQ decomposition based on
// the LAPACK dorglq() function for double precision column-major matrices that have already been
// factorized by the dgelqf() function. The \a info argument provides feedback on the success of
// the function call:
//
//   - = 0: The decomposition finished successfully.
//   - < 0: The i-th argument had an illegal value.
//
// For more information on the dorglq() function, see the LAPACK online documentation browser:
//
//        http://www.netlib.org/lapack/explore-html/
//
// \note This function can only be used if a fitting LAPACK library, which supports this function,
// is available and linked to the executable. Otherwise a call to this function will result in a
// linker error.
*/
inline void orglq(blas_int_t m, blas_int_t n, blas_int_t k, double *A, blas_int_t lda, const double *tau, double *work,
				  blas_int_t lwork, blas_int_t *info)
{
#if defined(INTEL_MKL_VERSION)
	METRIC_NUMERIC_STATIC_ASSERT(sizeof(MKL_INT) == sizeof(blas_int_t));
#endif

	dorglq_(&m, &n, &k, A, &lda, const_cast<double *>(tau), work, &lwork, info);
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
