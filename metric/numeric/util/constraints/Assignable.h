// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_CONSTRAINTS_ASSIGNABLE_H
#define METRIC_NUMERIC_UTIL_CONSTRAINTS_ASSIGNABLE_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/typetraits/IsAssignable.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_COPY_ASSIGNABLE_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// In case the given data type \a T does not provide a copy constructor, a compilation error
// is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_COPY_ASSIGNABLE_TYPE(T)                                                      \
	static_assert(::mtrc::numeric::IsCopyAssignable_v<T>, "Non-copy assignable type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_COPY_ASSIGNABLE_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// In case the given data type \a T does provide a copy constructor, a compilation error is
// created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_COPY_ASSIGNABLE_TYPE(T)                                                  \
	static_assert(!::mtrc::numeric::IsCopyAssignable_v<T>, "Copy assignable type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_BE_NOTHROW_COPY_ASSIGNABLE_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// In case the given data type \a T does not provide a noexcept copy constructor, a compilation
// error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_NOTHROW_COPY_ASSIGNABLE_TYPE(T)                                              \
	static_assert(::mtrc::numeric::IsNothrowCopyAssignable_v<T>, "Non-noexcept copy assignable type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_NOTHROW_COPY_ASSIGNABLE_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// In case the given data type \a T does provide a noexcept copy constructor, a compilation
// error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_NOTHROW_COPY_ASSIGNABLE_TYPE(T)                                          \
	static_assert(!::mtrc::numeric::IsNothrowCopyAssignable_v<T>, "Noexcept copy assignable type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_BE_MOVE_ASSIGNABLE_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// In case the given data type \a T does not provide a move constructor, a compilation error
// is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_MOVE_ASSIGNABLE_TYPE(T)                                                      \
	static_assert(::mtrc::numeric::IsMoveAssignable_v<T>, "Non-move assignable type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_MOVE_ASSIGNABLE_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// In case the given data type \a T does provide a move constructor, a compilation error is
// created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_MOVE_ASSIGNABLE_TYPE(T)                                                  \
	static_assert(!::mtrc::numeric::IsMoveAssignable_v<T>, "Move assignable type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_BE_NOTHROW_MOVE_ASSIGNABLE_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// In case the given data type \a T does not provide a noexcept move constructor, a compilation
// error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_NOTHROW_MOVE_ASSIGNABLE_TYPE(T)                                              \
	static_assert(::mtrc::numeric::IsNothrowMoveAssignable_v<T>, "Non-noexcept move assignable type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_NOTHROW_MOVE_ASSIGNABLE_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// In case the given data type \a T does provide a noexcept move constructor, a compilation
// error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_NOTHROW_MOVE_ASSIGNABLE_TYPE(T)                                          \
	static_assert(!::mtrc::numeric::IsNothrowMoveAssignable_v<T>, "Noexcept move assignable type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
