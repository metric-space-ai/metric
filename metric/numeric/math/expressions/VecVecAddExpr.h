// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_VECVECADDEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_VECVECADDEXPR_H
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
/*!\brief Base class for all vector/vector addition expression templates.
// \ingroup math
//
// The VecVecAddExpr class serves as a tag for all expression templates that implement a
// vector/vector addition. All classes, that represent a vector addition and that are used
// within the expression template environment of the Metric numeric library have to derive publicly
// from this class in order to qualify as vector addition expression template. Only in case
// a class is derived publicly from the VecVecAddExpr base class, the IsVecVecAddExpr type
// trait recognizes the class as valid vector addition expression template.
*/
template <typename VT> // Vector base type of the expression
struct VecVecAddExpr : public AddExpr<VT> {};
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
