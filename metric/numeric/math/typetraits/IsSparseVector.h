// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISSPARSEVECTOR_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISSPARSEVECTOR_H
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
/*!\brief Auxiliary helper functions for the IsSparseVector type trait.
// \ingroup math_type_traits
*/
template <typename VT, bool TF> TrueType isSparseVector_backend(const volatile SparseVector<VT, TF> *);

FalseType isSparseVector_backend(...);
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compile time check for sparse vector types.
// \ingroup math_type_traits
//
// This type trait tests whether or not the given template parameter is a sparse vector type
// (i.e. whether \a T is derived from the SparseVector base class). In case the type is a sparse
// vector type, the \a value member constant is set to \a true, the nested type definition
// \a Type is \a TrueType, and the class derives from \a TrueType. Otherwise \a value is set
// to \a false, \a Type is \a FalseType, and the class derives from \a FalseType.

   \code
   using namespace mtrc::numeric;

   IsSparseVector< CompressedVector<double> >::value      // Evaluates to 1
   IsSparseVector< const CompressedVector<float> >::Type  // Results in TrueType
   IsSparseVector< volatile CompressedVector<int> >       // Is derived from TrueType
   IsSparseVector< DynamicVector<double> >::value         // Evaluates to 0
   IsSparseVector< const DynamicMatrix<double> >::Type    // Results in FalseType
   IsSparseVector< CompressedMatrix<double> >             // Is derived from FalseType
   \endcode
*/
template <typename T> struct IsSparseVector : public decltype(isSparseVector_backend(std::declval<T *>())){};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsSparseVector type trait for references.
// \ingroup math_type_traits
*/
template <typename T> struct IsSparseVector<T &> : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsSparseVector type trait.
// \ingroup math_type_traits
//
// The IsSparseVector_v variable template provides a convenient shortcut to access the nested
// \a value of the IsSparseVector class template. For instance, given the type \a T the
// following two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsSparseVector<T>::value;
   constexpr bool value2 = mtrc::numeric::IsSparseVector_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsSparseVector_v = IsSparseVector<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
