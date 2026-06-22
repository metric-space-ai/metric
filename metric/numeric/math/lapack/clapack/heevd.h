// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_LAPACK_CLAPACK_HEEVD_H
#define METRIC_NUMERIC_MATH_LAPACK_CLAPACK_HEEVD_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/blas/Types.h>
#include <metric/numeric/util/Complex.h>
#include <metric/numeric/util/StaticAssert.h>
#include <metric/numeric/util/Types.h>

//=================================================================================================
//
//  LAPACK FORWARD DECLARATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
#if !defined(INTEL_MKL_VERSION)
extern "C" {

void cheevd_(char *jobz, char *uplo, mtrc::numeric::blas_int_t *n, float *A, mtrc::numeric::blas_int_t *lda,
			 float *w, float *work, mtrc::numeric::blas_int_t *lwork, float *rwork,
			 mtrc::numeric::blas_int_t *lrwork, mtrc::numeric::blas_int_t *iwork,
			 mtrc::numeric::blas_int_t *liwork, mtrc::numeric::blas_int_t *info,
			 mtrc::numeric::fortran_charlen_t njobz, mtrc::numeric::fortran_charlen_t nuplo);
void zheevd_(char *jobz, char *uplo, mtrc::numeric::blas_int_t *n, double *A, mtrc::numeric::blas_int_t *lda,
			 double *w, double *work, mtrc::numeric::blas_int_t *lwork, double *rwork,
			 mtrc::numeric::blas_int_t *lrwork, mtrc::numeric::blas_int_t *iwork,
			 mtrc::numeric::blas_int_t *liwork, mtrc::numeric::blas_int_t *info,
			 mtrc::numeric::fortran_charlen_t njobz, mtrc::numeric::fortran_charlen_t nuplo);
}
#endif
/*! \endcond */
//*************************************************************************************************

namespace mtrc::numeric {

//=================================================================================================
//
//  LAPACK HERMITIAN MATRIX EIGENVALUE FUNCTIONS (HEEVD)
//
//=================================================================================================

//*************************************************************************************************
/*!\name LAPACK Hermitian matrix eigenvalue functions (heevd) */
//@{
void heevd(char jobz, char uplo, blas_int_t n, complex<float> *A, blas_int_t lda, float *w, complex<float> *work,
		   blas_int_t lwork, float *rwork, blas_int_t lrwork, blas_int_t *iwork, blas_int_t liwork, blas_int_t *info);

void heevd(char jobz, char uplo, blas_int_t n, complex<double> *A, blas_int_t lda, double *w, complex<double> *work,
		   blas_int_t lwork, double *rwork, blas_int_t lrwork, blas_int_t *iwork, blas_int_t liwork, blas_int_t *info);
//@}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief LAPACK kernel for computing the eigenvalues of the given dense Hermitian single
//        precision column-major matrix.
// \ingroup lapack_eigenvalue
//
// \param jobz \c 'V' to compute the eigenvectors of \a A, \c 'N' to only compute the eigenvalues.
// \param uplo \c 'L' to use the lower part of the matrix, \c 'U' to use the upper part.
// \param n The number of rows and columns of the given matrix \f$[0..\infty)\f$.
// \param A Pointer to the first element of the single precision column-major matrix.
// \param lda The total number of elements between two columns of the matrix A \f$[0..\infty)\f$.
// \param w Pointer to the first element of the vector for the eigenvalues.
// \param work Auxiliary array; size >= max( 1, \a lwork ).
// \param lwork The dimension of the array \a work; see online reference for details.
// \param rwork Auxiliary array; size >= max( 1, \a lrwork ).
// \param lrwork The dimension of the array \a rwork; see online reference for details.
// \param iwork Auxiliary array; size >= max( 1, \a liwork ).
// \param liwork The dimension of the array \a iwork; see online reference for details.
// \param info Return code of the function call.
// \return void
//
// This function computes the eigenvalues of an Hermitian \a n-by-\a n single precision complex
// column-major matrix based on the LAPACK cheevd() function. Optionally, it computes the left
// and right eigenvectors using a divide-and-conquer strategy. The real eigenvalues are returned
// in ascending order in the given \a n-dimensional vector \a w.
//
// The parameter \a jobz specifies the computation of the orthonormal eigenvectors of \a A:
//
//   - \c 'V': The eigenvectors of \a A are computed and returned within the matrix \a A.
//   - \c 'N': The eigenvectors of \a A are not computed.
//
// The \a info argument provides feedback on the success of the function call:
//
//   - = 0: The computation finished successfully.
//   - < 0: If info = -i, the i-th argument had an illegal value.
//   - > 0: If info = i, the algorithm failed to converge; i values did not converge to zero.
//
// For more information on the cheevd() function, see the LAPACK online documentation browser:
//
//        http://www.netlib.org/lapack/explore-html/
//
// \note This function can only be used if a fitting LAPACK library, which supports this function,
// is available and linked to the executable. Otherwise a call to this function will result in a
// linker error.
*/
inline void heevd(char jobz, char uplo, blas_int_t n, complex<float> *A, blas_int_t lda, float *w, complex<float> *work,
				  blas_int_t lwork, float *rwork, blas_int_t lrwork, blas_int_t *iwork, blas_int_t liwork,
				  blas_int_t *info)
{
	METRIC_NUMERIC_STATIC_ASSERT(sizeof(complex<float>) == 2UL * sizeof(float));

#if defined(INTEL_MKL_VERSION)
	METRIC_NUMERIC_STATIC_ASSERT(sizeof(MKL_INT) == sizeof(blas_int_t));
	METRIC_NUMERIC_STATIC_ASSERT(sizeof(MKL_Complex8) == sizeof(complex<float>));
	using ET = MKL_Complex8;
#else
	using ET = float;
#endif

	cheevd_(&jobz, &uplo, &n, reinterpret_cast<ET *>(A), &lda, w, reinterpret_cast<ET *>(work), &lwork, rwork, &lrwork,
			iwork, &liwork, info
#if !defined(INTEL_MKL_VERSION)
			,
			mtrc::numeric::fortran_charlen_t(1), mtrc::numeric::fortran_charlen_t(1)
#endif
	);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief LAPACK kernel for computing the eigenvalues of the given dense Hermitian double
//        precision column-major matrix.
// \ingroup lapack_eigenvalue
//
// \param jobz \c 'V' to compute the eigenvectors of \a A, \c 'N' to only compute the eigenvalues.
// \param uplo \c 'L' to use the lower part of the matrix, \c 'U' to use the upper part.
// \param n The number of rows and columns of the given matrix \f$[0..\infty)\f$.
// \param A Pointer to the first element of the double precision column-major matrix.
// \param lda The total number of elements between two columns of the matrix A \f$[0..\infty)\f$.
// \param w Pointer to the first element of the vector for the eigenvalues.
// \param work Auxiliary array; size >= max( 1, \a lwork ).
// \param lwork The dimension of the array \a work; see online reference for details.
// \param rwork Auxiliary array; size >= max( 1, \a lrwork ).
// \param lrwork The dimension of the array \a rwork; see online reference for details.
// \param iwork Auxiliary array; size >= max( 1, \a liwork ).
// \param liwork The dimension of the array \a iwork; see online reference for details.
// \param info Return code of the function call.
// \return void
//
// This function computes the eigenvalues of an Hermitian \a n-by-\a n double precision complex
// column-major matrix based on the LAPACK cheevd() function. Optionally, it computes the left
// and right eigenvectors using a divide-and-conquer strategy. The real eigenvalues are returned
// in ascending order in the given \a n-dimensional vector \a w.
//
// The parameter \a jobz specifies the computation of the orthonormal eigenvectors of \a A:
//
//   - \c 'V': The eigenvectors of \a A are computed and returned within the matrix \a A.
//   - \c 'N': The eigenvectors of \a A are not computed.
//
// The \a info argument provides feedback on the success of the function call:
//
//   - = 0: The computation finished successfully.
//   - < 0: If info = -i, the i-th argument had an illegal value.
//   - > 0: If info = i, the algorithm failed to converge; i values did not converge to zero.
//
// For more information on the cheevd() function, see the LAPACK online documentation browser:
//
//        http://www.netlib.org/lapack/explore-html/
//
// \note This function can only be used if a fitting LAPACK library, which supports this function,
// is available and linked to the executable. Otherwise a call to this function will result in a
// linker error.
*/
inline void heevd(char jobz, char uplo, blas_int_t n, complex<double> *A, blas_int_t lda, double *w,
				  complex<double> *work, blas_int_t lwork, double *rwork, blas_int_t lrwork, blas_int_t *iwork,
				  blas_int_t liwork, blas_int_t *info)
{
	METRIC_NUMERIC_STATIC_ASSERT(sizeof(complex<double>) == 2UL * sizeof(double));

#if defined(INTEL_MKL_VERSION)
	METRIC_NUMERIC_STATIC_ASSERT(sizeof(MKL_INT) == sizeof(blas_int_t));
	METRIC_NUMERIC_STATIC_ASSERT(sizeof(MKL_Complex16) == sizeof(complex<double>));
	using ET = MKL_Complex16;
#else
	using ET = double;
#endif

	zheevd_(&jobz, &uplo, &n, reinterpret_cast<ET *>(A), &lda, w, reinterpret_cast<ET *>(work), &lwork, rwork, &lrwork,
			iwork, &liwork, info
#if !defined(INTEL_MKL_VERSION)
			,
			mtrc::numeric::fortran_charlen_t(1), mtrc::numeric::fortran_charlen_t(1)
#endif
	);
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
