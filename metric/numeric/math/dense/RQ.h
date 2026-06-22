// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_DENSE_RQ_H
#define METRIC_NUMERIC_MATH_DENSE_RQ_H
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
#include <metric/numeric/math/lapack/gerqf.h>
#include <metric/numeric/math/lapack/orgrq.h>
#include <metric/numeric/math/lapack/ungrq.h>
#include <metric/numeric/math/typetraits/IsResizable.h>
#include <metric/numeric/math/typetraits/IsSquare.h>
#include <metric/numeric/math/views/Submatrix.h>
#include <metric/numeric/util/EnableIf.h>
#include <metric/numeric/util/algorithms/Min.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  RQ DECOMPOSITION FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\name RQ decomposition functions */
//@{
template <typename MT1, bool SO1, typename MT2, bool SO2, typename MT3, bool SO3>
void rq(const DenseMatrix<MT1, SO1> &A, DenseMatrix<MT2, SO2> &R, DenseMatrix<MT3, SO3> &Q);
//@}
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Auxiliary function for the RQ decomposition.
// \ingroup dense_matrix
//
// \param A The RQ decomposed column-major matrix.
// \param tau Array for the scalar factors of the elementary reflectors.
// \return void
//
// This function is an auxiliary helper for the dense matrix RQ decomposition. It performs the
// reconstruction of the \c Q matrix from the RQ decomposition.
*/
template <typename MT1> // Type of matrix A
inline auto rq_backend(MT1 &A, const ElementType_t<MT1> *tau) -> EnableIf_t<IsBuiltin_v<ElementType_t<MT1>>>
{
	orgrq(A, tau);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Auxiliary function for the RQ decomposition.
// \ingroup dense_matrix
//
// \param A The RQ decomposed column-major matrix.
// \param tau Array for the scalar factors of the elementary reflectors.
// \return void
//
// This function is an auxiliary helper for the dense matrix RQ decomposition. It performs the
// reconstruction of the \c Q matrix from the RQ decomposition.
*/
template <typename MT1> // Type of matrix A
inline auto rq_backend(MT1 &A, const ElementType_t<MT1> *tau) -> EnableIf_t<IsComplex_v<ElementType_t<MT1>>>
{
	ungrq(A, tau);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief RQ decomposition of the given dense matrix.
// \ingroup dense_matrix
//
// \param A The matrix to be decomposed.
// \param R The resulting \c R matrix.
// \param Q The resulting \c Q matrix.
// \return void
// \exception std::invalid_argument Dimensions of fixed size matrix do not match.
// \exception std::invalid_argument Square matrix cannot be resized to \a m-by-\a n.
//
// This function performs the dense matrix RQ decomposition of a general \a m-by-\a n matrix.
// The resulting decomposition has the form

							  \f[ A = R \cdot Q, \f]

// where \c R is an upper trapezoidal \a m-by-min(\a m,\a n) matrix and \c Q is a general
// min(\a m,\a n)-by-\a n matrix. The decomposition is written to the two distinct matrices
// \c R and \c Q, which are resized to the correct dimensions (if possible and necessary).
//
// The function fails if ...
//
//  - ... either \a R or \a Q are fixed size matrices and the dimensions don't match;
//  - ... \a R is a compile time square matrix, but is required to be non-square.
//
// In all failure cases a \a std::invalid_argument exception is thrown.
//
// Example:

   \code
   mtrc::numeric::DynamicMatrix<double,mtrc::numeric::columnMajor> A( 32, 16 );
   // ... Initialization of A

   mtrc::numeric::DynamicMatrix<double,mtrc::numeric::columnMajor> R( 32, 16 );
   mtrc::numeric::DynamicMatrix<double,mtrc::numeric::columnMajor> Q( 16, 16 );


   rq( A, R, Q );

   assert( A == R * Q );
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
		  typename MT2 // Type of matrix R
		  ,
		  bool SO2 // Storage order of matrix R
		  ,
		  typename MT3 // Type of matrix Q
		  ,
		  bool SO3> // Storage order of matrix Q
void rq(const DenseMatrix<MT1, SO1> &A, DenseMatrix<MT2, SO2> &R, DenseMatrix<MT3, SO3> &Q)
{
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_STRICTLY_TRIANGULAR_MATRIX_TYPE(MT1);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_BLAS_COMPATIBLE_TYPE(ElementType_t<MT1>);

	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_SYMMETRIC_MATRIX_TYPE(MT2);
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_HERMITIAN_MATRIX_TYPE(MT2);
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_UNITRIANGULAR_MATRIX_TYPE(MT2);
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_STRICTLY_TRIANGULAR_MATRIX_TYPE(MT2);
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_LOWER_MATRIX_TYPE(MT2);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_BLAS_COMPATIBLE_TYPE(ElementType_t<MT2>);

	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_ADAPTOR_TYPE(MT3);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_BLAS_COMPATIBLE_TYPE(ElementType_t<MT3>);

	using ET1 = ElementType_t<MT1>;

	const size_t m((*A).rows());
	const size_t n((*A).columns());
	const size_t mindim(min(m, n));

	if ((!IsResizable_v<MT2> && ((*R).rows() != m || (*R).columns() != mindim)) ||
		(!IsResizable_v<MT3> && ((*Q).rows() != mindim || (*Q).columns() != n))) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Dimensions of fixed size matrix do not match");
	}

	if (IsSquare_v<MT2> && m != mindim) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Square matrix cannot be resized to m-by-min(m,n)");
	}

	const std::unique_ptr<ET1[]> tau(new ET1[mindim]);
	decltype(auto) r(derestrict(*R));

	if (m < n) {
		(*Q) = A;
		gerqf(*Q, tau.get());

		resize(*R, m, m, false);
		reset(r);

		for (size_t i = 0UL; i < m; ++i) {
			for (size_t j = i; j < m; ++j) {
				r(i, j) = (*Q)(i, j + n - m);
			}
		}

		rq_backend(*Q, tau.get());
	} else {
		r = A;
		gerqf(r, tau.get());
		(*Q) = submatrix(r, m - n, 0UL, n, n);
		rq_backend(*Q, tau.get());

		for (size_t i = m - n; i < m; ++i) {
			for (size_t j = 0UL; j < i + n - m; ++j) {
				reset(r(i, j));
			}
		}
	}
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
