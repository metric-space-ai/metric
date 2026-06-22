// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_DECLUPPEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_DECLUPPEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/expressions/DeclExpr.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Base class for all declupp expression templates.
// \ingroup math
//
// The DeclUppExpr class serves as a tag for all expression templates that represent an explicit
// upper declaration (declupp) operation. All classes, that represent a declupp operation and
// that are used within the expression template environment of the Metric numeric library have to derive
// publicly from this class in order to qualify as declupp expression template. Only in case
// class is derived publicly from the DeclUppExpr base class, the IsDeclUppExpr type trait a
// recognizes the class as valid declupp expression template.
*/
template <typename MT> // Matrix base type of the expression
struct DeclUppExpr : public DeclExpr<MT> {};
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
