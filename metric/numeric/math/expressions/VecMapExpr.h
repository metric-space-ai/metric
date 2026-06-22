// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_VECMAPEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_VECMAPEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/expressions/UnaryMapExpr.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Base class for all unary vector map expression templates.
// \ingroup math
//
// The VecMapExpr class serves as a tag for all expression templates that represent a unary map
// operation on a vector. All classes, that represent a unary vector map operation and that are
// used within the expression template environment of the Metric numeric library have to derive publicly
// from this class in order to qualify as unary vector map expression template. Only in case a
// class is derived publicly from the VecMapExpr base class, the IsVecMapExpr type trait
// recognizes the class as valid unary vector map expression template.
*/
template <typename VT> // Vector base type of the expression
struct VecMapExpr : public UnaryMapExpr<VT> {};
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
