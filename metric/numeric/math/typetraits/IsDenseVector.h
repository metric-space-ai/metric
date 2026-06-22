// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISDENSEVECTOR_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISDENSEVECTOR_H
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
/*!\brief Auxiliary helper functions for the IsDenseVector type trait.
// \ingroup math_type_traits
*/
template <typename VT, bool TF> TrueType isDenseVector_backend(const volatile DenseVector<VT, TF> *);

FalseType isDenseVector_backend(...);
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compile time check for dense vector types.
// \ingroup math_type_traits
//
// This type trait tests whether or not the given template parameter is a dense vector type
// (i.e. whether \a T is derived from the DenseVector base class). In case the type is a dense
// vector type, the \a value member constant is set to \a true, the nested type definition
// \a Type is \a TrueType, and the class derives from \a TrueType. Otherwise \a value is set
// to \a false, \a Type is \a FalseType, and the class derives from \a FalseType.

   \code
   using namespace mtrc::numeric;

   IsDenseVector< DynamicVector<double> >::value        // Evaluates to 1
   IsDenseVector< const StaticVector<float,3U> >::Type  // Results in TrueType
   IsDenseVector< volatile StaticVector<int,6U> >       // Is derived from TrueType
   IsDenseVector< CompressedVector<double> >::value     // Evaluates to 0
   IsDenseVector< CompressedMatrix<double> >::Type      // Results in FalseType
   IsDenseVector< DynamicMatrix<double> >               // Is derived from FalseType
   \endcode
*/
template <typename T> struct IsDenseVector : public decltype(isDenseVector_backend(std::declval<T *>())){};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the IsDenseVector type trait for references.
// \ingroup math_type_traits
*/
template <typename T> struct IsDenseVector<T &> : public FalseType {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsDenseVector type trait.
// \ingroup math_type_traits
//
// The IsDenseVector_v variable template provides a convenient shortcut to access the nested
// \a value of the IsDenseVector class template. For instance, given the type \a T the
// following two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsDenseVector<T>::value;
   constexpr bool value2 = mtrc::numeric::IsDenseVector_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsDenseVector_v = IsDenseVector<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
