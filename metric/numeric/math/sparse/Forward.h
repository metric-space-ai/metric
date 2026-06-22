// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_SPARSE_FORWARD_H
#define METRIC_NUMERIC_MATH_SPARSE_FORWARD_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/GroupTag.h>
#include <metric/numeric/system/StorageOrder.h>
#include <metric/numeric/system/TransposeFlag.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  mtrc::numeric NAMESPACE FORWARD DECLARATIONS
//
//=================================================================================================

template <typename Type // Data type of the vector
		  ,
		  bool TF = defaultTransposeFlag // Transpose flag
		  ,
		  typename Tag = Group0> // Type tag
class CompressedVector;

template <typename Type // Data type of the matrix
		  ,
		  bool SO = defaultStorageOrder // Storage order
		  ,
		  typename Tag = Group0> // Type tag
class CompressedMatrix;

template <typename Type // Data type of the matrix
		  ,
		  bool SO = defaultStorageOrder // Storage order
		  ,
		  typename Tag = Group0> // Type tag
class IdentityMatrix;

template <typename Type // Data type of the vector
		  ,
		  bool TF = defaultTransposeFlag // Transpose flag
		  ,
		  typename Tag = Group0> // Type tag
class ZeroVector;

template <typename Type // Data type of the matrix
		  ,
		  bool SO = defaultStorageOrder // Storage order
		  ,
		  typename Tag = Group0> // Type tag
class ZeroMatrix;

} // namespace mtrc::numeric

#endif
