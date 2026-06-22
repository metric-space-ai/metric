// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISCOLUMNMAJORMATRIX_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISCOLUMNMAJORMATRIX_H
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
/*!\brief Auxiliary helper functions for the IsColumnMajorMatrix type trait.
// \ingroup math_type_traits
*/
template <typename MT> TrueType isColumnMajorMatrix_backend(const volatile Matrix<MT, columnMajor> *);

FalseType isColumnMajorMatrix_backend(...);
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compile time check for column-major matrix types.
// \ingroup math_type_traits
//
// This type trait tests whether or not the given template argument is a column-major dense or
// sparse matrix type (i.e., a matrix whose storage order is set to \a true). In case the type
// is a column-major matrix type, the \a value member constant is set to \a true, the nested
// type definition \a Type is \a TrueType, and the class derives from \a TrueType. Otherwise
// \a value is set to \a false, \a Type is \a FalseType, and the class derives from \a FalseType.

   \code
   using mtrc::numeric::StaticMatrix;
   using mtrc::numeric::DynamicMatrix;
   using mtrc::numeric::CompressedMatrix;
   using mtrc::numeric::columnMajor;
   using mtrc::numeric::rowMajor;

   mtrc::numeric::IsColumnMajorMatrix< StaticMatrix<float,3U,3U,columnMajor> >::value   // Evaluates to 1
   mtrc::numeric::IsColumnMajorMatrix< const DynamicMatrix<double,columnMajor> >::Type  // Results in TrueType
   mtrc::numeric::IsColumnMajorMatrix< volatile CompressedMatrix<int,columnMajor> >     // Is derived from TrueType
   mtrc::numeric::IsColumnMajorMatrix< StaticMatrix<float,3U,3U,rowMajor> >::value      // Evaluates to 0
   mtrc::numeric::IsColumnMajorMatrix< const DynamicMatrix<double,rowMajor> >::Type     // Results in FalseType
   mtrc::numeric::IsColumnMajorMatrix< volatile CompressedMatrix<int,rowMajor> >        // Is derived from FalseType
   \endcode
*/
template <typename T> struct IsColumnMajorMatrix : public decltype(isColumnMajorMatrix_backend(std::declval<T *>())){};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsColumnMajorMatrix type trait for references.
// \ingroup math_type_traits
*/
template <typename T> struct IsColumnMajorMatrix<T &> : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsColumnMajorMatrix type trait.
// \ingroup math_type_traits
//
// The IsColumnMajorMatrix_v variable template provides a convenient shortcut to access the nested
// \a value of the IsColumnMajorMatrix class template. For instance, given the type \a T the
// following two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsColumnMajorMatrix<T>::value;
   constexpr bool value2 = mtrc::numeric::IsColumnMajorMatrix_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsColumnMajorMatrix_v = IsColumnMajorMatrix<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
