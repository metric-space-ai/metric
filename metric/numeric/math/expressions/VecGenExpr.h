// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_VECGENEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_VECGENEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/expressions/GenExpr.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Base class for all vector generator expression templates.
// \ingroup math
//
// The VecGenExpr class serves as a tag for all expression templates that represent a vector
// generator operation. All classes, that represent a vector generator operation and that are
// used within the expression template environment of the Metric numeric library have to derive publicly
// from this class in order to qualify as vector generator expression template. Only in case a
// class is derived publicly from the VecGenExpr base class, the IsVecGenExpr type trait
// recognizes the class as valid vector generator expression template.
*/
template <typename VT> // Vector base type of the expression
struct VecGenExpr : public GenExpr<VT> {};
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
