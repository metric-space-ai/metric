// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_ENABLEIF_H
#define METRIC_NUMERIC_UTIL_ENABLEIF_H
namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Substitution Failure Is Not An Error (SFINAE) class.
// \ingroup util
//
// The EnableIf class template is an auxiliary tool for an intentional application of the
// Substitution Failure Is Not An Error (SFINAE) principle. It allows a function template
// or a class template specialization to include or exclude itself from a set of matching
// functions or specializations based on properties of its template arguments. For instance,
// it can be used to restrict the selection of a function template to specific data types.
// The following example illustrates this in more detail.

   \code
   template< typename Type >
   void process( Type t ) { ... }
   \endcode

// Due to the general formulation of this function, it will always be a possible candidate for
// every possible argument. However, with the EnableIf class it is for example possible to
// restrict the valid argument types to built-in, numeric data types.

   \code
   template< typename Type >
   typename EnableIf< IsNumeric_v<Type> >::Type process( Type t ) { ... }
   \endcode

// In case the given data type is not a built-in, numeric data type, the access to the nested
// type defintion \a Type of the EnableIf template will fail. However, due to the SFINAE
// principle, this will only result in a compilation error in case the compiler cannot find
// another valid function.\n
// Note that in this application of the EnableIf template the default for the nested type
// definition \a Type is used, which corresponds to \a void. Via the second template argument
// it is possible to explicitly specify the type of \a Type:

   \code
   // Explicity specifying the default
   typename EnableIf< IsNumeric_v<Type>, void >::Type

   // In case the given data type is a boolean data type, the nested type definition
   // 'Type' is set to float
   typename EnableIf< IsBoolean_v<Type>, float >::Type
   \endcode

// For more information on the EnableIf functionality, see the standard library documentation
// of std::enable_if at:
//
//           \a http://en.cppreference.com/w/cpp/types/enable_if.
*/
template <bool Condition // Compile time condition
		  ,
		  typename T = void> // The type to be instantiated
struct EnableIf {
	//**********************************************************************************************
	using Type = T; //!< The instantiated type.
					//**********************************************************************************************
};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief EnableIf specialization for failed constraints.
// \ingroup util
//
// This specialization of the EnableIf template is selected if the first template parameter (the
// compile time condition) evaluates to \a false. This specialization does not contains a nested
// type definition \a Type and therefore always results in a compilation error in case \a Type
// is accessed. However, due to the SFINAE principle the compilation process is not necessarily
// stopped if another, valid instantiation is found by the compiler.
*/
template <typename T> // The type to be instantiated
struct EnableIf<false, T> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary type for the EnableIf class template.
// \ingroup util
//
// The EnableIf_t alias declaration provides a convenient shortcut to access the nested \a Type
// of the EnableIf class template. For instance, given the type \a T the following two type
// definitions are identical:

   \code
   using Type1 = typename EnableIf< IsBuiltin_v<T> >::Type;
   using Type2 = EnableIf_t< IsBuiltin_v<T> >;
   \endcode
*/
template <bool Condition // Compile time condition
		  ,
		  typename T = void> // The type to be instantiated
using EnableIf_t = typename EnableIf<Condition, T>::Type;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary type for the EnableIf class template.
// \ingroup util
//
// The DisableIf alias declaration provides a convenient shortcut for negated SFINAE conditions.
// For instance, given the type \a T the following two type definitions are identical:

   \code
   using Type1 = typename EnableIf< !IsBuiltin_v<T> >::Type;
   using Type2 = typename DisableIf< IsBuiltin_v<T> >::Type;
   \endcode
*/
template <bool Condition // Compile time condition
		  ,
		  typename T = void> // The type to be instantiated
using DisableIf = EnableIf<!Condition, T>;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary type for the EnableIf class template.
// \ingroup util
//
// The DisableIf_t alias declaration provides a convenient shortcut to access the nested \a Type
// of the negated EnableIf class template. For instance, given the type \a T the following two
// type definitions are identical:

   \code
   using Type1 = typename EnableIf< !IsBuiltin_v<T> >::Type;
   using Type2 = DisableIf_t< IsBuiltin_v<T> >;
   \endcode
*/
template <bool Condition // Compile time condition
		  ,
		  typename T = void> // The type to be instantiated
using DisableIf_t = typename EnableIf<!Condition, T>::Type;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
