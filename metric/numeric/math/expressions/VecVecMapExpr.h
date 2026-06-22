// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_VECVECMAPEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_VECVECMAPEXPR_H
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
/*!\brief Base class for all binary vector map expression templates.
// \ingroup math
//
// The VecVecMapExpr class serves as a tag for all expression templates that implement a binary
// vector map operation. All classes, that represent a binary vector map operation and that are
// used within the expression template environment of the Metric numeric library have to derive publicly
// from this class in order to qualify as binary vector map expression template. Only in case a
// class is derived publicly from the VecVecMapExpr base class, the IsVecVecMapExpr type trait
// recognizes the class as valid binary vector map expression template.
*/
template <typename VT> // Vector base type of the expression
struct VecVecMapExpr : public BinaryMapExpr<VT> {};
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
