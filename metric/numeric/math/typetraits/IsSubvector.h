// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISSUBVECTOR_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISSUBVECTOR_H
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
/*!\brief Compile time check for subvectors.
// \ingroup math_type_traits
//
// This type trait tests whether or not the given template parameter is a subvector (i.e. a view
// on the part of a dense or sparse vector). In case the type is a subvector, the \a value member
// constant is set to \a true, the nested type definition \a Type is \a TrueType, and the class
// derives from \a TrueType. Otherwise \a value is set to \a false, \a Type is \a FalseType, and
// the class derives from \a FalseType.

   \code
   using mtrc::numeric::aligned;

   using VectorType1 = mtrc::numeric::StaticVector<int,10UL>;
   using VectorType2 = mtrc::numeric::DynamicVector<double>;
   using VectorType3 = mtrc::numeric::CompressedVector<float>;

   VectorType1 a;
   VectorType2 b( 100UL );
   VectorType3 c( 200UL );

   using SubvectorType1 = decltype( mtrc::numeric::subvector<2UL,4UL>( a ) );
   using SubvectorType2 = decltype( mtrc::numeric::subvector<aligned>( b, 8UL 24UL ) );
   using SubvectorType3 = decltype( mtrc::numeric::subvector( c, 5UL, 13UL ) );

   mtrc::numeric::IsSubvector< SubvectorType1 >::value       // Evaluates to 1
   mtrc::numeric::IsSubvector< const SubvectorType2 >::Type  // Results in TrueType
   mtrc::numeric::IsSubvector< volatile SubvectorType3 >     // Is derived from TrueType
   mtrc::numeric::IsSubvector< VectorType1 >::value          // Evaluates to 0
   mtrc::numeric::IsSubvector< const VectorType2 >::Type     // Results in FalseType
   mtrc::numeric::IsSubvector< volatile VectorType3 >        // Is derived from FalseType
   \endcode
*/
template <typename T> struct IsSubvector : public FalseType {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsSubvector type trait for 'Subvector'.
// \ingroup math_type_traits
*/
template <typename VT, AlignmentFlag AF, bool TF, bool DF, size_t... CSAs>
struct IsSubvector<Subvector<VT, AF, TF, DF, CSAs...>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsSubvector type trait for 'const Subvector'.
// \ingroup math_type_traits
*/
template <typename VT, AlignmentFlag AF, bool TF, bool DF, size_t... CSAs>
struct IsSubvector<const Subvector<VT, AF, TF, DF, CSAs...>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsSubvector type trait for 'volatile Subvector'.
// \ingroup math_type_traits
*/
template <typename VT, AlignmentFlag AF, bool TF, bool DF, size_t... CSAs>
struct IsSubvector<volatile Subvector<VT, AF, TF, DF, CSAs...>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsSubvector type trait for 'const volatile Subvector'.
// \ingroup math_type_traits
*/
template <typename VT, AlignmentFlag AF, bool TF, bool DF, size_t... CSAs>
struct IsSubvector<const volatile Subvector<VT, AF, TF, DF, CSAs...>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsSubvector type trait.
// \ingroup math_type_traits
//
// The IsSubvector_v variable template provides a convenient shortcut to access the nested
// \a value of the IsSubvector class template. For instance, given the type \a T the following
// two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsSubvector<T>::value;
   constexpr bool value2 = mtrc::numeric::IsSubvector_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsSubvector_v = IsSubvector<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
