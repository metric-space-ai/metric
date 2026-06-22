// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_MATREDUCEEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_MATREDUCEEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/ReductionFlag.h>
#include <metric/numeric/math/expressions/ReduceExpr.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Base class for all matrix reduction expression templates.
// \ingroup math
//
// The MatReduceExpr class serves as a tag for all expression templates that implement a matrix
// reduction. All classes, that represent a matrix reduction and and that are used within the
// expression template environment of the Metric numeric library have to derive publicly from this class
// in order to qualify as matrix reduction expression template. Only in case a class is derived
// publicly from the MatReduceExpr base class, the IsMatReduceExpr type trait recognizes the
// class as valid matrix reduction expression template.
*/
template <typename VT // Vector base type of the expression
		  ,
		  ReductionFlag RF> // Reduction flag
struct MatReduceExpr : public ReduceExpr<VT> {};
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
