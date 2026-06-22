// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_BINARYMAPEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_BINARYMAPEXPR_H
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
/*!\brief Base class for all for-each expression templates.
// \ingroup math
//
// The BinaryMapExpr class serves as a tag for all expression templates that represent a binary
// map operation. All classes, that represent a binary map operation and that are used within
// the expression template environment of the Metric numeric library have to derive publicly from this
// class in order to qualify as binary map expression template. Only in case a class is derived
// publicly from the BinaryMapExpr base class, the IsBinaryMapExpr type trait recognizes the
// class as valid binary map expression template.
*/
template <typename T> // Base type of the expression
struct BinaryMapExpr : public Expression<T> {};
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
