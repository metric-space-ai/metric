// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_TYPETRAITS_MAKESIGNED_H
#define METRIC_NUMERIC_UTIL_TYPETRAITS_MAKESIGNED_H
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
/*!\brief Compile time type conversion into a signed integral type.
// \ingroup type_traits
//
// This type trait provides the feature to convert the given integral or constant type \a T to
// the corresponding signed integral data type with the same size and with the same cv-qualifiers.
// Note that in case \a T is bool or a non-integral data type, a compilation error is created.

   \code
   enum MyEnum { ... };

   mtrc::numeric::MakeSigned<int>::Type                  // Results in 'int'
   mtrc::numeric::MakeSigned<const unsigned int>::Type   // Results in 'const int'
   mtrc::numeric::MakeSigned<const unsigned long>::Type  // Results in 'const long'
   mtrc::numeric::MakeSigned<MyEnum>::Type               // Signed integer type with the same width as the enum
   mtrc::numeric::MakeSigned<wchar_t>::Type              // Signed integer type with the same width as wchar_t
   \endcode
*/
template <typename T> struct MakeSigned {
  public:
	//**********************************************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	using Type = typename std::make_signed<T>::type;
	/*! \endcond */
	//**********************************************************************************************
};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary alias declaration for the MakeSigned type trait.
// \ingroup type_traits
//
// The MakeSigned_t alias declaration provides a convenient shortcut to access the nested \a Type
// of the MakeSigned class template. For instance, given the type \a T the following two type
// definitions are identical:

   \code
   using Type1 = typename mtrc::numeric::MakeSigned<T>::Type;
   using Type2 = mtrc::numeric::MakeSigned_t<T>;
   \endcode
*/
template <typename T> using MakeSigned_t = typename MakeSigned<T>::Type;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
