// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_TRANSFORMATION_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_TRANSFORMATION_H
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
/*!\brief Base class for all transform expression templates.
// \ingroup math
//
// The Transformation class serves as a tag for all transformation expression templates. All
// classes, that represent a transformation (e.g. transpositions) and that are used within the
// expression template environment of the Metric numeric library have to derive from this class in order
// to qualify as transformation expression template. Only in case a class is derived from the
// Transformation base class, the IsTransformation type trait recognizes the class as valid
// transformation expression template.
*/
struct Transformation : private Operation {};
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
