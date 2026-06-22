// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_LAPACK_TRTRI_H
#define METRIC_NUMERIC_MATH_LAPACK_TRTRI_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/Aliases.h>
#include <metric/numeric/math/Exception.h>
#include <metric/numeric/math/constraints/Adaptor.h>
#include <metric/numeric/math/constraints/BLASCompatible.h>
#include <metric/numeric/math/constraints/Computation.h>
#include <metric/numeric/math/constraints/Contiguous.h>
#include <metric/numeric/math/constraints/MutableDataAccess.h>
#include <metric/numeric/math/expressions/DenseMatrix.h>
#include <metric/numeric/math/lapack/clapack/trtri.h>
#include <metric/numeric/math/typetraits/IsRowMajorMatrix.h>
#include <metric/numeric/util/Assert.h>
#include <metric/numeric/util/NumericCast.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  LAPACK TRIANGULAR MATRIX INVERSION FUNCTIONS (TRTRI)
//
//=================================================================================================

//*************************************************************************************************
/*!\name LAPACK triangular matrix inversion functions (trtri) */
//@{
template <typename MT, bool SO> void trtri(DenseMatrix<MT, SO> &A, char uplo, char diag);
//@}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief LAPACK kernel for the inversion of the given dense triangular matrix.
// \ingroup lapack_inversion
//
// \param A The triangular matrix to be inverted.
// \param uplo \c 'L' in case of a lower matrix, \c 'U' in case of an upper matrix.
// \param diag \c 'U' in case of a unitriangular matrix, \c 'N' otherwise.
// \return void
// \exception std::invalid_argument Invalid non-square matrix provided.
// \exception std::invalid_argument Invalid uplo argument provided.
// \exception std::invalid_argument Invalid diag argument provided.
// \exception std::runtime_error Inversion of singular matrix failed.
//
// This function performs the dense matrix inversion based on the LAPACK trtri() functions for
// a lower triangular (\a uplo = \c 'L') or upper triangular (\a uplo = \a 'U') matrix. Note
// that the function only works for general, non-adapted matrices with \c float, \c double,
// \c complex<float>, or \c complex<double> element type. The attempt to call the function with
// adaptors or matrices of any other element type results in a compile time error!
//
// The function fails if ...
//
//  - ... the given matrix is not a square matrix;
//  - ... the given \a uplo argument is neither \c 'L' nor \c 'U';
//  - ... the given \a diag argument is neither \c 'U' nor \c 'N';
//  - ... the given matrix is singular and not invertible.
//
// In all failure cases an exception is thrown.
//
// For more information on the trtri() functions (i.e. strtri(), dtrtri(), ctrtri(), and ztrtri())
// see the LAPACK online documentation browser:
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
inline void trtri(DenseMatrix<MT, SO> &A, char uplo, char diag)
{
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_ADAPTOR_TYPE(MT);
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_COMPUTATION_TYPE(MT);
	METRIC_NUMERIC_CONSTRAINT_MUST_HAVE_MUTABLE_DATA_ACCESS(MT);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_CONTIGUOUS_TYPE(MT);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_BLAS_COMPATIBLE_TYPE(ElementType_t<MT>);

	if (!isSquare(*A)) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid non-square matrix provided");
	}

	if (uplo != 'L' && uplo != 'U') {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid uplo argument provided");
	}

	if (diag != 'U' && diag != 'N') {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid diag argument provided");
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

	trtri(uplo, diag, n, (*A).data(), lda, &info);

	METRIC_NUMERIC_INTERNAL_ASSERT(info >= 0, "Invalid argument for matrix inversion");

	if (info > 0) {
		METRIC_NUMERIC_THROW_LAPACK_ERROR("Inversion of singular matrix failed");
	}
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
