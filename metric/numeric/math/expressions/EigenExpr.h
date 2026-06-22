// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_EIGENEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_EIGENEXPR_H
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
/*!\brief Base class for all eigenvalue expression templates.
// \ingroup math
//
// The EigenExpr class serves as a tag for all expression templates that implement an eigenvalue
// computation. All classes, that represent an eigenvalue computation and that are used within the
// expression template environment of the Metric numeric library have to derive publicly from this class
// in order to qualify as eigenvalue expression template. Only in case a class is derived publicly
// from the EigenExpr base class, the IsEigenExpr type trait recognizes the class as valid
// eigenvalue expression template.
*/
template <typename MT> // Matrix base type of the expression
struct EigenExpr : public Expression<MT> {};
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
