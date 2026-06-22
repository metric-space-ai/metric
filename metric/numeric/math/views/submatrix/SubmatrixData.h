// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_VIEWS_SUBMATRIX_SUBMATRIXDATA_H
#define METRIC_NUMERIC_MATH_VIEWS_SUBMATRIX_SUBMATRIXDATA_H
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
/*!\brief Auxiliary class template for the data members of the Submatrix class.
// \ingroup submatrix
//
// The auxiliary SubmatrixData class template represents an abstraction of the data members of
// the Submatrix class template. The necessary set of data members is selected depending on the
// number of compile time submatrix arguments.
*/
template <size_t... CSAs> // Compile time submatrix arguments
class SubmatrixData {};
//*************************************************************************************************

//=================================================================================================
//
//  CLASS TEMPLATE SPECIALIZATION FOR ZERO COMPILE TIME ARGUMENTS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the SubmatrixData class template for zero compile time submatrix
//        arguments.
// \ingroup submatrix
//
// This specialization of SubmatrixData adapts the class template to the requirements of zero
// compile time submatrix arguments.
*/
template <> class SubmatrixData<> {
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
	template <typename... RSAs> inline SubmatrixData(size_t rindex, size_t cindex, size_t m, size_t n, RSAs... args);

	SubmatrixData(const SubmatrixData &) = default;
	//@}
	//**********************************************************************************************

	//**Destructor**********************************************************************************
	/*!\name Destructor */
	//@{
	~SubmatrixData() = default;
	//@}
	//**********************************************************************************************

	//**Assignment operators************************************************************************
	/*!\name Assignment operators */
	//@{
	SubmatrixData &operator=(const SubmatrixData &) = delete;
	//@}
	//**********************************************************************************************

	//**Utility functions***************************************************************************
	/*!\name Utility functions */
	//@{
	inline size_t row() const noexcept;
	inline size_t column() const noexcept;
	inline size_t rows() const noexcept;
	inline size_t columns() const noexcept;
	//@}
	//**********************************************************************************************

  private:
	//**Member variables****************************************************************************
	/*!\name Member variables */
	//@{
	const size_t row_;	  //!< The first row of the submatrix.
	const size_t column_; //!< The first column of the submatrix.
	const size_t m_;	  //!< The number of rows of the submatrix.
	const size_t n_;	  //!< The number of columns of the submatrix.
						  //@}
	//**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief The constructor for SubmatrixData.
//
// \param rindex The index of the first row of the submatrix in the given matrix.
// \param cindex The index of the first column of the submatrix in the given matrix.
// \param m The number of rows of the submatrix.
// \param n The number of columns of the submatrix.
// \param args The optional submatrix arguments.
*/
template <typename... RSAs> // Optional submatrix arguments
inline SubmatrixData<>::SubmatrixData(size_t rindex, size_t cindex, size_t m, size_t n, RSAs... args)
	: row_(rindex) // The first row of the submatrix
	  ,
	  column_(cindex) // The first column of the submatrix
	  ,
	  m_(m) // The number of rows of the submatrix
	  ,
	  n_(n) // The number of columns of the submatrix
{
	MAYBE_UNUSED(args...);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Returns the index of the first row of the submatrix in the underlying matrix.
//
// \return The index of the first row.
*/
inline size_t SubmatrixData<>::row() const noexcept { return row_; }
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Returns the index of the first column of the submatrix in the underlying matrix.
//
// \return The index of the first column.
*/
inline size_t SubmatrixData<>::column() const noexcept { return column_; }
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Returns the number of rows of the submatrix.
//
// \return The number of rows of the submatrix.
*/
inline size_t SubmatrixData<>::rows() const noexcept { return m_; }
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Returns the number of columns of the submatrix.
//
// \return The number of columns of the submatrix.
*/
inline size_t SubmatrixData<>::columns() const noexcept { return n_; }
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  CLASS TEMPLATE SPECIALIZATION FOR FOUR COMPILE TIME ARGUMENTS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the SubmatrixData class template for four compile time submatrix
//        arguments.
// \ingroup submatrix
//
// This specialization of SubmatrixData adapts the class template to the requirements of two
// compile time arguments.
*/
template <size_t I // Index of the first row
		  ,
		  size_t J // Index of the first column
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N> // Number of columns
class SubmatrixData<I, J, M, N> {
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
	template <typename... RSAs> explicit inline SubmatrixData(RSAs... args);

	SubmatrixData(const SubmatrixData &) = default;
	//@}
	//**********************************************************************************************

	//**Destructor**********************************************************************************
	/*!\name Destructor */
	//@{
	~SubmatrixData() = default;
	//@}
	//**********************************************************************************************

	//**Assignment operators************************************************************************
	/*!\name Assignment operators */
	//@{
	SubmatrixData &operator=(const SubmatrixData &) = delete;
	//@}
	//**********************************************************************************************

	//**Utility functions***************************************************************************
	/*!\name Utility functions */
	//@{
	static constexpr size_t row() noexcept;
	static constexpr size_t column() noexcept;
	static constexpr size_t rows() noexcept;
	static constexpr size_t columns() noexcept;
	//@}
	//**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief The constructor for SubmatrixData.
//
// \param args The optional submatrix arguments.
*/
template <size_t I // Index of the first row
		  ,
		  size_t J // Index of the first column
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N>			// Number of columns
template <typename... RSAs> // Optional submatrix arguments
inline SubmatrixData<I, J, M, N>::SubmatrixData(RSAs... args)
{
	MAYBE_UNUSED(args...);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Returns the index of the first row of the submatrix in the underlying matrix.
//
// \return The index of the first row.
*/
template <size_t I // Index of the first row
		  ,
		  size_t J // Index of the first column
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N> // Number of columns
constexpr size_t SubmatrixData<I, J, M, N>::row() noexcept
{
	return I;
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Returns the index of the first column of the submatrix in the underlying matrix.
//
// \return The index of the first column.
*/
template <size_t I // Index of the first row
		  ,
		  size_t J // Index of the first column
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N> // Number of columns
constexpr size_t SubmatrixData<I, J, M, N>::column() noexcept
{
	return J;
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Returns the number of rows of the submatrix.
//
// \return The number of rows of the submatrix.
*/
template <size_t I // Index of the first row
		  ,
		  size_t J // Index of the first column
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N> // Number of columns
constexpr size_t SubmatrixData<I, J, M, N>::rows() noexcept
{
	return M;
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Returns the number of columns of the submatrix.
//
// \return The number of columns of the submatrix.
*/
template <size_t I // Index of the first row
		  ,
		  size_t J // Index of the first column
		  ,
		  size_t M // Number of rows
		  ,
		  size_t N> // Number of columns
constexpr size_t SubmatrixData<I, J, M, N>::columns() noexcept
{
	return N;
}
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
