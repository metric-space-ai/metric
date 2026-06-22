// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_MATNOALIASEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_MATNOALIASEXPR_H
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
/*!\brief Base class for all matrix no-alias expression templates.
// \ingroup math
//
// The MatNoAliasExpr class serves as a tag for all expression templates that implement a matrix
// no-alias operation. All classes, that represent a matrix no-alias operation and that are used
// within the expression template environment of the Metric numeric library have to derive publicly from
// this class in order to qualify as matrix no-alias expression template. Only in case a class is
// derived publicly from the MatNoAliasExpr base class, the IsMatNoAliasExpr type trait recognizes
// the class as valid matrix no-alias expression template.
*/
template <typename MT> // Matrix base type of the expression
struct MatNoAliasExpr : public NoAliasExpr<MT> {};
//*************************************************************************************************

//=================================================================================================
//
//  GLOBAL RESTRUCTURING FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Non-aliased evaluation of the given matrix no-alias expression.
// \ingroup math
//
// \param matrix The input no-alias expression.
// \return The non-aliased matrix.
//
// This function implements a performance optimized treatment of the non-aliased evaluation of
// a matrix no-alias expression.
*/
template <typename MT> // Matrix base type of the expression
inline decltype(auto) noalias(const MatNoAliasExpr<MT> &matrix)
{
	return *matrix;
}
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
