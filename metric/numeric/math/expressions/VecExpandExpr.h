// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_VECEXPANDEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_VECEXPANDEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/expressions/ExpandExpr.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Base class for all vector expansion expression templates.
// \ingroup math
//
// The VecExpandExpr class serves as a tag for all expression templates that implement a vector
// expansion. All classes, that represent a vector expansion and and that are used within the
// expression template environment of the Metric numeric library have to derive publicly from this class
// in order to qualify as vector expansion expression template. Only in case a class is derived
// publicly from the VecExpandExpr base class, the IsVecExpandExpr type trait recognizes the
// class as valid vector expansion expression template.
*/
template <typename MT // Matrix base type of the expression
		  ,
		  size_t... CEAs> // Compile time expansion arguments
struct VecExpandExpr : public ExpandExpr<MT> {};
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
