// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISSPARSEMATRIX_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISSPARSEMATRIX_H
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
/*!\brief Auxiliary helper functions for the IsSparseMatrix type trait.
// \ingroup math_type_traits
*/
template <typename MT, bool SO> TrueType isSparseMatrix_backend(const volatile SparseMatrix<MT, SO> *);

FalseType isSparseMatrix_backend(...);
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compile time check for sparse matrix types.
// \ingroup math_type_traits
//
// This type trait tests whether or not the given template parameter is a sparse matrix type
// (i.e. whether \a T is derived from the SparseMatrix base class). In case the type is a sparse
// matrix type, the \a value member constant is set to \a true, the nested type definition
// \a Type is \a TrueType, and the class derives from \a TrueType. Otherwise \a value is set
// to \a false, \a Type is \a FalseType, and the class derives from \a FalseType.

   \code
   using namespace mtrc::numeric;

   IsSparseMatrix< CompressedMatrix<double> >::value      // Evaluates to 1
   IsSparseMatrix< const CompressedMatrix<float> >::Type  // Results in TrueType
   IsSparseMatrix< volatile CompressedMatrix<int> >       // Is derived from TrueType
   IsSparseMatrix< DynamicVector<double> >::value         // Evaluates to 0
   IsSparseMatrix< const DynamicMatrix<double> >::Type    // Results in FalseType
   IsSparseMatrix< CompressedVector<double> >             // Is derived from FalseType
   \endcode
*/
template <typename T> struct IsSparseMatrix : public decltype(isSparseMatrix_backend(std::declval<T *>())){};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsSparseMatrix type trait for references.
// \ingroup math_type_traits
*/
template <typename T> struct IsSparseMatrix<T &> : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsSparseMatrix type trait.
// \ingroup math_type_traits
//
// The IsSparseMatrix_v variable template provides a convenient shortcut to access the nested
// \a value of the IsSparseMatrix class template. For instance, given the type \a T the
// following two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsSparseMatrix<T>::value;
   constexpr bool value2 = mtrc::numeric::IsSparseMatrix_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsSparseMatrix_v = IsSparseMatrix<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
