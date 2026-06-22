// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_VIEWS_ROW_ROWDATA_H
#define METRIC_NUMERIC_MATH_VIEWS_ROW_ROWDATA_H
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
/*!\brief Auxiliary class template for the data members of the Row class.
// \ingroup row
//
// The auxiliary RowData class template represents an abstraction of the data members of the
// Row class template. The necessary set of data members is selected depending on the number
// of compile time row arguments.
*/
template <size_t... CRAs> // Compile time row arguments
class RowData {};
//*************************************************************************************************

//=================================================================================================
//
//  CLASS TEMPLATE SPECIALIZATION FOR ZERO COMPILE TIME ROW INDICES
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the RowData class template for zero compile time row arguments.
// \ingroup row
//
// This specialization of RowData adapts the class template to the requirements of zero compile
// time row arguments.
*/
template <> class RowData<> {
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
	template <typename... RRAs> explicit inline RowData(size_t index, RRAs... args);

	RowData(const RowData &) = default;
	//@}
	//**********************************************************************************************

	//**Destructor**********************************************************************************
	/*!\name Destructor */
	//@{
	~RowData() = default;
	//@}
	//**********************************************************************************************

	//**Assignment operators************************************************************************
	/*!\name Assignment operators */
	//@{
	RowData &operator=(const RowData &) = delete;
	//@}
	//**********************************************************************************************

	//**Utility functions***************************************************************************
	/*!\name Utility functions */
	//@{
	inline size_t row() const noexcept;
	//@}
	//**********************************************************************************************

  private:
	//**Member variables****************************************************************************
	/*!\name Member variables */
	//@{
	const size_t row_; //!< The index of the row in the matrix.
					   //@}
					   //**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief The constructor for RowData.
//
// \param index The index of the row.
// \param args The optional row arguments.
*/
template <typename... RRAs>											// Optional row arguments
inline RowData<>::RowData(size_t index, RRAs... args) : row_(index) // The index of the row in the matrix
{
	MAYBE_UNUSED(args...);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Returns the index of the row of the underlying dense matrix.
//
// \return The index of the row.
*/
inline size_t RowData<>::row() const noexcept { return row_; }
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  CLASS TEMPLATE SPECIALIZATION FOR ONE COMPILE TIME ROW INDEX
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the RowData class template for a single compile time row argument.
// \ingroup row
//
// This specialization of RowData adapts the class template to the requirements of a single
// compile time row argument.
*/
template <size_t Index> // Compile time row index
class RowData<Index> {
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
	template <typename... RRAs> explicit inline RowData(RRAs... args);

	RowData(const RowData &) = default;
	//@}
	//**********************************************************************************************

	//**Destructor**********************************************************************************
	/*!\name Destructor */
	//@{
	~RowData() = default;
	//@}
	//**********************************************************************************************

	//**Assignment operators************************************************************************
	/*!\name Assignment operators */
	//@{
	RowData &operator=(const RowData &) = delete;
	//@}
	//**********************************************************************************************

	//**Utility functions***************************************************************************
	/*!\name Utility functions */
	//@{
	static constexpr size_t row() noexcept;
	//@}
	//**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief The constructor for RowData.
//
// \param args The optional row arguments.
*/
template <size_t Index>		// Compile time row index
template <typename... RRAs> // Optional row arguments
inline RowData<Index>::RowData(RRAs... args)
{
	MAYBE_UNUSED(args...);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Returns the index of the row of the underlying dense matrix.
//
// \return The index of the row.
*/
template <size_t Index> // Compile time row index
constexpr size_t RowData<Index>::row() noexcept
{
	return Index;
}
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
