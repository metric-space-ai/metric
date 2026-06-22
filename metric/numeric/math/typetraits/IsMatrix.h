// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISMATRIX_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISMATRIX_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

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
/*!\brief Auxiliary helper functions for the IsMatrix type trait.
// \ingroup math_type_traits
*/
template <typename MT, bool SO> TrueType isMatrix_backend(const volatile Matrix<MT, SO> *);

FalseType isMatrix_backend(...);
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compile time check for matrix types.
// \ingroup math_type_traits
//
// This type trait tests whether or not the given template parameter is a dense or sparse matrix
// type (i.e. whether \a T is derived from the Matrix base class). In case the type is a matrix
// type, the \a value member constant is set to \a true, the nested type definition \a Type is
// \a TrueType, and the class derives from \a TrueType. Otherwise \a yes is set to \a false,
// \a Type is \a FalseType, and the class derives from \a FalseType.

   \code
   using namespace mtrc::numeric;

   IsMatrix< StaticMatrix<float,3U,3U> >::value   // Evaluates to 1
   IsMatrix< const DynamicMatrix<double> >::Type  // Results in TrueType
   IsMatrix< volatile CompressedMatrix<int> >     // Is derived from TrueType
   IsMatrix< StaticVector<float,3U> >::value      // Evaluates to 0
   IsMatrix< const DynamicVector<double> >::Type  // Results in FalseType
   IsMatrix< volatile CompressedVector<int> >     // Is derived from FalseType
   \endcode
*/
template <typename T> struct IsMatrix : public decltype(isMatrix_backend(std::declval<T *>())){};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsMatrix type trait for references.
// \ingroup math_type_traits
*/
template <typename T> struct IsMatrix<T &> : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsMatrix type trait.
// \ingroup math_type_traits
//
// The IsMatrix_v variable template provides a convenient shortcut to access the nested \a value
// of the IsMatrix class template. For instance, given the type \a T the following two statements
// are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsMatrix<T>::value;
   constexpr bool value2 = mtrc::numeric::IsMatrix_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsMatrix_v = IsMatrix<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
