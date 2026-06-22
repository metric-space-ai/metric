// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_SMATNOSIMDEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_SMATNOSIMDEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/expressions/SparseMatrix.h>
#include <metric/numeric/util/FunctionTrace.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  GLOBAL FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Disables the SIMD evaluation of the given sparse matrix expression \a sm.
// \ingroup sparse_matrix
//
// \param sm The input matrix.
// \return The SIMD-disabled sparse matrix.
//
// The \a nosimd function disables the SIMD evaluation of the given sparse matrix expression
// \a sm. The function returns an expression representing the operation.\n
// The following example demonstrates the use of the \a nosimd function:

   \code
   mtrc::numeric::CompressedMatrix<double> A, B;
   // ... Resizing and initialization
   B = nosimd( A );
   \endcode
*/
template <typename MT // Type of the sparse matrix
		  ,
		  bool SO> // Storage order
inline decltype(auto) nosimd(const SparseMatrix<MT, SO> &sm)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return *sm;
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
