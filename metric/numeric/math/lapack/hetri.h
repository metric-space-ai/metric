// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_LAPACK_HETRI_H
#define METRIC_NUMERIC_MATH_LAPACK_HETRI_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <memory>
#include <metric/numeric/math/Aliases.h>
#include <metric/numeric/math/Exception.h>
#include <metric/numeric/math/constraints/Adaptor.h>
#include <metric/numeric/math/constraints/BLASCompatible.h>
#include <metric/numeric/math/constraints/Computation.h>
#include <metric/numeric/math/constraints/Contiguous.h>
#include <metric/numeric/math/constraints/MutableDataAccess.h>
#include <metric/numeric/math/expressions/DenseMatrix.h>
#include <metric/numeric/math/lapack/clapack/hetri.h>
#include <metric/numeric/math/typetraits/IsRowMajorMatrix.h>
#include <metric/numeric/util/Assert.h>
#include <metric/numeric/util/NumericCast.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  LAPACK LDLH-BASED INVERSION FUNCTIONS (HETRI)
//
//=================================================================================================

//*************************************************************************************************
/*!\name LAPACK LDLH-based inversion functions (hetri) */
//@{
template <typename MT, bool SO> void hetri(DenseMatrix<MT, SO> &A, char uplo, const blas_int_t *ipiv);
//@}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief LAPACK kernel for the inversion of the given dense Hermitian indefinite matrix.
// \ingroup lapack_inversion
//
// \param A The triangular matrix to be inverted.
// \param uplo \c 'L' in case of a lower matrix, \c 'U' in case of an upper matrix.
// \param ipiv Auxiliary array of size \a n for the pivot indices.
// \return void
// \exception std::invalid_argument Invalid non-square matrix provided.
// \exception std::invalid_argument Invalid uplo argument provided.
// \exception std::runtime_error Inversion of singular matrix failed.
//
// This function performs the dense matrix inversion based on the LAPACK hetri() functions for
// Hermitian indefinite matrices that have already been factorized by the hetrf() functions.
// Note that the function only works for general, non-adapted matrices with \c complex<float> or
// \c complex<double> element type. The attempt to call the function with adaptors or matrices
// of any other element type results in a compile time error!
//
// The function fails if ...
//
//  - ... the given matrix is not a square matrix;
//  - ... the given \a uplo argument is neither \c 'L' nor \c 'U';
//  - ... the given matrix is singular and not invertible.
//
// In all failure cases an exception is thrown.
//
// For more information on the hetri() functions (i.e. chetri() and zhetri()) see the LAPACK
// online documentation browser:
//
//        http://www.netlib.org/lapack/explore-html/
//
// \note This function can only be used if a fitting LAPACK library, which supports this function,
// is available and linked to the executable. Otherwise a call to this function will result in a
// linker error.
//
// \note This function does only provide the basic exception safety guarantee, i.e. in case of an
// exception \a A may already have been modified.
*/
template <typename MT // Type of the dense matrix
		  ,
		  bool SO> // Storage order of the dense matrix
inline void hetri(DenseMatrix<MT, SO> &A, char uplo, const blas_int_t *ipiv)
{
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_ADAPTOR_TYPE(MT);
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_COMPUTATION_TYPE(MT);
	METRIC_NUMERIC_CONSTRAINT_MUST_HAVE_MUTABLE_DATA_ACCESS(MT);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_CONTIGUOUS_TYPE(MT);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_BLAS_COMPATIBLE_TYPE(ElementType_t<MT>);

	using ET = ElementType_t<MT>;

	if (!isSquare(*A)) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid non-square matrix provided");
	}

	if (uplo != 'L' && uplo != 'U') {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid uplo argument provided");
	}

	blas_int_t n(numeric_cast<blas_int_t>((*A).columns()));
	blas_int_t lda(numeric_cast<blas_int_t>((*A).spacing()));
	blas_int_t info(0);

	if (n == 0) {
		return;
	}

	if (IsRowMajorMatrix_v<MT>) {
		(uplo == 'L') ? (uplo = 'U') : (uplo = 'L');
	}

	const std::unique_ptr<ET[]> work(new ET[n]);

	hetri(uplo, n, (*A).data(), lda, ipiv, work.get(), &info);

	METRIC_NUMERIC_INTERNAL_ASSERT(info >= 0, "Invalid argument for matrix inversion");

	if (info > 0) {
		METRIC_NUMERIC_THROW_LAPACK_ERROR("Inversion of singular matrix failed");
	}
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
