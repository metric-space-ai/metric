// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_TYPETRAITS_HAVESAMESIZE_H
#define METRIC_NUMERIC_UTIL_TYPETRAITS_HAVESAMESIZE_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/IntegralConstant.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Compile time size check.
// \ingroup type_traits
//
// This class offers the possibility to test the size of two types at compile time. If an object
// of type \a T1 has the same size as an object of type \a T2, the \a value member constant is
// set to \a true, the nested type definition \a Type is \a TrueType, and the class derives from
// \a TrueType. Otherwise \a value  is set to \a false, \a Type is \a FalseType, and the class
// derives from \a FalseType.

   \code
   mtrc::numeric::HaveSameSize<int,unsigned int>::value  // Evaluates to 'true'
   mtrc::numeric::HaveSameSize<int,unsigned int>::Type   // Results in TrueType
   mtrc::numeric::HaveSameSize<int,unsigned int>         // Is derived from TrueType
   mtrc::numeric::HaveSameSize<char,wchar_t>::value      // Evalutes to 'false'
   mtrc::numeric::HaveSameSize<char,wchar_t>::Type       // Results in FalseType
   mtrc::numeric::HaveSameSize<char,wchar_t>             // Is derived from FalseType
   \endcode

// One example for the application of this type trait is a compile time check if the compiler
// supports the 'Empty Derived class Optimization (EDO)':

   \code
   // Definition of the base class A
   struct A {
	  int i_;
   };

   // Definition of the derived class B
   struct B : public A {};

   // Testing whether or not an object of type B has the same size as the
   //   base class A and whether the compiler supports EDO
   mtrc::numeric::HaveSameSize( A, B );
   \endcode
*/
template <typename T1, typename T2> class HaveSameSize : public BoolConstant<sizeof(T1) == sizeof(T2)> {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Partial specialization of the compile time size constraint.
// \ingroup type_traits
//
// This class is a partial specialization of the HaveSameSize template for the type \a void
// as first template argument. The \a value member constant is automatically set to \a false,
// the nested type definition \a Type is \a FalseType, and the class derives from \a FalseType
// for any given type \a T since the \a void type has no size.
*/
template <typename T> class HaveSameSize<void, T> : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Partial specialization of the compile time size constraint.
// \ingroup type_traits
//
// This class is a partial specialization of the HaveSameSize template for the type \a void
// as second template argument. The \a value member constant is automatically set to \a false,
// the nested type definition \a Type is \a FalseType, and the class derives from \a FalseType
// for any given type \a T since the \a void type has no size.
*/
template <typename T> class HaveSameSize<T, void> : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Full specialization of the compile time size constraint.
// \ingroup type_traits
//
// This class is a full specialization of the HaveSameSize template for the type \a void
// as first and second template argument. The \a value member constant is automatically set
// to \a true, the nested type definition \a Type is \a TrueType, and the class derives from
// \a TrueType since both arguments are \a void.
*/
template <> class HaveSameSize<void, void> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the HaveSameSize type trait.
// \ingroup type_traits
//
// The HaveSameSize_v variable template provides a convenient shortcut to access the nested
// \a value of the HaveSameSize class template. For instance, given the typew \a T1 and \a T2
// the following two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::HaveSameSize<T1,T2>::value;
   constexpr bool value2 = mtrc::numeric::HaveSameSize_v<T1,T2>;
   \endcode
*/
template <typename T1, typename T2> constexpr bool HaveSameSize_v = HaveSameSize<T1, T2>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
