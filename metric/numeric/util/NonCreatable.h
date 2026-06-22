// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_NONCREATABLE_H
#define METRIC_NUMERIC_UTIL_NONCREATABLE_H
namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Base class for non-creatable (static) classes.
// \ingroup util
//
// The NonCreatable class is intended to work as a base class for non-creatable classes, i.e.
// classes that cannot be instantiated and exclusively offer static functions/data. Both the
// standard as well as the copy constructor and the copy assignment operator are declared
// private and left undefinded in order to prohibit the instantiation of objects of derived
// classes.\n
//
// \note It is not necessary to publicly derive from this class. It is sufficient to derive
// privately to prevent the instantiation of the derived class.

   \code
   class A : private NonCreatable
   { ... };
   \endcode
*/
class NonCreatable {
  protected:
	//**Constructors and copy assignment operator***************************************************
	/*!\name Constructors and copy assignment operator */
	//@{
	NonCreatable() = delete;								//!< Constructor (explicitly deleted)
	NonCreatable(const NonCreatable &) = delete;			//!< Copy constructor (explicitly deleted)
	NonCreatable &operator=(const NonCreatable &) = delete; //!< Copy assignment operator (explicitly deleted)
															//@}
	//**********************************************************************************************
};
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
