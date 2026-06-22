// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_HASIMAG_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_HASIMAG_H
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
/*!\brief Auxiliary helper struct for the HasImag type trait.
// \ingroup math_type_traits
*/
template <typename T, typename = void> struct HasImagHelper : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasImagHelper type trait for types providing the imag() operation.
// \ingroup math_type_traits
*/
template <typename T> struct HasImagHelper<T, Void_t<decltype(imag(std::declval<T>()))>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Availability of the imag() operation for the given data types.
// \ingroup math_type_traits
//
// This type trait provides the information whether the imag() operation exists for the given
// data type \a T (taking the cv-qualifiers into account). In case the operation is available,
// the \a value member constant is set to \a true, the nested type definition \a Type is
// \a TrueType, and the class derives from \a TrueType. Otherwise \a value is set to \a false,
// \a Type is \a FalseType, and the class derives from \a FalseType.

   \code
   struct NoImag {};  // Definition of a type without the imag() operation

   mtrc::numeric::HasImag< int >::value                  // Evaluates to 1
   mtrc::numeric::HasImag< DynamicVector<float> >::Type  // Results in TrueType
   mtrc::numeric::HasImag< DynamicMatrix<double> >       // Is derived from TrueType
   mtrc::numeric::HasImag< NoImag >::value               // Evaluates to 0
   mtrc::numeric::HasImag< NoImag >::Type                // Results in FalseType
   mtrc::numeric::HasImag< NoImag >                      // Is derived from FalseType
   \endcode
*/
template <typename T, typename = void> struct HasImag : public HasImagHelper<T> {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasImag type trait for vectors.
// \ingroup math_type_traits
*/
template <typename T> struct HasImag<T, EnableIf_t<IsVector_v<T>>> : public HasImag<typename T::ElementType> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the HasImag type trait for matrices.
// \ingroup math_type_traits
*/
template <typename T> struct HasImag<T, EnableIf_t<IsMatrix_v<T>>> : public HasImag<typename T::ElementType> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the HasImag type trait.
// \ingroup math_type_traits
//
// The HasImag_v variable template provides a convenient shortcut to access the nested \a value
// of the HasImag class template. For instance, given the type \a T the following two statements
// are identical:

   \code
   constexpr bool value1 = mtrc::numeric::HasImag<T>::value;
   constexpr bool value2 = mtrc::numeric::HasImag_v<T>;
   \endcode
*/
template <typename T> constexpr bool HasImag_v = HasImag<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
