// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_VIEWS_BAND_BANDDATA_H
#define METRIC_NUMERIC_MATH_VIEWS_BAND_BANDDATA_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/MaybeUnused.h>
#include <metric/numeric/util/Types.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Auxiliary class template for the data members of the Band class.
// \ingroup band
//
// The auxiliary BandData class template represents an abstraction of the data members of the
// Band class template. The necessary set of data member is selected depending on the number
// of compile time band arguments.
*/
template <ptrdiff_t... CBAs> // Compile time band arguments
class BandData {};
//*************************************************************************************************

//=================================================================================================
//
//  CLASS TEMPLATE SPECIALIZATION FOR ZERO COMPILE TIME BAND INDICES
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the BandData class template for zero compile time band arguments.
// \ingroup band
//
// This specialization of BandData adapts the class template to the requirements of zero compile
// time band arguments.
*/
template <> class BandData<> {
  public:
	//**Compile time flags**************************************************************************
	//! Compilation flag for compile time optimization.
	/*! The \a compileTimeArgs compilation flag indicates whether the view has been created by
		means of compile time arguments and whether these arguments can be queried at compile
		time. In that case, the \a compileTimeArgs compilation flag is set to \a true, otherwise
		it is set to \a false. */
	static constexpr bool compileTimeArgs = false;
	//**********************************************************************************************

	//**Constructors********************************************************************************
	/*!\name Constructors */
	//@{
	template <typename... RBAs> explicit inline BandData(ptrdiff_t index, RBAs... args);

	BandData(const BandData &) = default;
	//@}
	//**********************************************************************************************

	//**Destructor**********************************************************************************
	/*!\name Destructor */
	//@{
	~BandData() = default;
	//@}
	//**********************************************************************************************

	//**Assignment operators************************************************************************
	/*!\name Assignment operators */
	//@{
	BandData &operator=(const BandData &) = delete;
	//@}
	//**********************************************************************************************

	//**Utility functions***************************************************************************
	/*!\name Utility functions */
	//@{
	inline ptrdiff_t band() const noexcept;
	inline size_t row() const noexcept;
	inline size_t column() const noexcept;
	//@}
	//**********************************************************************************************

  private:
	//**Member variables****************************************************************************
	/*!\name Member variables */
	//@{
	const ptrdiff_t band_; //!< The band index.
	const size_t row_;	   //!< The index of the row containing the first element of the band.
	const size_t column_;  //!< The index of the column containing the first element of the band.
						   //@}
	//**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief The constructor for BandData.
//
// \param index The index of the band.
// \param args The optional band arguments.
*/
template <typename... RBAs> // Optional band arguments
inline BandData<>::BandData(ptrdiff_t index, RBAs... args)
	: band_(index) // The band index
	  ,
	  row_(index >= 0L ? 0UL : -index) // The index of the row containing the first element of the band
	  ,
	  column_(index >= 0L ? index : 0UL) // The index of the column containing the first element of the band
{
	MAYBE_UNUSED(args...);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Returns the index of the band of the underlying dense matrix.
//
// \return The index of the band.
*/
inline ptrdiff_t BandData<>::band() const noexcept { return band_; }
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Returns the index of the row containing the first element of the band.
//
// \return The first row index.
*/
inline size_t BandData<>::row() const noexcept { return row_; }
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Returns the index of the column containing the first element of the band.
//
// \return The first column index.
*/
inline size_t BandData<>::column() const noexcept { return column_; }
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  CLASS TEMPLATE SPECIALIZATION FOR ONE COMPILE TIME BAND INDEX
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the BandData class template for a single compile time band argument.
// \ingroup band
//
// This specialization of BandData adapts the class template to the requirements of a single
// compile time band argument.
*/
template <ptrdiff_t I> // Compile time band index
class BandData<I> {
  public:
	//**Compile time flags**************************************************************************
	//! Compilation flag for compile time optimization.
	/*! The \a compileTimeArgs compilation flag indicates whether the view has been created by
		means of compile time arguments and whether these arguments can be queried at compile
		time. In that case, the \a compileTimeArgs compilation flag is set to \a true, otherwise
		it is set to \a false. */
	static constexpr bool compileTimeArgs = true;
	//**********************************************************************************************

	//**Constructors********************************************************************************
	/*!\name Constructors */
	//@{
	template <typename... RBAs> explicit inline BandData(RBAs... args);

	BandData(const BandData &) = default;
	//@}
	//**********************************************************************************************

	//**Destructor**********************************************************************************
	/*!\name Destructor */
	//@{
	~BandData() = default;
	//@}
	//**********************************************************************************************

	//**Assignment operators************************************************************************
	/*!\name Assignment operators */
	//@{
	BandData &operator=(const BandData &) = delete;
	//@}
	//**********************************************************************************************

	//**Utility functions***************************************************************************
	/*!\name Utility functions */
	//@{
	static constexpr ptrdiff_t band() noexcept;
	static constexpr size_t row() noexcept;
	static constexpr size_t column() noexcept;
	//@}
	//**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief The constructor for BandData.
//
// \param args The optional band arguments.
*/
template <ptrdiff_t I>		// Compile time band index
template <typename... RBAs> // Optional band arguments
inline BandData<I>::BandData(RBAs... args)
{
	MAYBE_UNUSED(args...);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Returns the index of the band of the underlying dense matrix.
//
// \return The index of the band.
*/
template <ptrdiff_t I> // Compile time band index
constexpr ptrdiff_t BandData<I>::band() noexcept
{
	return I;
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Returns the index of the row containing the first element of the band.
//
// \return The first row index.
*/
template <ptrdiff_t I> // Compile time band index
constexpr size_t BandData<I>::row() noexcept
{
	return (I >= 0L ? 0UL : -I);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Returns the index of the column containing the first element of the band.
//
// \return The first column index.
*/
template <ptrdiff_t I> // Compile time band index
constexpr size_t BandData<I>::column() noexcept
{
	return (I >= 0L ? I : 0UL);
}
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
