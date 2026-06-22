// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_SMALLARRAY_SMALLARRAYDATA_H
#define METRIC_NUMERIC_UTIL_SMALLARRAY_SMALLARRAYDATA_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/Types.h>
#include <metric/numeric/util/typetraits/AlignmentOf.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Auxiliary class for the SmallArray class template.
// \ingroup util
//
// This auxiliary class template is used as backend by the SmallArray class template. It is
// responsible to provide a static storage of size \a N.
*/
template <typename T // Data type of the elements
		  ,
		  size_t N> // Number of preallocated elements
struct SmallArrayData {
  public:
	//**Constructors********************************************************************************
	/*!\name Constructors */
	//@{
	constexpr SmallArrayData() noexcept;
	//@}
	//**********************************************************************************************

	//**Data access functions***********************************************************************
	/*!\name Data access functions */
	//@{
	constexpr T *array() noexcept;
	constexpr const T *array() const noexcept;
	//@}
	//**********************************************************************************************

  private:
	//**Member variables****************************************************************************
	/*!\name Member variables */
	//@{
	alignas(AlignmentOf_v<T>) byte_t v_[N * sizeof(T)]; //!< The static storage.
														//@}
	//**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Default constructor for SmallArrayData. No element initialization is performed!
//
// \note This constructor does not perform any kind of element initialization!
*/
template <typename T // Data type of the elements
		  ,
		  size_t N> // Number of preallocated elements
constexpr SmallArrayData<T, N>::SmallArrayData() noexcept
// v_ is intentionally not initialized
{
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Returns a pointer to the first element of the static array.
//
// \return A pointer to the first element of the static array.
*/
template <typename T // Data type of the elements
		  ,
		  size_t N> // Number of preallocated elements
constexpr T *SmallArrayData<T, N>::array() noexcept
{
	return reinterpret_cast<T *>(v_);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Returns a pointer to the first element of the static array.
//
// \return A pointer to the first element of the static array.
*/
template <typename T // Data type of the elements
		  ,
		  size_t N> // Number of preallocated elements
constexpr const T *SmallArrayData<T, N>::array() const noexcept
{
	return reinterpret_cast<const T *>(v_);
}
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  CLASS TEMPLATE SPECIALIZATION FOR N = 0
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the SmallArrayData class template for N = 0.
// \ingroup util
//
// This specialization of SmallArrayData handles the request for zero static elements of type T.
*/
template <typename T> // Data type of the elements
struct SmallArrayData<T, 0UL> {
  public:
	//**Data access functions***********************************************************************
	/*!\name Data access functions */
	//@{
	constexpr T *array() noexcept;
	constexpr const T *array() const noexcept;
	//@}
	//**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Returns a pointer to the first element of the static array.
//
// \return A pointer to the first element of the static array.
*/
template <typename T> // Data type of the elements
constexpr T *SmallArrayData<T, 0UL>::array() noexcept
{
	return nullptr;
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Returns a pointer to the first element of the static array.
//
// \return A pointer to the first element of the static array.
*/
template <typename T> // Data type of the elements
constexpr const T *SmallArrayData<T, 0UL>::array() const noexcept
{
	return nullptr;
}
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
