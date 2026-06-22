// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_LAPACK_CLAPACK_PSTRF_H
#define METRIC_NUMERIC_MATH_LAPACK_CLAPACK_PSTRF_H
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
#if !defined(INTEL_MKL_VERSION) && !defined(BLAS_H)
extern "C" {

void spstrf_(char *uplo, mtrc::numeric::blas_int_t *n, float *A, mtrc::numeric::blas_int_t *lda,
			 mtrc::numeric::blas_int_t *piv, mtrc::numeric::blas_int_t *rank, float *tol, float *work,
			 mtrc::numeric::blas_int_t *info, mtrc::numeric::fortran_charlen_t nuplo);
void dpstrf_(char *uplo, mtrc::numeric::blas_int_t *n, double *A, mtrc::numeric::blas_int_t *lda,
			 mtrc::numeric::blas_int_t *piv, mtrc::numeric::blas_int_t *rank, double *tol, double *work,
			 mtrc::numeric::blas_int_t *info, mtrc::numeric::fortran_charlen_t nuplo);
void cpstrf_(char *uplo, mtrc::numeric::blas_int_t *n, float *A, mtrc::numeric::blas_int_t *lda,
			 mtrc::numeric::blas_int_t *piv, mtrc::numeric::blas_int_t *rank, float *tol, float *work,
			 mtrc::numeric::blas_int_t *info, mtrc::numeric::fortran_charlen_t nuplo);
void zpstrf_(char *uplo, mtrc::numeric::blas_int_t *n, double *A, mtrc::numeric::blas_int_t *lda,
			 mtrc::numeric::blas_int_t *piv, mtrc::numeric::blas_int_t *rank, double *tol, double *work,
			 mtrc::numeric::blas_int_t *info, mtrc::numeric::fortran_charlen_t nuplo);
}
#endif
/*! \endcond */
//*************************************************************************************************

namespace mtrc::numeric {

//=================================================================================================
//
//  LAPACK LLH PIVOTING (CHOLESKY) DECOMPOSITION FUNCTIONS (PSTRF)
//
//=================================================================================================

//*************************************************************************************************
/*!\name LAPACK LLH (Cholesky) decomposition functions (pstrf) */
//@{
void pstrf(char uplo, blas_int_t n, float *A, blas_int_t lda, mtrc::numeric::blas_int_t *piv,
		   mtrc::numeric::blas_int_t *rank, float tol, float *work, blas_int_t *info);

void pstrf(char uplo, blas_int_t n, double *A, blas_int_t lda, mtrc::numeric::blas_int_t *piv,
		   mtrc::numeric::blas_int_t *rank, double tol, double *work, blas_int_t *info);

void pstrf(char uplo, blas_int_t n, complex<float> *A, blas_int_t lda, mtrc::numeric::blas_int_t *piv,
		   mtrc::numeric::blas_int_t *rank, float tol, complex<float> *work, blas_int_t *info);

void pstrf(char uplo, blas_int_t n, complex<double> *A, blas_int_t lda, mtrc::numeric::blas_int_t *piv,
		   mtrc::numeric::blas_int_t *rank, double tol, complex<double> *work, blas_int_t *info);
//@}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief LAPACK kernel for the pivoting Cholesky decomposition of the given dense semi-positive
//        definite single precision column-major matrix.
// \ingroup lapack_decomposition
//
// \param uplo \c 'L' to use the lower part of the matrix, \c 'U' to use the upper part.
// \param n The number of rows/columns of the matrix \f$[0..\infty)\f$.
// \param A Pointer to the first element of the single precision column-major matrix.
// \param lda The total number of elements between two columns of the matrix \f$[0..\infty)\f$.
// \param piv The pivoting indices applied to \c A; size >= n.
// \param rank The rank of A given by the number of steps the algorithm completed.
// \param tol The user-defined tolerance.
// \param work Auxiliary array; size >= \f$ 2*n \f$.
// \param info Return code of the function call.
// \return void
//
// This function performs the dense matrix Cholesky decomposition of a symmetric semi-positive
// definite single precision column-major matrix based on the LAPACK spstrf() function. The
// decomposition has the form

					  \f[ A = P^T U^{H} U P \texttt{ (if uplo = 'U'), or }
						  A = P L L^{H} P^T \texttt{ (if uplo = 'L'), } \f]

// where \c U is an upper triangular matrix and \c L is a lower triangular matrix. The resulting
// decomposition is stored within \a A: In case \a uplo is set to \c 'L' the result is stored in
// the lower part of the matrix and the upper part remains untouched, in case \a uplo is set
// to \c 'U' the result is stored in the upper part and the lower part remains untouched. The
// pivoting matrix is stored as vector in the array \c piv such that the nonzero entries are
// \f$ P(piv(K),K) = 1 \f$.
//
// The \a info argument provides feedback on the success of the function call:
//
//   - = 0: The decomposition finished successfully.
//   - < 0: If info = -i, the i-th argument had an illegal value.
//   - > 0: If info = i, the leading minor of order i is not positive definite.
//
// For more information on the spstrf() function, see the LAPACK online documentation browser:
//
//        http://www.netlib.org/lapack/explore-html/
//
// \note This function can only be used if a fitting LAPACK library, which supports this function,
// is available and linked to the executable. Otherwise a call to this function will result in a
// linker error.
*/
inline void pstrf(char uplo, blas_int_t n, float *A, blas_int_t lda, mtrc::numeric::blas_int_t *piv,
				  mtrc::numeric::blas_int_t *rank, float tol, float *work, blas_int_t *info)
{
#if defined(INTEL_MKL_VERSION)
	METRIC_NUMERIC_STATIC_ASSERT(sizeof(MKL_INT) == sizeof(blas_int_t));
#endif

	spstrf_(&uplo, &n, A, &lda, piv, rank, &tol, work, info
#if !defined(INTEL_MKL_VERSION) && !defined(BLAS_H)
			,
			mtrc::numeric::fortran_charlen_t(1)
#endif
	);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief LAPACK kernel for the pivoting Cholesky decomposition of the given dense semi-positive
//        definite double precision column-major matrix.
// \ingroup lapack_decomposition
//
// \param uplo \c 'L' to use the lower part of the matrix, \c 'U' to use the upper part.
// \param n The number of rows/columns of the matrix \f$[0..\infty)\f$.
// \param A Pointer to the first element of the double precision column-major matrix.
// \param lda The total number of elements between two columns of the matrix \f$[0..\infty)\f$.
// \param piv The pivoting indices applied to \c A; size >= n.
// \param rank The rank of A given by the number of steps the algorithm completed.
// \param tol The user-defined tolerance.
// \param work Auxiliary array; size >= \f$ 2*n \f$.
// \param info Return code of the function call.
// \return void
//
// This function performs the dense matrix Cholesky decomposition of a symmetric semi-positive
// definite double precision column-major matrix based on the LAPACK dpstrf() function. The
// decomposition has the form

					  \f[ A = P^T U^{H} U P \texttt{ (if uplo = 'U'), or }
						  A = P L L^{H} P^T \texttt{ (if uplo = 'L'), } \f]

// where \c U is an upper triangular matrix and \c L is a lower triangular matrix. The resulting
// decomposition is stored within \a A: In case \a uplo is set to \c 'L' the result is stored in
// the lower part of the matrix and the upper part remains untouched, in case \a uplo is set
// to \c 'U' the result is stored in the upper part and the lower part remains untouched. The
// pivoting matrix is stored as vector in the array \c piv such that the nonzero entries are
// \f$ P(piv(K),K) = 1 \f$.
//
// The \a info argument provides feedback on the success of the function call:
//
//   - = 0: The decomposition finished successfully.
//   - < 0: If info = -i, the i-th argument had an illegal value.
//   - > 0: If info = i, the leading minor of order i is not positive definite.
//
// For more information on the dpstrf() function, see the LAPACK online documentation browser:
//
//        http://www.netlib.org/lapack/explore-html/
//
// \note This function can only be used if a fitting LAPACK library, which supports this function,
// is available and linked to the executable. Otherwise a call to this function will result in a
// linker error.
*/
inline void pstrf(char uplo, blas_int_t n, double *A, blas_int_t lda, mtrc::numeric::blas_int_t *piv,
				  mtrc::numeric::blas_int_t *rank, double tol, double *work, blas_int_t *info)
{
#if defined(INTEL_MKL_VERSION)
	METRIC_NUMERIC_STATIC_ASSERT(sizeof(MKL_INT) == sizeof(blas_int_t));
#endif

	dpstrf_(&uplo, &n, A, &lda, piv, rank, &tol, work, info
#if !defined(INTEL_MKL_VERSION) && !defined(BLAS_H)
			,
			mtrc::numeric::fortran_charlen_t(1)
#endif
	);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief LAPACK kernel for the pivoting Cholesky decomposition of the given dense semi-positive
//        definite single precision complex column-major matrix.
// \ingroup lapack_decomposition
//
// \param uplo \c 'L' to use the lower part of the matrix, \c 'U' to use the upper part.
// \param n The number of rows/columns of the matrix \f$[0..\infty)\f$.
// \param A Pointer to the first element of the single precision complex column-major matrix.
// \param lda The total number of elements between two columns of the matrix \f$[0..\infty)\f$.
// \param piv The pivoting indices applied to \c A; size >= n.
// \param rank The rank of A given by the number of steps the algorithm completed.
// \param tol The user-defined tolerance.
// \param work Auxiliary array; size >= \f$ 2*n \f$.
// \param info Return code of the function call.
// \return void
//
// This function performs the dense matrix Cholesky decomposition of a symmetric semi-positive
// definite single precision complex column-major matrix based on the LAPACK cpstrf() function.
// The decomposition has the form

					  \f[ A = P^T U^{H} U P \texttt{ (if uplo = 'U'), or }
						  A = P L L^{H} P^T \texttt{ (if uplo = 'L'), } \f]

// where \c U is an upper triangular matrix and \c L is a lower triangular matrix. The resulting
// decomposition is stored within \a A: In case \a uplo is set to \c 'L' the result is stored in
// the lower part of the matrix and the upper part remains untouched, in case \a uplo is set
// to \c 'U' the result is stored in the upper part and the lower part remains untouched. The
// pivoting matrix is stored as vector in the array \c piv such that the nonzero entries are
// \f$ P(piv(K),K) = 1 \f$.
//
// The \a info argument provides feedback on the success of the function call:
//
//   - = 0: The decomposition finished successfully.
//   - < 0: If info = -i, the i-th argument had an illegal value.
//   - > 0: If info = i, the leading minor of order i is not positive definite.
//
// For more information on the cpstrf() function, see the LAPACK online documentation browser:
//
//        http://www.netlib.org/lapack/explore-html/
//
// \note This function can only be used if a fitting LAPACK library, which supports this function,
// is available and linked to the executable. Otherwise a call to this function will result in a
// linker error.
*/
inline void pstrf(char uplo, blas_int_t n, complex<float> *A, blas_int_t lda, mtrc::numeric::blas_int_t *piv,
				  mtrc::numeric::blas_int_t *rank, float tol, complex<float> *work, blas_int_t *info)
{
#if defined(INTEL_MKL_VERSION)
	METRIC_NUMERIC_STATIC_ASSERT(sizeof(MKL_INT) == sizeof(blas_int_t));
#endif

#if defined(INTEL_MKL_VERSION)
	METRIC_NUMERIC_STATIC_ASSERT(sizeof(MKL_INT) == sizeof(blas_int_t));
	METRIC_NUMERIC_STATIC_ASSERT(sizeof(MKL_Complex8) == sizeof(complex<float>));
	using ET = MKL_Complex8;
#else
	using ET = float;
#endif

	cpstrf_(&uplo, &n, reinterpret_cast<ET *>(A), &lda, piv, rank, &tol, reinterpret_cast<ET *>(work), info
#if !defined(INTEL_MKL_VERSION) && !defined(BLAS_H)
			,
			mtrc::numeric::fortran_charlen_t(1)
#endif
	);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief LAPACK kernel for the pivoting Cholesky decomposition of the given dense semi-positive
//        definite double precision complex column-major matrix.
// \ingroup lapack_decomposition
//
// \param uplo \c 'L' to use the lower part of the matrix, \c 'U' to use the upper part.
// \param n The number of rows/columns of the matrix \f$[0..\infty)\f$.
// \param A Pointer to the first element of the double precision complex column-major matrix.
// \param lda The total number of elements between two columns of the matrix \f$[0..\infty)\f$.
// \param piv The pivoting indices applied to \c A; size >= n.
// \param rank The rank of A given by the number of steps the algorithm completed.
// \param tol The user-defined tolerance.
// \param work Auxiliary array; size >= \f$ 2*n \f$.
// \param info Return code of the function call.
// \return void
//
// This function performs the dense matrix Cholesky decomposition of a symmetric semi-positive
// definite double precision complex column-major matrix based on the LAPACK zpstrf() function.
// The decomposition has the form

					  \f[ A = P^T U^{H} U P \texttt{ (if uplo = 'U'), or }
						  A = P L L^{H} P^T \texttt{ (if uplo = 'L'), } \f]

// where \c U is an upper triangular matrix and \c L is a lower triangular matrix. The resulting
// decomposition is stored within \a A: In case \a uplo is set to \c 'L' the result is stored in
// the lower part of the matrix and the upper part remains untouched, in case \a uplo is set
// to \c 'U' the result is stored in the upper part and the lower part remains untouched. The
// pivoting matrix is stored as vector in the array \c piv such that the nonzero entries are
// \f$ P(piv(K),K) = 1 \f$.
//
// The \a info argument provides feedback on the success of the function call:
//
//   - = 0: The decomposition finished successfully.
//   - < 0: If info = -i, the i-th argument had an illegal value.
//   - > 0: If info = i, the leading minor of order i is not positive definite.
//
// For more information on the zpstrf() function, see the LAPACK online documentation browser:
//
//        http://www.netlib.org/lapack/explore-html/
//
// \note This function can only be used if a fitting LAPACK library, which supports this function,
// is available and linked to the executable. Otherwise a call to this function will result in a
// linker error.
*/
inline void pstrf(char uplo, blas_int_t n, complex<double> *A, blas_int_t lda, mtrc::numeric::blas_int_t *piv,
				  mtrc::numeric::blas_int_t *rank, double tol, complex<double> *work, blas_int_t *info)
{
#if defined(INTEL_MKL_VERSION)
	METRIC_NUMERIC_STATIC_ASSERT(sizeof(MKL_INT) == sizeof(blas_int_t));
#endif

#if defined(INTEL_MKL_VERSION)
	METRIC_NUMERIC_STATIC_ASSERT(sizeof(MKL_INT) == sizeof(blas_int_t));
	METRIC_NUMERIC_STATIC_ASSERT(sizeof(MKL_Complex16) == sizeof(complex<double>));
	using ET = MKL_Complex16;
#else
	using ET = double;
#endif

	zpstrf_(&uplo, &n, reinterpret_cast<ET *>(A), &lda, piv, rank, &tol, reinterpret_cast<ET *>(work), info
#if !defined(INTEL_MKL_VERSION) && !defined(BLAS_H)
			,
			mtrc::numeric::fortran_charlen_t(1)
#endif
	);
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
