// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_LAPACK_ORGQR_H
#define METRIC_NUMERIC_MATH_LAPACK_ORGQR_H
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
#include <metric/numeric/math/lapack/clapack/orglq.h>
#include <metric/numeric/math/lapack/clapack/orgqr.h>
#include <metric/numeric/util/Assert.h>
#include <metric/numeric/util/NumericCast.h>
#include <metric/numeric/util/constraints/Builtin.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  LAPACK FUNCTIONS TO RECONSTRUCT Q FROM A QR DECOMPOSITION (ORGQR)
//
//=================================================================================================

//*************************************************************************************************
/*!\name LAPACK functions to reconstruct Q from a QR decomposition (orgqr) */
//@{
template <typename MT, bool SO> void orgqr(DenseMatrix<MT, SO> &A, const ElementType_t<MT> *tau);
//@}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief LAPACK kernel for the reconstruction of the orthogonal matrix Q from a QR decomposition.
// \ingroup lapack_decomposition
//
// \param A The decomposed matrix.
// \param tau Array for the scalar factors of the elementary reflectors; size >= min( \a m, \a n ).
// \return void
//
// This function reconstructs the orthogonal matrix \a Q of a QR decomposition based on the LAPACK
// orgqr() functions from matrices that have already been QR factorized by the geqrf() functions.
// Note that this function can only be used for general, non-adapted matrices with \c float or
// \c double element type. The attempt to call the function with any adapted matrix or matrices
// of any other element type results in a compile time error!\n
//
// The \a m-by-min(\a m,\a n) \a Q matrix is stored within the given matrix \a A:

   \code
   using mtrc::numeric::DynamicMatrix;
   using mtrc::numeric::columnMajor;

   DynamicMatrix<double,columnMajor> A;
   DynamicVector<double> tau;
   // ... Resizing and initialization

   geqrf( A, tau.data() );  // Performing the QR decomposition
   orgqr( A, tau.data() );  // Reconstructing the Q matrix

   const size_t m( A.rows() );
   const size_t n( A.columns() );

   DynamicMatrix<double,columnMajor> Q( submatrix( A, 0UL, 0UL, m, min(m,n) ) );
   \endcode

// For more information on the orgqr() functions (i.e. sorgqr() and dorgqr()) see the LAPACK
// online documentation browser:
//
//        http://www.netlib.org/lapack/explore-html/
//
// \note This function can only be used if a fitting LAPACK library, which supports this function,
// is available and linked to the executable. Otherwise a call to this function will result in a
// linker error.
*/
template <typename MT, bool SO> inline void orgqr(DenseMatrix<MT, SO> &A, const ElementType_t<MT> *tau)
{
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_ADAPTOR_TYPE(MT);
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_COMPUTATION_TYPE(MT);
	METRIC_NUMERIC_CONSTRAINT_MUST_HAVE_MUTABLE_DATA_ACCESS(MT);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_CONTIGUOUS_TYPE(MT);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_BLAS_COMPATIBLE_TYPE(ElementType_t<MT>);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_BUILTIN_TYPE(ElementType_t<MT>);

	using ET = ElementType_t<MT>;

	blas_int_t m(numeric_cast<blas_int_t>(SO ? (*A).rows() : (*A).columns()));
	blas_int_t n(numeric_cast<blas_int_t>(SO ? (*A).columns() : (*A).rows()));
	blas_int_t k(min(m, n));
	blas_int_t lda(numeric_cast<blas_int_t>((*A).spacing()));
	blas_int_t info(0);

	if (k == 0) {
		return;
	}

	blas_int_t lwork(k * lda);
	const std::unique_ptr<ET[]> work(new ET[lwork]);

	if (SO) {
		orgqr(m, k, k, (*A).data(), lda, tau, work.get(), lwork, &info);
	} else {
		orglq(k, n, k, (*A).data(), lda, tau, work.get(), lwork, &info);
	}

	METRIC_NUMERIC_INTERNAL_ASSERT(info == 0, "Invalid argument for Q reconstruction");
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
