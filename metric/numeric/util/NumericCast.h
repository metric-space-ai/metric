// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_NUMERICCAST_H
#define METRIC_NUMERIC_UTIL_NUMERICCAST_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <limits>
#include <metric/numeric/util/Assert.h>
#include <metric/numeric/util/EnableIf.h>
#include <metric/numeric/util/Exception.h>
#include <metric/numeric/util/IntegralConstant.h>
#include <metric/numeric/util/constraints/Numeric.h>
#include <metric/numeric/util/typetraits/IsFloatingPoint.h>
#include <metric/numeric/util/typetraits/IsIntegral.h>
#include <metric/numeric/util/typetraits/IsSame.h>
#include <metric/numeric/util/typetraits/IsSigned.h>
#include <metric/numeric/util/typetraits/IsUnsigned.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  AUXILIARY VARIABLE TEMPLATE DEFINITIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Auxiliary variable template for the numeric_cast() function template.
// \ingroup util
*/
template <typename To, typename From>
constexpr bool IsCriticalIntIntConversion_v = (IsIntegral_v<To> && IsIntegral_v<From> && !IsSame_v<To, From>);
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Auxiliary variable template for the numeric_cast() function template.
// \ingroup util
*/
template <typename To, typename From>
constexpr bool IsCriticalFloatIntConversion_v = (IsIntegral_v<To> && IsFloatingPoint_v<From>);
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Auxiliary variable template for the numeric_cast() function template.
// \ingroup util
*/
template <typename To, typename From>
constexpr bool IsCriticalFloatFloatConversion_v =
	(IsFloatingPoint_v<To> && IsFloatingPoint_v<From> && (sizeof(To) < sizeof(From)));
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Auxiliary variable template for the numeric_cast() function template.
// \ingroup util
*/
template <typename To, typename From>
constexpr bool IsUncriticalConversion_v =
	(!IsCriticalIntIntConversion_v<To, From> && !IsCriticalFloatIntConversion_v<To, From> &&
	 !IsCriticalFloatFloatConversion_v<To, From>);
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  NUMERIC CAST OPERATORS
//
//=================================================================================================

//*************************************************************************************************
/*!\name Numeric cast operators */
//@{
template <typename To, typename From> inline To numeric_cast(From from);
//@}
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Backend for numeric_cast() for uncritical conversions.
// \ingroup util
//
// \param from The numeric value to be converted.
// \return The converted value.
*/
template <typename To, typename From>
inline EnableIf_t<IsUncriticalConversion_v<To, From>, To> numeric_cast_backend(From from) noexcept
{
	return from;
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Backend of numeric_cast() for critical conversions between integral types.
// \ingroup util
//
// \param from The numeric value to be converted.
// \return The converted value.
// \exception std::overflow_error Invalid numeric cast (overflow).
// \exception std::underflow_error Invalid numeric cast (underflow).
*/
template <typename To, typename From>
inline EnableIf_t<IsCriticalIntIntConversion_v<To, From>, To> numeric_cast_backend(From from)
{
	if ((sizeof(To) < sizeof(From) || (IsSigned_v<To> && IsUnsigned_v<From>)) &&
		(from > From(std::numeric_limits<To>::max()))) {
		METRIC_NUMERIC_THROW_OVERFLOW_ERROR("Invalid numeric cast (overflow)");
	}

	if (IsSigned_v<From> && (from < From(std::numeric_limits<To>::min()))) {
		METRIC_NUMERIC_THROW_UNDERFLOW_ERROR("Invalid numeric cast (underflow)");
	}

	METRIC_NUMERIC_INTERNAL_ASSERT(from == From(To(from)), "Numeric cast failed");

	return from;
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Backend of numeric_cast() for critical conversions from floating point to integral types.
// \ingroup util
//
// \param from The numeric value to be converted.
// \return The converted value.
// \exception std::overflow_error Invalid numeric cast (overflow).
// \exception std::underflow_error Invalid numeric cast (underflow).
*/
template <typename To, typename From>
inline EnableIf_t<IsCriticalFloatIntConversion_v<To, From>, To> numeric_cast_backend(From from)
{
	using std::trunc;

	if (from > From(std::numeric_limits<To>::max())) {
		METRIC_NUMERIC_THROW_OVERFLOW_ERROR("Invalid numeric cast (overflow)");
	}

	if (from < From(std::numeric_limits<To>::min())) {
		METRIC_NUMERIC_THROW_UNDERFLOW_ERROR("Invalid numeric cast (underflow)");
	}

	METRIC_NUMERIC_INTERNAL_ASSERT(trunc(from) == From(To(from)), "Numeric cast failed");

	return from;
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Backend of numeric_cast() for critical conversions between floating point types.
// \ingroup util
//
// \param from The numeric value to be converted.
// \return The converted value.
// \exception std::overflow_error Invalid numeric cast (overflow).
// \exception std::underflow_error Invalid numeric cast (underflow).
*/
template <typename To, typename From>
inline EnableIf_t<IsCriticalFloatFloatConversion_v<To, From>, To> numeric_cast_backend(From from)
{
	if (from > From(std::numeric_limits<To>::max())) {
		METRIC_NUMERIC_THROW_OVERFLOW_ERROR("Invalid numeric cast (overflow)");
	}

	if (from < From(std::numeric_limits<To>::min())) {
		METRIC_NUMERIC_THROW_UNDERFLOW_ERROR("Invalid numeric cast (underflow)");
	}

	METRIC_NUMERIC_INTERNAL_ASSERT(from == From(To(from)), "Numeric cast failed");

	return from;
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Checked conversion of values of numeric type.
// \ingroup util
//
// \param from The numeric value to be converted.
// \return The converted value.
// \exception std::overflow_error Invalid numeric cast (overflow).
// \exception std::underflow_error Invalid numeric cast (underflow).
//
// This function converts the given numeric value \a from to the specified type \a To. In case
// a loss of range is detected, either a \a std::underflow_error or \a std::overflow_error
// exception is thrown.
//
// Examples:

   \code
   // Triggers a std::overflow_error exception
   try {
	  const int a( std::numeric_limits<int>::max() );
	  const short b( mtrc::numeric::numeric_cast<short>( a ) );
   }
   catch( std::overflow_error& ) {}

   // Triggers a std::underflow_error exception
   try {
	  const int a( -1 );
	  const unsigned int b( mtrc::numeric::numeric_cast<unsigned int>( a ) );
   }
   catch( std::underflow_error& ) {}
   \endcode
*/
template <typename To // The target type
		  ,
		  typename From> // The source type
inline To numeric_cast(From from)
{
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_NUMERIC_TYPE(To);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_NUMERIC_TYPE(From);

	return numeric_cast_backend<To>(from);
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
