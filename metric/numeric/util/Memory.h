// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_MEMORY_H
#define METRIC_NUMERIC_UTIL_MEMORY_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <cstdlib>
#include <metric/numeric/system/Platform.h>
#include <metric/numeric/util/Assert.h>
#include <metric/numeric/util/EnableIf.h>
#include <metric/numeric/util/Exception.h>
#include <metric/numeric/util/Types.h>
#include <metric/numeric/util/algorithms/ConstructAt.h>
#include <metric/numeric/util/algorithms/Destroy.h>
#include <metric/numeric/util/algorithms/DestroyAt.h>
#include <metric/numeric/util/typetraits/AlignmentOf.h>
#include <metric/numeric/util/typetraits/IsBuiltin.h>
#include <new>

#if METRIC_NUMERIC_WIN32_PLATFORM || METRIC_NUMERIC_WIN64_PLATFORM || METRIC_NUMERIC_MINGW32_PLATFORM ||               \
	METRIC_NUMERIC_MINGW64_PLATFORM
#include <malloc.h>
#endif

namespace mtrc::numeric {

//=================================================================================================
//
//  BYTE-BASED ALLOCATION FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Aligned array allocation.
// \ingroup util
//
// \param size The number of bytes to be allocated.
// \param alignment The required minimum alignment.
// \return Byte pointer to the first element of the aligned array.
// \exception std::bad_alloc Allocation failed.
//
// This function provides the functionality to allocate memory based on the given alignment
// restrictions. For that purpose it uses the according system-specific memory allocation
// functions.
*/
inline byte_t *alignedAllocate(size_t size, size_t alignment)
{
	void *raw(nullptr);

#if METRIC_NUMERIC_WIN32_PLATFORM || METRIC_NUMERIC_WIN64_PLATFORM || METRIC_NUMERIC_MINGW64_PLATFORM
	raw = _aligned_malloc(size, alignment);
	if (raw == nullptr) {
#elif METRIC_NUMERIC_MINGW32_PLATFORM
	raw = __mingw_aligned_malloc(size, alignment);
	if (raw == nullptr) {
#else
	alignment = (alignment < sizeof(void *) ? sizeof(void *) : alignment);
	if (size > 0U && posix_memalign(&raw, alignment, size)) {
#endif
		METRIC_NUMERIC_THROW_BAD_ALLOC;
	}

	return reinterpret_cast<byte_t *>(raw);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Deallocation of aligned memory.
// \ingroup util
//
// \param address The address of the first element of the array to be deallocated.
// \return void
//
// This function deallocates the given memory that was previously allocated via the
// alignedAllocate() function. For that purpose it uses the according system-specific memory
// deallocation functions.
*/
inline void alignedDeallocate(const void *address) noexcept
{
#if METRIC_NUMERIC_WIN32_PLATFORM || METRIC_NUMERIC_WIN64_PLATFORM || METRIC_NUMERIC_MINGW64_PLATFORM
	_aligned_free(const_cast<void *>(address));
#elif METRIC_NUMERIC_MINGW32_PLATFORM
	__mingw_aligned_free(const_cast<void *>(address));
#else
	free(const_cast<void *>(address));
#endif
}
//*************************************************************************************************

//=================================================================================================
//
//  TYPE-BASED ALLOCATION FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Aligned array allocation for built-in data types.
// \ingroup util
//
// \param size The number of elements of the given type to allocate.
// \return Pointer to the first element of the aligned array.
// \exception std::bad_alloc Allocation failed.
//
// The allocate() function provides the functionality to allocate memory based on the alignment
// restrictions of the given built-in data type. For instance, in case SSE vectorization is
// possible, the returned memory is guaranteed to be at least 16-byte aligned. In case AVX is
// active, the memory is even guaranteed to be at least 32-byte aligned.
//
// Examples:

   \code
   // Guaranteed to be 16-byte aligned (32-byte aligned in case AVX is used)
   double* dp = allocate<double>( 10UL );
   \endcode
*/
template <typename T, EnableIf_t<IsBuiltin_v<T>> * = nullptr> T *allocate(size_t size)
{
	constexpr size_t alignment(AlignmentOf_v<T>);

	return reinterpret_cast<T *>(alignedAllocate(size * sizeof(T), alignment));
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Aligned array allocation for user-specific class types.
// \ingroup util
//
// \param size The number of elements of the given type to allocate.
// \return Pointer to the first element of the aligned array.
// \exception std::bad_alloc Allocation failed.
//
// The allocate() function provides the functionality to allocate memory based on the alignment
// restrictions of the given user-specific class type. For instance, in case the given type has
// the requirement to be 32-byte aligned, the returned pointer is guaranteed to be 32-byte
// aligned. Additionally, all elements of the array are guaranteed to be default constructed.
// Note that the allocate() function provides exception safety similar to the new operator: In
// case any element throws an exception during construction, all elements that have already been
// constructed are destroyed in reverse order and the allocated memory is deallocated again.
*/
template <typename T, DisableIf_t<IsBuiltin_v<T>> * = nullptr> T *allocate(size_t size)
{
	constexpr size_t alignment(AlignmentOf_v<T>);
	constexpr size_t headersize((sizeof(size_t) < alignment) ? (alignment) : (sizeof(size_t)));

	METRIC_NUMERIC_INTERNAL_ASSERT(headersize >= alignment, "Invalid header size detected");
	METRIC_NUMERIC_INTERNAL_ASSERT(headersize % alignment == 0UL, "Invalid header size detected");

	byte_t *const raw(alignedAllocate(size * sizeof(T) + headersize, alignment));

	*reinterpret_cast<size_t *>(raw) = size;

	T *const address(reinterpret_cast<T *>(raw + headersize));
	size_t i(0UL);

	try {
		for (; i < size; ++i) {
			mtrc::numeric::construct_at(address + i);
		}
	} catch (...) {
		for (; i > 0UL; --i) {
			mtrc::numeric::destroy_at(address + i);
		}
		alignedDeallocate(raw);
		throw;
	}

	return address;
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Deallocation of memory for built-in data types.
// \ingroup util
//
// \param address The address of the first element of the array to be deallocated.
// \return void
//
// This function deallocates the given memory that was previously allocated via the allocate()
// function.
*/
template <typename T, EnableIf_t<IsBuiltin_v<T>> * = nullptr> void deallocate(T *address) noexcept
{
	if (address == nullptr)
		return;

	alignedDeallocate(address);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Deallocation of memory for user-specific class types.
// \ingroup util
//
// \param address The address of the first element of the array to be deallocated.
// \return void
//
// This function deallocates the given memory that was previously allocated via the allocate()
// function.
*/
template <typename T, DisableIf_t<IsBuiltin_v<T>> * = nullptr> void deallocate(T *address)
{
	if (address == nullptr)
		return;

	constexpr size_t alignment(AlignmentOf_v<T>);
	constexpr size_t headersize((sizeof(size_t) < alignment) ? (alignment) : (sizeof(size_t)));

	METRIC_NUMERIC_INTERNAL_ASSERT(headersize >= alignment, "Invalid header size detected");
	METRIC_NUMERIC_INTERNAL_ASSERT(headersize % alignment == 0UL, "Invalid header size detected");

	const byte_t *const raw = reinterpret_cast<byte_t *>(address) - headersize;
	const size_t size(*reinterpret_cast<const size_t *>(raw));

	mtrc::numeric::destroy_n(address, size);
	alignedDeallocate(raw);
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
