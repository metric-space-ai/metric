// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_ISSPARSEELEMENT_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_ISSPARSEELEMENT_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/sparse/SparseElement.h>
#include <metric/numeric/util/IntegralConstant.h>
#include <metric/numeric/util/typetraits/IsBaseOf.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Compile time check whether the given type is a sparse element type.
// \ingroup math_type_traits
//
// This type trait class tests whether or not the given type \a Type is a Metric numeric sparse element
// type, i.e. if the type implements the sparse element concept by providing a value() and an
// index() member function. In order to qualify as a valid sparse element type, the given type
// has to derive (publicly or privately) from the SparseElement base class. In case the given
// type is a valid sparse element, the \a value member constant is set to \a true, the nested
// type definition \a Type is \a TrueType, and the class derives from \a TrueType. Otherwise
// \a value is set to \a false, \a Type is \a FalseType, and the class derives from \a FalseType.
*/
template <typename T>
struct IsSparseElement : public BoolConstant<IsBaseOf_v<SparseElement, T> && !IsBaseOf_v<T, SparseElement>> {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the IsSparseElement type trait.
// \ingroup math_type_traits
//
// The IsSparseElement_v variable template provides a convenient shortcut to access the nested
// \a value of the IsSparseElement class template. For instance, given the type \a T the
// following two statements are identical:

   \code
   constexpr bool value1 = mtrc::numeric::IsSparseElement<T>::value;
   constexpr bool value2 = mtrc::numeric::IsSparseElement_v<T>;
   \endcode
*/
template <typename T> constexpr bool IsSparseElement_v = IsSparseElement<T>::value;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
