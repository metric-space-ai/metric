// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISCOLUMNVECTOR_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISCOLUMNVECTOR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/TransposeFlag.h>
#include <metric/numeric/math/expressions/Forward.h>
#include <metric/numeric/util/IntegralConstant.h>
#include <utility>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Auxiliary helper functions for the IsColumnVector type trait.
// \ingroup math_type_traits
*/
template <typename VT> TrueType isColumnVector_backend(const volatile Vector<VT, columnVector> *);

FalseType isColumnVector_backend(...);
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compile time check for column vector types.
// \ingroup math_type_traits
//
// This type trait tests whether or not the given template argument is a column dense or sparse
// vector type (i.e. a vector whose transposition flag is set to mtrc::numeric::columnVector). In case
// the type is a column vector type, the \a value member constant is set to \a true, the nested
// type definition \a Type is \a TrueType, and the class derives from \a TrueType. Otherwise
// \a value is set to \a false, \a Type is \a FalseType, and the class derives from \a FalseType.

   \code
   using mtrc::numeric::columnVector;
   using mtrc::numeric::rowVector;

   mtrc::numeric::IsColumnVector< StaticVector<float,3U,columnVector> >::value      // Evaluates to 1
   mtrc::numeric::IsColumnVector< const DynamicVector<double,columnVector> >::Type  // Results in TrueType
   mtrc::numeric::IsColumnVector< volatile CompressedVector<int,columnVector> >     // Is derived from TrueType
   mtrc::numeric::IsColumnVector< StaticVector<float,3U,rowVector> >::value         // Evaluates to 0
   mtrc::numeric::IsColumnVector< const DynamicVector<double,rowVector> >::Type     // Results in FalseType
   mtrc::numeric::IsColumnVector< volatile CompressedVector<int,rowVector> >        // Is derived from FalseType
   \endcode
*/
template <typename T> struct IsColumnVector : public decltype(isColumnVector_backend(std::declval<T *>())){};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsColumnVector type trait for references.
// \ingroup math_type_traits
*/
template <typename T> struct IsColumnVector<T &> : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsColumnVector type trait.
// \ingroup math_type_traits
//
// The IsColumnVector_v variable template provides a convenient shortcut to access the nested
// \a value of the IsColumnVector class template. For instance, given the type \a T the
// following two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsColumnVector<T>::value;
   constexpr bool value2 = mtrc::numeric::IsColumnVector_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsColumnVector_v = IsColumnVector<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
