// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_DECLARATION_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_DECLARATION_H
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
/*!\brief Base class for all declaration expression templates.
// \ingroup math
//
// The Declaration class serves as a tag for all declaration expression templates. All classes
// that represent a declaration and that are used within the expression template environment of
// the Metric numeric library have to derive publicly from this class in order to qualify as declaration
// expression template. Only in case a class is derived publicly from the Declaration base class,
// the IsDeclaration type trait recognizes the class as valid declaration expression template.
*/
template <typename T>
struct Declaration
	: private If_t<IsComputation_v<T>, Computation, If_t<IsTransformation_v<T>, Transformation, EmptyType>> {};
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
