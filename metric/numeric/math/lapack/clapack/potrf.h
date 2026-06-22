// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_LAPACK_CLAPACK_POTRF_H
#define METRIC_NUMERIC_MATH_LAPACK_CLAPACK_POTRF_H
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

void spotrf_(char *uplo, mtrc::numeric::blas_int_t *n, float *A, mtrc::numeric::blas_int_t *lda,
			 mtrc::numeric::blas_int_t *info, mtrc::numeric::fortran_charlen_t nuplo);
void dpotrf_(char *uplo, mtrc::numeric::blas_int_t *n, double *A, mtrc::numeric::blas_int_t *lda,
			 mtrc::numeric::blas_int_t *info, mtrc::numeric::fortran_charlen_t nuplo);
void cpotrf_(char *uplo, mtrc::numeric::blas_int_t *n, float *A, mtrc::numeric::blas_int_t *lda,
			 mtrc::numeric::blas_int_t *info, mtrc::numeric::fortran_charlen_t nuplo);
void zpotrf_(char *uplo, mtrc::numeric::blas_int_t *n, double *A, mtrc::numeric::blas_int_t *lda,
			 mtrc::numeric::blas_int_t *info, mtrc::numeric::fortran_charlen_t nuplo);
}
#endif
/*! \endcond */
//*************************************************************************************************

namespace mtrc::numeric {

//=================================================================================================
//
//  LAPACK LLH (CHOLESKY) DECOMPOSITION FUNCTIONS (POTRF)
//
//=================================================================================================

//*************************************************************************************************
/*!\name LAPACK LLH (Cholesky) decomposition functions (potrf) */
//@{
void potrf(char uplo, blas_int_t n, float *A, blas_int_t lda, blas_int_t *info);

void potrf(char uplo, blas_int_t n, double *A, blas_int_t lda, blas_int_t *info);

void potrf(char uplo, blas_int_t n, complex<float> *A, blas_int_t lda, blas_int_t *info);

void potrf(char uplo, blas_int_t n, complex<double> *A, blas_int_t lda, blas_int_t *info);
//@}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief LAPACK kernel for the Cholesky decomposition of the given dense positive definite
//        single precision column-major matrix.
// \ingroup lapack_decomposition
//
// \param uplo \c 'L' to use the lower part of the matrix, \c 'U' to use the upper part.
// \param n The number of rows/columns of the matrix \f$[0..\infty)\f$.
// \param A Pointer to the first element of the single precision column-major matrix.
// \param lda The total number of elements between two columns of the matrix \f$[0..\infty)\f$.
// \param info Return code of the function call.
// \return void
//
// This function performs the dense matrix Cholesky decomposition of a symmetric positive definite
// single precision column-major matrix based on the LAPACK spotrf() function. The decomposition
// has the form

					  \f[ A = U^{T} U \texttt{ (if uplo = 'U'), or }
						  A = L L^{T} \texttt{ (if uplo = 'L'), } \f]

// where \c U is an upper triangular matrix and \c L is a lower triangular matrix. The resulting
// decomposition is stored within \a A: In case \a uplo is set to \c 'L' the result is stored in
// the lower part of the matrix and the upper part remains untouched, in case \a uplo is set to
// \c 'U' the result is stored in the upper part and the lower part remains untouched.
//
// The \a info argument provides feedback on the success of the function call:
//
//   - = 0: The decomposition finished successfully.
//   - < 0: If info = -i, the i-th argument had an illegal value.
//   - > 0: If info = i, the leading minor of order i is not positive definite.
//
// For more information on the spotrf() function, see the LAPACK online documentation browser:
//
//        http://www.netlib.org/lapack/explore-html/
//
// \note This function can only be used if a fitting LAPACK library, which supports this function,
// is available and linked to the executable. Otherwise a call to this function will result in a
// linker error.
*/
inline void potrf(char uplo, blas_int_t n, float *A, blas_int_t lda, blas_int_t *info)
{
#if defined(INTEL_MKL_VERSION)
	METRIC_NUMERIC_STATIC_ASSERT(sizeof(MKL_INT) == sizeof(blas_int_t));
#endif

	spotrf_(&uplo, &n, A, &lda, info
#if !defined(INTEL_MKL_VERSION) && !defined(BLAS_H)
			,
			mtrc::numeric::fortran_charlen_t(1)
#endif
	);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief LAPACK kernel for the Cholesky decomposition of the given dense positive definite
//        double precision column-major matrix.
// \ingroup lapack_decomposition
//
// \param uplo \c 'L' to use the lower part of the matrix, \c 'U' to use the upper part.
// \param n The number of rows/columns of the matrix \f$[0..\infty)\f$.
// \param A Pointer to the first element of the double precision column-major matrix.
// \param lda The total number of elements between two columns of the matrix \f$[0..\infty)\f$.
// \param info Return code of the function call.
// \return void
//
// This function performs the dense matrix Cholesky decomposition of a symmetric positive definite
// double precision column-major matrix based on the LAPACK dpotrf() function. The decomposition
// has the form

					  \f[ A = U^{T} U \texttt{ (if uplo = 'U'), or }
						  A = L L^{T} \texttt{ (if uplo = 'L'), } \f]

// where \c U is an upper triangular matrix and \c L is a lower triangular matrix. The resulting
// decomposition is stored within \a A: In case \a uplo is set to \c 'L' the result is stored in
// the lower part of the matrix and the upper part remains untouched, in case \a uplo is set to
// \c 'U' the result is stored in the upper part and the lower part remains untouched.
//
// The \a info argument provides feedback on the success of the function call:
//
//   - = 0: The decomposition finished successfully.
//   - < 0: If info = -i, the i-th argument had an illegal value.
//   - > 0: If info = i, the leading minor of order i is not positive definite.
//
// For more information on the dpotrf() function, see the LAPACK online documentation browser:
//
//        http://www.netlib.org/lapack/explore-html/
//
// \note This function can only be used if a fitting LAPACK library, which supports this function,
// is available and linked to the executable. Otherwise a call to this function will result in a
// linker error.
*/
inline void potrf(char uplo, blas_int_t n, double *A, blas_int_t lda, blas_int_t *info)
{
#if defined(INTEL_MKL_VERSION)
	METRIC_NUMERIC_STATIC_ASSERT(sizeof(MKL_INT) == sizeof(blas_int_t));
#endif

	dpotrf_(&uplo, &n, A, &lda, info
#if !defined(INTEL_MKL_VERSION) && !defined(BLAS_H)
			,
			mtrc::numeric::fortran_charlen_t(1)
#endif
	);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief LAPACK kernel for the Cholesky decomposition of the given dense positive definite
//        single precision complex column-major matrix.
// \ingroup lapack_decomposition
//
// \param uplo \c 'L' to use the lower part of the matrix, \c 'U' to use the upper part.
// \param n The number of rows/columns of the matrix \f$[0..\infty)\f$.
// \param A Pointer to the first element of the single precision complex column-major matrix.
// \param lda The total number of elements between two columns of the matrix \f$[0..\infty)\f$.
// \param info Return code of the function call.
// \return void
//
// This function performs the dense matrix Cholesky decomposition of a symmetric positive
// definite single precision complex column-major matrix based on the LAPACK cpotrf() function.
// The decomposition has the form

					  \f[ A = U^{H} U \texttt{ (if uplo = 'U'), or }
						  A = L L^{H} \texttt{ (if uplo = 'L'), } \f]

// where \c U is an upper triangular matrix and \c L is a lower triangular matrix. The resulting
// decomposition is stored within \a A: In case \a uplo is set to \c 'L' the result is stored in
// the lower part of the matrix and the upper part remains untouched, in case \a uplo is set to
// \c 'U' the result is stored in the upper part and the lower part remains untouched.
//
// The \a info argument provides feedback on the success of the function call:
//
//   - = 0: The decomposition finished successfully.
//   - < 0: If info = -i, the i-th argument had an illegal value.
//   - > 0: If info = i, the leading minor of order i is not positive definite.
//
// For more information on the cpotrf() function, see the LAPACK online documentation browser:
//
//        http://www.netlib.org/lapack/explore-html/
//
// \note This function can only be used if a fitting LAPACK library, which supports this function,
// is available and linked to the executable. Otherwise a call to this function will result in a
// linker error.
*/
inline void potrf(char uplo, blas_int_t n, complex<float> *A, blas_int_t lda, blas_int_t *info)
{
	METRIC_NUMERIC_STATIC_ASSERT(sizeof(complex<float>) == 2UL * sizeof(float));

#if defined(INTEL_MKL_VERSION)
	METRIC_NUMERIC_STATIC_ASSERT(sizeof(MKL_INT) == sizeof(blas_int_t));
	METRIC_NUMERIC_STATIC_ASSERT(sizeof(MKL_Complex8) == sizeof(complex<float>));
	using ET = MKL_Complex8;
#else
	using ET = float;
#endif

	cpotrf_(&uplo, &n, reinterpret_cast<ET *>(A), &lda, info
#if !defined(INTEL_MKL_VERSION) && !defined(BLAS_H)
			,
			mtrc::numeric::fortran_charlen_t(1)
#endif
	);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief LAPACK kernel for the Cholesky decomposition of the given dense positive definite
//        double precision complex column-major matrix.
// \ingroup lapack_decomposition
//
// \param uplo \c 'L' to use the lower part of the matrix, \c 'U' to use the upper part.
// \param n The number of rows/columns of the matrix \f$[0..\infty)\f$.
// \param A Pointer to the first element of the double precision complex column-major matrix.
// \param lda The total number of elements between two columns of the matrix \f$[0..\infty)\f$.
// \param info Return code of the function call.
// \return void
//
// This function performs the dense matrix Cholesky decomposition of a symmetric positive
// definite double precision complex column-major matrix based on the LAPACK zpotrf() function.
// The decomposition has the form

					  \f[ A = U^{H} U \texttt{ (if uplo = 'U'), or }
						  A = L L^{H} \texttt{ (if uplo = 'L'), } \f]

// where \c U is an upper triangular matrix and \c L is a lower triangular matrix. The resulting
// decomposition is stored within \a A: In case \a uplo is set to \c 'L' the result is stored in
// the lower part of the matrix and the upper part remains untouched, in case \a uplo is set to
// \c 'U' the result is stored in the upper part and the lower part remains untouched.
//
// The \a info argument provides feedback on the success of the function call:
//
//   - = 0: The decomposition finished successfully.
//   - < 0: If info = -i, the i-th argument had an illegal value.
//   - > 0: If info = i, the leading minor of order i is not positive definite.
//
// For more information on the zpotrf() function, see the LAPACK online documentation browser:
//
//        http://www.netlib.org/lapack/explore-html/
//
// \note This function can only be used if a fitting LAPACK library, which supports this function,
// is available and linked to the executable. Otherwise a call to this function will result in a
// linker error.
*/
inline void potrf(char uplo, blas_int_t n, complex<double> *A, blas_int_t lda, blas_int_t *info)
{
	METRIC_NUMERIC_STATIC_ASSERT(sizeof(complex<double>) == 2UL * sizeof(double));

#if defined(INTEL_MKL_VERSION)
	METRIC_NUMERIC_STATIC_ASSERT(sizeof(MKL_INT) == sizeof(blas_int_t));
	METRIC_NUMERIC_STATIC_ASSERT(sizeof(MKL_Complex16) == sizeof(complex<double>));
	using ET = MKL_Complex16;
#else
	using ET = double;
#endif

	zpotrf_(&uplo, &n, reinterpret_cast<ET *>(A), &lda, info
#if !defined(INTEL_MKL_VERSION) && !defined(BLAS_H)
			,
			mtrc::numeric::fortran_charlen_t(1)
#endif
	);
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
