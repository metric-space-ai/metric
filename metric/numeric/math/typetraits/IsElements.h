// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISELEMENTS_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISELEMENTS_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/views/Forward.h>
#include <metric/numeric/util/IntegralConstant.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Compile time check for element selections.
// \ingroup math_type_traits
//
// This type trait tests whether or not the given template parameter is an element selection (i.e.
// a view on elements of a dense or sparse vector). In case the type is an element selection, the
// \a value member constant is set to \a true, the nested type definition \a Type is \a TrueType,
// and the class derives from \a TrueType. Otherwise \a value is set to \a false, \a Type is
// \a FalseType, and the class derives from \a FalseType.

   \code
   using VectorType1 = mtrc::numeric::StaticVector<int,10UL>;
   using VectorType2 = mtrc::numeric::DynamicVector<double>;
   using VectorType3 = mtrc::numeric::CompressedVector<float>;

   VectorType1 a;
   VectorType2 b( 100UL );
   VectorType3 c( 200UL );

   using ElementsType1 = decltype( mtrc::numeric::elements<2UL,4UL>( a ) );
   using ElementsType2 = decltype( mtrc::numeric::elements( b, 8UL 24UL ) );
   using ElementsType3 = decltype( mtrc::numeric::elements( c, 5UL, 13UL ) );

   mtrc::numeric::IsElements< ElementsType1 >::value       // Evaluates to 1
   mtrc::numeric::IsElements< const ElementsType2 >::Type  // Results in TrueType
   mtrc::numeric::IsElements< volatile ElementsType3 >     // Is derived from TrueType
   mtrc::numeric::IsElements< VectorType1 >::value         // Evaluates to 0
   mtrc::numeric::IsElements< const VectorType2 >::Type    // Results in FalseType
   mtrc::numeric::IsElements< volatile VectorType3 >       // Is derived from FalseType
   \endcode
*/
template <typename T> struct IsElements : public FalseType {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsElements type trait for 'Elements'.
// \ingroup math_type_traits
*/
template <typename VT, bool TF, bool DF, typename... CEAs>
struct IsElements<Elements<VT, TF, DF, CEAs...>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsElements type trait for 'const Elements'.
// \ingroup math_type_traits
*/
template <typename VT, bool TF, bool DF, typename... CEAs>
struct IsElements<const Elements<VT, TF, DF, CEAs...>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsElements type trait for 'volatile Elements'.
// \ingroup math_type_traits
*/
template <typename VT, bool TF, bool DF, typename... CEAs>
struct IsElements<volatile Elements<VT, TF, DF, CEAs...>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsElements type trait for 'const volatile Elements'.
// \ingroup math_type_traits
*/
template <typename VT, bool TF, bool DF, typename... CEAs>
struct IsElements<const volatile Elements<VT, TF, DF, CEAs...>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsElements type trait.
// \ingroup math_type_traits
//
// The IsElements_v variable template provides a convenient shortcut to access the nested
// \a value of the IsElements class template. For instance, given the type \a T the following
// two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsElements<T>::value;
   constexpr bool value2 = mtrc::numeric::IsElements_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsElements_v = IsElements<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
