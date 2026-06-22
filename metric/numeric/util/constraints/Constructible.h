// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_CONSTRAINTS_CONSTRUCTIBLE_H
#define METRIC_NUMERIC_UTIL_CONSTRAINTS_CONSTRUCTIBLE_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/typetraits/IsConstructible.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_BE_DEFAULT_CONSTRUCTIBLE_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// In case the given data type \a T does not provide a default constructor, a compilation error
// is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_DEFAULT_CONSTRUCTIBLE_TYPE(T)                                                \
	static_assert(::mtrc::numeric::IsDefaultConstructible_v<T>, "Non-default constructible type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_DEFAULT_CONSTRUCTIBLE_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// In case the given data type \a T does provide a default constructor, a compilation error is
// created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_DEFAULT_CONSTRUCTIBLE_TYPE(T)                                            \
	static_assert(!::mtrc::numeric::IsDefaultConstructible_v<T>, "Default constructible type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_BE_NOTHROW_DEFAULT_CONSTRUCTIBLE_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// In case the given data type \a T does not provide a noexcept default constructor, a compilation
// error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_NOTHROW_DEFAULT_CONSTRUCTIBLE_TYPE(T)                                        \
	static_assert(::mtrc::numeric::IsNothrowDefaultConstructible_v<T>,                                               \
				  "Non-noexcept default constructible type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_NOTHROW_DEFAULT_CONSTRUCTIBLE_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// In case the given data type \a T does provide a noexcept default constructor, a compilation
// error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_NOTHROW_DEFAULT_CONSTRUCTIBLE_TYPE(T)                                    \
	static_assert(!::mtrc::numeric::IsNothrowDefaultConstructible_v<T>,                                              \
				  "Noexcept default constructible type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_BE_COPY_CONSTRUCTIBLE_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// In case the given data type \a T does not provide a copy constructor, a compilation error
// is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_COPY_CONSTRUCTIBLE_TYPE(T)                                                   \
	static_assert(::mtrc::numeric::IsCopyConstructible_v<T>, "Non-copy constructible type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_COPY_CONSTRUCTIBLE_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// In case the given data type \a T does provide a copy constructor, a compilation error is
// created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_COPY_CONSTRUCTIBLE_TYPE(T)                                               \
	static_assert(!::mtrc::numeric::IsCopyConstructible_v<T>, "Copy constructible type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_BE_NOTHROW_COPY_CONSTRUCTIBLE_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// In case the given data type \a T does not provide a noexcept copy constructor, a compilation
// error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_NOTHROW_COPY_CONSTRUCTIBLE_TYPE(T)                                           \
	static_assert(::mtrc::numeric::IsNothrowCopyConstructible_v<T>, "Non-noexcept copy constructible type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_NOTHROW_COPY_CONSTRUCTIBLE_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// In case the given data type \a T does provide a noexcept copy constructor, a compilation
// error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_NOTHROW_COPY_CONSTRUCTIBLE_TYPE(T)                                       \
	static_assert(!::mtrc::numeric::IsNothrowCopyConstructible_v<T>, "Noexcept copy constructible type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_BE_MOVE_CONSTRUCTIBLE_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// In case the given data type \a T does not provide a move constructor, a compilation error
// is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_MOVE_CONSTRUCTIBLE_TYPE(T)                                                   \
	static_assert(::mtrc::numeric::IsMoveConstructible_v<T>, "Non-move constructible type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_MOVE_CONSTRUCTIBLE_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// In case the given data type \a T does provide a move constructor, a compilation error is
// created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_MOVE_CONSTRUCTIBLE_TYPE(T)                                               \
	static_assert(!::mtrc::numeric::IsMoveConstructible_v<T>, "Move constructible type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_BE_NOTHROW_MOVE_CONSTRUCTIBLE_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// In case the given data type \a T does not provide a noexcept move constructor, a compilation
// error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_BE_NOTHROW_MOVE_CONSTRUCTIBLE_TYPE(T)                                           \
	static_assert(::mtrc::numeric::IsNothrowMoveConstructible_v<T>, "Non-noexcept move constructible type detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_BE_NOTHROW_MOVE_CONSTRUCTIBLE_TYPE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the data type.
// \ingroup constraints
//
// In case the given data type \a T does provide a noexcept move constructor, a compilation
// error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_NOTHROW_MOVE_CONSTRUCTIBLE_TYPE(T)                                       \
	static_assert(!::mtrc::numeric::IsNothrowMoveConstructible_v<T>, "Noexcept move constructible type detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
