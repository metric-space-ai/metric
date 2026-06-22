// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_SIZE_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_SIZE_H
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

template <typename, size_t, typename = void> struct SizeHelper1;
template <typename, size_t, typename> struct SizeHelper2;

//=================================================================================================
//
//  TYPE DEFINITIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Default size of the Size type trait.
// \ingroup math_type_traits
*/
constexpr ptrdiff_t DefaultSize_v = -1L;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Type representation of the default size of the Size type trait.
// \ingroup math_type_traits
*/
using DefaultSize = Ptrdiff_t<DefaultSize_v>;
//*************************************************************************************************

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Compile time evaluation of the size of vectors and matrices.
// \ingroup math_type_traits
//
// The Size type trait evaluates the size of a particular dimension of the given vector or matrix
// type at compile time. In case the given type \a T is a vector or matrix type with a fixed size
// (e.g. StaticVector or StaticMatrix) and \a N is a valid dimension, the \a value member constant
// is set to the according size. In all other cases, \a value is set to -1.

   \code
   using mtrc::numeric::StaticVector;
   using mtrc::numeric::StaticMatrix;
   using mtrc::numeric::HybridVector;
   using mtrc::numeric::DynamicVector;

   mtrc::numeric::Size< StaticVector<int,3UL>, 0UL >::value      // Evaluates to 3
   mtrc::numeric::Size< StaticMatrix<int,2UL,4UL>, 0UL >::value  // Evaluates to 2 (the number of rows)
   mtrc::numeric::Size< StaticMatrix<int,2UL,4UL>, 1UL >::value  // Evaluates to 4 (the number of columns)
   mtrc::numeric::Size< StaticVector<int,3UL>, 1UL >::value      // Evaluates to -1; 1 is not a valid vector
dimension! mtrc::numeric::Size< HybridVector<int,3UL>, 0UL >::value      // Evaluates to -1; Only maximum size is
fixed! mtrc::numeric::Size< DynamicVector<int>, 0UL >::value         // Evaluates to -1; Size not fixed at compile
time! mtrc::numeric::Size< int, 0UL >::value                        // Evaluates to -1
   \endcode
*/
template <typename T, size_t N> struct Size : public SizeHelper1<T, N> {};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the Size type trait for const types.
// \ingroup math_type_traits
*/
template <typename T, size_t N> struct Size<const T, N> : public Size<T, N> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the Size type trait for volatile types.
// \ingroup math_type_traits
*/
template <typename T, size_t N> struct Size<volatile T, N> : public Size<T, N> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the Size type trait for cv qualified types.
// \ingroup math_type_traits
*/
template <typename T, size_t N> struct Size<const volatile T, N> : public Size<T, N> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary variable template for the Size type trait.
// \ingroup math_type_traits
//
// The Size_v variable template provides a convenient shortcut to access the nested \a value
// of the Size class template. For instance, given the type \a T and the dimension \a N the
// following two statements are identical:

   \code
   constexpr size_t value1 = mtrc::numeric::Size<T,N>::value;
   constexpr size_t value2 = mtrc::numeric::Size_v<T,N>;
   \endcode
*/
template <typename T, size_t N> constexpr ptrdiff_t Size_v = Size<T, N>::value;
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief First auxiliary helper struct for the Size type trait.
// \ingroup math_type_traits
*/
template <typename T, size_t N, typename> struct SizeHelper1 : public DefaultSize {};

template <typename T, size_t N>
struct SizeHelper1<T, N, Void_t<typename T::ResultType>> : public SizeHelper2<T, N, typename T::ResultType> {};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Second auxiliary helper struct for the Size type trait.
// \ingroup math_type_traits
*/
template <typename T, size_t N, typename U> struct SizeHelper2 : public Size<U, N> {};

template <typename T, size_t N> struct SizeHelper2<T, N, T> : public DefaultSize {};
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
