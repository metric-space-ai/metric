// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_TYPETRAITS_MAKEUNSIGNED_H
#define METRIC_NUMERIC_UTIL_TYPETRAITS_MAKEUNSIGNED_H
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
/*!\brief Compile time type conversion into an unsigned integral type.
// \ingroup type_traits
//
// This type trait provides the feature to convert the given integral or constant type \a T to
// the corresponding unsigned integral data type with the same size and with the same cv-qualifiers.
// Note that in case \a T is bool or a non-integral data type, a compilation error is created.

   \code
   enum MyEnum { ... };

   mtrc::numeric::MakeUnsigned<int>::Type                  // Results in 'unsigned int'
   mtrc::numeric::MakeUnsigned<const unsigned int>::Type   // Results in 'const unsigned int'
   mtrc::numeric::MakeUnsigned<const unsigned long>::Type  // Results in 'const unsigned long'
   mtrc::numeric::MakeUnsigned<MyEnum>::Type               // Unsigned integer type with the same width as the enum
   mtrc::numeric::MakeUnsigned<wchar_t>::Type              // Unsigned integer type with the same width as wchar_t
   \endcode
*/
template <typename T> struct MakeUnsigned {
  public:
	//**********************************************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	using Type = typename std::make_unsigned<T>::type;
	/*! \endcond */
	//**********************************************************************************************
};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary alias declaration for the MakeUnsigned type trait.
// \ingroup type_traits
//
// The MakeUnsigned_t alias declaration provides a convenient shortcut to access the nested \a Type
// of the MakeUnsigned class template. For instance, given the type \a T the following two type
// definitions are identical:

   \code
   using Type1 = typename mtrc::numeric::MakeUnsigned<T>::Type;
   using Type2 = mtrc::numeric::MakeUnsigned_t<T>;
   \endcode
*/
template <typename T> using MakeUnsigned_t = typename MakeUnsigned<T>::Type;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
