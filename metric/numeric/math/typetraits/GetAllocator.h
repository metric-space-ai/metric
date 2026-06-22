// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TYPETRAITS_GETALLOCATOR_H
#define METRIC_NUMERIC_MATH_TYPETRAITS_GETALLOCATOR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/Aliases.h>
#include <metric/numeric/math/typetraits/UnderlyingElement.h>
#include <metric/numeric/util/NullAllocator.h>
#include <metric/numeric/util/typetraits/IsDetected.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Determines the type of allocator of the given type.
// \ingroup math_type_traits
//
// This type trait determines the type of allocator of the given type \a T. In case the type
// \a T exposes this allocator via a nested type \a AllocatorType, the nested type \a Type is
// set to the type of allocator. Otherwise the nested type \a Type is set to NullAllocator.

   \code
   using mtrc::numeric::DynamicVector;
   using mtrc::numeric::DynamicMatrix;
   using mtrc::numeric::StaticVector;
   using mtrc::numeric::CompressedMatrix;

   mtrc::numeric::GetAllocator< DynamicVector<int> >::Type              // Results in 'AlignedAllocator<int>'
   mtrc::numeric::GetAllocator< const DynamicVector<double> >::Type     // Results in 'AlignedAllocator<double>'
   mtrc::numeric::GetAllocator< volatile DynamicMatrix<int> >::Type     // Results in 'AlignedAllocator<int>'
   mtrc::numeric::GetAllocator< int >::Type                             // Results in 'NullAllocator<int>'
   mtrc::numeric::GetAllocator< const StaticVector<float,3UL> >::Type   // Results in 'NullAllocator<float>'
   mtrc::numeric::GetAllocator< volatile CompressedMatrix<int> >::Type  // Results in 'NullAllocator<int>'
   \endcode
*/
template <typename T> using GetAllocator = DetectedOr<NullAllocator<UnderlyingElement_t<T>>, AllocatorType_t, T>;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Auxiliary alias declaration for the GetAllocator type trait.
// \ingroup math_type_traits
//
// The GetAllocator_t alias declaration provides a convenient shortcut to access the nested
// \a Type of the GetAllocator class template. For instance, given the type \a T the following
// two type definitions are identical:

   \code
   using Type1 = typename mtrc::numeric::GetAllocator<T>::Type;
   using Type2 = mtrc::numeric::GetAllocator_t<T>;
   \endcode
*/
template <typename T> using GetAllocator_t = typename GetAllocator<T>::Type;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
