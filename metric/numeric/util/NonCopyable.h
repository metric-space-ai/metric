// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_NONCOPYABLE_H
#define METRIC_NUMERIC_UTIL_NONCOPYABLE_H
namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Base class for non-copyable class instances.
// \ingroup util
//
// The NonCopyable class is intended to work as a base class for non-copyable classes. Both the
// copy constructor and the copy assignment operator are explicitly deleted in order to prohibit
// copy operations of the derived classes.\n
//
// \note It is not necessary to publicly derive from this class. It is sufficient to derive
// privately to prevent copy operations on the derived class.

   \code
   class A : private NonCopyable
   { ... };
   \endcode
*/
class NonCopyable {
  protected:
	//**Constructor and destructor******************************************************************
	/*!\name Constructor and destructor */
	//@{
	inline NonCopyable() {}	 //!< Default constructor for the NonCopyable class.
	inline ~NonCopyable() {} //!< Destructor of the NonCopyable class.
	//@}
	//**********************************************************************************************

	//**Copy constructor and copy assignment operator***********************************************
	/*!\name Copy constructor and copy assignment operator */
	//@{
	NonCopyable(const NonCopyable &) = delete;			  //!< Copy constructor (explicitly deleted)
	NonCopyable &operator=(const NonCopyable &) = delete; //!< Copy assignment operator (explicitly deleted)
														  //@}
	//**********************************************************************************************
};
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
