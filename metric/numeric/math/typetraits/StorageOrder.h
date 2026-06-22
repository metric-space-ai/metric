// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_STORAGEORDER_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_STORAGEORDER_H
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
/*!\brief Evaluation of the storage order of a given matrix type.
// \ingroup math_type_traits
//
// Via this type trait it is possible to evaluate the storage order of a given matrix type.
// In case the given type is a row-major matrix type the nested boolean \a value is set to
// \a rowMajor, in case it is a column-major matrix type it is set to \a columnMajor. If the
// given type is not a matrix type a compilation error is created.

   \code
   using RowMajorMatrix    = mtrc::numeric::DynamicMatrix<int,mtrc::numeric::rowMajor>;
   using ColumnMajorMatrix = mtrc::numeric::DynamicMatrix<int,mtrc::numeric::columnMajor>;

   mtrc::numeric::StorageOrder<RowMajorMatrix>::value     // Evaluates to mtrc::numeric::rowMajor
   mtrc::numeric::StorageOrder<ColumnMajorMatrix>::value  // Evaluates to mtrc::numeric::columnMajor
   mtrc::numeric::StorageOrder<int>::value                // Compilation error!
   \endcode
*/
template <typename T> struct StorageOrder : public BoolConstant<T::storageOrder> {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the StorageOrder type trait.
// \ingroup math_type_traits
//
// The StorageOrder_v variable template provides a convenient shortcut to access the nested
// \a value of the StorageOrder class template. For instance, given the matrix type \a T the
// following two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::StorageOrder<T>::value;
   constexpr bool value2 = mtrc::numeric::StorageOrder_v<T>;
   \endcode
*/
template <typename T> constexpr bool StorageOrder_v = T::storageOrder;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
