// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_SHIMS_ISNAN_H
#define METRIC_NUMERIC_MATH_SHIMS_ISNAN_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/system/Inline.h>
#include <metric/numeric/util/EnableIf.h>
#include <metric/numeric/util/typetraits/IsNumeric.h>

//*************************************************************************************************
// Macro undefinition
//*************************************************************************************************

#ifdef isnan
#undef isnan
#endif

namespace mtrc::numeric {

//=================================================================================================
//
//  ISNAN SHIM
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Platform independent implementation of the C99 \a isnan function.
// \ingroup math_shims
//
// \param a Value to be checked.
// \return \a true if \a a is not a number (NaN), \a false otherwise.
//
// This function provides a platform independent check for NaN values. In contrast to the \a isnan
// function from the C standard, which is only supporting all floating point types, this function
// can be used for all numeric data types (i.e. all integral, floating point, and complex data
// types). The function returns \a true in case the given value is not a number (NaN). In all
// other cases the function returns \a false.
*/
template <typename T, EnableIf_t<IsNumeric_v<T>> * = nullptr> METRIC_NUMERIC_ALWAYS_INLINE bool isnan(T a) noexcept
{
	return a != a;
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
