// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_DENSE_PLLHP_H
#define METRIC_NUMERIC_MATH_DENSE_PLLHP_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/Aliases.h>
#include <metric/numeric/math/Exception.h>
#include <metric/numeric/math/constraints/BLASCompatible.h>
#include <metric/numeric/math/constraints/Hermitian.h>
#include <metric/numeric/math/constraints/StrictlyTriangular.h>
#include <metric/numeric/math/constraints/Symmetric.h>
#include <metric/numeric/math/constraints/UniTriangular.h>
#include <metric/numeric/math/constraints/Upper.h>
#include <metric/numeric/math/expressions/DenseMatrix.h>
#include <metric/numeric/math/lapack/pstrf.h>
#include <metric/numeric/math/typetraits/IsResizable.h>
#include <metric/numeric/math/typetraits/IsRowMajorMatrix.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  PLLHP DECOMPOSITION FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\name PLLHP decomposition functions */
//@{
template <typename MT1, bool SO1, typename MT2, bool SO2, typename ST>
int pllhp(const DenseMatrix<MT1, SO1> &A, DenseMatrix<MT2, SO2> &L, blas_int_t *P, ST tol);
//@}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Pivoting Cholesky (PLLHP) decomposition of the given dense matrix.
// \ingroup dense_matrix
//
// \param A The matrix to be decomposed.
// \param L The resulting lower triangular matrix.
// \param P Vector with the pivoting indices.
// \param tol The user-defined tolerance.
// \return The rank of \c A given by the number of steps the algorithm completed.
// \exception std::invalid_argument Invalid non-square matrix provided.
// \exception std::invalid_argument Dimensions of fixed size matrix do not match.
// \exception std::invalid_argument Decomposition of singular matrix failed.
//
// This function performs the dense matrix pivoting Cholesky (PLLHP) decomposition of a positive
// semi-definite n-by-n matrix. The resulting decomposition has the form

							  \f[ A = P L \cdot L^{H} P^{T}, \f]

// where \c L is a lower triangular n-by-n matrix. The decomposition is written to the matrix
// \c L, which is resized to the correct dimensions (if possible and necessary).
//
// The function fails if ...
//
//  - ... \a A is not a square matrix;
//  - ... \a L is a fixed size matrix and the dimensions don't match \a A.
//
// In all failure cases a \a std::invalid_argument exception is thrown.
//
// Example:

   \code
   const size_t N = 32;

   mtrc::numeric::DynamicMatrix<double,mtrc::numeric::columnMajor> A( 32, 32 );
   // ... Initialization of A as positive semi-definite matrix

   mtrc::numeric::DynamicMatrix<double,mtrc::numeric::columnMajor> L( 32, 32 );
   std::vector<mtrc::numeric::blas_int_t> pivot(n), ipivot(n);

   const size_t rank = pllhp( A, L, pivot.data(), 1E-8 );
   for( size_t i=0; i<32; ++i ) {
	  ipivot[pivot[i]] = i;
   }

   const auto PLLHP( evaluate( L * ctrans( L ) ) );
   const auto LLHP ( evaluate( rows( PLLHP, [&]( size_t i ){ return ipivot[i]; }, 32 ) ) );
   const auto LLH  ( evaluate( columns( LLHP, [&]( size_t i ){ return ipivot[i]; }, 32 ) ) );
   assert( A == LLH );
   \endcode

// \note This function only works for matrices with \c float, \c double, \c complex<float>, or
// \c complex<double> element type. The attempt to call the function with matrices of any other
// element type results in a compile time error!
//
// \note This function can only be used if a fitting LAPACK library is available and linked to
// the executable. Otherwise a call to this function will result in a linker error.
//
// \note This function does only provide the basic exception safety guarantee, i.e. in case of an
// exception \a L may already have been modified.
*/
template <typename MT1 // Type of matrix A
		  ,
		  bool SO1 // Storage order of matrix A
		  ,
		  typename MT2 // Type of matrix L
		  ,
		  bool SO2 // Storage order of matrix L
		  ,
		  typename ST> // Type of the scalar tolerance value
blas_int_t pllhp(const DenseMatrix<MT1, SO1> &A, DenseMatrix<MT2, SO2> &L, blas_int_t *P, ST tol)
{
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_STRICTLY_TRIANGULAR_MATRIX_TYPE(MT1);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_BLAS_COMPATIBLE_TYPE(ElementType_t<MT1>);

	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_SYMMETRIC_MATRIX_TYPE(MT2);
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_HERMITIAN_MATRIX_TYPE(MT2);
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_UNITRIANGULAR_MATRIX_TYPE(MT2);
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_UPPER_MATRIX_TYPE(MT2);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_BLAS_COMPATIBLE_TYPE(ElementType_t<MT2>);

	if (!isSquare(*A)) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid non-square matrix provided");
	}

	const size_t n((*A).rows());

	if ((!IsResizable_v<MT2> && ((*L).rows() != n || (*L).columns() != n))) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Dimensions of fixed size matrix do not match");
	}

	decltype(auto) l(derestrict(*L));

	resize(*L, n, n, false);
	reset(l);

	if (IsRowMajorMatrix_v<MT2>) {
		for (size_t i = 0UL; i < n; ++i) {
			for (size_t j = 0UL; j <= i; ++j) {
				l(i, j) = (*A)(i, j);
			}
		}
	} else {
		for (size_t j = 0UL; j < n; ++j) {
			for (size_t i = j; i < n; ++i) {
				l(i, j) = (*A)(i, j);
			}
		}
	}

	return pstrf(l, 'L', P, tol);
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
