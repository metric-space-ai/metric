// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_LAPACK_CLAPACK_TRTRI_H
#define METRIC_NUMERIC_MATH_LAPACK_CLAPACK_TRTRI_H
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

void strtri_(char *uplo, char *diag, mtrc::numeric::blas_int_t *n, float *A, mtrc::numeric::blas_int_t *lda,
			 mtrc::numeric::blas_int_t *info, mtrc::numeric::fortran_charlen_t nuplo,
			 mtrc::numeric::fortran_charlen_t ndiag);
void dtrtri_(char *uplo, char *diag, mtrc::numeric::blas_int_t *n, double *A, mtrc::numeric::blas_int_t *lda,
			 mtrc::numeric::blas_int_t *info, mtrc::numeric::fortran_charlen_t nuplo,
			 mtrc::numeric::fortran_charlen_t ndiag);
void ctrtri_(char *uplo, char *diag, mtrc::numeric::blas_int_t *n, float *A, mtrc::numeric::blas_int_t *lda,
			 mtrc::numeric::blas_int_t *info, mtrc::numeric::fortran_charlen_t nuplo,
			 mtrc::numeric::fortran_charlen_t ndiag);
void ztrtri_(char *uplo, char *diag, mtrc::numeric::blas_int_t *n, double *A, mtrc::numeric::blas_int_t *lda,
			 mtrc::numeric::blas_int_t *info, mtrc::numeric::fortran_charlen_t nuplo,
			 mtrc::numeric::fortran_charlen_t ndiag);
}
#endif
/*! \endcond */
//*************************************************************************************************

namespace mtrc::numeric {

//=================================================================================================
//
//  LAPACK TRIANGULAR MATRIX INVERSION FUNCTIONS (TRTRI)
//
//=================================================================================================

//*************************************************************************************************
/*!\name LAPACK triangular matrix inversion functions (trtri) */
//@{
void trtri(char uplo, char diag, blas_int_t n, float *A, blas_int_t lda, blas_int_t *info);

void trtri(char uplo, char diag, blas_int_t n, double *A, blas_int_t lda, blas_int_t *info);

void trtri(char uplo, char diag, blas_int_t n, complex<float> *A, blas_int_t lda, blas_int_t *info);

void trtri(char uplo, char diag, blas_int_t n, complex<double> *A, blas_int_t lda, blas_int_t *info);
//@}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief LAPACK kernel for the inversion of the given dense triangular single precision
//        column-major matrix.
// \ingroup lapack_inversion
//
// \param uplo \c 'L' in case of a lower matrix, \c 'U' in case of an upper matrix.
// \param diag \c 'U' in case of a unitriangular matrix, \c 'N' otherwise.
// \param n The number of rows/columns of the triangular matrix \f$[0..\infty)\f$.
// \param A Pointer to the first element of the single precision column-major matrix.
// \param lda The total number of elements between two columns of the matrix \f$[0..\infty)\f$.
// \param info Return code of the function call.
// \return void
//
// This function performs the dense matrix inversion based on the LAPACK strtri() function for
// lower triangular (\a uplo = \c 'L') or upper triangular (\a uplo = \a 'U') single precision
// column-major matrices.
//
// The \a info argument provides feedback on the success of the function call:
//
//   - = 0: The inversion finished successfully.
//   - < 0: If \a info = -i, the i-th argument had an illegal value.
//   - > 0: If \a info = i, element A(i,i) is exactly zero and the inverse could not be computed.
//
// For more information on the strtri() function, see the LAPACK online documentation browser:
//
//        http://www.netlib.org/lapack/explore-html/
//
// \note This function can only be used if a fitting LAPACK library, which supports this function,
// is available and linked to the executable. Otherwise a call to this function will result in a
// linker error.
*/
inline void trtri(char uplo, char diag, blas_int_t n, float *A, blas_int_t lda, blas_int_t *info)
{
#if defined(INTEL_MKL_VERSION)
	METRIC_NUMERIC_STATIC_ASSERT(sizeof(MKL_INT) == sizeof(blas_int_t));
#endif

	strtri_(&uplo, &diag, &n, A, &lda, info
#if !defined(INTEL_MKL_VERSION) && !defined(BLAS_H)
			,
			mtrc::numeric::fortran_charlen_t(1), mtrc::numeric::fortran_charlen_t(1)
#endif
	);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief LAPACK kernel for the inversion of the given dense triangular double precision
//        column-major matrix.
// \ingroup lapack_inversion
//
// \param uplo \c 'L' in case of a lower matrix, \c 'U' in case of an upper matrix.
// \param diag \c 'U' in case of a unitriangular matrix, \c 'N' otherwise.
// \param n The number of rows/columns of the triangular matrix \f$[0..\infty)\f$.
// \param A Pointer to the first element of the double precision column-major matrix.
// \param lda The total number of elements between two columns of the matrix \f$[0..\infty)\f$.
// \param info Return code of the function call.
// \return void
//
// This function performs the dense matrix inversion based on the LAPACK dtrtri() function for
// lower triangular (\a uplo = \c 'L') or upper triangular (\a uplo = \a 'U') double precision
// column-major matrices.
//
// The \a info argument provides feedback on the success of the function call:
//
//   - = 0: The inversion finished successfully.
//   - < 0: If \a info = -i, the i-th argument had an illegal value.
//   - > 0: If \a info = i, element A(i,i) is exactly zero and the inverse could not be computed.
//
// For more information on the dtrtri() function, see the LAPACK online documentation browser:
//
//        http://www.netlib.org/lapack/explore-html/
//
// \note This function can only be used if a fitting LAPACK library, which supports this function,
// is available and linked to the executable. Otherwise a call to this function will result in a
// linker error.
*/
inline void trtri(char uplo, char diag, blas_int_t n, double *A, blas_int_t lda, blas_int_t *info)
{
#if defined(INTEL_MKL_VERSION)
	METRIC_NUMERIC_STATIC_ASSERT(sizeof(MKL_INT) == sizeof(blas_int_t));
#endif

	dtrtri_(&uplo, &diag, &n, A, &lda, info
#if !defined(INTEL_MKL_VERSION) && !defined(BLAS_H)
			,
			mtrc::numeric::fortran_charlen_t(1), mtrc::numeric::fortran_charlen_t(1)
#endif
	);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief LAPACK kernel for the inversion of the given dense triangular single precision complex
//        column-major matrix.
// \ingroup lapack_inversion
//
// \param uplo \c 'L' in case of a lower matrix, \c 'U' in case of an upper matrix.
// \param diag \c 'U' in case of a unitriangular matrix, \c 'N' otherwise.
// \param n The number of rows/columns of the triangular matrix \f$[0..\infty)\f$.
// \param A Pointer to the first element of the single precision complex column-major matrix.
// \param lda The total number of elements between two columns of the matrix \f$[0..\infty)\f$.
// \param info Return code of the function call.
// \return void
//
// This function performs the dense matrix inversion based on the LAPACK ctrtri() function for
// lower triangular (\a uplo = \c 'L') or upper triangular (\a uplo = \a 'U') single precision
// complex column-major matrices.
//
// The \a info argument provides feedback on the success of the function call:
//
//   - = 0: The inversion finished successfully.
//   - < 0: If \a info = -i, the i-th argument had an illegal value.
//   - > 0: If \a info = i, element A(i,i) is exactly zero and the inverse could not be computed.
//
// For more information on the ctrtri() function, see the LAPACK online documentation browser:
//
//        http://www.netlib.org/lapack/explore-html/
//
// \note This function can only be used if a fitting LAPACK library, which supports this function,
// is available and linked to the executable. Otherwise a call to this function will result in a
// linker error.
*/
inline void trtri(char uplo, char diag, blas_int_t n, complex<float> *A, blas_int_t lda, blas_int_t *info)
{
	METRIC_NUMERIC_STATIC_ASSERT(sizeof(complex<float>) == 2UL * sizeof(float));

#if defined(INTEL_MKL_VERSION)
	METRIC_NUMERIC_STATIC_ASSERT(sizeof(MKL_INT) == sizeof(blas_int_t));
	METRIC_NUMERIC_STATIC_ASSERT(sizeof(MKL_Complex8) == sizeof(complex<float>));
	using ET = MKL_Complex8;
#else
	using ET = float;
#endif

	ctrtri_(&uplo, &diag, &n, reinterpret_cast<ET *>(A), &lda, info
#if !defined(INTEL_MKL_VERSION) && !defined(BLAS_H)
			,
			mtrc::numeric::fortran_charlen_t(1), mtrc::numeric::fortran_charlen_t(1)
#endif
	);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief LAPACK kernel for the inversion of the given dense triangular double precision complex
//        column-major matrix.
// \ingroup lapack_inversion
//
// \param uplo \c 'L' in case of a lower matrix, \c 'U' in case of an upper matrix.
// \param diag \c 'U' in case of a unitriangular matrix, \c 'N' otherwise.
// \param n The number of rows/columns of the triangular matrix \f$[0..\infty)\f$.
// \param A Pointer to the first element of the double precision complex column-major matrix.
// \param lda The total number of elements between two columns of the matrix \f$[0..\infty)\f$.
// \param info Return code of the function call.
// \return void
//
// This function performs the dense matrix inversion based on the LAPACK ztrtri() function for
// lower triangular (\a uplo = \c 'L') or upper triangular (\a uplo = \a 'U') double precision
// complex column-major matrices.
//
// The \a info argument provides feedback on the success of the function call:
//
//   - = 0: The inversion finished successfully.
//   - < 0: If \a info = -i, the i-th argument had an illegal value.
//   - > 0: If \a info = i, element A(i,i) is exactly zero and the inverse could not be computed.
//
// For more information on the ztrtri() function, see the LAPACK online documentation browser:
//
//        http://www.netlib.org/lapack/explore-html/
//
// \note This function can only be used if a fitting LAPACK library, which supports this function,
// is available and linked to the executable. Otherwise a call to this function will result in a
// linker error.
*/
inline void trtri(char uplo, char diag, blas_int_t n, complex<double> *A, blas_int_t lda, blas_int_t *info)
{
	METRIC_NUMERIC_STATIC_ASSERT(sizeof(complex<double>) == 2UL * sizeof(double));

#if defined(INTEL_MKL_VERSION)
	METRIC_NUMERIC_STATIC_ASSERT(sizeof(MKL_INT) == sizeof(blas_int_t));
	METRIC_NUMERIC_STATIC_ASSERT(sizeof(MKL_Complex16) == sizeof(complex<double>));
	using ET = MKL_Complex16;
#else
	using ET = double;
#endif

	ztrtri_(&uplo, &diag, &n, reinterpret_cast<ET *>(A), &lda, info
#if !defined(INTEL_MKL_VERSION) && !defined(BLAS_H)
			,
			mtrc::numeric::fortran_charlen_t(1), mtrc::numeric::fortran_charlen_t(1)
#endif
	);
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
