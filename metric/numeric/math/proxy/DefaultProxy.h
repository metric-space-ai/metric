// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_PROXY_DEFAULTPROXY_H
#define METRIC_NUMERIC_MATH_PROXY_DEFAULTPROXY_H
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
/*!\brief Default proxy backend for built-in and alternate user-specific class types.
// \ingroup math
//
// The DefaultProxy class serves as a backend for the Proxy class. It is used in case the data
// type represented by the proxy is a built-in or alternate user-specific class type. This proxy
// does not augment the Proxy interface by any additional interface.
*/
template <typename PT // Type of the proxy
		  ,
		  typename RT> // Type of the represented element
class DefaultProxy {
  public:
	//**Conversion operators************************************************************************
	/*!\name Conversion operators */
	//@{
	[[deprecated]] METRIC_NUMERIC_ALWAYS_INLINE PT &operator~() noexcept;
	[[deprecated]] METRIC_NUMERIC_ALWAYS_INLINE const PT &operator~() const noexcept;

	METRIC_NUMERIC_ALWAYS_INLINE PT &operator*() noexcept;
	METRIC_NUMERIC_ALWAYS_INLINE const PT &operator*() const noexcept;
	//@}
	//**********************************************************************************************

  protected:
	//**Special member functions********************************************************************
	/*!\name Special member functions */
	//@{
	DefaultProxy() = default;
	DefaultProxy(const DefaultProxy &) = default;
	DefaultProxy(DefaultProxy &&) = default;
	~DefaultProxy() = default;
	DefaultProxy &operator=(const DefaultProxy &) = default;
	DefaultProxy &operator=(DefaultProxy &&) = default;
	//@}
	//**********************************************************************************************
};
//*************************************************************************************************

//=================================================================================================
//
//  CONVERSION OPERATORS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Conversion operator for non-constant proxies.
//
// \return Mutable reference to the actual type of the proxy.
//
// This operator performs the CRTP-based type-safe downcast to the actual type \a PT of the
// proxy. It will return a mutable reference to the actual type \a PT.
*/
template <typename PT // Type of the proxy
		  ,
		  typename RT> // Type of the represented element
[[deprecated]] METRIC_NUMERIC_ALWAYS_INLINE PT &DefaultProxy<PT, RT>::operator~() noexcept
{
	return static_cast<PT &>(*this);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Conversion operator for constant proxies.
//
// \return Constant reference to the actual type of the proxy.
//
// This operator performs the CRTP-based type-safe downcast to the actual type \a PT of the
// proxy. It will return a constant reference to the actual type \a PT.
*/
template <typename PT // Type of the proxy
		  ,
		  typename RT> // Type of the represented element
[[deprecated]] METRIC_NUMERIC_ALWAYS_INLINE const PT &DefaultProxy<PT, RT>::operator~() const noexcept
{
	return static_cast<const PT &>(*this);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Conversion operator for non-constant proxies.
//
// \return Mutable reference to the actual type of the proxy.
//
// This operator performs the CRTP-based type-safe downcast to the actual type \a PT of the
// proxy. It will return a mutable reference to the actual type \a PT.
*/
template <typename PT // Type of the proxy
		  ,
		  typename RT> // Type of the represented element
METRIC_NUMERIC_ALWAYS_INLINE PT &DefaultProxy<PT, RT>::operator*() noexcept
{
	return static_cast<PT &>(*this);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Conversion operator for constant proxies.
//
// \return Constant reference to the actual type of the proxy.
//
// This operator performs the CRTP-based type-safe downcast to the actual type \a PT of the
// proxy. It will return a constant reference to the actual type \a PT.
*/
template <typename PT // Type of the proxy
		  ,
		  typename RT> // Type of the represented element
METRIC_NUMERIC_ALWAYS_INLINE const PT &DefaultProxy<PT, RT>::operator*() const noexcept
{
	return static_cast<const PT &>(*this);
}
//*************************************************************************************************

//=================================================================================================
//
//  GLOBAL FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\name DefaultProxy global functions */
//@{
template <typename PT, typename RT> PT &crtp_cast(DefaultProxy<PT, RT> &proxy);

template <typename PT, typename RT> const PT &crtp_cast(const DefaultProxy<PT, RT> &proxy);
//@}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief CRTP-based conversion operation for non-constant proxies.
//
// \param proxy The proxy to be downcast.
// \return Mutable reference of the actual type of the proxy.
//
// This operator performs the CRTP-based type-safe downcast to the actual type \a PT of the
// proxy. It will return a mutable reference to the actual type \a PT.
*/
template <typename PT // Type of the proxy
		  ,
		  typename RT> // Type of the represented element
METRIC_NUMERIC_ALWAYS_INLINE PT &crtp_cast(DefaultProxy<PT, RT> &proxy)
{
	return *proxy;
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief CRTP-based conversion operation for constant proxies.
//
// \param proxy The proxy to be downcast.
// \return Const reference of the actual type of the proxy.
//
// This operator performs the CRTP-based type-safe downcast to the actual type \a PT of the
// proxy. It will return a constant reference to the actual type \a PT.
*/
template <typename PT // Type of the proxy
		  ,
		  typename RT> // Type of the represented element
METRIC_NUMERIC_ALWAYS_INLINE const PT &crtp_cast(const DefaultProxy<PT, RT> &proxy)
{
	return *proxy;
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
