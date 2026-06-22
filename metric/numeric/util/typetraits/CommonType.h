// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_TYPETRAITS_COMMONTYPE_H
#define METRIC_NUMERIC_UTIL_TYPETRAITS_COMMONTYPE_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <type_traits>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Deduction of a type common to several types.
// \ingroup type_traits
//
// The CommonType type trait deduces the result type of a mixed-mode arithmetic expression between
// all types T..., that is the type all T... can be implicitly converted to. Note that cv and
// reference qualifiers are generally ignored.

   \code
   mtrc::numeric::CommonType<short,int>::Type                         // Results in 'int'
   mtrc::numeric::CommonType<const double,int&>::Type                 // Results in 'double'
   mtrc::numeric::CommonType<char&, volatile int, const float>::Type  // Results in 'float'
   \endcode
*/
template <typename... T> struct CommonType {
  public:
	//**********************************************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	using Type = typename std::common_type<T...>::type;
	/*! \endcond */
	//**********************************************************************************************
};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary alias declaration for the CommonType type trait.
// \ingroup type_traits
//
// The CommonType_t alias declaration provides a convenient shortcut to access the nested \a Type
// of the CommonType class template. For instance, given the types \a T1 and \a T2 the following
// two type definitions are identical:

   \code
   using Type1 = typename mtrc::numeric::CommonType<T1,T2>::Type;
   using Type2 = mtrc::numeric::CommonType_t<T1,T2>;
   \endcode
*/
template <typename... T> using CommonType_t = typename CommonType<T...>::Type;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
