// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_MAXSIZE_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_MAXSIZE_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/IntegralConstant.h>
#include <metric/numeric/util/Types.h>
#include <metric/numeric/util/typetraits/Void.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  mtrc::numeric NAMESPACE FORWARD DECLARATIONS
//
//=================================================================================================

template <typename, size_t, typename = void> struct MaxSizeHelper1;
template <typename, size_t, typename> struct MaxSizeHelper2;

//=================================================================================================
//
//  TYPE DEFINITIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Default size of the MaxSize type trait.
// \ingroup math_type_traits
*/
constexpr ptrdiff_t DefaultMaxSize_v = -1L;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Type representation of the default size of the MaxSize type trait.
// \ingroup math_type_traits
*/
using DefaultMaxSize = Ptrdiff_t<DefaultMaxSize_v>;
//*************************************************************************************************

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Compile time evaluation of the maximum size of vectors and matrices.
// \ingroup math_type_traits
//
// The MaxSize type trait evaluates the maximum size of a particular dimension of the given
// vector or matrix type at compile time. In case the given type \a T is a vector or matrix type
// with a fixed maximum size (e.g. StaticVector, HybridVector, StaticMatrix, or HybridMatrix)
// and \a N is a valid dimension, the \a value member constant is set to the according size.
// In all other cases, \a value is set to -1.

   \code
   using mtrc::numeric::StaticVector;
   using mtrc::numeric::HybridMatrix;
   using mtrc::numeric::DynamicVector;

   mtrc::numeric::MaxSize< StaticVector<int,3UL>, 0UL >::value      // Evaluates to 3
   mtrc::numeric::MaxSize< HybridMatrix<int,2UL,4UL>, 0UL >::value  // Evaluates to 2 (the number of rows)
   mtrc::numeric::MaxSize< HybridMatrix<int,2UL,4UL>, 1UL >::value  // Evaluates to 4 (the number of columns)
   mtrc::numeric::MaxSize< StaticVector<int,3UL>, 1UL >::value      // Evaluates to -1; 1 is not a valid vector
dimension! mtrc::numeric::MaxSize< DynamicVector<int>, 0UL >::value         // Evaluates to -1; Maximum size not fixed
at compile time! mtrc::numeric::MaxSize< int, 0UL >::value                        // Evaluates to -1
   \endcode
*/
template <typename T, size_t N> struct MaxSize : public MaxSizeHelper1<T, N> {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the MaxSize type trait for const types.
// \ingroup math_type_traits
*/
template <typename T, size_t N> struct MaxSize<const T, N> : public MaxSize<T, N> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the MaxSize type trait for volatile types.
// \ingroup math_type_traits
*/
template <typename T, size_t N> struct MaxSize<volatile T, N> : public MaxSize<T, N> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the MaxSize type trait for cv qualified types.
// \ingroup math_type_traits
*/
template <typename T, size_t N> struct MaxSize<const volatile T, N> : public MaxSize<T, N> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the MaxSize type trait.
// \ingroup math_type_traits
//
// The MaxSize_v variable template provides a convenient shortcut to access the nested \a value
// of the MaxSize class template. For instance, given the type \a T and the dimension \a N the
// following two statements are identical:

   \code
   constexpr size_t value1 = mtrc::numeric::MaxSize<T,N>::value;
   constexpr size_t value2 = mtrc::numeric::MaxSize_v<T,N>;
   \endcode
*/
template <typename T, size_t N> constexpr ptrdiff_t MaxSize_v = MaxSize<T, N>::value;
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief First auxiliary helper struct for the MaxSize type trait.
// \ingroup math_type_traits
*/
template <typename T, size_t N, typename> struct MaxSizeHelper1 : public DefaultMaxSize {};

template <typename T, size_t N>
struct MaxSizeHelper1<T, N, Void_t<typename T::ResultType>> : public MaxSizeHelper2<T, N, typename T::ResultType> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Second auxiliary helper struct for the MaxSize type trait.
// \ingroup math_type_traits
*/
template <typename T, size_t N, typename U> struct MaxSizeHelper2 : public MaxSize<U, N> {};

template <typename T, size_t N> struct MaxSizeHelper2<T, N, T> : public DefaultMaxSize {};
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
