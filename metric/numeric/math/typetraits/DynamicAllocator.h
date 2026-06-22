// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_DYNAMICALLOCATOR_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_DYNAMICALLOCATOR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <memory>
#include <metric/numeric/util/AlignedAllocator.h>
#include <metric/numeric/util/NullAllocator.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Deduction of an allocator type for dynamic vectors and matrices.
// \ingroup math_type_traits
//
// \section dynamicallocator_general General
//
// The DynamicAllocator type trait deduces the allocator type for dynamic vectors and matrices.
// Given one or two allocators, it provides a nested \a Type alias template, which results in
// the according allocator type:

   \code
   using A1 = AlignedAllocator<int>;
   using A2 = AlignedAllocator<double>;

   mtrc::numeric::DynamicAllocator<A1,A2>::Type<double>  // Results in 'AlignedAllocator<double>'
   \endcode

// In case no resulting allocator type can be determined, the nested \a Type template will result
// in \a mtrc::numeric::NullAllocator for all possible types.
//
//
// \n \section dynamicallocator_specializations Creating custom specializations
//
// DynamicAllocator is guaranteed to work only for all \b Metric numeric allocator types. In order to add
// support for user-defined allocator types it is possible to specialize the DynamicAllocator
// template. The following example demonstrates the according specialization for the
// \a mtrc::numeric::AlignedAllocator class template:

   \code
   template< typename T1, typename T2 >
   struct DynamicAllocator< AlignedAllocator<T1>, AlignedAllocator<T2> >
   {
	  template< typename U >
	  using Type = AlignedAllocator<U>;
   };
   \endcode
*/
template <typename A1, typename... As> struct DynamicAllocator {
  public:
	//**********************************************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	template <typename U> using Type = NullAllocator<U>;
	/*! \endcond */
	//**********************************************************************************************
};
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the DynamicAllocator type trait for 'AlignedAllocator'.
// \ingroup math_type_traits
*/
template <typename T> struct DynamicAllocator<AlignedAllocator<T>> {
	template <typename U> using Type = AlignedAllocator<U>;
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the DynamicAllocator type trait for 'NullAllocator'.
// \ingroup math_type_traits
*/
template <typename T> struct DynamicAllocator<NullAllocator<T>> {
	template <typename U> using Type = AlignedAllocator<U>;
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the DynamicAllocator type trait for two 'AlignedAllocator'.
// \ingroup math_type_traits
*/
template <typename T1, typename T2> struct DynamicAllocator<AlignedAllocator<T1>, AlignedAllocator<T2>> {
	template <typename U> using Type = AlignedAllocator<U>;
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the DynamicAllocator type trait for any allocator and 'NullAllocator'.
// \ingroup math_type_traits
*/
template <typename A1, typename T2> struct DynamicAllocator<A1, NullAllocator<T2>> {
	template <typename U> using Type = typename std::allocator_traits<A1>::template rebind_alloc<U>;
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the DynamicAllocator type trait for 'NullAllocator' and any allocator.
// \ingroup math_type_traits
*/
template <typename A2, typename T1> struct DynamicAllocator<NullAllocator<T1>, A2> {
	template <typename U> using Type = typename std::allocator_traits<A2>::template rebind_alloc<U>;
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the DynamicAllocator type trait for two 'NullAllocator'.
// \ingroup math_type_traits
*/
template <typename T1, typename T2> struct DynamicAllocator<NullAllocator<T1>, NullAllocator<T2>> {
	template <typename U> using Type = AlignedAllocator<U>;
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary alias declaration for the DynamicAllocator type trait.
// \ingroup math_type_traits
//
// The DynamicAllocator_t alias declaration provides a convenient shortcut to access the nested
// \a Type of the DynamicAllocator class template. For instance, given the types \a A1 and \a A2
// the following  two type definitions are identical:

   \code
   using Type1 = typename mtrc::numeric::DynamicAllocator<A1,A2>::template Type<T>;
   using Type2 = mtrc::numeric::DynamicAllocator_t<T,A1,A2>;
   \endcode
*/
template <typename T, typename... As> using DynamicAllocator_t = typename DynamicAllocator<As...>::template Type<T>;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
