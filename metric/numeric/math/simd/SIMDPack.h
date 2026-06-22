// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_SIMD_SIMDPACK_H
#define METRIC_NUMERIC_MATH_SIMD_SIMDPACK_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/system/Inline.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Base class for all SIMD data types.
// \ingroup simd
//
// The SIMDPack class template is a base class for all SIMD data types within the Metric numeric library.
// It provides an abstraction from the actual type of the SIMD pack, but enables a conversion
// back to this type via the 'Curiously Recurring Template Pattern' (CRTP).
*/
template <typename T> // Type of the SIMD pack
class SIMDPack {
  public:
	//**Conversion operators************************************************************************
	/*!\name Conversion operators */
	//@{
	[[deprecated]] METRIC_NUMERIC_ALWAYS_INLINE constexpr T &operator~() noexcept;
	[[deprecated]] METRIC_NUMERIC_ALWAYS_INLINE constexpr const T &operator~() const noexcept;

	constexpr T &operator*() noexcept;
	constexpr const T &operator*() const noexcept;
	//@}
	//**********************************************************************************************

  protected:
	//**Special member functions********************************************************************
	/*!\name Special member functions */
	//@{
	SIMDPack() = default;
	SIMDPack(const SIMDPack &) = default;
	SIMDPack(SIMDPack &&) = default;
	~SIMDPack() = default;
	SIMDPack &operator=(const SIMDPack &) = default;
	SIMDPack &operator=(SIMDPack &&) = default;
	//@}
	//**********************************************************************************************
};
//*************************************************************************************************

//=================================================================================================
//
//  CONVERSION OPERATIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Conversion operator for non-constant SIMD packs.
//
// \return Mutable reference of the actual type of the SIMD pack.
//
// This operator performs the CRTP-based type-safe downcast to the actual type \a T of the
// SIMD pack. It will return a mutable reference to the actual type \a T.
*/
template <typename T> // Type of the SIMD pack
[[deprecated]] METRIC_NUMERIC_ALWAYS_INLINE constexpr T &SIMDPack<T>::operator~() noexcept
{
	return static_cast<T &>(*this);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Conversion operator for constant SIMD packs.
//
// \return Constant reference of the actual type of the SIMD pack.
//
// This operator performs the CRTP-based type-safe downcast to the actual type \a T of the
// SIMD pack. It will return a constant reference to the actual type \a T.
*/
template <typename T> // Type of the SIMD pack
[[deprecated]] METRIC_NUMERIC_ALWAYS_INLINE constexpr const T &SIMDPack<T>::operator~() const noexcept
{
	return static_cast<const T &>(*this);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Conversion operator for non-constant SIMD packs.
//
// \return Mutable reference of the actual type of the SIMD pack.
//
// This operator performs the CRTP-based down-cast to the actual type \a T of the SIMD pack.
// It will return a mutable reference to the actual type \a T.
*/
template <typename T> // Type of the SIMD pack
METRIC_NUMERIC_ALWAYS_INLINE constexpr T &SIMDPack<T>::operator*() noexcept
{
	return static_cast<T &>(*this);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Conversion operator for constant SIMD packs.
//
// \return Constant reference of the actual type of the SIMD pack.
//
// This operator performs the CRTP-based down-cast to the actual type \a T of the SIMD pack.
// It will return a constant reference to the actual type \a T.
*/
template <typename T> // Type of the SIMD pack
METRIC_NUMERIC_ALWAYS_INLINE constexpr const T &SIMDPack<T>::operator*() const noexcept
{
	return static_cast<const T &>(*this);
}
//*************************************************************************************************

//=================================================================================================
//
//  GLOBAL FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\name SIMDPack global functions */
//@{
template <typename T> T &crtp_cast(SIMDPack<T> &pack);

template <typename T> const T &crtp_cast(const SIMDPack<T> &pack);
//@}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief CRTP-based conversion operation for non-constant SIMD packs.
//
// \param pack The SIMD pack to be downcast.
// \return Mutable reference of the actual type of the SIMD pack.
//
// This operator performs the CRTP-based type-safe downcast to the actual type \a T of the
// SIMD pack. It will return a mutable reference to the actual type \a T.
*/
template <typename T> // Type of the SIMD pack
METRIC_NUMERIC_ALWAYS_INLINE T &crtp_cast(SIMDPack<T> &pack)
{
	return *pack;
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief CRTP-based conversion operation for constant SIMD packs.
//
// \param pack The SIMD pack to be downcast.
// \return Const reference of the actual type of the SIMD pack.
//
// This operator performs the CRTP-based type-safe downcast to the actual type \a T of the
// SIMD pack. It will return a constant reference to the actual type \a T.
*/
template <typename T> // Type of the SIMD pack
METRIC_NUMERIC_ALWAYS_INLINE const T &crtp_cast(const SIMDPack<T> &pack)
{
	return *pack;
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
