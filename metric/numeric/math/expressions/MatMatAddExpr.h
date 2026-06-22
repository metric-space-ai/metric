// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_MATMATADDEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_MATMATADDEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/expressions/AddExpr.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Base class for all matrix/matrix addition expression templates.
// \ingroup math
//
// The MatMatAddExpr class serves as a tag for all expression templates that implement a
// matrix/matrix addition. All classes, that represent a matrix addition and that are used
// within the expression template environment of the Metric numeric library have to derive publicly
// from this class in order to qualify as matrix addition expression template. Only in case
// a class is derived publicly from the MatMatAddExpr base class, the IsMatMatAddExpr type
// trait recognizes the class as valid matrix addition expression template.
*/
template <typename MT> // Matrix base type of the expression
struct MatMatAddExpr : public AddExpr<MT> {};
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
