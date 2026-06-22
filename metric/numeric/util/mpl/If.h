// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_MPL_IF_H
#define METRIC_NUMERIC_UTIL_MPL_IF_H
namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Compile time type selection.
// \ingroup mpl
//
// The If class template selects one of the two given types \a T1 and \a T2 depending on the
// \a Condition template argument. In case the \a Condition compile time constant expression
// evaluates to \a true, the member type definition \a Type is set to \a T1. In case
// \a Condition evaluates to \a false, \a Type is set to \a T2.
*/
template <bool Condition> // Compile time selection
struct If {
  public:
	//**********************************************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	template <typename T1, typename T2> using Type = T1; //!< The selected type.
														 /*! \endcond */
	//**********************************************************************************************
};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the If class template.
// \ingroup mpl
//
// This specialization of the If template is selected in case the \a Condition compile time
// constant expression evaluates to \a false. The member type definition is set to the second
// given type \a T2.
*/
template <> struct If<false> {
  public:
	//**********************************************************************************************
	template <typename T1, typename T2> using Type = T2; //!< The selected type.
	//**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary alias template for the If class template.
// \ingroup util
//
// The If_t alias template provides a convenient shortcut to access the nested \a Type of
// the If class template. For instance, given the types \a C, \a T1, and \a T2 the following
// two type definitions are identical:

   \code
   using Type1 = typename If< IsBuiltin_v<C>, T1, T2 >::Type;
   using Type2 = If_t< IsBuiltin_v<C>, T1, T2 >;
   \endcode
*/
template <bool Condition // Compile time selection
		  ,
		  typename T1 // Type to be selected if Condition=true
		  ,
		  typename T2> // Type to be selected if Condition=false
using If_t = typename If<Condition>::template Type<T1, T2>;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
