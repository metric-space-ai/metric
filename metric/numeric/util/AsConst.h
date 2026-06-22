// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_ASCONST_H
#define METRIC_NUMERIC_UTIL_ASCONST_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/typetraits/AddConst.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  ASCONST FUNCTIONALITY
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Adding 'const' to the given lvalue.
// \ingroup util
//
// \param v The given lvalue.
// \return The const-qualified lvalue.
//
// This function adds the 'const' qualifier to the given lvalue.
*/
template <typename T> constexpr AddConst_t<T> &as_const(T &v) noexcept { return v; }
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Overload of the as_const() function for rvalues.
// \ingroup util
//
// This overload of the as_const() function disables its use on rvalues. This prevents potential
// misuse as in for instance the following example:

   \code
   for( const auto&& value : as_const( getTemporary() ) )
   {
	  // ...
   }
   \endcode
*/
template <typename T> void as_const(const T &&) = delete;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
