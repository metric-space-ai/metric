// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_HASCONJ_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_HASCONJ_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/typetraits/IsMatrix.h>
#include <metric/numeric/math/typetraits/IsVector.h>
#include <metric/numeric/util/EnableIf.h>
#include <metric/numeric/util/IntegralConstant.h>
#include <metric/numeric/util/typetraits/Void.h>
#include <utility>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Auxiliary helper struct for the HasConj type trait.
// \ingroup math_type_traits
*/
template <typename T, typename = void> struct HasConjHelper : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasConjHelper type trait for types providing the conj() operation.
// \ingroup math_type_traits
*/
template <typename T> struct HasConjHelper<T, Void_t<decltype(conj(std::declval<T>()))>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Availability of the conj() operation for the given data types.
// \ingroup math_type_traits
//
// This type trait provides the information whether the conj() operation exists for the given
// data type \a T (taking the cv-qualifiers into account). In case the operation is available,
// the \a value member constant is set to \a true, the nested type definition \a Type is
// \a TrueType, and the class derives from \a TrueType. Otherwise \a value is set to \a false,
// \a Type is \a FalseType, and the class derives from \a FalseType.

   \code
   struct NoConj {};  // Definition of a type without the conj() operation

   mtrc::numeric::HasConj< int >::value                  // Evaluates to 1
   mtrc::numeric::HasConj< DynamicVector<float> >::Type  // Results in TrueType
   mtrc::numeric::HasConj< DynamicMatrix<double> >       // Is derived from TrueType
   mtrc::numeric::HasConj< NoConj >::value               // Evaluates to 0
   mtrc::numeric::HasConj< NoConj >::Type                // Results in FalseType
   mtrc::numeric::HasConj< NoConj >                      // Is derived from FalseType
   \endcode
*/
template <typename T, typename = void> struct HasConj : public HasConjHelper<T> {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasConj type trait for vectors.
// \ingroup math_type_traits
*/
template <typename T> struct HasConj<T, EnableIf_t<IsVector_v<T>>> : public HasConj<typename T::ElementType> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasConj type trait for matrices.
// \ingroup math_type_traits
*/
template <typename T> struct HasConj<T, EnableIf_t<IsMatrix_v<T>>> : public HasConj<typename T::ElementType> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the HasConj type trait.
// \ingroup math_type_traits
//
// The HasConj_v variable template provides a convenient shortcut to access the nested \a value
// of the HasConj class template. For instance, given the type \a T the following two statements
// are identical:

   \code
   constexpr bool value1 = mtrc::numeric::HasConj<T>::value;
   constexpr bool value2 = mtrc::numeric::HasConj_v<T>;
   \endcode
*/
template <typename T> constexpr bool HasConj_v = HasConj<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
