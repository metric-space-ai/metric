// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_VIEWS_SUBVECTOR_H
#define METRIC_NUMERIC_MATH_VIEWS_SUBVECTOR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <algorithm>
#include <metric/numeric/math/Aliases.h>
#include <metric/numeric/math/AlignmentFlag.h>
#include <metric/numeric/math/Exception.h>
#include <metric/numeric/math/RelaxationFlag.h>
#include <metric/numeric/math/expressions/CrossExpr.h>
#include <metric/numeric/math/expressions/VecEvalExpr.h>
#include <metric/numeric/math/expressions/VecMapExpr.h>
#include <metric/numeric/math/expressions/VecNoAliasExpr.h>
#include <metric/numeric/math/expressions/VecNoSIMDExpr.h>
#include <metric/numeric/math/expressions/VecRepeatExpr.h>
#include <metric/numeric/math/expressions/VecScalarDivExpr.h>
#include <metric/numeric/math/expressions/VecScalarMultExpr.h>
#include <metric/numeric/math/expressions/VecSerialExpr.h>
#include <metric/numeric/math/expressions/VecTransExpr.h>
#include <metric/numeric/math/expressions/VecVecAddExpr.h>
#include <metric/numeric/math/expressions/VecVecDivExpr.h>
#include <metric/numeric/math/expressions/VecVecKronExpr.h>
#include <metric/numeric/math/expressions/VecVecMapExpr.h>
#include <metric/numeric/math/expressions/VecVecMultExpr.h>
#include <metric/numeric/math/expressions/VecVecSubExpr.h>
#include <metric/numeric/math/expressions/Vector.h>
#include <metric/numeric/math/shims/IsDefault.h>
#include <metric/numeric/math/shims/Serial.h>
#include <metric/numeric/math/typetraits/HasConstDataAccess.h>
#include <metric/numeric/math/typetraits/HasMutableDataAccess.h>
#include <metric/numeric/math/typetraits/IsAligned.h>
#include <metric/numeric/math/typetraits/IsContiguous.h>
#include <metric/numeric/math/typetraits/IsRestricted.h>
#include <metric/numeric/math/typetraits/IsSubvector.h>
#include <metric/numeric/math/typetraits/MaxSize.h>
#include <metric/numeric/math/typetraits/Size.h>
#include <metric/numeric/math/views/Check.h>
#include <metric/numeric/math/views/subvector/BaseTemplate.h>
#include <metric/numeric/math/views/subvector/Dense.h>
#include <metric/numeric/math/views/subvector/Sparse.h>
#include <metric/numeric/system/MacroDisable.h>
#include <metric/numeric/util/Assert.h>
#include <metric/numeric/util/EnableIf.h>
#include <metric/numeric/util/FunctionTrace.h>
#include <metric/numeric/util/IntegerSequence.h>
#include <metric/numeric/util/IntegralConstant.h>
#include <metric/numeric/util/SmallArray.h>
#include <metric/numeric/util/StaticAssert.h>
#include <metric/numeric/util/Types.h>
#include <metric/numeric/util/typetraits/RemoveReference.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  GLOBAL FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Creating a view on a specific subvector of the given vector.
// \ingroup subvector
//
// \param vector The vector containing the subvector.
// \param args Optional subvector arguments.
// \return View on the specific subvector of the vector.
// \exception std::invalid_argument Invalid subvector specification.
//
// This function returns an expression representing the specified subvector of the given vector.
// The following example demonstrates the creation of a dense and sparse subvector:

   \code
   mtrc::numeric::DynamicVector<double,mtrc::numeric::columnVector> d;
   mtrc::numeric::CompressedVector<int,mtrc::numeric::rowVector> s;
   // ... Resizing and initialization

   // Creating a dense subvector of size 8, starting from index 4
   auto dsv = subvector<4UL,8UL>( d );

   // Creating a sparse subvector of size 7, starting from index 5
   auto ssv = subvector<5UL,7UL>( s );
   \endcode

// By default, the provided subvector arguments are checked at runtime. In case the subvector
// is not properly specified (i.e. if the specified first index is greater than the total size
// of the given vector or the subvector is specified beyond the size of the vector) a
// \a std::invalid_argument exception is thrown. The checks can be skipped by providing the
// optional \a mtrc::numeric::unchecked argument.

   \code
   auto dsv = subvector<4UL,8UL>( d, unchecked );
   auto ssv = subvector<5UL,7UL>( s, unchecked );
   \endcode

// Please note that this function creates an unaligned dense or sparse subvector. For instance,
// the creation of the dense subvector is equivalent to the following function call:

   \code
   auto dsv = subvector<unaligned,4UL,8UL>( d );
   \endcode

// In contrast to unaligned subvectors, which provide full flexibility, aligned subvectors pose
// additional alignment restrictions. However, especially in case of dense subvectors this may
// result in considerable performance improvements. In order to create an aligned subvector the
// following function call has to be used:

   \code
   auto dsv = subvector<aligned,4UL,8UL>( d );
   \endcode

// Note however that in this case the given compile time arguments \a I and \a N are subject to
// additional checks to guarantee proper alignment.
*/
template <size_t I // Index of the first subvector element
		  ,
		  size_t N // Size of the subvector
		  ,
		  typename VT // Type of the vector
		  ,
		  bool TF // Transpose flag
		  ,
		  typename... RSAs> // Optional subvector arguments
inline decltype(auto) subvector(Vector<VT, TF> &vector, RSAs... args)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return subvector<unaligned, I, N>(*vector, args...);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Creating a view on a specific subvector of the given constant vector.
// \ingroup subvector
//
// \param vector The constant vector containing the subvector.
// \param args Optional subvector arguments.
// \return View on the specific subvector of the vector.
// \exception std::invalid_argument Invalid subvector specification.
//
// This function returns an expression representing the specified subvector of the given constant
// vector. The following example demonstrates the creation of a dense and sparse subvector:

   \code
   const mtrc::numeric::DynamicVector<double,mtrc::numeric::columnVector> d( ... );
   const mtrc::numeric::CompressedVector<int,mtrc::numeric::rowVector> s( ... );
   // ... Resizing and initialization

   // Creating a dense subvector of size 8, starting from index 4
   auto dsv = subvector<4UL,8UL>( d );

   // Creating a sparse subvector of size 7, starting from index 5
   auto ssv = subvector<5UL,7UL>( s );
   \endcode

// By default, the provided subvector arguments are checked at runtime. In case the subvector
// is not properly specified (i.e. if the specified first index is greater than the total size
// of the given vector or the subvector is specified beyond the size of the vector) a
// \a std::invalid_argument exception is thrown. The checks can be skipped by providing the
// optional \a mtrc::numeric::unchecked argument.

   \code
   auto dsv = subvector<4UL,8UL>( d, unchecked );
   auto ssv = subvector<5UL,7UL>( s, unchecked );
   \endcode

// Please note that this function creates an unaligned dense or sparse subvector. For instance,
// the creation of the dense subvector is equivalent to the following function call:

   \code
   auto dsv = subvector<unaligned,4UL,8UL>( d );
   \endcode

// In contrast to unaligned subvectors, which provide full flexibility, aligned subvectors pose
// additional alignment restrictions. However, especially in case of dense subvectors this may
// result in considerable performance improvements. In order to create an aligned subvector the
// following function call has to be used:

   \code
   auto dsv = subvector<aligned,4UL,8UL>( d );
   \endcode

// Note however that in this case the given compile time arguments \a I and \a N are subject to
// additional checks to guarantee proper alignment.
*/
template <size_t I // Index of the first subvector element
		  ,
		  size_t N // Size of the subvector
		  ,
		  typename VT // Type of the vector
		  ,
		  bool TF // Transpose flag
		  ,
		  typename... RSAs> // Optional subvector arguments
inline decltype(auto) subvector(const Vector<VT, TF> &vector, RSAs... args)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return subvector<unaligned, I, N>(*vector, args...);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Creating a view on a specific subvector of the given temporary vector.
// \ingroup subvector
//
// \param vector The temporary vector containing the subvector.
// \param args Optional subvector arguments.
// \return View on the specific subvector of the vector.
// \exception std::invalid_argument Invalid subvector specification.
//
// This function returns an expression representing the specified subvector of the given
// temporary vector. In case the subvector is not properly specified (i.e. if the specified
// first index is greater than the total size of the given vector or the subvector is specified
// beyond the size of the vector) a \a std::invalid_argument exception is thrown.
*/
template <size_t I // Index of the first subvector element
		  ,
		  size_t N // Size of the subvector
		  ,
		  typename VT // Type of the vector
		  ,
		  bool TF // Transpose flag
		  ,
		  typename... RSAs> // Optional subvector arguments
inline decltype(auto) subvector(Vector<VT, TF> &&vector, RSAs... args)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return subvector<unaligned, I, N>(std::move(*vector), args...);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Creating a view on a specific subvector of the given vector.
// \ingroup subvector
//
// \param vector The vector containing the subvector.
// \param args Optional subvector arguments.
// \return View on the specific subvector of the vector.
// \exception std::invalid_argument Invalid subvector specification.
//
// This function returns an expression representing an aligned or unaligned subvector of the
// given dense or sparse vector, based on the specified alignment flag \a AF. The following
// example demonstrates the creation of both an aligned and unaligned subvector:

   \code
   mtrc::numeric::DynamicVector<double,mtrc::numeric::columnVector> d;
   mtrc::numeric::CompressedVector<int,mtrc::numeric::rowVector> s;
   // ... Resizing and initialization

   // Creating an aligned dense subvector of size 8 starting from index 4
   auto dsv = subvector<aligned,4UL,8UL>( d );

   // Creating an unaligned subvector of size 7 starting from index 3
   auto ssv = subvector<unaligned,3UL,7UL>( s );
   \endcode

// By default, the provided subvector arguments are checked at runtime. In case the subvector
// is not properly specified (i.e. if the specified first index is greater than the total size
// of the given vector or the subvector is specified beyond the size of the vector) a
// \a std::invalid_argument exception is thrown. The checks can be skipped by providing the
// optional \a mtrc::numeric::unchecked argument.

   \code
   auto dsv = subvector<aligned,4UL,8UL>( d, unchecked );
   auto ssv = subvector<unaligned,3UL,7UL>( s, unchecked );
   \endcode

// In contrast to unaligned subvectors, which provide full flexibility, aligned subvectors pose
// additional alignment restrictions and the given index \a I is subject to additional checks to
// guarantee proper alignment. However, especially in case of dense subvectors this may result
// in considerable performance improvements.
//
// The alignment restrictions refer to system dependent address restrictions for the used element
// type and the available vectorization mode (SSE, AVX, ...). In order to be properly aligned the
// first element of the subvector must be aligned. The following source code gives some examples
// for a double precision dynamic vector, assuming that AVX is available, which packs 4 \c double
// values into a SIMD vector:

   \code
   mtrc::numeric::DynamicVector<double,mtrc::numeric::columnVector> d( 17UL );
   // ... Resizing and initialization

   // OK: Starts at the beginning, i.e. the first element is aligned
   auto dsv1 = subvector<aligned,0UL,13UL>( d );

   // OK: Start index is a multiple of 4, i.e. the first element is aligned
   auto dsv2 = subvector<aligned,4UL,7UL>( d );

   // OK: The start index is a multiple of 4 and the subvector includes the last element
   auto dsv3 = subvector<aligned,8UL,9UL>( d );

   // Error: Start index is not a multiple of 4, i.e. the first element is not aligned
   auto dsv4 = subvector<aligned,5UL,8UL>( d );
   \endcode

// In case any alignment restrictions are violated, a \a std::invalid_argument exception is thrown.
*/
template <AlignmentFlag AF // Alignment flag
		  ,
		  size_t I // Index of the first subvector element
		  ,
		  size_t N // Size of the subvector
		  ,
		  typename VT // Type of the vector
		  ,
		  bool TF // Transpose flag
		  ,
		  typename... RSAs> // Optional subvector arguments
inline decltype(auto) subvector(Vector<VT, TF> &vector, RSAs... args)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	using ReturnType = Subvector_<VT, AF, I, N>;
	return ReturnType(*vector, args...);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Creating a view on a specific subvector of the given constant vector.
// \ingroup subvector
//
// \param vector The constant vector containing the subvector.
// \param args Optional subvector arguments.
// \return View on the specific subvector of the vector.
// \exception std::invalid_argument Invalid subvector specification.
//
// This function returns an expression representing an aligned or unaligned subvector of the
// given constant dense or sparse vector, based on the specified alignment flag \a AF. The
// following example demonstrates the creation of both an aligned and unaligned subvector:

   \code
   const mtrc::numeric::DynamicVector<double,mtrc::numeric::columnVector> d( ... );
   const mtrc::numeric::CompressedVector<int,mtrc::numeric::rowVector> s( ... );

   // Creating an aligned dense subvector of size 8 starting from index 4
   auto dsv = subvector<aligned,4UL,8UL>( d );

   // Creating an unaligned subvector of size 7 starting from index 3
   auto ssv = subvector<unaligned,3UL,7UL>( s );
   \endcode

// By default, the provided subvector arguments are checked at runtime. In case the subvector
// is not properly specified (i.e. if the specified first index is greater than the total size
// of the given vector or the subvector is specified beyond the size of the vector) a
// \a std::invalid_argument exception is thrown. The checks can be skipped by providing the
// optional \a mtrc::numeric::unchecked argument.

   \code
   auto dsv = subvector<aligned,4UL,8UL>( d, unchecked );
   auto ssv = subvector<unaligned,3UL,7UL>( s, unchecked );
   \endcode

// In contrast to unaligned subvectors, which provide full flexibility, aligned subvectors pose
// additional alignment restrictions and the given index \a I is subject to additional checks to
// guarantee proper alignment. However, especially in case of dense subvectors this may result
// in considerable performance improvements.
//
// The alignment restrictions refer to system dependent address restrictions for the used element
// type and the available vectorization mode (SSE, AVX, ...). In order to be properly aligned the
// first element of the subvector must be aligned. The following source code gives some examples
// for a double precision dynamic vector, assuming that AVX is available, which packs 4 \c double
// values into a SIMD vector:

   \code
   const mtrc::numeric::DynamicVector<double,mtrc::numeric::columnVector> d( ... );

   // OK: Starts at the beginning, i.e. the first element is aligned
   auto dsv1 = subvector<aligned,0UL,13UL>( d );

   // OK: Start index is a multiple of 4, i.e. the first element is aligned
   auto dsv2 = subvector<aligned,4UL,7UL>( d );

   // OK: The start index is a multiple of 4 and the subvector includes the last element
   auto dsv3 = subvector<aligned,8UL,9UL>( d );

   // Error: Start index is not a multiple of 4, i.e. the first element is not aligned
   auto dsv4 = subvector<aligned,5UL,8UL>( d );
   \endcode

// In case any alignment restrictions are violated, a \a std::invalid_argument exception is thrown.
*/
template <AlignmentFlag AF // Alignment flag
		  ,
		  size_t I // Index of the first subvector element
		  ,
		  size_t N // Size of the subvector
		  ,
		  typename VT // Type of the vector
		  ,
		  bool TF // Transpose flag
		  ,
		  typename... RSAs> // Optional subvector arguments
inline decltype(auto) subvector(const Vector<VT, TF> &vector, RSAs... args)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	using ReturnType = const Subvector_<const VT, AF, I, N>;
	return ReturnType(*vector, args...);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Creating a view on a specific subvector of the given temporary vector.
// \ingroup subvector
//
// \param vector The temporary vector containing the subvector.
// \param args Optional subvector arguments.
// \return View on the specific subvector of the vector.
// \exception std::invalid_argument Invalid subvector specification.
//
// This function returns an expression representing an aligned or unaligned subvector of the
// given temporary dense or sparse vector, based on the specified alignment flag \a AF. In
// case the subvector is not properly specified (i.e. if the specified first index is greater
// than the total size of the given vector or the subvector is specified beyond the size of
// the vector) or any alignment restrictions are violated, a \a std::invalid_argument exception
// is thrown.
*/
template <AlignmentFlag AF // Alignment flag
		  ,
		  size_t I // Index of the first subvector element
		  ,
		  size_t N // Size of the subvector
		  ,
		  typename VT // Type of the vector
		  ,
		  bool TF // Transpose flag
		  ,
		  typename... RSAs> // Optional subvector arguments
inline decltype(auto) subvector(Vector<VT, TF> &&vector, RSAs... args)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	using ReturnType = Subvector_<VT, AF, I, N>;
	return ReturnType(*vector, args...);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Creating a view on a specific subvector of the given vector.
// \ingroup subvector
//
// \param vector The vector containing the subvector.
// \param index The index of the first element of the subvector.
// \param size The size of the subvector.
// \param args Optional subvector arguments.
// \return View on the specific subvector of the vector.
// \exception std::invalid_argument Invalid subvector specification.
//
// This function returns an expression representing the specified subvector of the given vector.
// The following example demonstrates the creation of a dense and sparse subvector:

   \code
   mtrc::numeric::DynamicVector<double,mtrc::numeric::columnVector> d;
   mtrc::numeric::CompressedVector<int,mtrc::numeric::rowVector> s;
   // ... Resizing and initialization

   // Creating a dense subvector of size 8, starting from index 4
   auto dsv = subvector( d, 4UL, 8UL );

   // Creating a sparse subvector of size 7, starting from index 5
   auto ssv = subvector( s, 5UL, 7UL );
   \endcode

// By default, the provided subvector arguments are checked at runtime. In case the subvector
// is not properly specified (i.e. if the specified first index is greater than the total size
// of the given vector or the subvector is specified beyond the size of the vector) a
// \a std::invalid_argument exception is thrown. The checks can be skipped by providing the
// optional \a mtrc::numeric::unchecked argument.

   \code
   auto dsv = subvector( d, 4UL, 8UL, unchecked );
   auto ssv = subvector( s, 5UL, 7UL, unchecked );
   \endcode

// Please note that this function creates an unaligned dense or sparse subvector. For instance,
// the creation of the dense subvector is equivalent to the following function call:

   \code
   auto dsv = subvector<unaligned>( d, 4UL, 8UL );
   \endcode

// In contrast to unaligned subvectors, which provide full flexibility, aligned subvectors pose
// additional alignment restrictions. However, especially in case of dense subvectors this may
// result in considerable performance improvements. In order to create an aligned subvector the
// following function call has to be used:

   \code
   auto dsv = subvector<aligned>( d, 4UL, 8UL );
   \endcode

// Note however that in this case the given \a index and \a size are subject to additional checks
// to guarantee proper alignment.
*/
template <typename VT // Type of the vector
		  ,
		  bool TF // Transpose flag
		  ,
		  typename... RSAs> // Optional subvector arguments
inline decltype(auto) subvector(Vector<VT, TF> &vector, size_t index, size_t size, RSAs... args)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return subvector<unaligned>(*vector, index, size, args...);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Creating a view on a specific subvector of the given constant vector.
// \ingroup subvector
//
// \param vector The constant vector containing the subvector.
// \param index The index of the first element of the subvector.
// \param size The size of the subvector.
// \param args Optional subvector arguments.
// \return View on the specific subvector of the vector.
// \exception std::invalid_argument Invalid subvector specification.
//
// This function returns an expression representing the specified subvector of the given constant
// vector. The following example demonstrates the creation of a dense and sparse subvector:

   \code
   const mtrc::numeric::DynamicVector<double,mtrc::numeric::columnVector> d( ... );
   const mtrc::numeric::CompressedVector<int,mtrc::numeric::rowVector> s( ... );
   // ... Resizing and initialization

   // Creating a dense subvector of size 8, starting from index 4
   auto dsv = subvector( d, 4UL, 8UL );

   // Creating a sparse subvector of size 7, starting from index 5
   auto ssv = subvector( s, 5UL, 7UL );
   \endcode

// By default, the provided subvector arguments are checked at runtime. In case the subvector
// is not properly specified (i.e. if the specified first index is greater than the total size
// of the given vector or the subvector is specified beyond the size of the vector) a
// \a std::invalid_argument exception is thrown. The checks can be skipped by providing the
// optional \a mtrc::numeric::unchecked argument.

   \code
   auto dsv = subvector( d, 4UL, 8UL, unchecked );
   auto ssv = subvector( s, 5UL, 7UL, unchecked );
   \endcode

// Please note that this function creates an unaligned dense or sparse subvector. For instance,
// the creation of the dense subvector is equivalent to the following function call:

   \code
   auto dsv = subvector<unaligned>( d, 4UL, 8UL );
   \endcode

// In contrast to unaligned subvectors, which provide full flexibility, aligned subvectors pose
// additional alignment restrictions. However, especially in case of dense subvectors this may
// result in considerable performance improvements. In order to create an aligned subvector the
// following function call has to be used:

   \code
   auto dsv = subvector<aligned>( d, 4UL, 8UL );
   \endcode

// Note however that in this case the given \a index and \a size are subject to additional checks
// to guarantee proper alignment.
*/
template <typename VT // Type of the vector
		  ,
		  bool TF // Transpose flag
		  ,
		  typename... RSAs> // Optional subvector arguments
inline decltype(auto) subvector(const Vector<VT, TF> &vector, size_t index, size_t size, RSAs... args)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return subvector<unaligned>(*vector, index, size, args...);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Creating a view on a specific subvector of the given temporary vector.
// \ingroup subvector
//
// \param vector The temporary vector containing the subvector.
// \param index The index of the first element of the subvector.
// \param size The size of the subvector.
// \param args Optional subvector arguments.
// \return View on the specific subvector of the vector.
// \exception std::invalid_argument Invalid subvector specification.
//
// This function returns an expression representing the specified subvector of the given
// temporary vector. In case the subvector is not properly specified (i.e. if the specified
// first index is greater than the total size of the given vector or the subvector is specified
// beyond the size of the vector) a \a std::invalid_argument exception is thrown.
*/
template <typename VT // Type of the vector
		  ,
		  bool TF // Transpose flag
		  ,
		  typename... RSAs> // Optional subvector arguments
inline decltype(auto) subvector(Vector<VT, TF> &&vector, size_t index, size_t size, RSAs... args)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return subvector<unaligned>(std::move(*vector), index, size, args...);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Creating a view on a specific subvector of the given vector.
// \ingroup subvector
//
// \param vector The vector containing the subvector.
// \param index The index of the first element of the subvector.
// \param size The size of the subvector.
// \param args Optional subvector arguments.
// \return View on the specific subvector of the vector.
// \exception std::invalid_argument Invalid subvector specification.
//
// This function returns an expression representing an aligned or unaligned subvector of the
// given dense or sparse vector, based on the specified alignment flag \a AF. The following
// example demonstrates the creation of both an aligned and unaligned subvector:

   \code
   mtrc::numeric::DynamicVector<double,mtrc::numeric::columnVector> d;
   mtrc::numeric::CompressedVector<int,mtrc::numeric::rowVector> s;
   // ... Resizing and initialization

   // Creating an aligned dense subvector of size 8 starting from index 4
   auto dsv = subvector<aligned>( d, 4UL, 8UL );

   // Creating an unaligned subvector of size 7 starting from index 3
   auto ssv = subvector<unaligned>( s, 3UL, 7UL );
   \endcode

// By default, the provided subvector arguments are checked at runtime. In case the subvector
// is not properly specified (i.e. if the specified first index is greater than the total size
// of the given vector or the subvector is specified beyond the size of the vector) a
// \a std::invalid_argument exception is thrown. The checks can be skipped by providing the
// optional \a mtrc::numeric::unchecked argument.

   \code
   auto dsv = subvector<aligned>( d, 4UL, 8UL, unchecked );
   auto ssv = subvector<unaligned>( s, 3UL, 7UL, unchecked );
   \endcode

// In contrast to unaligned subvectors, which provide full flexibility, aligned subvectors pose
// additional alignment restrictions and the given \a index is subject to additional checks to
// guarantee proper alignment. However, especially in case of dense subvectors this may result
// in considerable performance improvements.
//
// The alignment restrictions refer to system dependent address restrictions for the used element
// type and the available vectorization mode (SSE, AVX, ...). In order to be properly aligned the
// first element of the subvector must be aligned. The following source code gives some examples
// for a double precision dynamic vector, assuming that AVX is available, which packs 4 \c double
// values into a SIMD vector:

   \code
   mtrc::numeric::DynamicVector<double,mtrc::numeric::columnVector> d( 17UL );
   // ... Resizing and initialization

   // OK: Starts at the beginning, i.e. the first element is aligned
   auto dsv1 = subvector<aligned>( d, 0UL, 13UL );

   // OK: Start index is a multiple of 4, i.e. the first element is aligned
   auto dsv2 = subvector<aligned>( d, 4UL, 7UL );

   // OK: The start index is a multiple of 4 and the subvector includes the last element
   auto dsv3 = subvector<aligned>( d, 8UL, 9UL );

   // Error: Start index is not a multiple of 4, i.e. the first element is not aligned
   auto dsv4 = subvector<aligned>( d, 5UL, 8UL );
   \endcode

// In case any alignment restrictions are violated, a \a std::invalid_argument exception is thrown.
*/
template <AlignmentFlag AF // Alignment flag
		  ,
		  typename VT // Type of the vector
		  ,
		  bool TF // Transpose flag
		  ,
		  typename... RSAs> // Optional subvector arguments
inline decltype(auto) subvector(Vector<VT, TF> &vector, size_t index, size_t size, RSAs... args)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	using ReturnType = Subvector_<VT, AF>;
	return ReturnType(*vector, index, size, args...);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Creating a view on a specific subvector of the given constant vector.
// \ingroup subvector
//
// \param vector The constant vector containing the subvector.
// \param index The index of the first element of the subvector.
// \param size The size of the subvector.
// \param args Optional subvector arguments.
// \return View on the specific subvector of the vector.
// \exception std::invalid_argument Invalid subvector specification.
//
// This function returns an expression representing an aligned or unaligned subvector of the
// given constant dense or sparse vector, based on the specified alignment flag \a AF. The
// following example demonstrates the creation of both an aligned and unaligned subvector:

   \code
   const mtrc::numeric::DynamicVector<double,mtrc::numeric::columnVector> d( ... );
   const mtrc::numeric::CompressedVector<int,mtrc::numeric::rowVector> s( ... );

   // Creating an aligned dense subvector of size 8 starting from index 4
   auto dsv = subvector<aligned>( d, 4UL, 8UL );

   // Creating an unaligned subvector of size 7 starting from index 3
   auto ssv = subvector<unaligned>( s, 3UL, 7UL );
   \endcode

// By default, the provided subvector arguments are checked at runtime. In case the subvector
// is not properly specified (i.e. if the specified first index is greater than the total size
// of the given vector or the subvector is specified beyond the size of the vector) a
// \a std::invalid_argument exception is thrown. The checks can be skipped by providing the
// optional \a mtrc::numeric::unchecked argument.

   \code
   auto dsv = subvector<aligned>( d, 4UL, 8UL, unchecked );
   auto ssv = subvector<unaligned>( s, 3UL, 7UL, unchecked );
   \endcode

// In contrast to unaligned subvectors, which provide full flexibility, aligned subvectors pose
// additional alignment restrictions and the given \a index is subject to additional checks to
// guarantee proper alignment. However, especially in case of dense subvectors this may result
// in considerable performance improvements.
//
// The alignment restrictions refer to system dependent address restrictions for the used element
// type and the available vectorization mode (SSE, AVX, ...). In order to be properly aligned the
// first element of the subvector must be aligned. The following source code gives some examples
// for a double precision dynamic vector, assuming that AVX is available, which packs 4 \c double
// values into a SIMD vector:

   \code
   const mtrc::numeric::DynamicVector<double,mtrc::numeric::columnVector> d( ... );

   // OK: Starts at the beginning, i.e. the first element is aligned
   auto dsv1 = subvector<aligned>( d, 0UL, 13UL );

   // OK: Start index is a multiple of 4, i.e. the first element is aligned
   auto dsv2 = subvector<aligned>( d, 4UL, 7UL );

   // OK: The start index is a multiple of 4 and the subvector includes the last element
   auto dsv3 = subvector<aligned>( d, 8UL, 9UL );

   // Error: Start index is not a multiple of 4, i.e. the first element is not aligned
   auto dsv4 = subvector<aligned>( d, 5UL, 8UL );
   \endcode

// In case any alignment restrictions are violated, a \a std::invalid_argument exception is thrown.
*/
template <AlignmentFlag AF // Alignment flag
		  ,
		  typename VT // Type of the vector
		  ,
		  bool TF // Transpose flag
		  ,
		  typename... RSAs> // Optional subvector arguments
inline decltype(auto) subvector(const Vector<VT, TF> &vector, size_t index, size_t size, RSAs... args)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	using ReturnType = const Subvector_<const VT, AF>;
	return ReturnType(*vector, index, size, args...);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Creating a view on a specific subvector of the given temporary vector.
// \ingroup subvector
//
// \param vector The temporary vector containing the subvector.
// \param index The index of the first element of the subvector.
// \param size The size of the subvector.
// \param args Optional subvector arguments.
// \return View on the specific subvector of the vector.
// \exception std::invalid_argument Invalid subvector specification.
//
// This function returns an expression representing an aligned or unaligned subvector of the
// given temporary dense or sparse vector, based on the specified alignment flag \a AF. In
// case the subvector is not properly specified (i.e. if the specified first index is greater
// than the total size of the given vector or the subvector is specified beyond the size of
// the vector) or any alignment restrictions are violated, a \a std::invalid_argument exception
// is thrown.
*/
template <AlignmentFlag AF // Alignment flag
		  ,
		  typename VT // Type of the vector
		  ,
		  bool TF // Transpose flag
		  ,
		  typename... RSAs> // Optional subvector arguments
inline decltype(auto) subvector(Vector<VT, TF> &&vector, size_t index, size_t size, RSAs... args)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	using ReturnType = Subvector_<VT, AF>;
	return ReturnType(*vector, index, size, args...);
}
//*************************************************************************************************

//=================================================================================================
//
//  GLOBAL RESTRUCTURING FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Creating a view on a specific subvector of the given vector/vector addition.
// \ingroup subvector
//
// \param vector The constant vector/vector addition.
// \param args The runtime subvector arguments.
// \return View on the specified subvector of the addition.
// \exception std::invalid_argument Invalid subvector specification.
//
// This function returns an expression representing the specified subvector of the given
// vector/vector addition.
*/
template <AlignmentFlag AF // Alignment flag
		  ,
		  size_t... CSAs // Compile time subvector arguments
		  ,
		  typename VT // Vector base type of the expression
		  ,
		  typename... RSAs> // Runtime subvector arguments
inline decltype(auto) subvector(const VecVecAddExpr<VT> &vector, RSAs... args)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return subvector<AF, CSAs...>((*vector).leftOperand(), args...) +
		   subvector<AF, CSAs...>((*vector).rightOperand(), args...);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Creating a view on a specific subvector of the given vector/vector subtraction.
// \ingroup subvector
//
// \param vector The constant vector/vector subtraction.
// \param args The runtime subvector arguments.
// \return View on the specified subvector of the subtraction.
// \exception std::invalid_argument Invalid subvector specification.
//
// This function returns an expression representing the specified subvector of the given
// vector/vector subtraction.
*/
template <AlignmentFlag AF // Alignment flag
		  ,
		  size_t... CSAs // Compile time subvector arguments
		  ,
		  typename VT // Vector base type of the expression
		  ,
		  typename... RSAs> // Runtime subvector arguments
inline decltype(auto) subvector(const VecVecSubExpr<VT> &vector, RSAs... args)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return subvector<AF, CSAs...>((*vector).leftOperand(), args...) -
		   subvector<AF, CSAs...>((*vector).rightOperand(), args...);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Creating a view on a specific subvector of the given vector/vector multiplication.
// \ingroup subvector
//
// \param vector The constant vector/vector multiplication.
// \param args The runtime subvector arguments.
// \return View on the specified subvector of the multiplication.
// \exception std::invalid_argument Invalid subvector specification.
//
// This function returns an expression representing the specified subvector of the given
// vector/vector multiplication.
*/
template <AlignmentFlag AF // Alignment flag
		  ,
		  size_t... CSAs // Compile time subvector arguments
		  ,
		  typename VT // Vector base type of the expression
		  ,
		  typename... RSAs> // Runtime subvector arguments
inline decltype(auto) subvector(const VecVecMultExpr<VT> &vector, RSAs... args)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return subvector<AF, CSAs...>((*vector).leftOperand(), args...) *
		   subvector<AF, CSAs...>((*vector).rightOperand(), args...);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Creating a view on a specific subvector of the given vector/vector Kronecker product.
// \ingroup subvector
//
// \param vector The constant vector/vector Kronecker product.
// \param args Optional subvector arguments.
// \return View on the specified subvector of the Kronecker product.
// \exception std::invalid_argument Invalid subvector specification.
//
// This function returns an expression representing the specified subvector of the given
// vector/vector Kronecker product.
*/
template <AlignmentFlag AF // Alignment flag
		  ,
		  size_t I // Index of the first subvector element
		  ,
		  size_t N // Size of the subvector
		  ,
		  typename VT // Vector base type of the expression
		  ,
		  typename... RSAs> // Optional subvector arguments
inline decltype(auto) subvector(const VecVecKronExpr<VT> &vector, RSAs... args)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	try {
		return elements(*vector, make_shifted_index_sequence<I, N>(), args...);
	} catch (...) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid subvector specification");
	}
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Creating a view on a specific subvector of the given vector/vector Kronecker product.
// \ingroup subvector
//
// \param vector The constant vector/vector Kronecker product.
// \param index The index of the first element of the subvector.
// \param size The size of the subvector.
// \param args Optional subvector arguments.
// \return View on the specified subvector of the Kronecker product.
// \exception std::invalid_argument Invalid subvector specification.
//
// This function returns an expression representing the specified subvector of the given
// vector/vector Kronecker product.
*/
template <AlignmentFlag AF // Alignment flag
		  ,
		  typename VT // Vector base type of the expression
		  ,
		  typename... RSAs> // Optional subvector arguments
inline decltype(auto) subvector(const VecVecKronExpr<VT> &vector, size_t index, size_t size, RSAs... args)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	try {
		return elements(*vector, [index](size_t i) { return i + index; }, size, args...);
	} catch (...) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid subvector specification");
	}
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Creating a view on a specific subvector of the given vector/vector division.
// \ingroup subvector
//
// \param vector The constant vector/vector division.
// \param args The runtime subvector arguments.
// \return View on the specified subvector of the division.
// \exception std::invalid_argument Invalid subvector specification.
//
// This function returns an expression representing the specified subvector of the given
// vector/vector division.
*/
template <AlignmentFlag AF // Alignment flag
		  ,
		  size_t... CSAs // Compile time subvector arguments
		  ,
		  typename VT // Vector base type of the expression
		  ,
		  typename... RSAs> // Runtime subvector arguments
inline decltype(auto) subvector(const VecVecDivExpr<VT> &vector, RSAs... args)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return subvector<AF, CSAs...>((*vector).leftOperand(), args...) /
		   subvector<AF, CSAs...>((*vector).rightOperand(), args...);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Creating a view on a specific subvector of the given vector/vector cross product.
// \ingroup subvector
//
// \param vector The constant vector/vector cross product.
// \param args The runtime subvector arguments.
// \return View on the specified subvector of the cross product.
// \exception std::invalid_argument Invalid subvector specification.
//
// This function returns an expression representing the specified subvector of the given
// vector/vector cross product.
*/
template <AlignmentFlag AF // Alignment flag
		  ,
		  size_t... CSAs // Compile time subvector arguments
		  ,
		  typename VT // Vector base type of the expression
		  ,
		  typename... RSAs> // Runtime subvector arguments
inline decltype(auto) subvector(const CrossExpr<VT> &vector, RSAs... args)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	using ReturnType = Subvector_<VectorType_t<VT>, unaligned, CSAs...>;
	return ReturnType(*vector, args...);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Creating a view on a specific subvector of the given vector/scalar multiplication.
// \ingroup subvector
//
// \param vector The constant vector/scalar multiplication.
// \param args The runtime subvector arguments.
// \return View on the specified subvector of the multiplication.
// \exception std::invalid_argument Invalid subvector specification.
//
// This function returns an expression representing the specified subvector of the given
// vector/scalar multiplication.
*/
template <AlignmentFlag AF // Alignment flag
		  ,
		  size_t... CSAs // Compile time subvector arguments
		  ,
		  typename VT // Vector base type of the expression
		  ,
		  typename... RSAs> // Runtime subvector arguments
inline decltype(auto) subvector(const VecScalarMultExpr<VT> &vector, RSAs... args)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return subvector<AF, CSAs...>((*vector).leftOperand(), args...) * (*vector).rightOperand();
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Creating a view on a specific subvector of the given vector/scalar division.
// \ingroup subvector
//
// \param vector The constant vector/scalar division.
// \param args The runtime subvector arguments.
// \return View on the specified subvector of the division.
// \exception std::invalid_argument Invalid subvector specification.
//
// This function returns an expression representing the specified subvector of the given
// vector/scalar division.
*/
template <AlignmentFlag AF // Alignment flag
		  ,
		  size_t... CSAs // Compile time subvector arguments
		  ,
		  typename VT // Vector base type of the expression
		  ,
		  typename... RSAs> // Runtime subvector arguments
inline decltype(auto) subvector(const VecScalarDivExpr<VT> &vector, RSAs... args)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return subvector<AF, CSAs...>((*vector).leftOperand(), args...) / (*vector).rightOperand();
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Creating a view on a specific subvector of the given unary vector map operation.
// \ingroup subvector
//
// \param vector The constant unary vector map operation.
// \param args The runtime subvector arguments.
// \return View on the specified subvector of the unary map operation.
// \exception std::invalid_argument Invalid subvector specification.
//
// This function returns an expression representing the specified subvector of the given unary
// vector map operation.
*/
template <AlignmentFlag AF // Alignment flag
		  ,
		  size_t... CSAs // Compile time subvector arguments
		  ,
		  typename VT // Vector base type of the expression
		  ,
		  typename... RSAs> // Runtime subvector arguments
inline decltype(auto) subvector(const VecMapExpr<VT> &vector, RSAs... args)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return map(subvector<AF, CSAs...>((*vector).operand(), args...), (*vector).operation());
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Creating a view on a specific subvector of the given binary vector map operation.
// \ingroup subvector
//
// \param vector The constant binary vector map operation.
// \param args The runtime subvector arguments.
// \return View on the specified subvector of the binary map operation.
// \exception std::invalid_argument Invalid subvector specification.
//
// This function returns an expression representing the specified subvector of the given binary
// vector map operation.
*/
template <AlignmentFlag AF // Alignment flag
		  ,
		  size_t... CSAs // Compile time subvector arguments
		  ,
		  typename VT // Vector base type of the expression
		  ,
		  typename... RSAs> // Runtime subvector arguments
inline decltype(auto) subvector(const VecVecMapExpr<VT> &vector, RSAs... args)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return map(subvector<AF, CSAs...>((*vector).leftOperand(), args...),
			   subvector<AF, CSAs...>((*vector).rightOperand(), args...), (*vector).operation());
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Creating a view on a specific subvector of the given vector evaluation operation.
// \ingroup subvector
//
// \param vector The constant vector evaluation operation.
// \param args The runtime subvector arguments.
// \return View on the specified subvector of the evaluation operation.
// \exception std::invalid_argument Invalid subvector specification.
//
// This function returns an expression representing the specified subvector of the given vector
// evaluation operation.
*/
template <AlignmentFlag AF // Alignment flag
		  ,
		  size_t... CSAs // Compile time subvector arguments
		  ,
		  typename VT // Vector base type of the expression
		  ,
		  typename... RSAs> // Runtime subvector arguments
inline decltype(auto) subvector(const VecEvalExpr<VT> &vector, RSAs... args)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return eval(subvector<AF, CSAs...>((*vector).operand(), args...));
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Creating a view on a specific subvector of the given vector serialization operation.
// \ingroup subvector
//
// \param vector The constant vector serialization operation.
// \param args The runtime subvector arguments.
// \return View on the specified subvector of the serialization operation.
// \exception std::invalid_argument Invalid subvector specification.
//
// This function returns an expression representing the specified subvector of the given vector
// serialization operation.
*/
template <AlignmentFlag AF // Alignment flag
		  ,
		  size_t... CSAs // Compile time subvector arguments
		  ,
		  typename VT // Vector base type of the expression
		  ,
		  typename... RSAs> // Runtime subvector arguments
inline decltype(auto) subvector(const VecSerialExpr<VT> &vector, RSAs... args)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return serial(subvector<AF, CSAs...>((*vector).operand(), args...));
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Creating a view on a specific subvector of the given vector no-alias operation.
// \ingroup subvector
//
// \param vector The constant vector no-alias operation.
// \param args The runtime subvector arguments.
// \return View on the specified subvector of the no-alias operation.
// \exception std::invalid_argument Invalid subvector specification.
//
// This function returns an expression representing the specified subvector of the given vector
// no-alias operation.
*/
template <AlignmentFlag AF // Alignment flag
		  ,
		  size_t... CSAs // Compile time subvector arguments
		  ,
		  typename VT // Vector base type of the expression
		  ,
		  typename... RSAs> // Runtime subvector arguments
inline decltype(auto) subvector(const VecNoAliasExpr<VT> &vector, RSAs... args)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return noalias(subvector<AF, CSAs...>((*vector).operand(), args...));
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Creating a view on a specific subvector of the given vector no-SIMD operation.
// \ingroup subvector
//
// \param vector The constant vector no-SIMD operation.
// \param args The runtime subvector arguments.
// \return View on the specified subvector of the no-SIMD operation.
// \exception std::invalid_argument Invalid subvector specification.
//
// This function returns an expression representing the specified subvector of the given vector
// no-SIMD operation.
*/
template <AlignmentFlag AF // Alignment flag
		  ,
		  size_t... CSAs // Compile time subvector arguments
		  ,
		  typename VT // Vector base type of the expression
		  ,
		  typename... RSAs> // Runtime subvector arguments
inline decltype(auto) subvector(const VecNoSIMDExpr<VT> &vector, RSAs... args)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return nosimd(subvector<AF, CSAs...>((*vector).operand(), args...));
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Creating a view on a specific subvector of the given vector transpose operation.
// \ingroup subvector
//
// \param vector The constant vector transpose operation.
// \param args The runtime subvector arguments.
// \return View on the specified subvector of the transpose operation.
// \exception std::invalid_argument Invalid subvector specification.
//
// This function returns an expression representing the specified subvector of the given vector
// transpose operation.
*/
template <AlignmentFlag AF // Alignment flag
		  ,
		  size_t... CSAs // Compile time subvector arguments
		  ,
		  typename VT // Vector base type of the expression
		  ,
		  typename... RSAs> // Runtime subvector arguments
inline decltype(auto) subvector(const VecTransExpr<VT> &vector, RSAs... args)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return trans(subvector<AF, CSAs...>((*vector).operand(), args...));
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Creating a view on a specific subvector of the given vector repeat operation.
// \ingroup subvector
//
// \param vector The vector repeat operation containing the subvector.
// \param args The optional subvector arguments.
// \return View on the specified subvector of the vector repeat operation.
// \exception std::invalid_argument Invalid subvector specification.
//
// This function returns an expression representing the specified subvector of the given vector
// repeat operation.
*/
template <AlignmentFlag AF // Alignment flag
		  ,
		  size_t I // Index of the first subvector element
		  ,
		  size_t N // Size of the subvector
		  ,
		  typename VT // Vector base type of the expression
		  ,
		  size_t... CRAs // Compile time repeater arguments
		  ,
		  typename... RSAs> // Optional subvector arguments
inline decltype(auto) subvector(const VecRepeatExpr<VT, CRAs...> &vector, RSAs... args)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	if (isChecked(args...)) {
		if (I + N > (*vector).size()) {
			METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid subvector specification");
		}
	} else {
		METRIC_NUMERIC_USER_ASSERT(I + N <= (*vector).size(), "Invalid subvector specification");
	}

	const size_t n = (*vector).operand().size();

	return elements((*vector).operand(), [n](size_t i) { return (i + I) % n; }, N, unchecked);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Creating a view on a specific subvector of the given vector repeat operation.
// \ingroup subvector
//
// \param vector The vector repeat operation containing the subvector.
// \param index The index of the first element of the subvector.
// \param size The size of the subvector.
// \param args The optional subvector arguments.
// \return View on the specified subvector of the vector repeat operation.
// \exception std::invalid_argument Invalid subvector specification.
//
// This function returns an expression representing the specified subvector of the given vector
// repeat operation.
*/
template <AlignmentFlag AF // Alignment flag
		  ,
		  typename VT // Vector base type of the expression
		  ,
		  size_t... CRAs // Compile time repeater arguments
		  ,
		  typename... RSAs> // Optional subvector arguments
inline decltype(auto) subvector(const VecRepeatExpr<VT, CRAs...> &vector, size_t index, size_t size, RSAs... args)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	if (isChecked(args...)) {
		if (index + size > (*vector).size()) {
			METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid subvector specification");
		}
	} else {
		METRIC_NUMERIC_USER_ASSERT(index + size <= (*vector).size(), "Invalid subvector specification");
	}

	const size_t n = (*vector).operand().size();

	return elements((*vector).operand(), [index, n](size_t i) { return (i + index) % n; }, size, unchecked);
}
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  GLOBAL RESTRUCTURING FUNCTIONS (SUBVECTOR)
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Creating a view on a specific subvector of another subvector.
// \ingroup subvector
//
// \param sv The given subvector.
// \param args The optional subvector arguments.
// \return View on the specified subvector of the other subvector.
//
// This function returns an expression representing the specified subvector of the given subvector.
*/
template <AlignmentFlag AF // Required alignment flag
		  ,
		  size_t I // Required subvector offset
		  ,
		  size_t N // Required size of the subvector
		  ,
		  typename VT // Type of the vector
		  ,
		  typename... RSAs // Optional subvector arguments
		  ,
		  EnableIf_t<IsSubvector_v<RemoveReference_t<VT>> && RemoveReference_t<VT>::compileTimeArgs> * = nullptr>
inline decltype(auto) subvector(VT &&sv, RSAs... args)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	constexpr size_t I2(RemoveReference_t<VT>::offset());
	constexpr size_t N2(RemoveReference_t<VT>::size());

	METRIC_NUMERIC_STATIC_ASSERT_MSG(I + N <= N2, "Invalid subvector specification");

	return subvector<AF, I + I2, N>(sv.operand(), args...);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Creating a view on a specific subvector of another subvector.
// \ingroup subvector
//
// \param sv The given subvector.
// \param args The optional subvector arguments.
// \return View on the specified subvector of the other subvector.
// \exception std::invalid_argument Invalid subvector specification.
//
// This function returns an expression representing the specified subvector of the given subvector.
*/
template <AlignmentFlag AF // Required alignment flag
		  ,
		  size_t I // Index of the first subvector element
		  ,
		  size_t N // Size of the subvector
		  ,
		  typename VT // Type of the vector
		  ,
		  typename... RSAs // Optional subvector arguments
		  ,
		  EnableIf_t<IsSubvector_v<RemoveReference_t<VT>> && !RemoveReference_t<VT>::compileTimeArgs> * = nullptr>
inline decltype(auto) subvector(VT &&sv, RSAs... args)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	if (isChecked(args...)) {
		if (I + N > sv.size()) {
			METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid subvector specification");
		}
	} else {
		METRIC_NUMERIC_USER_ASSERT(I + N <= sv.size(), "Invalid subvector specification");
	}

	return subvector<AF>(sv.operand(), sv.offset() + I, N, args...);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Creating a view on a specific subvector of another subvector.
// \ingroup subvector
//
// \param sv The given subvector.
// \param index The index of the first element of the subvector.
// \param size The size of the subvector.
// \param args The optional subvector arguments.
// \return View on the specified subvector of the other subvector.
// \exception std::invalid_argument Invalid subvector specification.
//
// This function returns an expression representing the specified subvector of the given subvector.
*/
template <AlignmentFlag AF // Required alignment flag
		  ,
		  typename VT // Type of the vector
		  ,
		  typename... RSAs // Optional subvector arguments
		  ,
		  EnableIf_t<IsSubvector_v<RemoveReference_t<VT>>> * = nullptr>
inline decltype(auto) subvector(VT &&sv, size_t index, size_t size, RSAs... args)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	if (isChecked(args...)) {
		if (index + size > sv.size()) {
			METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid subvector specification");
		}
	} else {
		METRIC_NUMERIC_USER_ASSERT(index + size <= sv.size(), "Invalid subvector specification");
	}

	return subvector<AF>(sv.operand(), sv.offset() + index, size, args...);
}
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  GLOBAL RESTRUCTURING FUNCTIONS (ELEMENTS)
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Creating a view on a selection of elements on a subvector.
// \ingroup subvector
//
// \param sv The given subvector.
// \param args The optional element arguments.
// \return View on the specified selection of elements on the subvector.
//
// This function returns an expression representing the specified selection of elements on the
// given subvector.
*/
template <size_t I // First element index
		  ,
		  size_t... Is // Remaining element indices
		  ,
		  typename VT // Type of the vector
		  ,
		  typename... REAs // Optional element arguments
		  ,
		  EnableIf_t<IsSubvector_v<RemoveReference_t<VT>> && RemoveReference_t<VT>::compileTimeArgs> * = nullptr>
inline decltype(auto) elements(VT &&sv, REAs... args)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	constexpr size_t I2 = RemoveReference_t<VT>::offset();
	constexpr size_t N = RemoveReference_t<VT>::size();

	return elements(sv.operand(), make_shifted_index_subsequence<I2, N, I, Is...>(), args...);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Creating a view on a selection of elements on a subvector.
// \ingroup subvector
//
// \param sv The given subvector.
// \param args The optional element arguments.
// \return View on the specified selection of elements on the subvector.
// \exception std::invalid_argument Invalid element access index.
//
// This function returns an expression representing the specified selection of elements on the
// given subvector.
*/
template <size_t I // First element index
		  ,
		  size_t... Is // Remaining element indices
		  ,
		  typename VT // Type of the vector
		  ,
		  typename... REAs // Optional element arguments
		  ,
		  EnableIf_t<IsSubvector_v<RemoveReference_t<VT>> && !RemoveReference_t<VT>::compileTimeArgs> * = nullptr>
inline decltype(auto) elements(VT &&sv, REAs... args)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	if (isChecked(args...)) {
		constexpr size_t indices[] = {I, Is...};
		for (size_t i = 0UL; i < sizeof...(Is) + 1UL; ++i) {
			if (sv.size() <= indices[i]) {
				METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid element access index");
			}
		}
	}

	return elements(sv.operand(), {I + sv.offset(), Is + sv.offset()...}, args...);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Creating a view on a selection of elements on a subvector.
// \ingroup subvector
//
// \param sv The given subvector.
// \param indices The container of element indices.
// \param n The total number of indices.
// \param args The optional element arguments.
// \return View on the specified selection of elements on the subvector.
// \exception std::invalid_argument Invalid element access index.
//
// This function returns an expression representing the specified selection of elements on the
// given subvector.
*/
template <typename VT // Type of the vector
		  ,
		  typename T // Type of the element indices
		  ,
		  typename... REAs // Optional element arguments
		  ,
		  EnableIf_t<IsSubvector_v<RemoveReference_t<VT>>> * = nullptr>
inline decltype(auto) elements(VT &&sv, T *indices, size_t n, REAs... args)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	if (isChecked(args...)) {
		for (size_t i = 0UL; i < n; ++i) {
			if (sv.size() <= size_t(indices[i])) {
				METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid element access index");
			}
		}
	}

	SmallArray<size_t, 128UL> newIndices(indices, indices + n);
	std::for_each(newIndices.begin(), newIndices.end(), [offset = sv.offset()](size_t &index) { index += offset; });

	return elements(sv.operand(), newIndices.data(), n, args...);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Creating a view on a selection of elements on a subvector.
// \ingroup subvector
//
// \param sv The given subvector.
// \param p Callable producing the indices.
// \param n The total number of indices.
// \param args The optional element arguments.
// \return View on the specified selection of elements on the subvector.
// \exception std::invalid_argument Invalid element access index.
//
// This function returns an expression representing the specified selection of elements on the
// given subvector.
*/
template <typename VT // Type of the vector
		  ,
		  typename P // Type of the index producer
		  ,
		  typename... REAs // Optional element arguments
		  ,
		  EnableIf_t<IsSubvector_v<RemoveReference_t<VT>>> * = nullptr>
inline decltype(auto) elements(VT &&sv, P p, size_t n, REAs... args)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	if (isChecked(args...)) {
		for (size_t i = 0UL; i < n; ++i) {
			if (sv.size() <= size_t(p(i))) {
				METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid element access index");
			}
		}
	}

	return elements(sv.operand(), [p, offset = sv.offset()](size_t i) { return p(i) + offset; }, n, args...);
}
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  SUBVECTOR OPERATORS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Returns whether the given dense subvector is in default state.
// \ingroup subvector
//
// \param sv The dense subvector to be tested for its default state.
// \return \a true in case the given dense subvector is component-wise zero, \a false otherwise.
//
// This function checks whether the dense subvector is in default state. For instance, in case
// the subvector is instantiated for a vector of built-in integral or floating point data type,
// the function returns \a true in case all subvector elements are 0 and \a false in case any
// subvector element is not 0. The following example demonstrates the use of the \a isDefault
// function:

   \code
   mtrc::numeric::DynamicVector<int,rowVector> v;
   // ... Resizing and initialization
   if( isDefault( subvector( v, 10UL, 20UL ) ) ) { ... }
   \endcode

// Optionally, it is possible to switch between strict semantics (mtrc::numeric::strict) and relaxed
// semantics (mtrc::numeric::relaxed):

   \code
   if( isDefault<relaxed>( subvector( v, 10UL, 20UL ) ) ) { ... }
   \endcode
*/
template <RelaxationFlag RF // Relaxation flag
		  ,
		  typename VT // Type of the dense vector
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  bool TF // Transpose flag
		  ,
		  size_t... CSAs> // Compile time subvector arguments
inline bool isDefault(const Subvector<VT, AF, TF, true, CSAs...> &sv)
{
	using mtrc::numeric::isDefault;

	for (size_t i = 0UL; i < sv.size(); ++i)
		if (!isDefault<RF>(sv[i]))
			return false;
	return true;
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Returns whether the given sparse subvector is in default state.
// \ingroup subvector
//
// \param sv The sparse subvector to be tested for its default state.
// \return \a true in case the given sparse subvector is component-wise zero, \a false otherwise.
//
// This function checks whether the sparse subvector is in default state. For instance, in case
// the subvector is instantiated for a vector of built-in integral or floating point data type,
// the function returns \a true in case all subvector elements are 0 and \a false in case any
// subvector element is not 0. The following example demonstrates the use of the \a isDefault
// function:

   \code
   mtrc::numeric::CompressedVector<int,rowVector> v;
   // ... Resizing and initialization
   if( isDefault( subvector( v, 10UL, 20UL ) ) ) { ... }
   \endcode

// Optionally, it is possible to switch between strict semantics (mtrc::numeric::strict) and relaxed
// semantics (mtrc::numeric::relaxed):

   \code
   if( isDefault<relaxed>( subvector( v, 10UL, 20UL ) ) ) { ... }
   \endcode
*/
template <RelaxationFlag RF // Relaxation flag
		  ,
		  typename VT // Type of the sparse vector
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  bool TF // Transpose flag
		  ,
		  size_t... CSAs> // Compile time subvector arguments
inline bool isDefault(const Subvector<VT, AF, TF, false, CSAs...> &sv)
{
	using mtrc::numeric::isDefault;

	for (const auto &element : *sv)
		if (!isDefault<RF>(element.value()))
			return false;
	return true;
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Returns whether the invariants of the given subvector are intact.
// \ingroup subvector
//
// \param sv The subvector to be tested.
// \return \a true in case the given subvector's invariants are intact, \a false otherwise.
//
// This function checks whether the invariants of the subvector are intact, i.e. if its state
// is valid. In case the invariants are intact, the function returns \a true, else it will
// return \a false. The following example demonstrates the use of the \a isIntact() function:

   \code
   mtrc::numeric::DynamicVector<int,rowVector> v;
   // ... Resizing and initialization
   if( isIntact( subvector( v, 10UL, 20UL ) ) ) { ... }
   \endcode
*/
template <typename VT // Type of the vector
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  bool TF // Transpose flag
		  ,
		  bool DF // Density flag
		  ,
		  size_t... CSAs> // Compile time subvector arguments
inline bool isIntact(const Subvector<VT, AF, TF, DF, CSAs...> &sv) noexcept
{
	return (sv.offset() + sv.size() <= sv.operand().size() && isIntact(sv.operand()));
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Returns whether the given subvector and vector represent the same observable state.
// \ingroup subvector
//
// \param a The subvector to be tested for its state.
// \param b The vector to be tested for its state.
// \return \a true in case the subvector and vector share a state, \a false otherwise.
//
// This overload of the isSame function tests if the given subvector refers to the entire
// range of the given vector and by that represents the same observable state. In this case,
// the function returns \a true, otherwise it returns \a false.
*/
template <typename VT // Type of the vector
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  bool TF // Transpose flag
		  ,
		  bool DF // Density flag
		  ,
		  size_t... CSAs> // Compile time subvector arguments
inline bool isSame(const Subvector<VT, AF, TF, DF, CSAs...> &a, const Vector<VT, TF> &b) noexcept
{
	return (isSame(a.operand(), *b) && (a.size() == (*b).size()));
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Returns whether the given vector and subvector represent the same observable state.
// \ingroup subvector
//
// \param a The vector to be tested for its state.
// \param b The subvector to be tested for its state.
// \return \a true in case the vector and subvector share a state, \a false otherwise.
//
// This overload of the isSame function tests if the given subvector refers to the entire
// range of the given vector and by that represents the same observable state. In this case,
// the function returns \a true, otherwise it returns \a false.
*/
template <typename VT // Type of the vector
		  ,
		  bool TF // Transpose flag
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  bool DF // Density flag
		  ,
		  size_t... CSAs> // Compile time subvector arguments
inline bool isSame(const Vector<VT, TF> &a, const Subvector<VT, AF, TF, DF, CSAs...> &b) noexcept
{
	return isSame(b, a);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Returns whether the two given subvectors represent the same observable state.
// \ingroup subvector
//
// \param a The first subvector to be tested for its state.
// \param b The second subvector to be tested for its state.
// \return \a true in case the two subvectors share a state, \a false otherwise.
//
// This overload of the isSame function tests if the two given subvectors refer to exactly the
// same range of the same vector. In case both subvectors represent the same observable state,
// the function returns \a true, otherwise it returns \a false.
*/
template <typename VT1 // Type of the vector of the left-hand side subvector
		  ,
		  AlignmentFlag AF1 // Alignment flag of the left-hand side subvector
		  ,
		  bool TF1 // Transpose flag of the left-hand side subvector
		  ,
		  bool DF1 // Density flag of the left-hand side subvector
		  ,
		  size_t... CSAs1 // Compile time subvector arguments of the left-hand side subvector
		  ,
		  typename VT2 // Type of the vector of the right-hand side subvector
		  ,
		  AlignmentFlag AF2 // Alignment flag of the right-hand side subvector
		  ,
		  bool TF2 // Transpose flag of the right-hand side subvector
		  ,
		  bool DF2 // Density flag of the right-hand side subvector
		  ,
		  size_t... CSAs2> // Compile time subvector arguments of the right-hand side subvector
inline bool isSame(const Subvector<VT1, AF1, TF1, DF1, CSAs1...> &a,
				   const Subvector<VT2, AF2, TF2, DF2, CSAs2...> &b) noexcept
{
	return (isSame(a.operand(), b.operand()) && (a.offset() == b.offset()) && (a.size() == b.size()));
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Predict invariant violations by setting a single element of a subvector.
// \ingroup subvector
//
// \param sv The target subvector.
// \param index The index of the element to be set.
// \param value The value to be set to the element.
// \return \a true in case the operation would be successful, \a false if not.
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename VT // Type of the vector
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  bool TF // Transpose flag
		  ,
		  bool DF // Density flag
		  ,
		  size_t... CSAs // Compile time subvector arguments
		  ,
		  typename ET> // Type of the element
inline bool trySet(const Subvector<VT, AF, TF, DF, CSAs...> &sv, size_t index, const ET &value)
{
	METRIC_NUMERIC_INTERNAL_ASSERT(index < sv.size(), "Invalid vector access index");

	return trySet(sv.operand(), sv.offset() + index, value);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Predict invariant violations by setting a range of elements of a subvector.
// \ingroup subvector
//
// \param sv The target subvector.
// \param index The index of the first element of the range to be modified.
// \param size The number of elements of the range to be modified.
// \param value The value to be set to the range of elements.
// \return \a true in case the operation would be successful, \a false if not.
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename VT // Type of the vector
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  bool TF // Transpose flag
		  ,
		  bool DF // Density flag
		  ,
		  size_t... CSAs // Compile time subvector arguments
		  ,
		  typename ET> // Type of the element
METRIC_NUMERIC_ALWAYS_INLINE bool trySet(const Subvector<VT, AF, TF, DF, CSAs...> &sv, size_t index, size_t size,
										 const ET &value)
{
	METRIC_NUMERIC_INTERNAL_ASSERT(index <= (*sv).size(), "Invalid vector access index");
	METRIC_NUMERIC_INTERNAL_ASSERT(index + size <= (*sv).size(), "Invalid range size");

	return trySet(sv.operand(), sv.offset() + index, size, value);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Predict invariant violations by adding to a single element of a subvector.
// \ingroup subvector
//
// \param sv The target subvector.
// \param index The index of the element to be modified.
// \param value The value to be added to the element.
// \return \a true in case the operation would be successful, \a false if not.
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename VT // Type of the vector
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  bool TF // Transpose flag
		  ,
		  bool DF // Density flag
		  ,
		  size_t... CSAs // Compile time subvector arguments
		  ,
		  typename ET> // Type of the element
inline bool tryAdd(const Subvector<VT, AF, TF, DF, CSAs...> &sv, size_t index, const ET &value)
{
	METRIC_NUMERIC_INTERNAL_ASSERT(index < sv.size(), "Invalid vector access index");

	return tryAdd(sv.operand(), sv.offset() + index, value);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Predict invariant violations by adding to a range of elements of a subvector.
// \ingroup subvector
//
// \param sv The target subvector.
// \param index The index of the first element of the range to be modified.
// \param size The number of elements of the range to be modified.
// \param value The value to be added to the range of elements.
// \return \a true in case the operation would be successful, \a false if not.
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename VT // Type of the vector
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  bool TF // Transpose flag
		  ,
		  bool DF // Density flag
		  ,
		  size_t... CSAs // Compile time subvector arguments
		  ,
		  typename ET> // Type of the element
METRIC_NUMERIC_ALWAYS_INLINE bool tryAdd(const Subvector<VT, AF, TF, DF, CSAs...> &sv, size_t index, size_t size,
										 const ET &value)
{
	METRIC_NUMERIC_INTERNAL_ASSERT(index <= (*sv).size(), "Invalid vector access index");
	METRIC_NUMERIC_INTERNAL_ASSERT(index + size <= (*sv).size(), "Invalid range size");

	return tryAdd(sv.operand(), sv.offset() + index, size, value);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Predict invariant violations by subtracting from a single element of a subvector.
// \ingroup subvector
//
// \param sv The target subvector.
// \param index The index of the element to be modified.
// \param value The value to be subtracting from the element.
// \return \a true in case the operation would be successful, \a false if not.
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename VT // Type of the vector
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  bool TF // Transpose flag
		  ,
		  bool DF // Density flag
		  ,
		  size_t... CSAs // Compile time subvector arguments
		  ,
		  typename ET> // Type of the element
inline bool trySub(const Subvector<VT, AF, TF, DF, CSAs...> &sv, size_t index, const ET &value)
{
	METRIC_NUMERIC_INTERNAL_ASSERT(index < sv.size(), "Invalid vector access index");

	return trySub(sv.operand(), sv.offset() + index, value);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Predict invariant violations by subtracting from a range of elements of a subvector.
// \ingroup subvector
//
// \param sv The target subvector.
// \param index The index of the first element of the range to be modified.
// \param size The number of elements of the range to be modified.
// \param value The value to be subtracted from the range of elements.
// \return \a true in case the operation would be successful, \a false if not.
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename VT // Type of the vector
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  bool TF // Transpose flag
		  ,
		  bool DF // Density flag
		  ,
		  size_t... CSAs // Compile time subvector arguments
		  ,
		  typename ET> // Type of the element
METRIC_NUMERIC_ALWAYS_INLINE bool trySub(const Subvector<VT, AF, TF, DF, CSAs...> &sv, size_t index, size_t size,
										 const ET &value)
{
	METRIC_NUMERIC_INTERNAL_ASSERT(index <= (*sv).size(), "Invalid vector access index");
	METRIC_NUMERIC_INTERNAL_ASSERT(index + size <= (*sv).size(), "Invalid range size");

	return trySub(sv.operand(), sv.offset() + index, size, value);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Predict invariant violations by scaling a single element of a subvector.
// \ingroup subvector
//
// \param sv The target subvector.
// \param index The index of the element to be modified.
// \param value The factor for the element.
// \return \a true in case the operation would be successful, \a false if not.
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename VT // Type of the vector
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  bool TF // Transpose flag
		  ,
		  bool DF // Density flag
		  ,
		  size_t... CSAs // Compile time subvector arguments
		  ,
		  typename ET> // Type of the element
inline bool tryMult(const Subvector<VT, AF, TF, DF, CSAs...> &sv, size_t index, const ET &value)
{
	METRIC_NUMERIC_INTERNAL_ASSERT(index < sv.size(), "Invalid vector access index");

	return tryMult(sv.operand(), sv.offset() + index, value);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Predict invariant violations by scaling a range of elements of a subvector.
// \ingroup subvector
//
// \param sv The target subvector.
// \param index The index of the first element of the range to be modified.
// \param size The number of elements of the range to be modified.
// \param value The factor for the elements.
// \return \a true in case the operation would be successful, \a false if not.
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename VT // Type of the vector
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  bool TF // Transpose flag
		  ,
		  bool DF // Density flag
		  ,
		  size_t... CSAs // Compile time subvector arguments
		  ,
		  typename ET> // Type of the element
METRIC_NUMERIC_ALWAYS_INLINE bool tryMult(const Subvector<VT, AF, TF, DF, CSAs...> &sv, size_t index, size_t size,
										  const ET &value)
{
	METRIC_NUMERIC_INTERNAL_ASSERT(index <= (*sv).size(), "Invalid vector access index");
	METRIC_NUMERIC_INTERNAL_ASSERT(index + size <= (*sv).size(), "Invalid range size");

	return tryMult(sv.operand(), sv.offset() + index, size, value);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Predict invariant violations by scaling a single element of a subvector.
// \ingroup subvector
//
// \param sv The target subvector.
// \param index The index of the element to be modified.
// \param value The divisor for the element.
// \return \a true in case the operation would be successful, \a false if not.
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename VT // Type of the vector
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  bool TF // Transpose flag
		  ,
		  bool DF // Density flag
		  ,
		  size_t... CSAs // Compile time subvector arguments
		  ,
		  typename ET> // Type of the element
inline bool tryDiv(const Subvector<VT, AF, TF, DF, CSAs...> &sv, size_t index, const ET &value)
{
	METRIC_NUMERIC_INTERNAL_ASSERT(index < sv.size(), "Invalid vector access index");

	return tryDiv(sv.operand(), sv.offset() + index, value);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Predict invariant violations by scaling a range of elements of a subvector.
// \ingroup subvector
//
// \param sv The target subvector.
// \param index The index of the first element of the range to be modified.
// \param size The number of elements of the range to be modified.
// \param value The divisor for the elements.
// \return \a true in case the operation would be successful, \a false if not.
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename VT // Type of the vector
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  bool TF // Transpose flag
		  ,
		  bool DF // Density flag
		  ,
		  size_t... CSAs // Compile time subvector arguments
		  ,
		  typename ET> // Type of the element
METRIC_NUMERIC_ALWAYS_INLINE bool tryDiv(const Subvector<VT, AF, TF, DF, CSAs...> &sv, size_t index, size_t size,
										 const ET &value)
{
	METRIC_NUMERIC_INTERNAL_ASSERT(index <= (*sv).size(), "Invalid vector access index");
	METRIC_NUMERIC_INTERNAL_ASSERT(index + size <= (*sv).size(), "Invalid range size");

	return tryDiv(sv.operand(), sv.offset() + index, size, value);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Predict invariant violations by shifting a single element of a subvector.
// \ingroup subvector
//
// \param sv The target subvector.
// \param index The index of the element to be modified.
// \param count The number of bits to shift the element.
// \return \a true in case the operation would be successful, \a false if not.
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename VT // Type of the vector
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  bool TF // Transpose flag
		  ,
		  bool DF // Density flag
		  ,
		  size_t... CSAs> // Compile time subvector arguments
inline bool tryShift(const Subvector<VT, AF, TF, DF, CSAs...> &sv, size_t index, int count)
{
	METRIC_NUMERIC_INTERNAL_ASSERT(index < sv.size(), "Invalid vector access index");

	return tryShift(sv.operand(), sv.offset() + index, count);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Predict invariant violations by shifting a range of elements of a subvector.
// \ingroup subvector
//
// \param sv The target subvector.
// \param index The index of the first element of the range to be modified.
// \param size The number of elements of the range to be modified.
// \param count The number of bits to shift the range of elements.
// \return \a true in case the operation would be successful, \a false if not.
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename VT // Type of the vector
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  bool TF // Transpose flag
		  ,
		  bool DF // Density flag
		  ,
		  size_t... CSAs> // Compile time subvector arguments
METRIC_NUMERIC_ALWAYS_INLINE bool tryShift(const Subvector<VT, AF, TF, DF, CSAs...> &sv, size_t index, size_t size,
										   int count)
{
	METRIC_NUMERIC_INTERNAL_ASSERT(index <= (*sv).size(), "Invalid vector access index");
	METRIC_NUMERIC_INTERNAL_ASSERT(index + size <= (*sv).size(), "Invalid range size");

	return tryShift(sv.operand(), sv.offset() + index, size, count);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Predict invariant violations by a bitwise AND on a single element of a subvector.
// \ingroup subvector
//
// \param sv The target subvector.
// \param index The index of the element to be modified.
// \param value The bit pattern to be used on the element.
// \return \a true in case the operation would be successful, \a false if not.
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename VT // Type of the vector
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  bool TF // Transpose flag
		  ,
		  bool DF // Density flag
		  ,
		  size_t... CSAs // Compile time subvector arguments
		  ,
		  typename ET> // Type of the element
inline bool tryBitand(const Subvector<VT, AF, TF, DF, CSAs...> &sv, size_t index, const ET &value)
{
	METRIC_NUMERIC_INTERNAL_ASSERT(index < sv.size(), "Invalid vector access index");

	return tryBitand(sv.operand(), sv.offset() + index, value);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Predict invariant violations by a bitwise AND on a range of elements of a subvector.
// \ingroup subvector
//
// \param sv The target subvector.
// \param index The index of the first element of the range to be modified.
// \param size The number of elements of the range to be modified.
// \param value The bit pattern to be used on the range of elements.
// \return \a true in case the operation would be successful, \a false if not.
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename VT // Type of the vector
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  bool TF // Transpose flag
		  ,
		  bool DF // Density flag
		  ,
		  size_t... CSAs // Compile time subvector arguments
		  ,
		  typename ET> // Type of the element
METRIC_NUMERIC_ALWAYS_INLINE bool tryBitand(const Subvector<VT, AF, TF, DF, CSAs...> &sv, size_t index, size_t size,
											const ET &value)
{
	METRIC_NUMERIC_INTERNAL_ASSERT(index <= (*sv).size(), "Invalid vector access index");
	METRIC_NUMERIC_INTERNAL_ASSERT(index + size <= (*sv).size(), "Invalid range size");

	return tryBitand(sv.operand(), sv.offset() + index, size, value);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Predict invariant violations by a bitwise OR on a single element of a subvector.
// \ingroup subvector
//
// \param sv The target subvector.
// \param index The index of the element to be modified.
// \param value The bit pattern to be used on the element.
// \return \a true in case the operation would be successful, \a false if not.
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename VT // Type of the vector
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  bool TF // Transpose flag
		  ,
		  bool DF // Density flag
		  ,
		  size_t... CSAs // Compile time subvector arguments
		  ,
		  typename ET> // Type of the element
inline bool tryBitor(const Subvector<VT, AF, TF, DF, CSAs...> &sv, size_t index, const ET &value)
{
	METRIC_NUMERIC_INTERNAL_ASSERT(index < sv.size(), "Invalid vector access index");

	return tryBitor(sv.operand(), sv.offset() + index, value);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Predict invariant violations by a bitwise OR on a range of elements of a subvector.
// \ingroup subvector
//
// \param sv The target subvector.
// \param index The index of the first element of the range to be modified.
// \param size The number of elements of the range to be modified.
// \param value The bit pattern to be used on the range of elements.
// \return \a true in case the operation would be successful, \a false if not.
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename VT // Type of the vector
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  bool TF // Transpose flag
		  ,
		  bool DF // Density flag
		  ,
		  size_t... CSAs // Compile time subvector arguments
		  ,
		  typename ET> // Type of the element
METRIC_NUMERIC_ALWAYS_INLINE bool tryBitor(const Subvector<VT, AF, TF, DF, CSAs...> &sv, size_t index, size_t size,
										   const ET &value)
{
	METRIC_NUMERIC_INTERNAL_ASSERT(index <= (*sv).size(), "Invalid vector access index");
	METRIC_NUMERIC_INTERNAL_ASSERT(index + size <= (*sv).size(), "Invalid range size");

	return tryBitor(sv.operand(), sv.offset() + index, size, value);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Predict invariant violations by a bitwise XOR on a single element of a subvector.
// \ingroup subvector
//
// \param sv The target subvector.
// \param index The index of the element to be modified.
// \param value The bit pattern to be used on the element.
// \return \a true in case the operation would be successful, \a false if not.
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename VT // Type of the vector
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  bool TF // Transpose flag
		  ,
		  bool DF // Density flag
		  ,
		  size_t... CSAs // Compile time subvector arguments
		  ,
		  typename ET> // Type of the element
inline bool tryBitxor(const Subvector<VT, AF, TF, DF, CSAs...> &sv, size_t index, const ET &value)
{
	METRIC_NUMERIC_INTERNAL_ASSERT(index < sv.size(), "Invalid vector access index");

	return tryBitxor(sv.operand(), sv.offset() + index, value);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Predict invariant violations by a bitwise XOR on a range of elements of a subvector.
// \ingroup subvector
//
// \param sv The target subvector.
// \param index The index of the first element of the range to be modified.
// \param size The number of elements of the range to be modified.
// \param value The bit pattern to be used on the range of elements.
// \return \a true in case the operation would be successful, \a false if not.
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename VT // Type of the vector
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  bool TF // Transpose flag
		  ,
		  bool DF // Density flag
		  ,
		  size_t... CSAs // Compile time subvector arguments
		  ,
		  typename ET> // Type of the element
METRIC_NUMERIC_ALWAYS_INLINE bool tryBitxor(const Subvector<VT, AF, TF, DF, CSAs...> &sv, size_t index, size_t size,
											const ET &value)
{
	METRIC_NUMERIC_INTERNAL_ASSERT(index <= (*sv).size(), "Invalid vector access index");
	METRIC_NUMERIC_INTERNAL_ASSERT(index + size <= (*sv).size(), "Invalid range size");

	return tryBitxor(sv.operand(), sv.offset() + index, size, value);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Predict invariant violations by the assignment of a vector to a subvector.
// \ingroup subvector
//
// \param lhs The target left-hand side subvector.
// \param rhs The right-hand side vector to be assigned.
// \param index The index of the first element to be modified.
// \return \a true in case the assignment would be successful, \a false if not.
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename VT1 // Type of the vector
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  bool TF // Transpose flag
		  ,
		  bool DF // Density flag
		  ,
		  size_t... CSAs // Compile time subvector arguments
		  ,
		  typename VT2> // Type of the right-hand side vector
inline bool tryAssign(const Subvector<VT1, AF, TF, DF, CSAs...> &lhs, const Vector<VT2, TF> &rhs, size_t index)
{
	METRIC_NUMERIC_INTERNAL_ASSERT(index <= lhs.size(), "Invalid vector access index");
	METRIC_NUMERIC_INTERNAL_ASSERT(index + (*rhs).size() <= lhs.size(), "Invalid vector size");

	return tryAssign(lhs.operand(), *rhs, lhs.offset() + index);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Predict invariant violations by the addition assignment of a vector to a subvector.
// \ingroup subvector
//
// \param lhs The target left-hand side subvector.
// \param rhs The right-hand side vector to be added.
// \param index The index of the first element to be modified.
// \return \a true in case the assignment would be successful, \a false if not.
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename VT1 // Type of the vector
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  bool TF // Transpose flag
		  ,
		  bool DF // Density flag
		  ,
		  size_t... CSAs // Compile time subvector arguments
		  ,
		  typename VT2> // Type of the right-hand side vector
inline bool tryAddAssign(const Subvector<VT1, AF, TF, DF, CSAs...> &lhs, const Vector<VT2, TF> &rhs, size_t index)
{
	METRIC_NUMERIC_INTERNAL_ASSERT(index <= lhs.size(), "Invalid vector access index");
	METRIC_NUMERIC_INTERNAL_ASSERT(index + (*rhs).size() <= lhs.size(), "Invalid vector size");

	return tryAddAssign(lhs.operand(), *rhs, lhs.offset() + index);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Predict invariant violations by the subtraction assignment of a vector to a subvector.
// \ingroup subvector
//
// \param lhs The target left-hand side subvector.
// \param rhs The right-hand side vector to be subtracted.
// \param index The index of the first element to be modified.
// \return \a true in case the assignment would be successful, \a false if not.
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename VT1 // Type of the vector
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  bool TF // Transpose flag
		  ,
		  bool DF // Density flag
		  ,
		  size_t... CSAs // Compile time subvector arguments
		  ,
		  typename VT2> // Type of the right-hand side vector
inline bool trySubAssign(const Subvector<VT1, AF, TF, DF, CSAs...> &lhs, const Vector<VT2, TF> &rhs, size_t index)
{
	METRIC_NUMERIC_INTERNAL_ASSERT(index <= lhs.size(), "Invalid vector access index");
	METRIC_NUMERIC_INTERNAL_ASSERT(index + (*rhs).size() <= lhs.size(), "Invalid vector size");

	return trySubAssign(lhs.operand(), *rhs, lhs.offset() + index);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Predict invariant violations by the multiplication assignment of a vector to a subvector.
// \ingroup subvector
//
// \param lhs The target left-hand side subvector.
// \param rhs The right-hand side vector to be multiplied.
// \param index The index of the first element to be modified.
// \return \a true in case the assignment would be successful, \a false if not.
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename VT1 // Type of the vector
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  bool TF // Transpose flag
		  ,
		  bool DF // Density flag
		  ,
		  size_t... CSAs // Compile time subvector arguments
		  ,
		  typename VT2> // Type of the right-hand side vector
inline bool tryMultAssign(const Subvector<VT1, AF, TF, DF, CSAs...> &lhs, const Vector<VT2, TF> &rhs, size_t index)
{
	METRIC_NUMERIC_INTERNAL_ASSERT(index <= lhs.size(), "Invalid vector access index");
	METRIC_NUMERIC_INTERNAL_ASSERT(index + (*rhs).size() <= lhs.size(), "Invalid vector size");

	return tryMultAssign(lhs.operand(), *rhs, lhs.offset() + index);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Predict invariant violations by the division assignment of a vector to a subvector.
// \ingroup subvector
//
// \param lhs The target left-hand side subvector.
// \param rhs The right-hand side vector divisor.
// \param index The index of the first element to be modified.
// \return \a true in case the assignment would be successful, \a false if not.
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename VT1 // Type of the vector
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  bool TF // Transpose flag
		  ,
		  bool DF // Density flag
		  ,
		  size_t... CSAs // Compile time subvector arguments
		  ,
		  typename VT2> // Type of the right-hand side vector
inline bool tryDivAssign(const Subvector<VT1, AF, TF, DF, CSAs...> &lhs, const Vector<VT2, TF> &rhs, size_t index)
{
	METRIC_NUMERIC_INTERNAL_ASSERT(index <= lhs.size(), "Invalid vector access index");
	METRIC_NUMERIC_INTERNAL_ASSERT(index + (*rhs).size() <= lhs.size(), "Invalid vector size");

	return tryDivAssign(lhs.operand(), *rhs, lhs.offset() + index);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Predict invariant violations by the shift assignment of a vector to a subvector.
// \ingroup subvector
//
// \param lhs The target left-hand side subvector.
// \param rhs The right-hand side vector of bits to shift.
// \param index The index of the first element to be modified.
// \return \a true in case the assignment would be successful, \a false if not.
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename VT1 // Type of the vector
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  bool TF // Transpose flag
		  ,
		  bool DF // Density flag
		  ,
		  size_t... CSAs // Compile time subvector arguments
		  ,
		  typename VT2> // Type of the right-hand side vector
inline bool tryShiftAssign(const Subvector<VT1, AF, TF, DF, CSAs...> &lhs, const Vector<VT2, TF> &rhs, size_t index)
{
	METRIC_NUMERIC_INTERNAL_ASSERT(index <= lhs.size(), "Invalid vector access index");
	METRIC_NUMERIC_INTERNAL_ASSERT(index + (*rhs).size() <= lhs.size(), "Invalid vector size");

	return tryShiftAssign(lhs.operand(), *rhs, lhs.offset() + index);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Predict invariant violations by the bitwise AND assignment of a vector to a subvector.
// \ingroup subvector
//
// \param lhs The target left-hand side subvector.
// \param rhs The right-hand side vector for the bitwise AND operation.
// \param index The index of the first element to be modified.
// \return \a true in case the assignment would be successful, \a false if not.
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename VT1 // Type of the vector
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  bool TF // Transpose flag
		  ,
		  bool DF // Density flag
		  ,
		  size_t... CSAs // Compile time subvector arguments
		  ,
		  typename VT2> // Type of the right-hand side vector
inline bool tryBitandAssign(const Subvector<VT1, AF, TF, DF, CSAs...> &lhs, const Vector<VT2, TF> &rhs, size_t index)
{
	METRIC_NUMERIC_INTERNAL_ASSERT(index <= lhs.size(), "Invalid vector access index");
	METRIC_NUMERIC_INTERNAL_ASSERT(index + (*rhs).size() <= lhs.size(), "Invalid vector size");

	return tryBitandAssign(lhs.operand(), *rhs, lhs.offset() + index);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Predict invariant violations by the bitwise OR assignment of a vector to a subvector.
// \ingroup subvector
//
// \param lhs The target left-hand side subvector.
// \param rhs The right-hand side vector for the bitwise OR operation.
// \param index The index of the first element to be modified.
// \return \a true in case the assignment would be successful, \a false if not.
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename VT1 // Type of the vector
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  bool TF // Transpose flag
		  ,
		  bool DF // Density flag
		  ,
		  size_t... CSAs // Compile time subvector arguments
		  ,
		  typename VT2> // Type of the right-hand side vector
inline bool tryBitorAssign(const Subvector<VT1, AF, TF, DF, CSAs...> &lhs, const Vector<VT2, TF> &rhs, size_t index)
{
	METRIC_NUMERIC_INTERNAL_ASSERT(index <= lhs.size(), "Invalid vector access index");
	METRIC_NUMERIC_INTERNAL_ASSERT(index + (*rhs).size() <= lhs.size(), "Invalid vector size");

	return tryBitorAssign(lhs.operand(), *rhs, lhs.offset() + index);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Predict invariant violations by the bitwise XOR assignment of a vector to a subvector.
// \ingroup subvector
//
// \param lhs The target left-hand side subvector.
// \param rhs The right-hand side vector for the bitwise XOR operation.
// \param index The index of the first element to be modified.
// \return \a true in case the assignment would be successful, \a false if not.
//
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in erroneous results and/or in compilation errors. Instead of using this function use the
// assignment operator.
*/
template <typename VT1 // Type of the vector
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  bool TF // Transpose flag
		  ,
		  bool DF // Density flag
		  ,
		  size_t... CSAs // Compile time subvector arguments
		  ,
		  typename VT2> // Type of the right-hand side vector
inline bool tryBitxorAssign(const Subvector<VT1, AF, TF, DF, CSAs...> &lhs, const Vector<VT2, TF> &rhs, size_t index)
{
	METRIC_NUMERIC_INTERNAL_ASSERT(index <= lhs.size(), "Invalid vector access index");
	METRIC_NUMERIC_INTERNAL_ASSERT(index + (*rhs).size() <= lhs.size(), "Invalid vector size");

	return tryBitxorAssign(lhs.operand(), *rhs, lhs.offset() + index);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Removal of all restrictions on the data access to the given subvector.
// \ingroup subvector
//
// \param sv The subvector to be derestricted.
// \return Subvector without access restrictions.
//
// This function removes all restrictions on the data access to the given subvector. It returns a
// subvector that does provide the same interface but does not have any restrictions on the data
// access.\n
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in the violation of invariants, erroneous results and/or in compilation errors.
*/
template <typename VT // Type of the vector
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  bool TF // Transpose flag
		  ,
		  bool DF // Density flag
		  ,
		  size_t I // Index of the first element
		  ,
		  size_t N> // Number of elements
inline decltype(auto) derestrict(Subvector<VT, AF, TF, DF, I, N> &sv)
{
	return subvector<AF, I, N>(derestrict(sv.operand()), unchecked);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Removal of all restrictions on the data access to the given temporary subvector.
// \ingroup subvector
//
// \param sv The temporary subvector to be derestricted.
// \return Subvector without access restrictions.
//
// This function removes all restrictions on the data access to the given temporary subvector. It
// returns a subvector that does provide the same interface but does not have any restrictions on
// the data access.\n
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in the violation of invariants, erroneous results and/or in compilation errors.
*/
template <typename VT // Type of the vector
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  bool TF // Transpose flag
		  ,
		  bool DF // Density flag
		  ,
		  size_t I // Index of the first element
		  ,
		  size_t N> // Number of elements
inline decltype(auto) derestrict(Subvector<VT, AF, TF, DF, I, N> &&sv)
{
	return subvector<AF, I, N>(derestrict(sv.operand()), unchecked);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Removal of all restrictions on the data access to the given subvector.
// \ingroup subvector
//
// \param sv The subvector to be derestricted.
// \return Subvector without access restrictions.
//
// This function removes all restrictions on the data access to the given subvector. It returns a
// subvector that does provide the same interface but does not have any restrictions on the data
// access.\n
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in the violation of invariants, erroneous results and/or in compilation errors.
*/
template <typename VT // Type of the vector
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  bool TF // Transpose flag
		  ,
		  bool DF> // Density flag
inline decltype(auto) derestrict(Subvector<VT, AF, TF, DF> &sv)
{
	return subvector<AF>(derestrict(sv.operand()), sv.offset(), sv.size(), unchecked);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Removal of all restrictions on the data access to the given temporary subvector.
// \ingroup subvector
//
// \param sv The temporary subvector to be derestricted.
// \return Subvector without access restrictions.
//
// This function removes all restrictions on the data access to the given temporary subvector. It
// returns a subvector that does provide the same interface but does not have any restrictions on
// the data access.\n
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in the violation of invariants, erroneous results and/or in compilation errors.
*/
template <typename VT // Type of the vector
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  bool TF // Transpose flag
		  ,
		  bool DF> // Density flag
inline decltype(auto) derestrict(Subvector<VT, AF, TF, DF> &&sv)
{
	return subvector<AF>(derestrict(sv.operand()), sv.offset(), sv.size(), unchecked);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Returns a reference to the underlying vector of the given subvector.
// \ingroup subvector
//
// \param sv The given subvector.
// \return Reference to the underlying vector.
//
// This function returns a reference to the underlying vector of the given subvector.\n
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in the violation of invariants, erroneous results and/or in compilation errors.
*/
template <typename VT // Type of the vector
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  bool TF // Transpose flag
		  ,
		  bool DF // Density flag
		  ,
		  size_t... CSAs> // Compile time subvector arguments
inline decltype(auto) unview(Subvector<VT, AF, TF, DF, CSAs...> &sv)
{
	return sv.operand();
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Returns a reference to the underlying vector of the given constant subvector.
// \ingroup subvector
//
// \param sv The given constant subvector.
// \return Reference to the underlying vector.
//
// This function returns a reference to the underlying vector of the given constant subvector.\n
// This function must \b NOT be called explicitly! It is used internally for the performance
// optimized evaluation of expression templates. Calling this function explicitly might result
// in the violation of invariants, erroneous results and/or in compilation errors.
*/
template <typename VT // Type of the vector
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  bool TF // Transpose flag
		  ,
		  bool DF // Density flag
		  ,
		  size_t... CSAs> // Compile time subvector arguments
inline decltype(auto) unview(const Subvector<VT, AF, TF, DF, CSAs...> &sv)
{
	return sv.operand();
}
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  SIZE SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename VT, AlignmentFlag AF, bool TF, bool DF, size_t I, size_t N>
struct Size<Subvector<VT, AF, TF, DF, I, N>, 0UL> : public Ptrdiff_t<static_cast<ptrdiff_t>(N)> {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  MAXSIZE SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename VT, AlignmentFlag AF, bool TF, bool DF, size_t I, size_t N>
struct MaxSize<Subvector<VT, AF, TF, DF, I, N>, 0UL> : public Ptrdiff_t<static_cast<ptrdiff_t>(N)> {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  ISRESTRICTED SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename VT, AlignmentFlag AF, bool TF, bool DF, size_t... CSAs>
struct IsRestricted<Subvector<VT, AF, TF, DF, CSAs...>> : public IsRestricted<VT> {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  HASCONSTDATAACCESS SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename VT, AlignmentFlag AF, bool TF, size_t... CSAs>
struct HasConstDataAccess<Subvector<VT, AF, TF, true, CSAs...>> : public HasConstDataAccess<VT> {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  HASMUTABLEDATAACCESS SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename VT, AlignmentFlag AF, bool TF, size_t... CSAs>
struct HasMutableDataAccess<Subvector<VT, AF, TF, true, CSAs...>> : public HasMutableDataAccess<VT> {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  ISALIGNED SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename VT, bool TF, size_t... CSAs>
struct IsAligned<Subvector<VT, aligned, TF, true, CSAs...>> : public TrueType {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  ISCONTIGUOUS SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename VT, AlignmentFlag AF, bool TF, size_t... CSAs>
struct IsContiguous<Subvector<VT, AF, TF, true, CSAs...>> : public IsContiguous<VT> {};
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
