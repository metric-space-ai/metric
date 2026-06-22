// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_EXPANDEXPRDATA_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_EXPANDEXPRDATA_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/Types.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Auxiliary class template for the data members of expansion expression classes.
// \ingroup math
//
// The auxiliary ExpandExprData class template represents an abstraction of the data members of
// expansion expression template classes. The necessary set of data member is selected depending
// on the number of compile time expansion arguments.
*/
template <size_t... CEAs> // Compile time expansion arguments
class ExpandExprData {};
//*************************************************************************************************

//=================================================================================================
//
//  CLASS TEMPLATE SPECIALIZATION FOR ZERO COMPILE TIME EXPANSION ARGUMENTS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the ExpandExprData class template for zero compile time expansion
//        arguments.
// \ingroup math
//
// This specialization of ExpandExprData adapts the class template to the requirements of zero
// compile time expansion arguments.
*/
template <> class ExpandExprData<> {
  public:
	//**Constructors********************************************************************************
	/*!\name Constructors */
	//@{
	inline ExpandExprData(size_t expansion) noexcept;

	ExpandExprData(const ExpandExprData &) = default;
	//@}
	//**********************************************************************************************

	//**Destructor**********************************************************************************
	/*!\name Destructor */
	//@{
	~ExpandExprData() = default;
	//@}
	//**********************************************************************************************

	//**Assignment operators************************************************************************
	/*!\name Assignment operators */
	//@{
	ExpandExprData &operator=(const ExpandExprData &) = delete;
	//@}
	//**********************************************************************************************

	//**Utility functions***************************************************************************
	/*!\name Utility functions */
	//@{
	inline size_t expansion() const noexcept;
	//@}
	//**********************************************************************************************

  private:
	//**Member variables****************************************************************************
	/*!\name Member variables */
	//@{
	const size_t expansion_; //!< The expansion of the expansion expression.
							 //@}
	//**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief The constructor for ExpandExprData.
//
// \param expansion The expansion of the expansion expression.
*/
inline ExpandExprData<>::ExpandExprData(size_t expansion) noexcept
	: expansion_(expansion) // The expansion of the expansion expression
{
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Returns the expansion of the expansion expression.
//
// \return The expansion.
*/
inline size_t ExpandExprData<>::expansion() const noexcept { return expansion_; }
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  CLASS TEMPLATE SPECIALIZATION FOR ONE COMPILE TIME EXPANSION ARGUMENT
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the ExpandExprData class template for one compile time expansion
//        argument.
// \ingroup math
//
// This specialization of ExpandExprData adapts the class template to the requirements of a
// single compile time expansion argument.
*/
template <size_t E> // Compile time expansion
class ExpandExprData<E> {
  public:
	//**Constructors********************************************************************************
	/*!\name Constructors */
	//@{
	constexpr ExpandExprData() noexcept;

	ExpandExprData(const ExpandExprData &) = default;
	//@}
	//**********************************************************************************************

	//**Destructor**********************************************************************************
	/*!\name Destructor */
	//@{
	~ExpandExprData() = default;
	//@}
	//**********************************************************************************************

	//**Assignment operators************************************************************************
	/*!\name Assignment operators */
	//@{
	ExpandExprData &operator=(const ExpandExprData &) = delete;
	//@}
	//**********************************************************************************************

	//**Utility functions***************************************************************************
	/*!\name Utility functions */
	//@{
	static constexpr size_t expansion() noexcept;
	//@}
	//**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief The constructor for ExpandExprData.
 */
template <size_t E> // Compile time expansion
constexpr ExpandExprData<E>::ExpandExprData() noexcept
{
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Returns the expansion of the expansion expression.
//
// \return The expansion.
*/
template <size_t E> // Compile time expansion
constexpr size_t ExpandExprData<E>::expansion() noexcept
{
	return E;
}

/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
