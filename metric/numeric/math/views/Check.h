// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_VIEWS_CHECK_H
#define METRIC_NUMERIC_MATH_VIEWS_CHECK_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/IntegralConstant.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Template for the mtrc::numeric::checked and mtrc::numeric::unchecked instances.
// \ingroup views
//
// mtrc::numeric::Check is the template for the mtrc::numeric::checked and mtrc::numeric::unchecked instance,
which is
// an optional token for the creation of views. It can be used to enforce or skip all runtime
// checks during the creation of a view (subvectors, submatrices, rows, columns, bands, ...).
*/
template <bool C> struct Check : public BoolConstant<C> {
	//**Constructor*********************************************************************************
	/*!\name Constructor */
	//@{
	constexpr Check() = default;
	//@}
	//**********************************************************************************************
};
//*************************************************************************************************

//=================================================================================================
//
//  TYPE ALIASES
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Type of the mtrc::numeric::checked instance.
// \ingroup views
//
// mtrc::numeric::Checked is the type of the mtrc::numeric::checked instance, which is an optional token for the
// creation of views. It can be used to enforce runtime checks during the creation of a view
// (subvectors, submatrices, rows, columns, bands, ...).
*/
using Checked = Check<true>;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Type of the mtrc::numeric::unchecked instance.
// \ingroup views
//
// mtrc::numeric::Unchecked is the type of the mtrc::numeric::unchecked instance, which is an optional token for
// the creation of views. It can be used to skip all runtime checks during the creation of a view
// (subvectors, submatrices, rows, columns, bands, ...).
*/
using Unchecked = Check<false>;
//*************************************************************************************************

//=================================================================================================
//
//  GLOBAL CHECK INSTANCES
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Global Checked instance.
// \ingroup views
//
// The mtrc::numeric::checked instance is an optional token for the creation of views. It can be used
// used to enforce runtime checks during the creation of a view (subvectors, submatrices, rows,
// columns, bands, ...). The following example demonstrates the setup of a checked subvector:

   \code
   mtrc::numeric::DynamicVector<int> v( 100UL );
   auto sv = subvector( v, 10UL, 20UL, checked );  // Creating an checked subvector
   \endcode
*/
constexpr Checked checked;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Global Unchecked instance.
// \ingroup views
//
// The mtrc::numeric::unchecked instance is an optional token for the creation of views. It can be
// used to skip all runtime checks during the creation of a view (subvectors, submatrices, rows,
// columns, bands, ...). The following example demonstrates the setup of an unchecked subvector:

   \code
   mtrc::numeric::DynamicVector<int> v( 100UL );
   auto sv = subvector( v, 10UL, 20UL, unchecked );  // Creating an unchecked subvector
   \endcode
*/
constexpr Unchecked unchecked;
//*************************************************************************************************

//=================================================================================================
//
//  UTILITY FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Termination condition for the variadic \c getCheck() function (zero arguments).
// \ingroup views
//
// \return An instance of type mtrc::numeric::Checked.
*/
constexpr Checked getCheck() noexcept { return Checked{}; }
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Termination condition for the variadic \c getCheck() function (mtrc::numeric::Unchecked found).
// \ingroup views
//
// \param a The instance of type mtrc::numeric::Unchecked.
// \param args The remaining arguments.
// \return An instance of type mtrc::numeric::Unchecked.
*/
template <typename... Ts> constexpr Unchecked getCheck(const Unchecked &a, const Ts &...args) noexcept
{
	MAYBE_UNUSED(a, args...);
	return Unchecked{};
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Extracting mtrc::numeric::Check arguments from a given list of arguments.
// \ingroup views
//
// \param a The first given argument.
// \param args The remaining given arguments.
// \return mtrc::numeric::Unchecked if at least one mtrc::numeric::Unchecked is given, mtrc::numeric::Checked
otherwise.
//
// This function extracts any argument of type mtrc::numeric::Check from the given list of arguments.
// It returns an instance of type mtrc::numeric::Unchecked if at least one argument of type
// mtrc::numeric::Unchecked is given, otherwise an instance of mtrc::numeric::Checked.
*/
template <typename T, typename... Ts> constexpr auto getCheck(const T &a, const Ts &...args) noexcept
{
	MAYBE_UNUSED(a);
	return getCheck(args...);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Extracting mtrc::numeric::Check arguments from a given list of arguments.
// \ingroup views
//
// \param args The given arguments.
// \return \a false if at least one mtrc::numeric::Unchecked is given, \a true otherwise.
//
// This function extracts any argument of type mtrc::numeric::Check from the given list of arguments.
// It returns \a false if at least one argument of type mtrc::numeric::Unchecked is given, otherwise
// it returns \a true.
*/
template <typename... Ts> constexpr bool isChecked(const Ts &...args) { return getCheck(args...).value; }
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
