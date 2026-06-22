// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_SINGLETON_DEPENDENCY_H
#define METRIC_NUMERIC_UTIL_SINGLETON_DEPENDENCY_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <memory>
#include <metric/numeric/util/constraints/DerivedFrom.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Lifetime dependency on a singleton object.
// \ingroup singleton
//
// The Dependency template class represents a lifetime dependency on a singleton object based
// on the Metric numeric Singleton functionality. By use of the Dependency template, any class can by
// either public or non-public inheritance or composition define a single or multiple lifetime
// dependencies on one or several singletons, which guarantees that the singleton instance(s)
// will be destroyed after the dependent object. The following example demonstrates both the
// inheritance as well as the composition approach:

   \code
   // Definition of the Viewer class, which is depending on the Logger singleton instance

   // #1: Approach by non-public inheritance
   class Viewer : private Dependency<Logger>
   {
	  ...
   };

   // #2: Approach by composition
   class Viewer
   {
	private:
	  Dependency<Logger> dependency_;
   };
   \endcode
*/
template <typename T> // Type of the lifetime dependency
class Dependency {
  public:
	//**Constructors********************************************************************************
	/*!\name Constructors */
	//@{
	inline Dependency();
	//@}
	//**********************************************************************************************

  private:
	//**Member variables****************************************************************************
	/*!\name Member variables */
	//@{
	std::shared_ptr<T> dependency_; //!< Handle to the lifetime dependency.
									//@}
	//**********************************************************************************************
};
//*************************************************************************************************

//=================================================================================================
//
//  CONSTRUCTORS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Default constructor for Dependency.
 */
template <typename T>											// Type of the lifetime dependency
inline Dependency<T>::Dependency() : dependency_(T::instance()) // Handle to the lifetime dependency
{
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_DERIVED_FROM(T, typename T::SingletonType);
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
