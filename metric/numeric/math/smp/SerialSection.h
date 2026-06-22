// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_SMP_SERIALSECTION_H
#define METRIC_NUMERIC_MATH_SMP_SERIALSECTION_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/Exception.h>
#include <metric/numeric/util/Suffix.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Section to enforce the serial execution of operations.
// \ingroup smp
//
// The SerialSection class is an auxiliary helper class for the \a METRIC_NUMERIC_SERIAL_SECTION macro.
// It provides the functionality to detect whether a serial section is active, i.e. if the
// currently executed code is inside a serial section.
*/
template <typename T> class SerialSection {
  public:
	//**Constructor*********************************************************************************
	/*!\name Constructor */
	//@{
	inline SerialSection(bool activate);
	//@}
	//**********************************************************************************************

	//**Destructor**********************************************************************************
	/*!\name Destructor */
	//@{
	inline ~SerialSection();
	//@}
	//**********************************************************************************************

	//**Conversion operator*************************************************************************
	/*!\name Conversion operator */
	//@{
	inline operator bool() const;
	//@}
	//**********************************************************************************************

  private:
	//**Member variables****************************************************************************
	/*!\name Member variables */
	//@{
	static bool active_; //!< Activity flag for the serial section.
						 /*!< In case a serial section is active (i.e. the currently executed
							  code is inside a serial section), the flag is set to \a true,
							  otherwise it is \a false. */
	//@}
	//**********************************************************************************************

	//**Friend declarations*************************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	friend bool isSerialSectionActive();
	/*! \endcond */
	//**********************************************************************************************
};
//*************************************************************************************************

//=================================================================================================
//
//  DEFINITION AND INITIALIZATION OF THE STATIC MEMBER VARIABLES
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename T> bool SerialSection<T>::active_ = false;
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  CONSTRUCTOR
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Constructor for the SerialSection class.
//
// \param activate Activation flag for the serial section.
// \exception std::runtime_error Nested serial sections detected.
*/
template <typename T> inline SerialSection<T>::SerialSection(bool activate)
{
	if (active_) {
		METRIC_NUMERIC_THROW_RUNTIME_ERROR("Nested serial sections detected");
	}

	active_ = activate;
}
//*************************************************************************************************

//=================================================================================================
//
//  DESTRUCTOR
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Destructor of the SerialSection class.
 */
template <typename T> inline SerialSection<T>::~SerialSection()
{
	active_ = false; // Resetting the activity flag
}
//*************************************************************************************************

//=================================================================================================
//
//  CONVERSION OPERATOR
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Conversion operator to \a bool.
//
// The conversion operator returns \a true in case a serial section is active and \a false
// otherwise.
*/
template <typename T> inline SerialSection<T>::operator bool() const { return active_; }
//*************************************************************************************************

//=================================================================================================
//
//  GLOBAL FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\name SerialSection functions */
//@{
inline bool isSerialSectionActive();
//@}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Returns whether a serial section is active or not.
// \ingroup smp
//
// \return \a true if a serial section is active, \a false if not.
*/
inline bool isSerialSectionActive() { return SerialSection<int>::active_; }
//*************************************************************************************************

//=================================================================================================
//
//  SERIAL SECTION MACRO
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Section to enforce the serial execution of operations.
// \ingroup smp
//
// This macro provides the option to start a serial section to enforce the serial execution of
// operations. The following example demonstrates how a serial section is used:

   \code
   using mtrc::numeric::rowMajor;
   using mtrc::numeric::columnVector;

   mtrc::numeric::DynamicMatrix<double,rowMajor> A;
   mtrc::numeric::DynamicVector<double,columnVector> b, c, d, x, y, z;

   // ... Resizing and initialization

   // Start of a serial section
   // All operations executed within the serial section are guaranteed to be executed in
   // serial (even if a parallel execution would be possible and/or beneficial).
   METRIC_NUMERIC_SERIAL_SECTION {
	  x = A * b;
	  y = A * c;
	  z = A * d;
   }
   \endcode

// Note that it is not allowed to use nested serial sections (i.e. a serial section within
// another serial section). In case the nested use of a serial section is detected, a
// \a std::runtime_error exception is thrown.
*/
#define METRIC_NUMERIC_SERIAL_SECTION                                                                                  \
	if (mtrc::numeric::SerialSection<int> METRIC_NUMERIC_JOIN(serialSection, __LINE__) = true)
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
