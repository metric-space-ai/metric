// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_LAPACK_GETRF_H
#define METRIC_NUMERIC_MATH_LAPACK_GETRF_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/Aliases.h>
#include <metric/numeric/math/constraints/Adaptor.h>
#include <metric/numeric/math/constraints/BLASCompatible.h>
#include <metric/numeric/math/constraints/Computation.h>
#include <metric/numeric/math/constraints/Contiguous.h>
#include <metric/numeric/math/constraints/MutableDataAccess.h>
#include <metric/numeric/math/expressions/DenseMatrix.h>
#include <metric/numeric/math/lapack/clapack/getrf.h>
#include <metric/numeric/util/Assert.h>
#include <metric/numeric/util/NumericCast.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  LAPACK LU DECOMPOSITION FUNCTIONS (GETRF)
//
//=================================================================================================

//*************************************************************************************************
/*!\name LAPACK LU decomposition functions (getrf) */
//@{
template <typename MT, bool SO> void getrf(DenseMatrix<MT, SO> &A, blas_int_t *ipiv);
//@}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief LAPACK kernel for the LU decomposition of the given dense general matrix.
// \ingroup lapack_decomposition
//
// \param A The matrix to be decomposed.
// \param ipiv Auxiliary array for the pivot indices; size >= min( \a m, \a n ).
// \return void
//
// This function performs the dense matrix LU decomposition of a general \a m-by-\a n matrix based
// on the LAPACK \c getrf() functions, which use partial pivoting with row/column interchanges.
// Note that the function only works for general, non-adapted matrices with \c float, \c double,
// \c complex<float>, or \c complex<double> element type. The attempt to call the function with
// adaptors or matrices of any other element type results in a compile time error!\n
//
// In case of a column-major matrix, the resulting decomposition has the form

						  \f[ A = P \cdot L \cdot U, \f]

// where \c L is a lower unitriangular matrix (lower trapezoidal if \a m > \a n), \c U is an upper
// triangular matrix (upper trapezoidal if \a m < \a n), and \c P is an m-by-m permutation matrix,
// which represents the pivoting indices for the applied row interchanges.
//
// In case of a row-major matrix, the resulting decomposition has the form

						  \f[ A = L \cdot U \cdot P, \f]

// where \c P is an \a n-by-\a n permutation matrix, which represents the pivoting indices for
// the applied column interchanges, \c L is a lower triangular matrix (lower trapezoidal if
// \a m > \a n), and \c U is an upper unitriangular matrix (upper trapezoidal if \a m < \a n).
//
// The resulting decomposition is stored within the matrix \a A: \c L is stored in the lower part
// of \a A and \c U is stored in the upper part. The unit diagonal elements of \c L or \c U are
// not stored.
//
// For more information on the getrf() functions (i.e. sgetrf(), dgetrf(), cgetrf(), and zgetrf())
// see the LAPACK online documentation browser:
//
//        http://www.netlib.org/lapack/explore-html/
//
// \note This function can only be used if a fitting LAPACK library, which supports this function,
// is available and linked to the executable. Otherwise a call to this function will result in a
// linker error.
//
// \note The LU decomposition will never fail, even for singular matrices. However, in case of a
// singular matrix the resulting decomposition cannot be used for a matrix inversion or solving
// a linear system of equations.
*/
template <typename MT // Type of the dense matrix
		  ,
		  bool SO> // Storage order of the dense matrix
inline void getrf(DenseMatrix<MT, SO> &A, blas_int_t *ipiv)
{
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_ADAPTOR_TYPE(MT);
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_COMPUTATION_TYPE(MT);
	METRIC_NUMERIC_CONSTRAINT_MUST_HAVE_MUTABLE_DATA_ACCESS(MT);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_CONTIGUOUS_TYPE(MT);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_BLAS_COMPATIBLE_TYPE(ElementType_t<MT>);

	blas_int_t m(numeric_cast<blas_int_t>(SO ? (*A).rows() : (*A).columns()));
	blas_int_t n(numeric_cast<blas_int_t>(SO ? (*A).columns() : (*A).rows()));
	blas_int_t lda(numeric_cast<blas_int_t>((*A).spacing()));
	blas_int_t info(0);

	if (m == 0 || n == 0) {
		return;
	}

	getrf(m, n, (*A).data(), lda, ipiv, &info);

	METRIC_NUMERIC_INTERNAL_ASSERT(info >= 0, "Invalid argument for LU decomposition");
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
