// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_MATREPEATEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_MATREPEATEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/expressions/RepeatExpr.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Base class for all matrix repeat expression templates.
// \ingroup math
//
// The MatRepeatExpr class serves as a tag for all expression templates that represent a matrix
// repeat operation. All classes, that represent a matrix repeat operation and that are used
// within the expression template environment of the Metric numeric library have to derive publicly from
// this class in order to qualify as matrix repeater expression template. Only in case a class is
// derived publicly from the MatRepeatExpr base class, the IsMatRepeatExpr type trait recognizes
// the class as valid matrix repeater expression template.
*/
template <typename MT // Matrix base type of the expression
		  ,
		  size_t... CEAs> // Compile time repeater arguments
struct MatRepeatExpr : public RepeatExpr<MT> {};
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
