// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_DENSE_FORWARD_H
#define METRIC_NUMERIC_MATH_DENSE_FORWARD_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/AlignmentFlag.h>
#include <metric/numeric/math/GroupTag.h>
#include <metric/numeric/math/PaddingFlag.h>
#include <metric/numeric/system/Alignment.h>
#include <metric/numeric/system/Padding.h>
#include <metric/numeric/system/StorageOrder.h>
#include <metric/numeric/system/TransposeFlag.h>
#include <metric/numeric/util/Forward.h>
#include <metric/numeric/util/Types.h>
#include <metric/numeric/util/typetraits/RemoveConst.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  mtrc::numeric NAMESPACE FORWARD DECLARATIONS
//
//=================================================================================================

template <typename Type // Data type of the vector
		  ,
		  size_t N // Number of elements
		  ,
		  bool TF = defaultTransposeFlag // Transpose flag
		  ,
		  AlignmentFlag AF = defaultAlignmentFlag // Alignment flag
		  ,
		  PaddingFlag PF = defaultPaddingFlag // Padding flag
		  ,
		  typename Tag = Group0> // Type tag
class StaticVector;

template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO = defaultStorageOrder // Storage order
		  ,
		  AlignmentFlag AF = defaultAlignmentFlag // Alignment flag
		  ,
		  PaddingFlag PF = defaultPaddingFlag // Padding flag
		  ,
		  typename Tag = Group0> // Type tag
class StaticMatrix;

template <typename Type // Data type of the vector
		  ,
		  size_t N // Number of elements
		  ,
		  bool TF = defaultTransposeFlag // Transpose flag
		  ,
		  AlignmentFlag AF = defaultAlignmentFlag // Alignment flag
		  ,
		  PaddingFlag PF = defaultPaddingFlag // Padding flag
		  ,
		  typename Tag = Group0> // Type tag
class HybridVector;

template <typename Type // Data type of the matrix
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N // Number of columns
		  ,
		  bool SO = defaultStorageOrder // Storage order
		  ,
		  AlignmentFlag AF = defaultAlignmentFlag // Alignment flag
		  ,
		  PaddingFlag PF = defaultPaddingFlag // Padding flag
		  ,
		  typename Tag = Group0> // Type tag
class HybridMatrix;

template <typename Type // Data type of the vector
		  ,
		  bool TF = defaultTransposeFlag // Transpose flag
		  ,
		  typename Alloc = AlignedAllocator<Type> // Type of the allocator
		  ,
		  typename Tag = Group0> // Type tag
class DynamicVector;

template <typename Type // Data type of the matrix
		  ,
		  bool SO = defaultStorageOrder // Storage order
		  ,
		  typename Alloc = AlignedAllocator<Type> // Type of the allocator
		  ,
		  typename Tag = Group0> // Type tag
class DynamicMatrix;

template <typename Type // Data type of the vector
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  bool TF = defaultTransposeFlag // Transpose flag
		  ,
		  typename Tag = Group0 // Type tag
		  ,
		  typename RT = // Result type
		  DynamicVector<RemoveConst_t<Type>, TF, AlignedAllocator<Type>, Tag>>
class CustomVector;

template <typename Type // Data type of the matrix
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  bool SO = defaultStorageOrder // Storage order
		  ,
		  typename Tag = Group0 // Type tag
		  ,
		  typename RT = // Result type
		  DynamicMatrix<RemoveConst_t<Type>, SO, AlignedAllocator<Type>, Tag>>
class CustomMatrix;

template <typename Type // Data type of the vector
		  ,
		  bool TF = defaultTransposeFlag // Transpose flag
		  ,
		  typename Tag = Group0> // Type tag
class UniformVector;

template <typename Type // Data type of the matrix
		  ,
		  bool SO = defaultStorageOrder // Storage order
		  ,
		  typename Tag = Group0> // Type tag
class UniformMatrix;

template <typename Type // Data type of the vector
		  ,
		  bool TF = defaultTransposeFlag // Transpose flag
		  ,
		  typename Tag = Group0> // Type tag
class InitializerVector;

template <typename Type // Data type of the matrix
		  ,
		  typename Tag = Group0> // Type tag
class InitializerMatrix;

} // namespace mtrc::numeric

#endif
