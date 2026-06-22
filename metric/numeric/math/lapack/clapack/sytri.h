// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_LAPACK_CLAPACK_SYTRI_H
#define METRIC_NUMERIC_MATH_LAPACK_CLAPACK_SYTRI_H
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

void ssytri_(char *uplo, mtrc::numeric::blas_int_t *n, float *A, mtrc::numeric::blas_int_t *lda,
			 mtrc::numeric::blas_int_t *ipiv, float *work, mtrc::numeric::blas_int_t *info,
			 mtrc::numeric::fortran_charlen_t nuplo);
void dsytri_(char *uplo, mtrc::numeric::blas_int_t *n, double *A, mtrc::numeric::blas_int_t *lda,
			 mtrc::numeric::blas_int_t *ipiv, double *work, mtrc::numeric::blas_int_t *info,
			 mtrc::numeric::fortran_charlen_t nuplo);
void csytri_(char *uplo, mtrc::numeric::blas_int_t *n, float *A, mtrc::numeric::blas_int_t *lda,
			 mtrc::numeric::blas_int_t *ipiv, float *work, mtrc::numeric::blas_int_t *info,
			 mtrc::numeric::fortran_charlen_t nuplo);
void zsytri_(char *uplo, mtrc::numeric::blas_int_t *n, double *A, mtrc::numeric::blas_int_t *lda,
			 mtrc::numeric::blas_int_t *ipiv, double *work, mtrc::numeric::blas_int_t *info,
			 mtrc::numeric::fortran_charlen_t nuplo);
}
#endif
/*! \endcond */
//*************************************************************************************************

namespace mtrc::numeric {

//=================================================================================================
//
//  LAPACK LDLT-BASED INVERSION FUNCTIONS (SYTRI)
//
//=================================================================================================

//*************************************************************************************************
/*!\name LAPACK LDLT-based inversion functions (sytri) */
//@{
void sytri(char uplo, blas_int_t n, float *A, blas_int_t lda, const blas_int_t *ipiv, float *work, blas_int_t *info);

void sytri(char uplo, blas_int_t n, double *A, blas_int_t lda, const blas_int_t *ipiv, double *work, blas_int_t *info);

void sytri(char uplo, blas_int_t n, complex<float> *A, blas_int_t lda, const blas_int_t *ipiv, complex<float> *work,
		   blas_int_t *info);

void sytri(char uplo, blas_int_t n, complex<double> *A, blas_int_t lda, const blas_int_t *ipiv, complex<double> *work,
		   blas_int_t *info);
//@}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief LAPACK kernel for the inversion of the given dense symmetric indefinite single precision
//        column-major square matrix.
// \ingroup lapack_inversion
//
// \param uplo \c 'L' in case of a lower matrix, \c 'U' in case of an upper matrix.
// \param n The number of rows/columns of the symmetric matrix \f$[0..\infty)\f$.
// \param A Pointer to the first element of the single precision column-major matrix.
// \param lda The total number of elements between two columns of the matrix \f$[0..\infty)\f$.
// \param ipiv Auxiliary array of size \a n for the pivot indices.
// \param work Auxiliary array of size \a n.
// \param info Return code of the function call.
// \return void
//
// This function performs the dense matrix inversion based on the LAPACK ssytri() function for
// symmetric indefinite single precision column-major matrices that have already been factorized
// by the ssytrf() function.
//
// The \a info argument provides feedback on the success of the function call:
//
//   - = 0: The inversion finished successfully.
//   - < 0: If \a info = -i, the i-th argument had an illegal value.
//   - > 0: If \a info = i, element D(i,i) is exactly zero and the inverse could not be computed.
//
// For more information on the ssytri() function, see the LAPACK online documentation browser:
//
//        http://www.netlib.org/lapack/explore-html/
//
// \note This function can only be used if a fitting LAPACK library, which supports this function,
// is available and linked to the executable. Otherwise a call to this function will result in a
// linker error.
*/
inline void sytri(char uplo, blas_int_t n, float *A, blas_int_t lda, const blas_int_t *ipiv, float *work,
				  blas_int_t *info)
{
#if defined(INTEL_MKL_VERSION)
	METRIC_NUMERIC_STATIC_ASSERT(sizeof(MKL_INT) == sizeof(blas_int_t));
#endif

	ssytri_(&uplo, &n, A, &lda, const_cast<blas_int_t *>(ipiv), work, info
#if !defined(INTEL_MKL_VERSION)
			,
			mtrc::numeric::fortran_charlen_t(1)
#endif
	);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief LAPACK kernel for the inversion of the given dense symmetric indefinite double precision
//        column-major square matrix.
// \ingroup lapack_inversion
//
// \param uplo \c 'L' in case of a lower matrix, \c 'U' in case of an upper matrix.
// \param n The number of rows/columns of the symmetric matrix \f$[0..\infty)\f$.
// \param A Pointer to the first element of the double precision column-major matrix.
// \param lda The total number of elements between two columns of the matrix \f$[0..\infty)\f$.
// \param ipiv Auxiliary array of size \a n for the pivot indices.
// \param work Auxiliary array of size \a n.
// \param info Return code of the function call.
// \return void
//
// This function performs the dense matrix inversion based on the LAPACK dsytri() function for
// symmetric indefinite double precision column-major matrices that have already been factorized
// by the dsytrf() function.
//
// The \a info argument provides feedback on the success of the function call:
//
//   - = 0: The inversion finished successfully.
//   - < 0: If \a info = -i, the i-th argument had an illegal value.
//   - > 0: If \a info = i, element D(i,i) is exactly zero and the inverse could not be computed.
//
// For more information on the dsytri() function, see the LAPACK online documentation browser:
//
//        http://www.netlib.org/lapack/explore-html/
//
// \note This function can only be used if a fitting LAPACK library, which supports this function,
// is available and linked to the executable. Otherwise a call to this function will result in a
// linker error.
*/
inline void sytri(char uplo, blas_int_t n, double *A, blas_int_t lda, const blas_int_t *ipiv, double *work,
				  blas_int_t *info)
{
#if defined(INTEL_MKL_VERSION)
	METRIC_NUMERIC_STATIC_ASSERT(sizeof(MKL_INT) == sizeof(blas_int_t));
#endif

	dsytri_(&uplo, &n, A, &lda, const_cast<blas_int_t *>(ipiv), work, info
#if !defined(INTEL_MKL_VERSION)
			,
			mtrc::numeric::fortran_charlen_t(1)
#endif
	);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief LAPACK kernel for the inversion of the given dense symmetric indefinite single precision
//        complex column-major square matrix.
// \ingroup lapack_inversion
//
// \param uplo \c 'L' in case of a lower matrix, \c 'U' in case of an upper matrix.
// \param n The number of rows/columns of the symmetric matrix \f$[0..\infty)\f$.
// \param A Pointer to the first element of the single precision complex column-major matrix.
// \param lda The total number of elements between two columns of the matrix \f$[0..\infty)\f$.
// \param ipiv Auxiliary array of size \a n for the pivot indices.
// \param work Auxiliary array of size \a n.
// \param info Return code of the function call.
// \return void
//
// This function performs the dense matrix inversion based on the LAPACK csytri() function for
// symmetric indefinite single precision complex column-major matrices that have already been
// factorized by the csytrf() function.
//
// The \a info argument provides feedback on the success of the function call:
//
//   - = 0: The inversion finished successfully.
//   - < 0: If \a info = -i, the i-th argument had an illegal value.
//   - > 0: If \a info = i, element D(i,i) is exactly zero and the inverse could not be computed.
//
// For more information on the csytri() function, see the LAPACK online documentation browser:
//
//        http://www.netlib.org/lapack/explore-html/
//
// \note This function can only be used if a fitting LAPACK library, which supports this function,
// is available and linked to the executable. Otherwise a call to this function will result in a
// linker error.
*/
inline void sytri(char uplo, blas_int_t n, complex<float> *A, blas_int_t lda, const blas_int_t *ipiv,
				  complex<float> *work, blas_int_t *info)
{
	METRIC_NUMERIC_STATIC_ASSERT(sizeof(complex<float>) == 2UL * sizeof(float));

#if defined(INTEL_MKL_VERSION)
	METRIC_NUMERIC_STATIC_ASSERT(sizeof(MKL_INT) == sizeof(blas_int_t));
	METRIC_NUMERIC_STATIC_ASSERT(sizeof(MKL_Complex8) == sizeof(complex<float>));
	using ET = MKL_Complex8;
#else
	using ET = float;
#endif

	csytri_(&uplo, &n, reinterpret_cast<ET *>(A), &lda, const_cast<blas_int_t *>(ipiv), reinterpret_cast<ET *>(work),
			info
#if !defined(INTEL_MKL_VERSION)
			,
			mtrc::numeric::fortran_charlen_t(1)
#endif
	);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief LAPACK kernel for the inversion of the given dense symmetric indefinite double precision
//        complex column-major square matrix.
// \ingroup lapack_inversion
//
// \param uplo \c 'L' in case of a lower matrix, \c 'U' in case of an upper matrix.
// \param n The number of rows/columns of the symmetric matrix \f$[0..\infty)\f$.
// \param A Pointer to the first element of the double precision complex column-major matrix.
// \param lda The total number of elements between two columns of the matrix \f$[0..\infty)\f$.
// \param ipiv Auxiliary array of size \a n for the pivot indices.
// \param work Auxiliary array of size \a n.
// \param info Return code of the function call.
// \return void
//
// This function performs the dense matrix inversion based on the LAPACK zsytri() function for
// symmetric indefinite double precision complex column-major matrices that have already been
// factorized by the zsytrf() function.
//
// The \a info argument provides feedback on the success of the function call:
//
//   - = 0: The inversion finished successfully.
//   - < 0: If \a info = -i, the i-th argument had an illegal value.
//   - > 0: If \a info = i, element D(i,i) is exactly zero and the inverse could not be computed.
//
// For more information on the zsytri() function, see the LAPACK online documentation browser:
//
//        http://www.netlib.org/lapack/explore-html/
//
// \note This function can only be used if a fitting LAPACK library, which supports this function,
// is available and linked to the executable. Otherwise a call to this function will result in a
// linker error.
*/
inline void sytri(char uplo, blas_int_t n, complex<double> *A, blas_int_t lda, const blas_int_t *ipiv,
				  complex<double> *work, blas_int_t *info)
{
	METRIC_NUMERIC_STATIC_ASSERT(sizeof(complex<double>) == 2UL * sizeof(double));

#if defined(INTEL_MKL_VERSION)
	METRIC_NUMERIC_STATIC_ASSERT(sizeof(MKL_INT) == sizeof(blas_int_t));
	METRIC_NUMERIC_STATIC_ASSERT(sizeof(MKL_Complex16) == sizeof(complex<double>));
	using ET = MKL_Complex16;
#else
	using ET = double;
#endif

	zsytri_(&uplo, &n, reinterpret_cast<ET *>(A), &lda, const_cast<blas_int_t *>(ipiv), reinterpret_cast<ET *>(work),
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
