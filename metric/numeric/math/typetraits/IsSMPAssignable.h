// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISSMPASSIGNABLE_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISSMPASSIGNABLE_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/IntegralConstant.h>
#include <metric/numeric/util/typetraits/Void.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Auxiliary helper struct for the IsSMPAssignable type trait.
// \ingroup math_type_traits
*/
template <typename T, typename = void> struct IsSMPAssignableHelper : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename T>
struct IsSMPAssignableHelper<T, Void_t<decltype(T::smpAssignable)>> : public BoolConstant<T::smpAssignable> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compile time check for data types.
// \ingroup math_type_traits
//
// This type trait tests whether or not the given template parameter is an SMP-assignable data
// type (i.e. if it is a data type that can possibly and efficiently be assigned by several
// threads). In this context, built-in data types as well as complex numbers are non-SMP-assignable,
// whereas several vector and matrix types (as for instance DynamicVector and DynamicMatrix) can be
// SMP-assignable. If the type is SMP-assignable, the \a value member constant is set to \a true,
// the nested type definition \a Type is \a TrueType, and the class derives from \a TrueType.
// Otherwise \a value is set to \a false, \a Type is \a FalseType, and the class derives from
// \a FalseType.

   \code
   using mtrc::numeric::StaticVector;
   using mtrc::numeric::StaticMatrix;
   using mtrc::numeric::DynamicVector;
   using mtrc::numeric::DynamicMatrix;

   using VectorType = DynamicVector<int,columnVector>;

   VectorType a( 100UL );

   using SubvectorType = decltype( mtrc::numeric::subvector( a, 8UL, 16UL ) );

   mtrc::numeric::IsSMPAssignable< VectorType >::value            // Evaluates to 1
   mtrc::numeric::IsSMPAssignable< SubvectorType >::Type          // Results in TrueType
   mtrc::numeric::IsSMPAssignable< DynamicMatrix<int> >           // Is derived from TrueType
   mtrc::numeric::IsSMPAssignable< int >::value                   // Evaluates to 0
   mtrc::numeric::IsSMPAssignable< StaticVector<int,3UL> >::Type  // Results in FalseType
   mtrc::numeric::IsSMPAssignable< StaticMatrix<int,4UL,5UL> >    // Is derived from FalseType
   \endcode
*/
template <typename T> struct IsSMPAssignable : public BoolConstant<IsSMPAssignableHelper<T>::value> {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsSMPAssignable type trait.
// \ingroup math_type_traits
//
// The IsSMPAssignable_v variable template provides a convenient shortcut to access the nested
// \a value of the IsSMPAssignable class template. For instance, given the type \a T the
// following two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsSMPAssignable<T>::value;
   constexpr bool value2 = mtrc::numeric::IsSMPAssignable_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsSMPAssignable_v = IsSMPAssignable<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
