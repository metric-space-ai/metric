// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_MODIFICATION_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_MODIFICATION_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/expressions/Computation.h>
#include <metric/numeric/math/expressions/Transformation.h>
#include <metric/numeric/math/typetraits/IsComputation.h>
#include <metric/numeric/math/typetraits/IsTransformation.h>
#include <metric/numeric/util/EmptyType.h>
#include <metric/numeric/util/mpl/If.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Base class for all modification expression templates.
// \ingroup math
//
// The Modification class serves as a tag for all modification expression templates. All classes
// that represent a modification and that are used within the expression template environment of
// the Metric numeric library have to derive publicly from this class in order to qualify as modification
// expression template. Only in case a class is derived publicly from the Modification base class,
// the IsModification type trait recognizes the class as valid modification expression template.
*/
template <typename T>
struct Modification
	: private If_t<IsComputation_v<T>, Computation, If_t<IsTransformation_v<T>, Transformation, EmptyType>> {};
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
