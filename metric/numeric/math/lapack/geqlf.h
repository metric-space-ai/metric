// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_LAPACK_GEQLF_H
#define METRIC_NUMERIC_MATH_LAPACK_GEQLF_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <memory>
#include <metric/numeric/math/Aliases.h>
#include <metric/numeric/math/constraints/Adaptor.h>
#include <metric/numeric/math/constraints/BLASCompatible.h>
#include <metric/numeric/math/constraints/Computation.h>
#include <metric/numeric/math/constraints/Contiguous.h>
#include <metric/numeric/math/constraints/MutableDataAccess.h>
#include <metric/numeric/math/expressions/DenseMatrix.h>
#include <metric/numeric/math/lapack/clapack/geqlf.h>
#include <metric/numeric/math/lapack/clapack/gerqf.h>
#include <metric/numeric/util/Assert.h>
#include <metric/numeric/util/NumericCast.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  LAPACK QL DECOMPOSITION FUNCTIONS (GEQLF)
//
//=================================================================================================

//*************************************************************************************************
/*!\name LAPACK QL decomposition functions (geqlf) */
//@{
template <typename MT, bool SO> void geqlf(DenseMatrix<MT, SO> &A, ElementType_t<MT> *tau);
//@}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief LAPACK kernel for the QL decomposition of the given dense matrix.
// \ingroup lapack_decomposition
//
// \param A The matrix to be decomposed.
// \param tau Array for the scalar factors of the elementary reflectors; size >= min( \a m, \a n ).
// \return void
//
// This function performs the dense matrix QL decomposition of a general \a m-by-\a n matrix
// based on the LAPACK geqlf() functions. Note that this function can only be used for general,
// non-adapted matrices with \c float, \c double, \c complex<float>, or \c complex<double> element
// type. The attempt to call the function with any adapted matrix or matrices of any other element
// type results in a compile time error!\n
//
// In case of a column-major matrix, the resulting decomposition has the form

							  \f[ A = Q \cdot L, \f]

// where the \c Q is represented as a product of elementary reflectors

			   \f[ Q = H(k) ... H(2) H(1) \texttt{, with k = min(m,n).} \f]

// Each H(i) has the form

					  \f[ H(i) = I - tau \cdot v \cdot v^T, \f]

// where \c tau is a real scalar, and \c v is a real vector with <tt>v(m-k+i+1:m) = 0</tt> and
// <tt>v(m-k+i) = 1</tt>. <tt>v(1:m-k+i-1)</tt> is stored on exit in <tt>A(1:m-k+i-1,n-k+i)</tt>,
// and \c tau in \c tau(i). Thus in case \a m >= \a n, the lower triangle of the subarray
// A(m-n+1:m,1:n) contains the \a n-by-\a n lower triangular matrix \c L and in case \a m <= \a n,
// the elements on and below the (\a n-\a m)-th subdiagonal contain the \a m-by-\a n lower
// trapezoidal matrix \c L; the remaining elements in combination with the array \c tau represent
// the orthogonal matrix \c Q as a product of min(\a m,\a n) elementary reflectors.
//
// For more information on the geqlf() functions (i.e. sgeqlf(), dgeqlf(), cgeqlf(), and zgeqlf())
// see the LAPACK online documentation browser:
//
//        http://www.netlib.org/lapack/explore-html/
//
// \note This function can only be used if a fitting LAPACK library, which supports this function,
// is available and linked to the executable. Otherwise a call to this function will result in a
// linker error.
*/
template <typename MT // Type of the dense matrix
		  ,
		  bool SO> // Storage order of the dense matrix
inline void geqlf(DenseMatrix<MT, SO> &A, ElementType_t<MT> *tau)
{
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_ADAPTOR_TYPE(MT);
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_COMPUTATION_TYPE(MT);
	METRIC_NUMERIC_CONSTRAINT_MUST_HAVE_MUTABLE_DATA_ACCESS(MT);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_CONTIGUOUS_TYPE(MT);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_BLAS_COMPATIBLE_TYPE(ElementType_t<MT>);

	using ET = ElementType_t<MT>;

	blas_int_t m(numeric_cast<blas_int_t>(SO ? (*A).rows() : (*A).columns()));
	blas_int_t n(numeric_cast<blas_int_t>(SO ? (*A).columns() : (*A).rows()));
	blas_int_t lda(numeric_cast<blas_int_t>((*A).spacing()));
	blas_int_t info(0);

	if (m == 0 || n == 0) {
		return;
	}

	blas_int_t lwork((SO ? n : m) * lda);
	const std::unique_ptr<ET[]> work(new ET[lwork]);

	if (SO) {
		geqlf(m, n, (*A).data(), lda, tau, work.get(), lwork, &info);
	} else {
		gerqf(m, n, (*A).data(), lda, tau, work.get(), lwork, &info);
	}

	METRIC_NUMERIC_INTERNAL_ASSERT(info == 0, "Invalid argument for QL decomposition");
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
