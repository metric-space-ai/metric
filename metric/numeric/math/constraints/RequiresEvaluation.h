// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_REQUIRESEVALUATION_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_REQUIRESEVALUATION_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/RequiresEvaluation.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_REQUIRE_EVALUATION CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T does not require an intermediate evaluation within composite
// expressions, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_REQUIRE_EVALUATION(T)                                                           \
	static_assert(::mtrc::numeric::RequiresEvaluation_v<T>, "Type without evaluation requirement detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_REQUIRE_EVALUATION CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data type \a T requires an intermediate evaluation within composite
// expressions, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_REQUIRE_EVALUATION(T)                                                       \
	static_assert(!::mtrc::numeric::RequiresEvaluation_v<T>, "Type with evaluation requirement detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
