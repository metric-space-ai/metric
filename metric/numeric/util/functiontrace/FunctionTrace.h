// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_TRACING_FUNCTIONTRACE_H
#define METRIC_NUMERIC_UTIL_TRACING_FUNCTIONTRACE_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <iostream>
#include <metric/numeric/system/Debugging.h>
#include <new>
#include <sstream>
#include <string>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief RAII object for function tracing.
// \ingroup util
//
// The FunctionTrace class is an auxiliary helper class for the tracing of function calls. It
// is implemented as a wrapper around \c std::cerr and is responsible for the atomicity of the
// output of trace information.
*/
class FunctionTrace {
  public:
	//**Constructors********************************************************************************
	/*!\name Constructors */
	//@{
	inline FunctionTrace(const std::string &file, const std::string &function);
	//@}
	//**********************************************************************************************

	//**Destructor**********************************************************************************
	/*!\name Destructor */
	//@{
	inline ~FunctionTrace();
	//@}
	//**********************************************************************************************

	//**Forbidden operations************************************************************************
	/*!\name Forbidden operations */
	//@{
	FunctionTrace(const FunctionTrace &) = delete;
	FunctionTrace(FunctionTrace &&) = delete;

	FunctionTrace &operator=(const FunctionTrace &) = delete;
	FunctionTrace &operator=(FunctionTrace &&) = delete;

	void *operator new(std::size_t) = delete;
	void *operator new[](std::size_t) = delete;
	void *operator new(std::size_t, const std::nothrow_t &) noexcept = delete;
	void *operator new[](std::size_t, const std::nothrow_t &) noexcept = delete;

	void operator delete(void *) noexcept = delete;
	void operator delete[](void *) noexcept = delete;
	void operator delete(void *, const std::nothrow_t &) noexcept = delete;
	void operator delete[](void *, const std::nothrow_t &) noexcept = delete;
	//@}
	//**********************************************************************************************

  private:
	//**Member variables****************************************************************************
	/*!\name Member variables */
	//@{
	std::string file_;	   //!< The file name the traced function is contained in.
	std::string function_; //!< The name of the traced function.
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
/*!\brief Constructor for the FunctionTrace class.
//
// \param file The name of the file the traced function is contained in
// \param function The name of the traced function
*/
inline FunctionTrace::FunctionTrace(const std::string &file, const std::string &function)
	: file_(file) // The file name the traced function is contained in
	  ,
	  function_(function) // The name of the traced function
{
	std::ostringstream oss;
	oss << " + ";

	oss << " Entering function '" << function_ << "' in file '" << file_ << "'\n";
	std::cerr << oss.str();
}
//*************************************************************************************************

//=================================================================================================
//
//  DESTRUCTOR
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Destructor for the FunctionTrace class.
 */
inline FunctionTrace::~FunctionTrace()
{
	std::ostringstream oss;
	oss << " - ";

	oss << " Leaving function '" << function_ << "' in file '" << file_ << "'\n";
	std::cerr << oss.str();
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
