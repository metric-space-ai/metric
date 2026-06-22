// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISROWMAJORMATRIX_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISROWMAJORMATRIX_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/StorageOrder.h>
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
/*!\brief Auxiliary helper functions for the IsRowMajorMatrix type trait.
// \ingroup math_type_traits
*/
template <typename MT> TrueType isRowMajorMatrix_backend(const volatile Matrix<MT, rowMajor> *);

FalseType isRowMajorMatrix_backend(...);
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compile time check for row-major matrix types.
// \ingroup math_type_traits
//
// This type trait tests whether or not the given template argument is a row-major dense or
// sparse matrix type (i.e., a matrix whose storage order is set to \a true). In case the type
// is a row-major matrix type, the \a value member constant is set to \a true, the nested type
// definition \a Type is \a TrueType, and the class derives from \a TrueType. Otherwise \a value
// is set to \a false, \a Type is \a FalseType, and the class derives from \a FalseType.

   \code
   using mtrc::numeric::StaticMatrix;
   using mtrc::numeric::DynamicMatrix;
   using mtrc::numeric::CompressedMatrix;
   using mtrc::numeric::rowMajor;
   using mtrc::numeric::columnMajor;

   mtrc::numeric::IsRowMajorMatrix< StaticMatrix<float,3U,3U,rowMajor> >::value      // Evaluates to 1
   mtrc::numeric::IsRowMajorMatrix< const DynamicMatrix<double,rowMajor> >::Type     // Results in TrueType
   mtrc::numeric::IsRowMajorMatrix< volatile CompressedMatrix<int,rowMajor> >        // Is derived from TrueType
   mtrc::numeric::IsRowMajorMatrix< StaticMatrix<float,3U,3U,columnMajor> >::value   // Evaluates to 0
   mtrc::numeric::IsRowMajorMatrix< const DynamicMatrix<double,columnMajor> >::Type  // Results in FalseType
   mtrc::numeric::IsRowMajorMatrix< volatile CompressedMatrix<int,columnMajor> >     // Is derived from FalseType
   \endcode
*/
template <typename T> struct IsRowMajorMatrix : public decltype(isRowMajorMatrix_backend(std::declval<T *>())){};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsRowMajorMatrix type trait for references.
// \ingroup math_type_traits
*/
template <typename T> struct IsRowMajorMatrix<T &> : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsRowMajorMatrix type trait.
// \ingroup math_type_traits
//
// The IsRowMajorMatrix_v variable template provides a convenient shortcut to access the nested
// \a value of the IsRowMajorMatrix class template. For instance, given the type \a T the
// following two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsRowMajorMatrix<T>::value;
   constexpr bool value2 = mtrc::numeric::IsRowMajorMatrix_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsRowMajorMatrix_v = IsRowMajorMatrix<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
