// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_LAPACK_PSTRF_H
#define METRIC_NUMERIC_MATH_LAPACK_PSTRF_H
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
#include <metric/numeric/math/lapack/clapack/pstrf.h>
#include <metric/numeric/util/Assert.h>
#include <metric/numeric/util/NumericCast.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  LAPACK LLH PIVOTING (CHOLESKY) DECOMPOSITION FUNCTIONS (PsTRF)
//
//=================================================================================================

//*************************************************************************************************
/*!\name LAPACK LLH Pivoting (Cholesky) decomposition functions (pstrf) */
//@{
template <typename MT, bool SO, typename ST>
blas_int_t pstrf(DenseMatrix<MT, SO> &A, char uplo, blas_int_t *piv, ST tol);
//@}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief LAPACK kernel for the Cholesky decomposition of the given dense positive definite matrix.
// \ingroup lapack_decomposition
//
// \param A The matrix to be decomposed.
// \param uplo \c 'L' to use the lower part of the matrix, \c 'U' to use the upper part.
// \param piv The pivoting indices applied to \c A
// \param piv The pivoting indices applied to \c A; size >= n.
// \param tol The user-defined tolerance.
// \return The rank of \c A given by the number of steps the algorithm completed.
// \exception std::invalid_argument Invalid non-square matrix provided.
// \exception std::invalid_argument Invalid uplo argument provided.
// \exception std::runtime_error Decomposition of singular matrix failed.
//
// This function performs the dense matrix Cholesky decomposition of a symmetric positive
// semi-definite \a n-by-\a n matrix based on the LAPACK pstrf() functions. Note that
// the function only works for general, non-adapted matrices with \c float, \c double,
// \c complex<float>, or \c complex<double> element type. The attempt to call the function
// with any adapted matrix or matrices of any other element type results in a compile time
// error!\n
//
// The decomposition has the form

					  \f[ A = P^T U^{H} U P \texttt{ (if uplo = 'U'), or }
						  A = P L L^{H} P^T \texttt{ (if uplo = 'L'), } \f]

// where \c U is an upper triangular matrix and \c L is a lower triangular matrix.
// The pivoting indices are represented by the pivoting matrix \c P.
// The Cholesky decomposition fails if ...
//
//  - ... the given system matrix \c A is not a symmetric positive semi-definite matrix;
//  - ... the given \a uplo argument is neither \c 'L' nor \c 'U'.
//
// In all failure cases an exception is thrown.
//
// For more information on the pstrf() functions (i.e. spstrf(), dpstrf(), cpstrf(), and zpstrf())
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
		  bool SO // Storage order of the dense matrix
		  ,
		  typename ST> // Type of the scalar tolerance value
inline blas_int_t pstrf(DenseMatrix<MT, SO> &A, char uplo, blas_int_t *piv, ST tol)
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

	using ET = ElementType_t<MT>;

	blas_int_t n(numeric_cast<blas_int_t>((*A).rows()));
	blas_int_t lda(numeric_cast<blas_int_t>((*A).spacing()));
	blas_int_t info(0);
	blas_int_t rank(0);

	if (n == 0) {
		return rank;
	}

	if (IsRowMajorMatrix_v<MT>) {
		(uplo == 'L') ? (uplo = 'U') : (uplo = 'L');
	}

	const std::unique_ptr<ET[]> work(new ET[n * 2]);

	pstrf(uplo, n, (*A).data(), lda, piv, &rank, tol, work.get(), &info);

	METRIC_NUMERIC_INTERNAL_ASSERT(info >= 0, "Invalid argument for Cholesky decomposition");

	for (size_t i = 0UL; i < n; ++i) {
		--piv[i]; // Adapt from Fortran 1-based to C 0-based indexing
	}

	return rank;
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
