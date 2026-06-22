// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_MATMATMAPEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_MATMATMAPEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/expressions/BinaryMapExpr.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Base class for all binary matrix map expression templates.
// \ingroup math
//
// The MatMatMapExpr class serves as a tag for all expression templates that implement a binary
// matrix map operation. All classes, that represent a binary matrix map operation and that are
// used within the expression template environment of the Metric numeric library have to derive publicly
// from this class in order to qualify as binary matrix map expression template. Only in case a
// class is derived publicly from the MatMatMapExpr base class, the IsMatMatMapExpr type trait
// recognizes the class as valid binary matrix map expression template.
*/
template <typename MT> // Matrix base type of the expression
struct MatMatMapExpr : public BinaryMapExpr<MT> {};
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
