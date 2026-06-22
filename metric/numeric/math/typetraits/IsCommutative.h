// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISCOMMUTATIVE_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISCOMMUTATIVE_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsMatrix.h>
#include <metric/numeric/math/typetraits/IsVector.h>
#include <metric/numeric/math/typetraits/UnderlyingElement.h>
#include <metric/numeric/util/EnableIf.h>
#include <metric/numeric/util/IntegralConstant.h>
#include <metric/numeric/util/typetraits/IsNumeric.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Compile time check for the commutativity of data types.
// \ingroup math_type_traits
//
// This type trait tests whether or not the two given data types \a T1 and T2 are commutative
// with respect to mathematical operations. If the types are commutative, the \a value member
// constant is set to \a true, the nested type definition \a Type is \a TrueType, and the class
// derives from \a TrueType. Otherwise \a value is set to \a false, \a Type is \a FalseType,
// and the class derives from \a FalseType.

   \code
   using VT = StaticVector<int,3UL>;
   using MT = StaticMatrix<int,3UL,3UL>;

   mtrc::numeric::IsCommutative< double, complex<double> >::value                 // Evaluates to 1
   mtrc::numeric::IsCommutative< DynamicVector<int>, DynamicVector<int> >::Type   // Results in TrueType
   mtrc::numeric::IsCommutative< DynamicMatrix<VT>, DynamicMatrix<VT> >           // Is derived from TrueType
   mtrc::numeric::IsCommutative< DynamicMatrix<int>, DynamicVector<int> >::value  // Evaluates to 0
   mtrc::numeric::IsCommutative< DynamicVector<MT>, DynamicVector<VT> >::Type     // Results in FalseType
   mtrc::numeric::IsCommutative< DynamicMatrix<VT>, DynamicMatrix<MT> >           // Is derived from FalseType
   \endcode
*/
template <typename T1, typename T2, typename = void>
struct IsCommutative : public BoolConstant<IsNumeric_v<T1> || IsNumeric_v<T2>> {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsCommutative type trait for vectors and matrices.
// \ingroup math_type_traits
*/
template <typename T1, typename T2>
struct IsCommutative<T1, T2, EnableIf_t<(IsVector_v<T1> && IsVector_v<T2>) || (IsMatrix_v<T1> && IsMatrix_v<T2>)>>
	: public BoolConstant<IsCommutative<UnderlyingElement_t<T1>, UnderlyingElement_t<T2>>::value> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsCommutative type trait.
// \ingroup math_type_traits
//
// The IsCommutative_v variable template provides a convenient shortcut to access the nested
// \a value of the IsCommutative class template. For instance, given the type1 \a T1 and T2
// the following two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsCommutative<T1,T2>::value;
   constexpr bool value2 = mtrc::numeric::IsCommutative_v<T1,T2>;
   \endcode
*/
template <typename T1, typename T2> constexpr bool IsCommutative_v = IsCommutative<T1, T2>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
