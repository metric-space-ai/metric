// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_VECREPEATEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_VECREPEATEXPR_H
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
/*!\brief Base class for all vector repeater expression templates.
// \ingroup math
//
// The VecRepeatExpr class serves as a tag for all expression templates that represent a vector
// repeat operation. All classes, that represent a vector repeat operation and that are used
// within the expression template environment of the Metric numeric library have to derive publicly from
// this class in order to qualify as vector repeater expression template. Only in case a class is
// derived publicly from the VecRepeatExpr base class, the IsVecRepeatExpr type trait recognizes
// the class as valid vector repeater expression template.
*/
template <typename VT // Vector base type of the expression
		  ,
		  size_t... CRAs> // Compile time repeater arguments
struct VecRepeatExpr : public RepeatExpr<VT> {};
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
