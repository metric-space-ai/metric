// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_TRANSPOSEFLAG_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_TRANSPOSEFLAG_H
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
/*!\brief Evaluation of the transpose flag of a given matrix type.
// \ingroup math_type_traits
//
// Via this type trait it is possible to evaluate the transpose flag of a given vector type. In
// case the given type is a row vector type the nested boolean \a value is set to \a rowVector,
// in case it is a column vector type it is set to \a columnVector. If the given type is not a
// vector type a compilation error is created.

   \code
   using RowVector    = mtrc::numeric::DynamicVector<int,mtrc::numeric::rowVector>;
   using ColumnVector = mtrc::numeric::DynamicVector<int,mtrc::numeric::columnVector>;

   mtrc::numeric::TransposeFlag<RowVector>::value     // Evaluates to mtrc::numeric::rowVector
   mtrc::numeric::TransposeFlag<ColumnVector>::value  // Evaluates to mtrc::numeric::columnVector
   mtrc::numeric::TransposeFlag<int>::value           // Compilation error!
   \endcode
*/
template <typename T> struct TransposeFlag : public BoolConstant<T::transposeFlag> {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the TransposeFlag type trait.
// \ingroup math_type_traits
//
// The TransposeFlag_v variable template provides a convenient shortcut to access the nested
// \a value of the TransposeFlag class template. For instance, given the vector type \a T the
// following two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::TransposeFlag<T>::value;
   constexpr bool value2 = mtrc::numeric::TransposeFlag_v<T>;
   \endcode
*/
template <typename T> constexpr bool TransposeFlag_v = T::transposeFlag;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
