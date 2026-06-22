// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_SHIMS_DIGITS_H
#define METRIC_NUMERIC_MATH_SHIMS_DIGITS_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/EnableIf.h>
#include <metric/numeric/util/Types.h>
#include <metric/numeric/util/typetraits/IsIntegral.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  DIGITS SHIM
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Returns the number of valid digits of an integral value.
// \ingroup math_shims
//
// \param a The integral value.
// \return The number of valid digits.
//
// This function counts the number of valid digits in the given integral value.

   \code
   digits( 100   );  // Returns 3
   digits( 12345 );  // Returns 5
   digits( 0     );  // Returns 0
   \endcode

// The digits function only works for integral built-in data types. The attempt to use any
// other type will result in a compile time error.
*/
template <typename T, EnableIf_t<IsIntegral_v<T>> * = nullptr> constexpr size_t digits(T a) noexcept
{
	size_t count(0);

	while (a != 0) {
		a /= 10;
		++count;
	}

	return count;
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
