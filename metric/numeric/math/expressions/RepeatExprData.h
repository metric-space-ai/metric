// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_REPEATEXPRDATA_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_REPEATEXPRDATA_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/StaticAssert.h>
#include <metric/numeric/util/Types.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  FORWARD DECLARATIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Auxiliary class template for the data members of repeater expression classes.
// \ingroup math
//
// The auxiliary RepeatExprData class template represents an abstraction of the data members of
// repeater expression template classes. The necessary set of data member is selected depending
// on the number of compile time repeater arguments.
*/
template <size_t Dim // Number of dimensions
		  ,
		  size_t... CRAs> // Compile time repeater arguments
class RepeatExprData;
//*************************************************************************************************

//=================================================================================================
//
//  BASE TEMPLATE FOR MANY COMPILE TIME REPEATER ARGUMENTS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Base template of the RepeatExprData class template.
// \ingroup math
//
// The base template of RepeatExprData adapts the class template to the requirements of a many
// compile time repeater arguments.
*/
template <size_t Dim // Number of dimensions
		  ,
		  size_t... Rs> // Compile time repetitions
class RepeatExprData {
  public:
	//**Constructors********************************************************************************
	/*!\name Constructors */
	//@{
	constexpr RepeatExprData() noexcept;

	RepeatExprData(const RepeatExprData &) = default;
	//@}
	//**********************************************************************************************

	//**Destructor**********************************************************************************
	/*!\name Destructor */
	//@{
	~RepeatExprData() = default;
	//@}
	//**********************************************************************************************

	//**Assignment operators************************************************************************
	/*!\name Assignment operators */
	//@{
	RepeatExprData &operator=(const RepeatExprData &) = delete;
	//@}
	//**********************************************************************************************

	//**Utility functions***************************************************************************
	/*!\name Utility functions */
	//@{
	template <size_t I> static constexpr size_t repetitions() noexcept;
	//@}
	//**********************************************************************************************

  private:
	//**Compile time checks*************************************************************************
	METRIC_NUMERIC_STATIC_ASSERT(Dim == sizeof...(Rs));
	//**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief The constructor for RepeatExprData.
 */
template <size_t Dim // Number of dimensions
		  ,
		  size_t... Rs> // Compile time repetitions
constexpr RepeatExprData<Dim, Rs...>::RepeatExprData() noexcept
{
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Returns the number of repetitions of the repeater expression for the given dimension.
//
// \return The number of repetitions for the given dimension.
*/
template <size_t Dim // Number of dimensions
		  ,
		  size_t... Rs> // Compile time repetitions
template <size_t I>		// Dimension index
constexpr size_t RepeatExprData<Dim, Rs...>::repetitions() noexcept
{
	METRIC_NUMERIC_STATIC_ASSERT(I < Dim);
	constexpr size_t reps[] = {Rs...};
	return reps[I];
}

/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  CLASS TEMPLATE SPECIALIZATION FOR ZERO COMPILE TIME REPEATER ARGUMENTS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the RepeatExprData class template for zero compile time repeater
//        arguments.
// \ingroup math
//
// This specialization of RepeatExprData adapts the class template to the requirements of zero
// compile time repeater arguments.
*/
template <size_t Dim> // Number of dimensions
class RepeatExprData<Dim> {
  public:
	//**Constructors********************************************************************************
	/*!\name Constructors */
	//@{
	template <typename... Reps> inline RepeatExprData(Reps... reps) noexcept;

	RepeatExprData(const RepeatExprData &) = default;
	//@}
	//**********************************************************************************************

	//**Destructor**********************************************************************************
	/*!\name Destructor */
	//@{
	~RepeatExprData() = default;
	//@}
	//**********************************************************************************************

	//**Assignment operators************************************************************************
	/*!\name Assignment operators */
	//@{
	RepeatExprData &operator=(const RepeatExprData &) = delete;
	//@}
	//**********************************************************************************************

	//**Utility functions***************************************************************************
	/*!\name Utility functions */
	//@{
	template <size_t I> inline size_t repetitions() const noexcept;
	//@}
	//**********************************************************************************************

  private:
	//**Member variables****************************************************************************
	/*!\name Member variables */
	//@{
	const size_t repetitions_[Dim]; //!< The number of repetitions of the repeater expression.
									//@}
	//**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief The constructor for RepeatExprData.
//
// \param reps The number of repetitions of the repeater expression.
*/
template <size_t Dim>		// Number of dimensions
template <typename... Reps> // Runtime repetitions
inline RepeatExprData<Dim>::RepeatExprData(Reps... reps) noexcept
	: repetitions_{reps...} // The number of repetitions of the repeater expression
{
	METRIC_NUMERIC_STATIC_ASSERT(sizeof...(Reps) == Dim);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Returns the number of repetitions of the repeater expression for the given dimension.
//
// \return The number of repetitions for the given dimension.
*/
template <size_t Dim> // Number of dimensions
template <size_t I>	  // Dimension index
inline size_t RepeatExprData<Dim>::repetitions() const noexcept
{
	METRIC_NUMERIC_STATIC_ASSERT(I < 3UL);
	return repetitions_[I];
}
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
