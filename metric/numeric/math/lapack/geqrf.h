// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_LAPACK_GEQRF_H
#define METRIC_NUMERIC_MATH_LAPACK_GEQRF_H
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
#include <metric/numeric/math/lapack/clapack/gelqf.h>
#include <metric/numeric/math/lapack/clapack/geqrf.h>
#include <metric/numeric/util/Assert.h>
#include <metric/numeric/util/NumericCast.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  LAPACK QR DECOMPOSITION FUNCTIONS (GEQRF)
//
//=================================================================================================

//*************************************************************************************************
/*!\name LAPACK QR decomposition functions (geqrf) */
//@{
template <typename MT, bool SO> void geqrf(DenseMatrix<MT, SO> &A, ElementType_t<MT> *tau);
//@}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief LAPACK kernel for the QR decomposition of the given dense matrix.
// \ingroup lapack_decomposition
//
// \param A The matrix to be decomposed.
// \param tau Array for the scalar factors of the elementary reflectors; size >= min( \a m, \a n ).
// \return void
//
// This function performs the dense matrix QR decomposition of a general \a m-by-\a n matrix
// based on the LAPACK geqrf() functions. Note that this function can only be used for general,
// non-adapted matrices with \c float, \c double, \c complex<float>, or \c complex<double> element
// type. The attempt to call the function with any adapted matrix or matrices of any other element
// type results in a compile time error!\n
//
// In case of a column-major matrix, the resulting decomposition has the form

							  \f[ A = Q \cdot R, \f]

// where the \c Q is represented as a product of elementary reflectors

			   \f[ Q = H(1) H(2) . . . H(k) \texttt{, with k = min(m,n).} \f]

// Each H(i) has the form

					  \f[ H(i) = I - tau \cdot v \cdot v^T, \f]

// where \c tau is a real scalar, and \c v is a real vector with <tt>v(0:i-1) = 0</tt> and
// <tt>v(i) = 1</tt>. <tt>v(i+1:m)</tt> is stored on exit in <tt>A(i+1:m,i)</tt>, and \c tau
// in \c tau(i). Thus on exit the elements on and above the diagonal of the matrix contain the
// min(\a m,\a n)-by-\a n upper trapezoidal matrix \c R (\c R is upper triangular if \a m >= \a n);
// the elements below the diagonal, with the array \c tau, represent the orthogonal matrix \c Q
// as a product of min(\a m,\a n) elementary reflectors.
//
// For more information on the geqrf() functions (i.e. sgeqrf(), dgeqrf(), cgeqrf(), and zgeqrf())
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
inline void geqrf(DenseMatrix<MT, SO> &A, ElementType_t<MT> *tau)
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
		geqrf(m, n, (*A).data(), lda, tau, work.get(), lwork, &info);
	} else {
		gelqf(m, n, (*A).data(), lda, tau, work.get(), lwork, &info);
	}

	METRIC_NUMERIC_INTERNAL_ASSERT(info == 0, "Invalid argument for QR decomposition");
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
