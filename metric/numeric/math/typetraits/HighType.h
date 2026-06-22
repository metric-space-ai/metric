// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_HIGHTYPE_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_HIGHTYPE_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/Complex.h>
#include <metric/numeric/util/EnableIf.h>
#include <metric/numeric/util/InvalidType.h>
#include <metric/numeric/util/mpl/If.h>
#include <metric/numeric/util/typetraits/IsComplex.h>
#include <metric/numeric/util/typetraits/IsFloatingPoint.h>
#include <metric/numeric/util/typetraits/IsIntegral.h>
#include <metric/numeric/util/typetraits/IsSigned.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Auxiliary helper struct for the HighType type trait.
// \ingroup math_traits
*/
template <typename T1 // First operand
		  ,
		  typename T2 // Second operand
		  ,
		  typename = void> // Restricting condition
struct HighTypeHelper {
  public:
	//**********************************************************************************************
	using Type = INVALID_TYPE;
	//**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HighTypeHelper class template for a small and a large integral type.
// \ingroup math_traits
*/
template <typename T1, typename T2>
struct HighTypeHelper<T1, T2, EnableIf_t<IsIntegral_v<T1> && IsIntegral_v<T2> && (sizeof(T1) < sizeof(T2))>> {
  public:
	//**********************************************************************************************
	using Type = T2;
	//**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HighTypeHelper class template for a large and a small integral type.
// \ingroup math_traits
*/
template <typename T1, typename T2>
struct HighTypeHelper<T1, T2, EnableIf_t<IsIntegral_v<T1> && IsIntegral_v<T2> && (sizeof(T1) > sizeof(T2))>> {
  public:
	//**********************************************************************************************
	using Type = T1;
	//**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HighTypeHelper class template for two integral types.
// \ingroup math_traits
*/
template <typename T1, typename T2>
struct HighTypeHelper<T1, T2, EnableIf_t<IsIntegral_v<T1> && IsIntegral_v<T2> && (sizeof(T1) == sizeof(T2))>> {
  public:
	//**********************************************************************************************
	using Type = If_t<IsSigned_v<T1>, T1, T2>;
	//**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HighTypeHelper class template for an integral and a floating point type.
// \ingroup math_traits
*/
template <typename T1, typename T2>
struct HighTypeHelper<T1, T2, EnableIf_t<IsIntegral_v<T1> && IsFloatingPoint_v<T2>>> {
  public:
	//**********************************************************************************************
	using Type = T2;
	//**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HighTypeHelper class template for a floating point and an integral type.
// \ingroup math_traits
*/
template <typename T1, typename T2>
struct HighTypeHelper<T1, T2, EnableIf_t<IsFloatingPoint_v<T1> && IsIntegral_v<T2>>> {
  public:
	//**********************************************************************************************
	using Type = T1;
	//**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HighTypeHelper class template for two floating point types.
// \ingroup math_traits
*/
template <typename T1, typename T2>
struct HighTypeHelper<T1, T2, EnableIf_t<IsFloatingPoint_v<T1> && IsFloatingPoint_v<T2>>> {
  public:
	//**********************************************************************************************
	using Type = If_t<(sizeof(T1) < sizeof(T2)), T2, T1>;
	//**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HighTypeHelper class template for a complex and another type.
// \ingroup math_traits
*/
template <typename T1, typename T2> struct HighTypeHelper<T1, T2, EnableIf_t<IsComplex_v<T1> && !IsComplex_v<T2>>> {
  public:
	//**********************************************************************************************
	using Type = T1;
	//**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HighTypeHelper class template for another and a complex type.
// \ingroup math_traits
*/
template <typename T1, typename T2> struct HighTypeHelper<T1, T2, EnableIf_t<!IsComplex_v<T1> && IsComplex_v<T2>>> {
  public:
	//**********************************************************************************************
	using Type = T2;
	//**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HighType class template for two complex types.
// \ingroup math_traits
*/
template <typename T1, typename T2> struct HighTypeHelper<T1, T2, EnableIf_t<IsComplex_v<T1> && IsComplex_v<T2>>> {
  public:
	//**********************************************************************************************
	using Type = complex<typename HighTypeHelper<typename T1::value_type, typename T2::value_type>::Type>;
	//**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Base template for the HighType type trait.
// \ingroup math_traits
//
// \section hightype_general General
//
// The HighType class template determines the more significant, dominating data type of the two
// given data types \a T1 and \a T2. In case both \a T1 and \a T2 are built-in data types, the
// nested type \a Type is set to the larger or signed data type. If case both built-in types have
// the same size and the same signedness, the selected type is implementation defined. In case no
// higher data type can be selected, \a Type is set to \a INVALID_TYPE.
//
// Per default, the HighType template provides support for the following built-in data types:
//
// <ul>
//    <li>Integral types</li>
//    <ul>
//       <li>unsigned char, signed char, char, wchar_t</li>
//       <li>char16_t, char32_t</li>
//       <li>unsigned short, short</li>
//       <li>unsigned int, int</li>
//       <li>unsigned long, long</li>
//       <li>std::size_t, std::ptrdiff_t (for certain 64-bit compilers)</li>
//    </ul>
//    <li>Floating point types</li>
//    <ul>
//       <li>float</li>
//       <li>double</li>
//       <li>long double</li>
//    </ul>
// </ul>
//
// Additionally, the Metric numeric library provides specializations for the following user-defined
// arithmetic types, wherever a more significant data type can be selected:
//
// <ul>
//    <li>std::complex</li>
//    <li>mtrc::numeric::StaticVector</li>
//    <li>mtrc::numeric::HybridVector</li>
//    <li>mtrc::numeric::DynamicVector</li>
//    <li>mtrc::numeric::CompressedVector</li>
//    <li>mtrc::numeric::StaticMatrix</li>
//    <li>mtrc::numeric::HybridMatrix</li>
//    <li>mtrc::numeric::DynamicMatrix</li>
//    <li>mtrc::numeric::CompressedMatrix</li>
//    <li>mtrc::numeric::SymmetricMatrix</li>
//    <li>mtrc::numeric::HermitianMatrix</li>
//    <li>mtrc::numeric::LowerMatrix</li>
//    <li>mtrc::numeric::UniLowerMatrix</li>
//    <li>mtrc::numeric::StrictlyLowerMatrix</li>
//    <li>mtrc::numeric::UpperMatrix</li>
//    <li>mtrc::numeric::UniUpperMatrix</li>
//    <li>mtrc::numeric::StrictlyUpperMatrix</li>
//    <li>mtrc::numeric::DiagonalMatrix</li>
// </ul>
//
//
// \n \section hightype_specializations Creating custom specializations
//
// It is possible to specialize the HighType template for additional user-defined data types.
// The following example shows the according specialization for two dynamic column vectors:

   \code
   template< typename T1, typename T2 >
   struct HighType< DynamicVector<T1,false>, DynamicVector<T2,false> >
   {
	  using Type = DynamicVector< typename HighType<T1,T2>::Type, false >;
   };
   \endcode
*/
template <typename T1 // First operand
		  ,
		  typename T2 // Second operand
		  ,
		  typename = void> // Restricting condition
struct HighType {
  public:
	//**********************************************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	using Type = typename HighTypeHelper<T1, T2>::Type;
	/*! \endcond */
	//**********************************************************************************************
};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary alias declaration for the HighType type trait.
// \ingroup math_type_traits
//
// The HighType_t alias declaration provides a convenient shortcut to access the nested \a Type of
// the HighType class template. For instance, given the types \a T1 and \a T2 the following two
// type definitions are identical:

   \code
   using Type1 = typename mtrc::numeric::HighType<T1,T2>::Type;
   using Type2 = mtrc::numeric::HighType_t<T1,T2>;
   \endcode
*/
template <typename T1 // First operand
		  ,
		  typename T2> // Second operand
using HighType_t = typename HighType<T1, T2>::Type;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
