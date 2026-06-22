// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_DENSE_QL_H
#define METRIC_NUMERIC_MATH_DENSE_QL_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <memory>
#include <metric/numeric/math/Aliases.h>
#include <metric/numeric/math/Exception.h>
#include <metric/numeric/math/constraints/Adaptor.h>
#include <metric/numeric/math/constraints/BLASCompatible.h>
#include <metric/numeric/math/constraints/Hermitian.h>
#include <metric/numeric/math/constraints/StrictlyTriangular.h>
#include <metric/numeric/math/constraints/Symmetric.h>
#include <metric/numeric/math/constraints/UniTriangular.h>
#include <metric/numeric/math/constraints/Upper.h>
#include <metric/numeric/math/expressions/DenseMatrix.h>
#include <metric/numeric/math/lapack/geqlf.h>
#include <metric/numeric/math/lapack/orgql.h>
#include <metric/numeric/math/lapack/ungql.h>
#include <metric/numeric/math/typetraits/IsResizable.h>
#include <metric/numeric/math/typetraits/IsSquare.h>
#include <metric/numeric/math/views/Submatrix.h>
#include <metric/numeric/util/EnableIf.h>
#include <metric/numeric/util/algorithms/Min.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  QL DECOMPOSITION FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\name QL decomposition functions */
//@{
template <typename MT1, bool SO1, typename MT2, bool SO2, typename MT3, bool SO3>
void ql(const DenseMatrix<MT1, SO1> &A, DenseMatrix<MT2, SO2> &Q, DenseMatrix<MT3, SO3> &L);
//@}
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Auxiliary function for the QL decomposition.
// \ingroup dense_matrix
//
// \param A The QL decomposed column-major matrix.
// \param tau Array for the scalar factors of the elementary reflectors.
// \return void
//
// This function is an auxiliary helper for the dense matrix QL decomposition. It performs the
// reconstruction of the \c Q matrix from the QL decomposition.
*/
template <typename MT1> // Type of matrix A
inline auto ql_backend(MT1 &A, const ElementType_t<MT1> *tau) -> EnableIf_t<IsBuiltin_v<ElementType_t<MT1>>>
{
	orgql(A, tau);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Auxiliary function for the QL decomposition.
// \ingroup dense_matrix
//
// \param A The QL decomposed column-major matrix.
// \param tau Array for the scalar factors of the elementary reflectors.
// \return void
//
// This function is an auxiliary helper for the dense matrix QL decomposition. It performs the
// reconstruction of the \c Q matrix from the QL decomposition.
*/
template <typename MT1> // Type of matrix A
inline auto ql_backend(MT1 &A, const ElementType_t<MT1> *tau) -> EnableIf_t<IsComplex_v<ElementType_t<MT1>>>
{
	ungql(A, tau);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief QL decomposition of the given dense matrix.
// \ingroup dense_matrix
//
// \param A The matrix to be decomposed.
// \param Q The resulting \c Q matrix.
// \param L The resulting \c L matrix.
// \return void
// \exception std::invalid_argument Dimensions of fixed size matrix do not match.
// \exception std::invalid_argument Square matrix cannot be resized to \a m-by-\a n.
//
// This function performs the dense matrix QL decomposition of a general \a m-by-\a n matrix.
// The resulting decomposition has the form

							  \f[ A = Q \cdot L, \f]

// where \c Q is a general \a m-by-min(\a m,\a n) matrix and \c L is a lower trapezoidal
// \a min(\a m,\a n)-by-\a n matrix. The decomposition is written to the two distinct matrices
// \c Q and \c L, which are resized to the correct dimensions (if possible and necessary).
//
// The function fails if ...
//
//  - ... either \a Q or \a L are fixed size matrices and the dimensions don't match;
//  - ... \a L is a compile time square matrix, but is required to be non-square.
//
// In all failure cases a \a std::invalid_argument exception is thrown.
//
// Example:

   \code
   mtrc::numeric::DynamicMatrix<double,mtrc::numeric::columnMajor> A( 32, 16 );
   // ... Initialization of A

   mtrc::numeric::DynamicMatrix<double,mtrc::numeric::columnMajor> Q( 32, 16 );
   mtrc::numeric::DynamicMatrix<double,mtrc::numeric::columnMajor> L( 16, 16 );

   ql( A, Q, L );

   assert( A == Q * L );
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
		  typename MT3 // Type of matrix L
		  ,
		  bool SO3> // Storage order of matrix L
void ql(const DenseMatrix<MT1, SO1> &A, DenseMatrix<MT2, SO2> &Q, DenseMatrix<MT3, SO3> &L)
{
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_STRICTLY_TRIANGULAR_MATRIX_TYPE(MT1);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_BLAS_COMPATIBLE_TYPE(ElementType_t<MT1>);

	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_ADAPTOR_TYPE(MT2);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_BLAS_COMPATIBLE_TYPE(ElementType_t<MT2>);

	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_SYMMETRIC_MATRIX_TYPE(MT3);
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_HERMITIAN_MATRIX_TYPE(MT3);
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_UNITRIANGULAR_MATRIX_TYPE(MT3);
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_STRICTLY_TRIANGULAR_MATRIX_TYPE(MT3);
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_UPPER_MATRIX_TYPE(MT3);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_BLAS_COMPATIBLE_TYPE(ElementType_t<MT3>);

	using ET1 = ElementType_t<MT1>;

	const size_t m((*A).rows());
	const size_t n((*A).columns());
	const size_t mindim(min(m, n));

	if ((!IsResizable_v<MT2> && ((*Q).rows() != m || (*Q).columns() != mindim)) ||
		(!IsResizable_v<MT3> && ((*L).rows() != mindim || (*L).columns() != n))) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Dimensions of fixed size matrix do not match");
	}

	if (IsSquare_v<MT3> && mindim != n) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Square matrix cannot be resized to min(m,n)-by-n");
	}

	const std::unique_ptr<ET1[]> tau(new ET1[mindim]);
	decltype(auto) l(derestrict(*L));

	if (m < n) {
		l = A;
		geqlf(l, tau.get());
		(*Q) = submatrix(l, 0UL, n - m, m, m);
		ql_backend(*Q, tau.get());

		for (size_t i = 0UL; i < m; ++i) {
			for (size_t j = i + n - m + 1UL; j < n; ++j) {
				reset(l(i, j));
			}
		}
	} else {
		(*Q) = A;
		geqlf(*Q, tau.get());

		resize(*L, n, n, false);
		reset(l);

		for (size_t i = 0UL; i < n; ++i) {
			for (size_t j = 0UL; j < i + 1UL; ++j) {
				l(i, j) = (*Q)(i + m - n, j);
			}
		}

		ql_backend(*Q, tau.get());
	}
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
