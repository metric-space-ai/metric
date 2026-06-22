// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_CONSTRAINTS_SIZE_H
#define METRIC_NUMERIC_UTIL_CONSTRAINTS_SIZE_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/typetraits/HasSize.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MUST_HAVE_SIZE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the size of a data type.
// \ingroup constraints
//
// In case the type \a T doesn't have a size of \a S bytes, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_HAVE_SIZE(T, S)                                                                 \
	static_assert(::mtrc::numeric::HasSize_v<T, S>, "Invalid size detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_HAVE_SIZE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the size of a data type.
// \ingroup constraints
//
// In case the type \a T has a size of \a S bytes, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_HAVE_SIZE(T, S)                                                             \
	static_assert(!::mtrc::numeric::HasSize_v<T, S>, "Invalid size detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_HAVE_1_BYTE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the size of a data type.
// \ingroup constraints
//
// In case the type \a T doesn't have a size of exactly 1 byte, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_HAVE_1_BYTE(T)                                                                  \
	static_assert(::mtrc::numeric::Has1Byte_v<T>, "Invalid size detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_HAVE_1_BYTE CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the size of a data type.
// \ingroup constraints
//
// In case the type \a T has a size of exactly 1 byte, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_HAVE_1_BYTE(T)                                                              \
	static_assert(!::mtrc::numeric::Has1Byte_v<T>, "Invalid size detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_HAVE_2_BYTES CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the size of a data type.
// \ingroup constraints
//
// In case the type \a T doesn't have a size of exactly 2 bytes, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_HAVE_2_BYTES(T)                                                                 \
	static_assert(::mtrc::numeric::Has2Byte_v<T>, "Invalid size detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_HAVE_2_BYTES CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the size of a data type.
// \ingroup constraints
//
// In case the type \a T has a size of exactly 2 bytes, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_HAVE_2_BYTES(T)                                                             \
	static_assert(!::mtrc::numeric::Has2Byte_v<T>, "Invalid size detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_HAVE_4_BYTES CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the size of a data type.
// \ingroup constraints
//
// In case the type \a T doesn't have a size of exactly 4 bytes, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_HAVE_4_BYTES(T)                                                                 \
	static_assert(::mtrc::numeric::Has4Byte_v<T>, "Invalid size detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_HAVE_4_BYTES CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the size of a data type.
// \ingroup constraints
//
// In case the type \a T has a size of exactly 4 bytes, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_HAVE_4_BYTES(T)                                                             \
	static_assert(!::mtrc::numeric::Has4Byte_v<T>, "Invalid size detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_HAVE_8_BYTES CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the size of a data type.
// \ingroup constraints
//
// In case the type \a T doesn't have a size of exactly 8 bytes, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_HAVE_8_BYTES(T)                                                                 \
	static_assert(::mtrc::numeric::Has8Byte_v<T>, "Invalid size detected")
//*************************************************************************************************

//=================================================================================================
//
//  MUST_NOT_HAVE_8_BYTES CONSTRAINT
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constraint on the size of a data type.
// \ingroup constraints
//
// In case the type \a T has a size of exactly 8 bytes, a compilation error is created.
*/
#define METRIC_NUMERIC_CONSTRAINT_MUST_NOT_HAVE_8_BYTES(T)                                                             \
	static_assert(!::mtrc::numeric::Has8Byte_v<T>, "Invalid size detected")
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
