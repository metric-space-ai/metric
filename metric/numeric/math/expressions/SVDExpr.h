// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_SVDEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_SVDEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/expressions/Expression.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Base class for all singular value expression templates.
// \ingroup math
//
// The SVDExpr class serves as a tag for all expression templates that implement a singular value
// computation. All classes, that represent a singular value computation and that are used within
// the expression template environment of the Metric numeric library have to derive publicly from this class
// in order to qualify as singular value expression template. Only in case a class is derived
// publicly from the SVDExpr base class, the IsSVDExpr type trait recognizes the class as valid
// singular value expression template.
*/
template <typename MT> // Matrix base type of the expression
struct SVDExpr : public Expression<MT> {};
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
