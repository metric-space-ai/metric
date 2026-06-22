// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_VECNOALIASEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_VECNOALIASEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/expressions/NoAliasExpr.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Base class for all vector no-alias expression templates.
// \ingroup math
//
// The VecNoAliasExpr class serves as a tag for all expression templates that implement a vector
// no-alias operation. All classes, that represent a vector no-alias operation and that are used
// within the expression template environment of the Metric numeric library have to derive publicly from
// this class in order to qualify as vector no-alias expression template. Only in case a class is
// derived publicly from the VecNoAliasExpr base class, the IsVecNoAliasExpr type trait recognizes
// the class as valid vector no-alias expression template.
*/
template <typename VT> // Vector base type of the expression
struct VecNoAliasExpr : public NoAliasExpr<VT> {};
//*************************************************************************************************

//=================================================================================================
//
//  GLOBAL RESTRUCTURING FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Non-aliased evaluation of the given vector no-alias expression.
// \ingroup math
//
// \param vector The input no-alias expression.
// \return The non-aliased vector.
//
// This function implements a performance optimized treatment of the non-aliased evaluation of
// a vector no-alias expression.
*/
template <typename VT> // Vector base type of the expression
inline decltype(auto) noalias(const VecNoAliasExpr<VT> &vector)
{
	return *vector;
}
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
