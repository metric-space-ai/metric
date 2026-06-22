// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_DENSE_QR_H
#define METRIC_NUMERIC_MATH_DENSE_QR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <memory>
#include <metric/numeric/math/Aliases.h>
#include <metric/numeric/math/Exception.h>
#include <metric/numeric/math/constraints/Adaptor.h>
#include <metric/numeric/math/constraints/BLASCompatible.h>
#include <metric/numeric/math/constraints/Hermitian.h>
#include <metric/numeric/math/constraints/Lower.h>
#include <metric/numeric/math/constraints/StrictlyTriangular.h>
#include <metric/numeric/math/constraints/Symmetric.h>
#include <metric/numeric/math/constraints/UniTriangular.h>
#include <metric/numeric/math/expressions/DenseMatrix.h>
#include <metric/numeric/math/lapack/geqrf.h>
#include <metric/numeric/math/lapack/orgqr.h>
#include <metric/numeric/math/lapack/ungqr.h>
#include <metric/numeric/math/typetraits/IsResizable.h>
#include <metric/numeric/math/typetraits/IsSquare.h>
#include <metric/numeric/math/views/Submatrix.h>
#include <metric/numeric/util/EnableIf.h>
#include <metric/numeric/util/algorithms/Min.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  QR DECOMPOSITION FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\name QR decomposition functions */
//@{
template <typename MT1, bool SO1, typename MT2, bool SO2, typename MT3, bool SO3>
void qr(const DenseMatrix<MT1, SO1> &A, DenseMatrix<MT2, SO2> &Q, DenseMatrix<MT3, SO3> &R);
//@}
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Auxiliary function for the QR decomposition.
// \ingroup dense_matrix
//
// \param A The QR decomposed column-major matrix.
// \param tau Array for the scalar factors of the elementary reflectors.
// \return void
//
// This function is an auxiliary helper for the dense matrix QR decomposition. It performs the
// reconstruction of the \c Q matrix from the RQ decomposition.
*/
template <typename MT1> // Type of matrix A
inline auto qr_backend(MT1 &A, const ElementType_t<MT1> *tau) -> EnableIf_t<IsBuiltin_v<ElementType_t<MT1>>>
{
	orgqr(A, tau);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Auxiliary function for the QR decomposition.
// \ingroup dense_matrix
//
// \param A The QR decomposed column-major matrix.
// \param tau Array for the scalar factors of the elementary reflectors.
// \return void
//
// This function is an auxiliary helper for the dense matrix QR decomposition. It performs the
// reconstruction of the \c Q matrix from the RQ decomposition.
*/
template <typename MT1> // Type of matrix A
inline auto qr_backend(MT1 &A, const ElementType_t<MT1> *tau) -> EnableIf_t<IsComplex_v<ElementType_t<MT1>>>
{
	ungqr(A, tau);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief QR decomposition of the given dense matrix.
// \ingroup dense_matrix
//
// \param A The matrix to be decomposed.
// \param Q The resulting \c Q matrix.
// \param R The resulting \c R matrix.
// \return void
// \exception std::invalid_argument Dimensions of fixed size matrix do not match.
// \exception std::invalid_argument Square matrix cannot be resized to m-by-n.
//
// This function performs the dense matrix QR decomposition of a general \a m-by-\a n matrix.
// The resulting decomposition has the form

							  \f[ A = Q \cdot R, \f]

// where \c Q is a general \a m-by-min(\a m,\a n) matrix and \c R is an upper trapezoidal
// min(\a m,\a n)-by-\a n matrix. The decomposition is written to the two distinct matrices
// \c Q and \c R, which are resized to the correct dimensions (if possible and necessary).
//
// The function fails if ...
//
//  - ... either \a Q or \a R are fixed size matrices and the dimensions don't match;
//  - ... \a R is a compile time square matrix, but is required to be non-square.
//
// In all failure cases a \a std::invalid_argument exception is thrown.
//
// Example:

   \code
   mtrc::numeric::DynamicMatrix<double,mtrc::numeric::columnMajor> A( 32, 16 );
   // ... Initialization of A

   mtrc::numeric::DynamicMatrix<double,mtrc::numeric::columnMajor> Q( 32, 16 );
   mtrc::numeric::DynamicMatrix<double,mtrc::numeric::columnMajor> R( 16, 16 );

   qr( A, Q, R );

   assert( A == Q * R );
   \endcode

// \note This function only works for matrices with \c float, \c double, \c complex<float>, or
// \c complex<double> element type. The attempt to call the function with matrices of any other
// element type results in a compile time error!
//
// \note This function can only be used if a fitting LAPACK library is available and linked to
// the executable. Otherwise a call to this function will result in a linker error.
*/
template <typename MT1 // Type of matrix A
		  ,
		  bool SO1 // Storage order of matrix A
		  ,
		  typename MT2 // Type of matrix Q
		  ,
		  bool SO2 // Storage order of matrix Q
		  ,
		  typename MT3 // Type of matrix R
		  ,
		  bool SO3> // Storage order of matrix R
void qr(const DenseMatrix<MT1, SO1> &A, DenseMatrix<MT2, SO2> &Q, DenseMatrix<MT3, SO3> &R)
{
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_STRICTLY_TRIANGULAR_MATRIX_TYPE(MT1);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_BLAS_COMPATIBLE_TYPE(ElementType_t<MT1>);

	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_ADAPTOR_TYPE(MT2);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_BLAS_COMPATIBLE_TYPE(ElementType_t<MT2>);

	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_SYMMETRIC_MATRIX_TYPE(MT3);
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_HERMITIAN_MATRIX_TYPE(MT3);
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_UNITRIANGULAR_MATRIX_TYPE(MT3);
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_LOWER_MATRIX_TYPE(MT3);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_BLAS_COMPATIBLE_TYPE(ElementType_t<MT3>);

	using ET1 = ElementType_t<MT1>;

	const size_t m((*A).rows());
	const size_t n((*A).columns());
	const size_t mindim(min(m, n));

	if ((!IsResizable_v<MT2> && ((*Q).rows() != m || (*Q).columns() != mindim)) ||
		(!IsResizable_v<MT3> && ((*R).rows() != mindim || (*R).columns() != n))) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Dimensions of fixed size matrix do not match");
	}

	if (IsSquare_v<MT3> && mindim != n) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Square matrix cannot be resized to min(m,n)-by-n");
	}

	const std::unique_ptr<ET1[]> tau(new ET1[mindim]);
	decltype(auto) r(derestrict(*R));

	if (m < n) {
		r = A;
		geqrf(r, tau.get());
		(*Q) = submatrix(r, 0UL, 0UL, m, m);
		qr_backend(*Q, tau.get());

		for (size_t i = 1UL; i < m; ++i) {
			for (size_t j = 0UL; j < i; ++j) {
				reset(r(i, j));
			}
		}
	} else {
		(*Q) = A;
		geqrf(*Q, tau.get());

		resize(*R, n, n, false);
		reset(r);

		for (size_t i = 0UL; i < n; ++i) {
			for (size_t j = i; j < n; ++j) {
				r(i, j) = (*Q)(i, j);
			}
		}

		qr_backend(*Q, tau.get());
	}
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
