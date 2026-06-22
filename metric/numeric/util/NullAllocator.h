// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_NULLALLOCATOR_H
#define METRIC_NUMERIC_UTIL_NULLALLOCATOR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/MaybeUnused.h>
#include <metric/numeric/util/Memory.h>
#include <metric/numeric/util/Types.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Allocator returning nullptr.
// \ingroup util
//
// The NullAllocator class template represents an implementation of the allocator concept of
// the standard library and acts as a stand-in for situation where no memory allocation is
// required. The NullAllocator will never allocate any memory and will always return nullptr.
*/
template <typename T> class NullAllocator {
  public:
	//**Type definitions****************************************************************************
	using ValueType = T;			  //!< Type of the allocated values.
	using SizeType = size_t;		  //!< Size type of the null allocator.
	using DifferenceType = ptrdiff_t; //!< Difference type of the null allocator.

	// STL allocator requirements
	using value_type = ValueType;			//!< Type of the allocated values.
	using size_type = SizeType;				//!< Size type of the null allocator.
	using difference_type = DifferenceType; //!< Difference type of the null allocator.
	//**********************************************************************************************

	//**rebind class definition*********************************************************************
	/*!\brief Implementation of the NullAllocator rebind mechanism.
	 */
	template <typename U> struct rebind {
		using other = NullAllocator<U>; //!< Type of the other allocator.
	};
	//**********************************************************************************************

	//**Constructors********************************************************************************
	/*!\name Constructors */
	//@{
	NullAllocator() = default;

	template <typename U> inline NullAllocator(const NullAllocator<U> &);
	//@}
	//**********************************************************************************************

	//**Allocation functions************************************************************************
	/*!\name Allocation functions */
	//@{
	inline T *allocate(size_t numObjects);
	inline void deallocate(T *ptr, size_t numObjects) noexcept;
	//@}
	//**********************************************************************************************
};
//*************************************************************************************************

//=================================================================================================
//
//  CONSTRUCTOR
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Conversion constructor from different NullAllocator instances.
//
// \param allocator The other null allocator to be copied.
*/
template <typename T> template <typename U> inline NullAllocator<T>::NullAllocator(const NullAllocator<U> &allocator)
{
	MAYBE_UNUSED(allocator);
}
//*************************************************************************************************

//=================================================================================================
//
//  ALLOCATION FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Performs no memory allocation and returns nullptr.
//
// \param numObjects The number of objects to be allocated.
// \return Pointer to the newly allocated memory.
//
// This function does not perform any memory allocation and always returns nullptr.
*/
template <typename T> inline T *NullAllocator<T>::allocate(size_t numObjects)
{
	MAYBE_UNUSED(numObjects);

	return static_cast<T *>(nullptr);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Deallocation of memory.
//
// \param ptr The address of the first element of the array to be deallocated.
// \param numObjects The number of objects to be deallocated.
// \return void
//
// This function deallocates a junk of memory that was previously allocated via the allocate()
// function. Note that the argument \a numObjects must be equal ot the first argument of the call
// to allocate() that origianlly produced \a ptr.
*/
template <typename T> inline void NullAllocator<T>::deallocate(T *ptr, size_t numObjects) noexcept
{
	MAYBE_UNUSED(ptr, numObjects);
}
//*************************************************************************************************

//=================================================================================================
//
//  GLOBAL OPERATORS
//
//=================================================================================================

//*************************************************************************************************
/*!\name NullAllocator operators */
//@{
template <typename T1, typename T2>
inline bool operator==(const NullAllocator<T1> &lhs, const NullAllocator<T2> &rhs) noexcept;

template <typename T1, typename T2>
inline bool operator!=(const NullAllocator<T1> &lhs, const NullAllocator<T2> &rhs) noexcept;
//@}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Equality comparison between two NullAllocator objects.
//
// \param lhs The left-hand side null allocator.
// \param rhs The right-hand side null allocator.
// \return \a true.
*/
template <typename T1 // Type of the left-hand side null allocator
		  ,
		  typename T2> // Type of the right-hand side null allocator
inline bool operator==(const NullAllocator<T1> &lhs, const NullAllocator<T2> &rhs) noexcept
{
	MAYBE_UNUSED(lhs, rhs);
	return true;
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Inequality comparison between two NullAllocator objects.
//
// \param lhs The left-hand side null allocator.
// \param rhs The right-hand side null allocator.
// \return \a false.
*/
template <typename T1 // Type of the left-hand side null allocator
		  ,
		  typename T2> // Type of the right-hand side null allocator
inline bool operator!=(const NullAllocator<T1> &lhs, const NullAllocator<T2> &rhs) noexcept
{
	MAYBE_UNUSED(lhs, rhs);
	return false;
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
