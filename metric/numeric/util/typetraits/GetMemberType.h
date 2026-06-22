// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_TYPETRAITS_GETMEMBERTYPE_H
#define METRIC_NUMERIC_UTIL_TYPETRAITS_GETMEMBERTYPE_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/typetraits/Void.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  MACRO DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Macro for the creation of a type trait to acquire member types.
// \ingroup math_type_traits
//
// This macro creates the definition of a type trait \a TYPE_TRAIT_NAME that can determine a
// specified member type of a given type. The first macro parameter \a TYPE_TRAIT_NAME specifies
// the resulting name of the type trait. The second parameter \a MEMBER_NAME specifies the name
// of the member type to be acquired and the third parameter \a FALLBACK_TYPE specifies the type
// to acquire in case the given type doesn't contain the specified member type. The following
// example demonstrates the use of the macro and the resulting type trait:

   \code
   struct MyType1 {
	  using ElementType = float;
   };

   struct MyType2 {
	  using ElementType = double;
   };

   struct MyType3 {};

   METRIC_NUMERIC_CREATE_GET_TYPE_MEMBER_TYPE_TRAIT( GetElementType, ElementType, int );

   GetElementType<MyType1>::Type  // Results in 'float'
   GetElementType<MyType2>::Type  // Results in 'double'
   GetElementType<MyType3>::Type  // Results in 'int'
   \endcode

// The macro results in the definition of a new class with the specified name \a TYPE_TRAIT_NAME
// and an associated alias template called TYPE_TRAIT_NAME_t within the current namespace. This
// may cause name collisions with any other entity called \a TYPE_TRAIT_NAME in the same namespace.
// Therefore it is advisable to create the type trait as locally as possible to minimize the
// probability of name collisions. Note however that the macro cannot be used within function
// scope since a template declaration cannot appear at block scope.
*/
#define METRIC_NUMERIC_CREATE_GET_TYPE_MEMBER_TYPE_TRAIT(TYPE_TRAIT_NAME, MEMBER_NAME, FALLBACK_TYPE)                  \
                                                                                                                       \
	template <typename Type1233, typename = void> struct TYPE_TRAIT_NAME {                                             \
		using Type = FALLBACK_TYPE;                                                                                    \
	};                                                                                                                 \
                                                                                                                       \
	template <typename Type1233>                                                                                       \
	struct TYPE_TRAIT_NAME<Type1233, mtrc::numeric::Void_t<typename Type1233::MEMBER_NAME>> {                        \
		using Type = typename Type1233::MEMBER_NAME;                                                                   \
	};                                                                                                                 \
                                                                                                                       \
	template <typename Type1233> using TYPE_TRAIT_NAME##_t = typename TYPE_TRAIT_NAME<Type1233>::Type
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
