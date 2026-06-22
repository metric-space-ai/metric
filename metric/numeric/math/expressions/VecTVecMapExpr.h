// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_VECTVECMAPEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_VECTVECMAPEXPR_H
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
/*!\brief Base class for all outer map expression templates.
// \ingroup math
//
// The VecTVecMapExpr class serves as a tag for all expression templates that implement outer map
// operations (i.e. custom operations between a column vector and a row vector). All classes, that
// represent an outer map operation and that are used within the expression template environment
// of the Metric numeric library have to derive publicly from this class in order to qualify as outer map
// expression template. Only in case a class is derived publicly from the VecVecMapExpr base class,
// the IsVecVecMapExpr type trait recognizes the class as valid outer map expression template.
*/
template <typename MT> // Matrix base type of the expression
struct VecTVecMapExpr : public BinaryMapExpr<MT> {};
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
