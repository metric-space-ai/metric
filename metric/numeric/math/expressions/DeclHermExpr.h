// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_DECLHERMEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_DECLHERMEXPR_H
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
/*!\brief Base class for all declherm expression templates.
// \ingroup math
//
// The DeclHermExpr class serves as a tag for all expression templates that represent an
// explicit Hermitian declaration (declherm) operation. All classes, that represent a declherm
// operation and that are used within the expression template environment of the Metric numeric library
// have to derive publicly from this class in order to qualify as declherm expression template.
// Only in case a class is derived publicly from the DeclHermExpr base class, the IsDeclHermExpr
// type trait recognizes the class as valid declherm expression template.
*/
template <typename MT> // Matrix base type of the expression
struct DeclHermExpr : public DeclExpr<MT> {};
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
