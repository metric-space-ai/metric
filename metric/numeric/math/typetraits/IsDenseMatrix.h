// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISDENSEMATRIX_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISDENSEMATRIX_H
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
/*!\brief Auxiliary helper functions for the IsDenseMatrix type trait.
// \ingroup math_type_traits
*/
template <typename MT, bool SO> TrueType isDenseMatrix_backend(const volatile DenseMatrix<MT, SO> *);

FalseType isDenseMatrix_backend(...);
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compile time check for dense matrix types.
// \ingroup math_type_traits
//
// This type trait tests whether or not the given template parameter is a dense matrix type
// (i.e. whether \a T is derived from the DenseMatrix base class). In case the type is a dense
// matrix type, the \a value member constant is set to \a true, the nested type definition
// \a Type is \a TrueType, and the class derives from \a TrueType. Otherwise \a value is set
// to \a false, \a Type is \a FalseType, and the class derives from \a FalseType.

   \code
   using namespace mtrc::numeric;

   IsDenseMatrix< DynamicMatrix<double> >::value      // Evaluates to 1
   IsDenseMatrix< const DynamicMatrix<float> >::Type  // Results in TrueType
   IsDenseMatrix< volatile DynamicMatrix<int> >       // Is derived from TrueType
   IsDenseMatrix< CompressedMatrix<double>::value     // Evaluates to 0
   IsDenseMatrix< CompressedVector<double> >::Type    // Results in FalseType
   IsDenseMatrix< DynamicVector<double> >             // Is derived from FalseType
   \endcode
*/
template <typename T> struct IsDenseMatrix : public decltype(isDenseMatrix_backend(std::declval<T *>())){};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsDenseMatrix type trait for references.
// \ingroup math_type_traits
*/
template <typename T> struct IsDenseMatrix<T &> : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsDenseMatrix type trait.
// \ingroup math_type_traits
//
// The IsDenseMatrix_v variable template provides a convenient shortcut to access the nested
// \a value of the IsDenseMatrix class template. For instance, given the type \a T the
// following two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsDenseMatrix<T>::value;
   constexpr bool value2 = mtrc::numeric::IsDenseMatrix_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsDenseMatrix_v = IsDenseMatrix<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
