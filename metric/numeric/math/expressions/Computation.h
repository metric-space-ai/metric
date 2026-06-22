// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_COMPUTATION_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_COMPUTATION_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/expressions/Operation.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Base class for all compute expression templates.
// \ingroup math
//
// The Computation class serves as a tag for all computational expression templates. All
// classes, that represent a mathematical computation (addition, subtraction, multiplication,
// division, absolute value calculation, ...) and that are used within the expression template
// environment of the Metric numeric library have to derive from this class in order to qualify as
// computational expression template. Only in case a class is derived from the Computation base
// class, the IsComputation type trait recognizes the class as valid computational expression
// template.
*/
struct Computation : private Operation {};
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
