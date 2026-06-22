// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_LAPACK_CLAPACK_POTRS_H
#define METRIC_NUMERIC_MATH_LAPACK_CLAPACK_POTRS_H
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

void spotrs_(char *uplo, mtrc::numeric::blas_int_t *n, mtrc::numeric::blas_int_t *nrhs, float *A,
			 mtrc::numeric::blas_int_t *lda, float *B, mtrc::numeric::blas_int_t *ldb,
			 mtrc::numeric::blas_int_t *info, mtrc::numeric::fortran_charlen_t nuplo);
void dpotrs_(char *uplo, mtrc::numeric::blas_int_t *n, mtrc::numeric::blas_int_t *nrhs, double *A,
			 mtrc::numeric::blas_int_t *lda, double *B, mtrc::numeric::blas_int_t *ldb,
			 mtrc::numeric::blas_int_t *info, mtrc::numeric::fortran_charlen_t nuplo);
void cpotrs_(char *uplo, mtrc::numeric::blas_int_t *n, mtrc::numeric::blas_int_t *nrhs, float *A,
			 mtrc::numeric::blas_int_t *lda, float *B, mtrc::numeric::blas_int_t *ldb,
			 mtrc::numeric::blas_int_t *info, mtrc::numeric::fortran_charlen_t nuplo);
void zpotrs_(char *uplo, mtrc::numeric::blas_int_t *n, mtrc::numeric::blas_int_t *nrhs, double *A,
			 mtrc::numeric::blas_int_t *lda, double *B, mtrc::numeric::blas_int_t *ldb,
			 mtrc::numeric::blas_int_t *info, mtrc::numeric::fortran_charlen_t nuplo);
}
#endif
/*! \endcond */
//*************************************************************************************************

namespace mtrc::numeric {

//=================================================================================================
//
//  LAPACK LLH-BASED SUBSTITUTION FUNCTIONS (POTRS)
//
//=================================================================================================

//*************************************************************************************************
/*!\name LAPACK LLH-based substitution functions (potrs) */
//@{
void potrs(char uplo, blas_int_t n, blas_int_t nrhs, const float *A, blas_int_t lda, float *B, blas_int_t ldb,
		   blas_int_t *info);

void potrs(char uplo, blas_int_t n, blas_int_t nrhs, const double *A, blas_int_t lda, double *B, blas_int_t ldb,
		   blas_int_t *info);

void potrs(char uplo, blas_int_t n, blas_int_t nrhs, const complex<float> *A, blas_int_t lda, complex<float> *B,
		   blas_int_t ldb, blas_int_t *info);

void potrs(char uplo, blas_int_t n, blas_int_t nrhs, const complex<double> *A, blas_int_t lda, complex<double> *B,
		   blas_int_t ldb, blas_int_t *info);
//@}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief LAPACK kernel for the substitution step of solving a positive definite single precision
//        linear system of equations (\f$ A*X=B \f$).
// \ingroup lapack_substitution
//
// \param uplo \c 'L' to use the lower part of the matrix, \c 'U' to use the upper part.
// \param n The number of rows/columns of the column-major matrix \f$[0..\infty)\f$.
// \param nrhs The number of right-hand side vectors \f$[0..\infty)\f$.
// \param A Pointer to the first element of the single precision column-major square matrix.
// \param lda The total number of elements between two columns of matrix \a A \f$[0..\infty)\f$.
// \param B Pointer to the first element of the single precision column-major matrix.
// \param ldb The total number of elements between two columns of matrix \a B \f$[0..\infty)\f$.
// \param info Return code of the function call.
// \return void
//
// This function uses the LAPACK spotrs() function to perform the substitution step to compute
// the solution to the positive definite system of linear equations \f$ A*X=B \f$, where \a A is
// a \a n-by-\a n matrix that has already been factorized by the spotrf() function and \a X and
// \a B are column-major \a n-by-\a nrhs matrices.
//
// The \a info argument provides feedback on the success of the function call:
//
//   - = 0: The function finished successfully.
//   - < 0: If info = -i, the i-th argument had an illegal value.
//
// For more information on the spotrs() function, see the LAPACK online documentation browser:
//
//        http://www.netlib.org/lapack/explore-html/
//
// \note This function can only be used if a fitting LAPACK library, which supports this function,
// is available and linked to the executable. Otherwise a call to this function will result in a
// linker error.
*/
inline void potrs(char uplo, blas_int_t n, blas_int_t nrhs, const float *A, blas_int_t lda, float *B, blas_int_t ldb,
				  blas_int_t *info)
{
#if defined(INTEL_MKL_VERSION)
	METRIC_NUMERIC_STATIC_ASSERT(sizeof(MKL_INT) == sizeof(blas_int_t));
#endif

	spotrs_(&uplo, &n, &nrhs, const_cast<float *>(A), &lda, B, &ldb, info
#if !defined(INTEL_MKL_VERSION)
			,
			mtrc::numeric::fortran_charlen_t(1)
#endif
	);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief LAPACK kernel for the substitution step of solving a positive definite double precision
//        linear system of equations (\f$ A*X=B \f$).
// \ingroup lapack_substitution
//
// \param uplo \c 'L' to use the lower part of the matrix, \c 'U' to use the upper part.
// \param n The number of rows/columns of the column-major matrix \f$[0..\infty)\f$.
// \param nrhs The number of right-hand side vectors \f$[0..\infty)\f$.
// \param A Pointer to the first element of the double precision column-major square matrix.
// \param lda The total number of elements between two columns of matrix \a A \f$[0..\infty)\f$.
// \param B Pointer to the first element of the double precision column-major matrix.
// \param ldb The total number of elements between two columns of matrix \a B \f$[0..\infty)\f$.
// \param info Return code of the function call.
// \return void
//
// This function uses the LAPACK dpotrs() function to perform the substitution step to compute
// the solution to the positive definite system of linear equations \f$ A*X=B \f$, where \a A is
// a \a n-by-\a n matrix that has already been factorized by the dpotrf() function and \a X and
// \a B are column-major \a n-by-\a nrhs matrices.
//
// The \a info argument provides feedback on the success of the function call:
//
//   - = 0: The function finished successfully.
//   - < 0: If info = -i, the i-th argument had an illegal value.
//
// For more information on the dpotrs() function, see the LAPACK online documentation browser:
//
//        http://www.netlib.org/lapack/explore-html/
//
// \note This function can only be used if a fitting LAPACK library, which supports this function,
// is available and linked to the executable. Otherwise a call to this function will result in a
// linker error.
*/
inline void potrs(char uplo, blas_int_t n, blas_int_t nrhs, const double *A, blas_int_t lda, double *B, blas_int_t ldb,
				  blas_int_t *info)
{
#if defined(INTEL_MKL_VERSION)
	METRIC_NUMERIC_STATIC_ASSERT(sizeof(MKL_INT) == sizeof(blas_int_t));
#endif

	dpotrs_(&uplo, &n, &nrhs, const_cast<double *>(A), &lda, B, &ldb, info
#if !defined(INTEL_MKL_VERSION)
			,
			mtrc::numeric::fortran_charlen_t(1)
#endif
	);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief LAPACK kernel for the substitution step of solving a positive definite single precision
//        complex linear system of equations (\f$ A*X=B \f$).
// \ingroup lapack_substitution
//
// \param uplo \c 'L' to use the lower part of the matrix, \c 'U' to use the upper part.
// \param n The number of rows/columns of the column-major matrix \f$[0..\infty)\f$.
// \param nrhs The number of right-hand side vectors \f$[0..\infty)\f$.
// \param A Pointer to the first element of the single precision complex column-major square matrix.
// \param lda The total number of elements between two columns of matrix \a A \f$[0..\infty)\f$.
// \param B Pointer to the first element of the single precision complex column-major matrix.
// \param ldb The total number of elements between two columns of matrix \a B \f$[0..\infty)\f$.
// \param info Return code of the function call.
// \return void
//
// This function uses the LAPACK cpotrs() function to perform the substitution step to compute
// the solution to the positive definite system of linear equations \f$ A*X=B \f$, where \a A is
// a \a n-by-\a n matrix that has already been factorized by the cpotrf() function and \a X and
// \a B are column-major \a n-by-\a nrhs matrices.
//
// The \a info argument provides feedback on the success of the function call:
//
//   - = 0: The function finished successfully.
//   - < 0: If info = -i, the i-th argument had an illegal value.
//
// For more information on the cpotrs() function, see the LAPACK online documentation browser:
//
//        http://www.netlib.org/lapack/explore-html/
//
// \note This function can only be used if a fitting LAPACK library, which supports this function,
// is available and linked to the executable. Otherwise a call to this function will result in a
// linker error.
*/
inline void potrs(char uplo, blas_int_t n, blas_int_t nrhs, const complex<float> *A, blas_int_t lda, complex<float> *B,
				  blas_int_t ldb, blas_int_t *info)
{
	METRIC_NUMERIC_STATIC_ASSERT(sizeof(complex<float>) == 2UL * sizeof(float));

#if defined(INTEL_MKL_VERSION)
	METRIC_NUMERIC_STATIC_ASSERT(sizeof(MKL_INT) == sizeof(blas_int_t));
	METRIC_NUMERIC_STATIC_ASSERT(sizeof(MKL_Complex8) == sizeof(complex<float>));
	using ET = MKL_Complex8;
#else
	using ET = float;
#endif

	cpotrs_(&uplo, &n, &nrhs, const_cast<ET *>(reinterpret_cast<const ET *>(A)), &lda, reinterpret_cast<ET *>(B), &ldb,
			info
#if !defined(INTEL_MKL_VERSION)
			,
			mtrc::numeric::fortran_charlen_t(1)
#endif
	);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief LAPACK kernel for the substitution step of solving a positive definite double precision
//        complex linear system of equations (\f$ A*X=B \f$).
// \ingroup lapack_substitution
//
// \param uplo \c 'L' to use the lower part of the matrix, \c 'U' to use the upper part.
// \param n The number of rows/columns of the column-major matrix \f$[0..\infty)\f$.
// \param nrhs The number of right-hand side vectors \f$[0..\infty)\f$.
// \param A Pointer to the first element of the double precision complex column-major square matrix.
// \param lda The total number of elements between two columns of matrix \a A \f$[0..\infty)\f$.
// \param B Pointer to the first element of the double precision complex column-major matrix.
// \param ldb The total number of elements between two columns of matrix \a B \f$[0..\infty)\f$.
// \param info Return code of the function call.
// \return void
//
// This function uses the LAPACK zpotrs() function to perform the substitution step to compute
// the solution to the positive definite system of linear equations \f$ A*X=B \f$, where \a A is
// a \a n-by-\a n matrix that has already been factorized by the zpotrf() function and \a X and
// \a B are column-major \a n-by-\a nrhs matrices.
//
// The \a info argument provides feedback on the success of the function call:
//
//   - = 0: The function finished successfully.
//   - < 0: If info = -i, the i-th argument had an illegal value.
//
// For more information on the zpotrs() function, see the LAPACK online documentation browser:
//
//        http://www.netlib.org/lapack/explore-html/
//
// \note This function can only be used if a fitting LAPACK library, which supports this function,
// is available and linked to the executable. Otherwise a call to this function will result in a
// linker error.
*/
inline void potrs(char uplo, blas_int_t n, blas_int_t nrhs, const complex<double> *A, blas_int_t lda,
				  complex<double> *B, blas_int_t ldb, blas_int_t *info)
{
	METRIC_NUMERIC_STATIC_ASSERT(sizeof(complex<double>) == 2UL * sizeof(double));

#if defined(INTEL_MKL_VERSION)
	METRIC_NUMERIC_STATIC_ASSERT(sizeof(MKL_INT) == sizeof(blas_int_t));
	METRIC_NUMERIC_STATIC_ASSERT(sizeof(MKL_Complex16) == sizeof(complex<double>));
	using ET = MKL_Complex16;
#else
	using ET = double;
#endif

	zpotrs_(&uplo, &n, &nrhs, const_cast<ET *>(reinterpret_cast<const ET *>(A)), &lda, reinterpret_cast<ET *>(B), &ldb,
			info
#if !defined(INTEL_MKL_VERSION)
			,
			mtrc::numeric::fortran_charlen_t(1)
#endif
	);
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
