// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_HASASIN_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_HASASIN_H
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
/*!\brief Auxiliary helper struct for the HasAsin type trait.
// \ingroup math_type_traits
*/
template <typename T, typename = void> struct HasAsinHelper : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasAsinHelper type trait for types providing the asin() operation.
// \ingroup math_type_traits
*/
template <typename T> struct HasAsinHelper<T, Void_t<decltype(asin(std::declval<T>()))>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Availability of the asin() operation for the given data types.
// \ingroup math_type_traits
//
// This type trait provides the information whether the asin() operation exists for the given
// data type \a T (taking the cv-qualifiers into account). In case the operation is available,
// the \a value member constant is set to \a true, the nested type definition \a Type is
// \a TrueType, and the class derives from \a TrueType. Otherwise \a value is set to \a false,
// \a Type is \a FalseType, and the class derives from \a FalseType.

   \code
   struct NoAsin {};  // Definition of a type without the asin() operation

   mtrc::numeric::HasAsin< int >::value                  // Evaluates to 1
   mtrc::numeric::HasAsin< DynamicVector<float> >::Type  // Results in TrueType
   mtrc::numeric::HasAsin< DynamicMatrix<double> >       // Is derived from TrueType
   mtrc::numeric::HasAsin< NoAsin >::value               // Evaluates to 0
   mtrc::numeric::HasAsin< NoAsin >::Type                // Results in FalseType
   mtrc::numeric::HasAsin< NoAsin >                      // Is derived from FalseType
   \endcode
*/
template <typename T, typename = void> struct HasAsin : public HasAsinHelper<T> {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasAsin type trait for vectors.
// \ingroup math_type_traits
*/
template <typename T> struct HasAsin<T, EnableIf_t<IsVector_v<T>>> : public HasAsin<typename T::ElementType> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasAsin type trait for matrices.
// \ingroup math_type_traits
*/
template <typename T> struct HasAsin<T, EnableIf_t<IsMatrix_v<T>>> : public HasAsin<typename T::ElementType> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the HasAsin type trait.
// \ingroup math_type_traits
//
// The HasAsin_v variable template provides a convenient shortcut to access the nested \a value
// of the HasAsin class template. For instance, given the type \a T the following two statements
// are identical:

   \code
   constexpr bool value1 = mtrc::numeric::HasAsin<T>::value;
   constexpr bool value2 = mtrc::numeric::HasAsin_v<T>;
   \endcode
*/
template <typename T> constexpr bool HasAsin_v = HasAsin<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
