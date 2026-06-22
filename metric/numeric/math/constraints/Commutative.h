// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_CONSTRAINTS_COMMUTATIVE_H
#define METRIC_NUMERIC_MATH_CONSTRAINTS_COMMUTATIVE_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsCommutative.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_COMMUTATIVE_TYPES CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data types \a T1 and \a T2 are not commutative (i.e. cannot be swapped in
// mathematical operations), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_COMMUTATIVE_TYPES(T1, T2)                                                    \
	static_assert(::mtrc::numeric::IsCommutative_v<T1, T2>, "Non-commutative types detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_COMMUTATIVE_TYPES CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup math_constraints
//
// In case the given data types \a T1 and \a T2 are commutative (i.e. can be swapped in
// mathematical operations), a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_COMMUTATIVE_TYPES(T1, T2)                                                \
	static_assert(!::mtrc::numeric::IsCommutative_v<T1, T2>, "Commutative types detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
