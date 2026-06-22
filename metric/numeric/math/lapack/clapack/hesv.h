// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_LAPACK_CLAPACK_HESV_H
#define METRIC_NUMERIC_MATH_LAPACK_CLAPACK_HESV_H
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

void chesv_(char *uplo, mtrc::numeric::blas_int_t *n, mtrc::numeric::blas_int_t *nrhs, float *A,
			mtrc::numeric::blas_int_t *lda, mtrc::numeric::blas_int_t *ipiv, float *b,
			mtrc::numeric::blas_int_t *ldb, float *work, mtrc::numeric::blas_int_t *lwork,
			mtrc::numeric::blas_int_t *info, mtrc::numeric::fortran_charlen_t nuplo);
void zhesv_(char *uplo, mtrc::numeric::blas_int_t *n, mtrc::numeric::blas_int_t *nrhs, double *A,
			mtrc::numeric::blas_int_t *lda, mtrc::numeric::blas_int_t *ipiv, double *b,
			mtrc::numeric::blas_int_t *ldb, double *work, mtrc::numeric::blas_int_t *lwork,
			mtrc::numeric::blas_int_t *info, mtrc::numeric::fortran_charlen_t nuplo);
}
#endif
/*! \endcond */
//*************************************************************************************************

namespace mtrc::numeric {

//=================================================================================================
//
//  LAPACK HERMITIAN INDEFINITE LINEAR SYSTEM FUNCTIONS (HESV)
//
//=================================================================================================

//*************************************************************************************************
/*!\name LAPACK Hermitian indefinite linear system functions (hesv) */
//@{
void hesv(char uplo, blas_int_t n, blas_int_t nrhs, complex<float> *A, blas_int_t lda, blas_int_t *ipiv,
		  complex<float> *B, blas_int_t ldb, complex<float> *work, blas_int_t lwork, blas_int_t *info);

void hesv(char uplo, blas_int_t n, blas_int_t nrhs, complex<double> *A, blas_int_t lda, blas_int_t *ipiv,
		  complex<double> *B, blas_int_t ldb, complex<double> *work, blas_int_t lwork, blas_int_t *info);
//@}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief LAPACK kernel for solving a Hermitian indefinite single precision complex linear system
//        of equations (\f$ A*X=B \f$).
// \ingroup lapack_solver
//
// \param uplo \c 'L' to use the lower part of the matrix, \c 'U' to use the upper part.
// \param n The number of rows/columns of matrix \a A \f$[0..\infty)\f$.
// \param nrhs The number of right-hand side vectors \f$[0..\infty)\f$.
// \param A Pointer to the first element of the single precision complex column-major square matrix.
// \param lda The total number of elements between two columns of matrix \a A \f$[0..\infty)\f$.
// \param ipiv Auxiliary array of size \a n for the pivot indices.
// \param B Pointer to the first element of the column-major matrix.
// \param ldb The total number of elements between two columns of matrix \a B \f$[0..\infty)\f$.
// \param work Auxiliary array; size >= max( 1, \a lwork ).
// \param lwork The dimension of the array \a work; size >= max( 1, \a n ).
// \param info Return code of the function call.
// \return void
//
// This function uses the LAPACK chesv() function to compute the solution to the Hermitian
// indefinite system of linear equations \f$ A*X=B \f$, where \a A is a \a n-by-\a n matrix and
// \a X and \a B are \a n-by-\a nrhs matrices.
//
// The Bunch-Kaufman decomposition is used to factor \a A as

					  \f[ A = U D U^{H} \texttt{ (if uplo = 'U'), or }
						  A = L D L^{H} \texttt{ (if uplo = 'L'), } \f]

// where \c U (or \c L) is a product of permutation and unit upper (lower) triangular matrices,
// and \c D is Hermitian and block diagonal with 1-by-1 and 2-by-2 diagonal blocks. The resulting
// decomposition is stored within \a A: In case \a uplo is set to \c 'L' the result is stored in
// the lower part of the matrix and the upper part remains untouched, in case \a uplo is set to
// \c 'U' the result is stored in the upper part and the lower part remains untouched. The factored
// form of \a A is then used to solve the system of equations.
//
// The \a info argument provides feedback on the success of the function call:
//
//   - = 0: The function finished successfully.
//   - < 0: If info = -i, the i-th argument had an illegal value.
//   - > 0: If info = i, the decomposition has been completed, but since factor D(i,i) is exactly
//          zero the solution could not be computed.
//
// For more information on the chesv() function, see the LAPACK online documentation browser:
//
//        http://www.netlib.org/lapack/explore-html/
//
// \note This function can only be used if a fitting LAPACK library, which supports this function,
// is available and linked to the executable. Otherwise a call to this function will result in a
// linker error.
*/
inline void hesv(char uplo, blas_int_t n, blas_int_t nrhs, complex<float> *A, blas_int_t lda, blas_int_t *ipiv,
				 complex<float> *B, blas_int_t ldb, complex<float> *work, blas_int_t lwork, blas_int_t *info)
{
	METRIC_NUMERIC_STATIC_ASSERT(sizeof(complex<float>) == 2UL * sizeof(float));

#if defined(INTEL_MKL_VERSION)
	METRIC_NUMERIC_STATIC_ASSERT(sizeof(MKL_INT) == sizeof(blas_int_t));
	METRIC_NUMERIC_STATIC_ASSERT(sizeof(MKL_Complex8) == sizeof(complex<float>));
	using ET = MKL_Complex8;
#else
	using ET = float;
#endif

	chesv_(&uplo, &n, &nrhs, reinterpret_cast<ET *>(A), &lda, ipiv, reinterpret_cast<ET *>(B), &ldb,
		   reinterpret_cast<ET *>(work), &lwork, info
#if !defined(INTEL_MKL_VERSION)
		   ,
		   mtrc::numeric::fortran_charlen_t(1)
#endif
	);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief LAPACK kernel for solving a Hermitian indefinite double precision complex linear system
//        of equations (\f$ A*X=B \f$).
// \ingroup lapack_solver
//
// \param uplo \c 'L' to use the lower part of the matrix, \c 'U' to use the upper part.
// \param n The number of rows/columns of matrix \a A \f$[0..\infty)\f$.
// \param nrhs The number of right-hand side vectors \f$[0..\infty)\f$.
// \param A Pointer to the first element of the double precision complex column-major square matrix.
// \param lda The total number of elements between two columns of matrix \a A \f$[0..\infty)\f$.
// \param ipiv Auxiliary array of size \a n for the pivot indices.
// \param B Pointer to the first element of the column-major matrix.
// \param ldb The total number of elements between two columns of matrix \a B \f$[0..\infty)\f$.
// \param work Auxiliary array; size >= max( 1, \a lwork ).
// \param lwork The dimension of the array \a work; size >= max( 1, \a n ).
// \param info Return code of the function call.
// \return void
//
// This function uses the LAPACK zhesv() function to compute the solution to the Hermitian
// indefinite system of linear equations \f$ A*X=B \f$, where \a A is a \a n-by-\a n matrix and
// \a X and \a B are \a n-by-\a nrhs matrices.
//
// The Bunch-Kaufman decomposition is used to factor \a A as

					  \f[ A = U D U^{H} \texttt{ (if uplo = 'U'), or }
						  A = L D L^{H} \texttt{ (if uplo = 'L'), } \f]

// where \c U (or \c L) is a product of permutation and unit upper (lower) triangular matrices,
// and \c D is Hermitian and block diagonal with 1-by-1 and 2-by-2 diagonal blocks. The resulting
// decomposition is stored within \a A: In case \a uplo is set to \c 'L' the result is stored in
// the lower part of the matrix and the upper part remains untouched, in case \a uplo is set to
// \c 'U' the result is stored in the upper part and the lower part remains untouched. The factored
// form of \a A is then used to solve the system of equations.
//
// The \a info argument provides feedback on the success of the function call:
//
//   - = 0: The function finished successfully.
//   - < 0: If info = -i, the i-th argument had an illegal value.
//   - > 0: If info = i, the decomposition has been completed, but since factor D(i,i) is exactly
//          zero the solution could not be computed.
//
// For more information on the zhesv() function, see the LAPACK online documentation browser:
//
//        http://www.netlib.org/lapack/explore-html/
//
// \note This function can only be used if a fitting LAPACK library, which supports this function,
// is available and linked to the executable. Otherwise a call to this function will result in a
// linker error.
*/
inline void hesv(char uplo, blas_int_t n, blas_int_t nrhs, complex<double> *A, blas_int_t lda, blas_int_t *ipiv,
				 complex<double> *B, blas_int_t ldb, complex<double> *work, blas_int_t lwork, blas_int_t *info)
{
	METRIC_NUMERIC_STATIC_ASSERT(sizeof(complex<double>) == 2UL * sizeof(double));

#if defined(INTEL_MKL_VERSION)
	METRIC_NUMERIC_STATIC_ASSERT(sizeof(MKL_INT) == sizeof(blas_int_t));
	METRIC_NUMERIC_STATIC_ASSERT(sizeof(MKL_Complex16) == sizeof(complex<double>));
	using ET = MKL_Complex16;
#else
	using ET = double;
#endif

	zhesv_(&uplo, &n, &nrhs, reinterpret_cast<ET *>(A), &lda, ipiv, reinterpret_cast<ET *>(B), &ldb,
		   reinterpret_cast<ET *>(work), &lwork, info
#if !defined(INTEL_MKL_VERSION)
		   ,
		   mtrc::numeric::fortran_charlen_t(1)
#endif
	);
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
