// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_SVECNOSIMDEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_SVECNOSIMDEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/expressions/SparseVector.h>
#include <metric/numeric/util/FunctionTrace.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  GLOBAL FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Disables the SIMD evaluation of the given sparse vector expression \a sv.
// \ingroup sparse_vector
//
// \param sv The input vector.
// \return The SIMD-disabled sparse vector.
//
// The \a nosimd function disables the SIMD evaluation of the given sparse vector expression
// \a sv. The function returns an expression representing this operation.\n
// The following example demonstrates the use of the \a nosimd function:

   \code
   mtrc::numeric::CompressedVector<double> a, b;
   // ... Resizing and initialization
   b = nosimd( a );
   \endcode
*/
template <typename VT // Type of the dense vector
		  ,
		  bool TF> // Transpose flag
inline decltype(auto) nosimd(const SparseVector<VT, TF> &sv)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return *sv;
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
