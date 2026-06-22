// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_EXCEPTION_H
#define METRIC_NUMERIC_UTIL_EXCEPTION_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <stdexcept>

//=================================================================================================
//
//  EXCEPTION MACROS
//
//=================================================================================================

//*************************************************************************************************
/*!\def METRIC_NUMERIC_THROW
// \brief Macro for the error reporting mechanism of the \b Metric numeric library.
// \ingroup util
//
// This macro encapsulates the default, general way of the \b Metric numeric library to report errors of
// any kind by throwing an exception. Also, since under certain conditions and environments it
// may be desirable to replace exceptions by a different error reporting mechanism this macro
// provides an opportunity to customize the error reporting approach.
//
// The macro excepts a single argument, which specifies the exception to be thrown:

   \code
   #define METRIC_NUMERIC_THROW( EXCEPTION ) \
	  throw EXCEPTION
   \endcode

// In order to customize the error reporing mechanism all that needs to be done is to define
// the macro prior to including any \a Metric numeric header file. This will cause the \b Metric numeric specific
// mechanism to be overridden. The following example demonstrates this by replacing exceptions
// by a call to a \a log() function and a direct call to abort:

   \code
   #define METRIC_NUMERIC_THROW( EXCEPTION ) \
	  log( "..." ); \
	  abort()

   #include <metric/numeric/Numeric.h>
   \endcode

// \note It is possible to execute several statements instead of executing a single statement to
// throw an exception. Also note that it is recommended to define the macro such that a subsequent
// semicolon is required!
//
// \warning This macro is provided with the intention to assist in adapting \b Metric numeric to special
// conditions and environments. However, the customization of the error reporting mechanism via
// this macro can have a significant effect on the library. Thus be advised to use the macro
// with due care!
*/
#ifndef METRIC_NUMERIC_THROW
#define METRIC_NUMERIC_THROW(EXCEPTION) throw EXCEPTION
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\def METRIC_NUMERIC_THROW_BAD_ALLOC
// \brief Macro for the emission of a \a std::bad_alloc exception.
// \ingroup util
//
// This macro encapsulates the default way of \b Metric numeric to throw a \a std::bad_alloc exception.
// Also, since it may be desirable to replace the type of exception by a custom exception type
// this macro provides an opportunity to customize the behavior.

   \code
   #define METRIC_NUMERIC_THROW_BAD_ALLOC \
	  METRIC_NUMERIC_THROW( std::bad_alloc() )
   \endcode

// In order to customize the type of exception all that needs to be done is to define the macro
// prior to including any \a Metric numeric header file. This will override the \b Metric numeric default behavior.
// The following example demonstrates this by replacing \a std::bad_alloc by a custom exception
// type:

   \code
   class BadAlloc
   {
	public:
	  BadAlloc();
	  // ...
   };

   #define METRIC_NUMERIC_THROW_BAD_ALLOC \
	  throw BadAlloc()

   #include <metric/numeric/Numeric.h>
   \endcode

// \note It is recommended to define the macro such that a subsequent semicolon is required!
//
// \warning This macro is provided with the intention to assist in adapting \b Metric numeric to special
// conditions and environments. However, the customization of the type of exception via this
// macro may have an effect on the library. Thus be advised to use the macro with due care!
*/
#ifndef METRIC_NUMERIC_THROW_BAD_ALLOC
#define METRIC_NUMERIC_THROW_BAD_ALLOC METRIC_NUMERIC_THROW(std::bad_alloc())
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\def METRIC_NUMERIC_THROW_LOGIC_ERROR
// \brief Macro for the emission of a \a std::logic_error exception.
// \ingroup util
//
// This macro encapsulates the default way of \b Metric numeric to throw a \a std::logic_error exception.
// Also, since it may be desirable to replace the type of exception by a custom exception type
// this macro provides an opportunity to customize the behavior.
//
// The macro excepts a single argument, which specifies the message of the exception:

   \code
   #define METRIC_NUMERIC_THROW_LOGIC_ERROR( MESSAGE ) \
	  METRIC_NUMERIC_THROW( std::logic_error( MESSAGE ) )
   \endcode

// In order to customize the type of exception all that needs to be done is to define the macro
// prior to including any \a Metric numeric header file. This will override the \b Metric numeric default behavior.
// The following example demonstrates this by replacing \a std::logic_error by a custom exception
// type:

   \code
   class LogicError
   {
	public:
	  LogicError();
	  explicit LogicError( const std::string& message );
	  // ...
   };

   #define METRIC_NUMERIC_THROW_LOGIC_ERROR( MESSAGE ) \
	  throw LogicError( MESSAGE )

   #include <metric/numeric/Numeric.h>
   \endcode

// \note It is recommended to define the macro such that a subsequent semicolon is required!
//
// \warning This macro is provided with the intention to assist in adapting \b Metric numeric to special
// conditions and environments. However, the customization of the type of exception via this
// macro may have an effect on the library. Thus be advised to use the macro with due care!
*/
#ifndef METRIC_NUMERIC_THROW_LOGIC_ERROR
#define METRIC_NUMERIC_THROW_LOGIC_ERROR(MESSAGE) METRIC_NUMERIC_THROW(std::logic_error(MESSAGE))
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\def METRIC_NUMERIC_THROW_INVALID_ARGUMENT
// \brief Macro for the emission of a \a std::invalid_argument exception.
// \ingroup util
//
// This macro encapsulates the default way of \b Metric numeric to throw a \a std::invalid_argument
// exception. Also, since it may be desirable to replace the type of exception by a custom
// exception type this macro provides an opportunity to customize the behavior.
//
// The macro excepts a single argument, which specifies the message of the exception:

   \code
   #define METRIC_NUMERIC_THROW_INVALID_ARGUMENT( MESSAGE ) \
	  METRIC_NUMERIC_THROW( std::invalid_argument( MESSAGE ) )
   \endcode

// In order to customize the type of exception all that needs to be done is to define the macro
// prior to including any \a Metric numeric header file. This will override the \b Metric numeric default behavior.
// The following example demonstrates this by replacing \a std::invalid_argument by a custom
// exception type:

   \code
   class InvalidArgument
   {
	public:
	  InvalidArgument();
	  explicit InvalidArgument( const std::string& message );
	  // ...
   };

   #define METRIC_NUMERIC_THROW_INVALID_ARGUMENT( MESSAGE ) \
	  throw InvalidArgument( MESSAGE )

   #include <metric/numeric/Numeric.h>
   \endcode

// \note It is recommended to define the macro such that a subsequent semicolon is required!
//
// \warning This macro is provided with the intention to assist in adapting \b Metric numeric to special
// conditions and environments. However, the customization of the type of exception via this
// macro may have an effect on the library. Thus be advised to use the macro with due care!
*/
#ifndef METRIC_NUMERIC_THROW_INVALID_ARGUMENT
#define METRIC_NUMERIC_THROW_INVALID_ARGUMENT(MESSAGE) METRIC_NUMERIC_THROW(std::invalid_argument(MESSAGE))
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\def METRIC_NUMERIC_THROW_LENGTH_ERROR
// \brief Macro for the emission of a \a std::length_error exception.
// \ingroup util
//
// This macro encapsulates the default way of \b Metric numeric to throw a \a std::length_error exception.
// Also, since it may be desirable to replace the type of exception by a custom exception type
// this macro provides an opportunity to customize the behavior.
//
// The macro excepts a single argument, which specifies the message of the exception:

   \code
   #define METRIC_NUMERIC_THROW_LENGTH_ERROR( MESSAGE ) \
	  METRIC_NUMERIC_THROW( std::length_error( MESSAGE ) )
   \endcode

// In order to customize the type of exception all that needs to be done is to define the macro
// prior to including any \a Metric numeric header file. This will override the \b Metric numeric default behavior.
// The following example demonstrates this by replacing \a std::length_error by a custom
// exception type:

   \code
   class LengthError
   {
	public:
	  LengthError();
	  explicit LengthError( const std::string& message );
	  // ...
   };

   #define METRIC_NUMERIC_THROW_LENGTH_ERROR( MESSAGE ) \
	  throw LengthError( MESSAGE )

   #include <metric/numeric/Numeric.h>
   \endcode

// \note It is recommended to define the macro such that a subsequent semicolon is required!
//
// \warning This macro is provided with the intention to assist in adapting \b Metric numeric to special
// conditions and environments. However, the customization of the type of exception via this
// macro may have an effect on the library. Thus be advised to use the macro with due care!
*/
#ifndef METRIC_NUMERIC_THROW_LENGTH_ERROR
#define METRIC_NUMERIC_THROW_LENGTH_ERROR(MESSAGE) METRIC_NUMERIC_THROW(std::length_error(MESSAGE))
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\def METRIC_NUMERIC_THROW_OUT_OF_RANGE
// \brief Macro for the emission of a \a std::out_of_range exception.
// \ingroup util
//
// This macro encapsulates the default way of \b Metric numeric to throw a \a std::out_of_range exception.
// Also, since it may be desirable to replace the type of exception by a custom exception type
// this macro provides an opportunity to customize the behavior.
//
// The macro excepts a single argument, which specifies the message of the exception:

   \code
   #define METRIC_NUMERIC_THROW_OUT_OF_RANGE( MESSAGE ) \
	  METRIC_NUMERIC_THROW( std::out_of_range( MESSAGE ) )
   \endcode

// In order to customize the type of exception all that needs to be done is to define the macro
// prior to including any \a Metric numeric header file. This will override the \b Metric numeric default behavior.
// The following example demonstrates this by replacing \a std::out_of_range by a custom exception
// type:

   \code
   class OutOfRange
   {
	public:
	  OutOfRange();
	  explicit OutOfRange( const std::string& message );
	  // ...
   };

   #define METRIC_NUMERIC_THROW_OUT_OF_RANGE( MESSAGE ) \
	  throw OutOfRange( MESSAGE )

   #include <metric/numeric/Numeric.h>
   \endcode

// \note It is recommended to define the macro such that a subsequent semicolon is required!
//
// \warning This macro is provided with the intention to assist in adapting \b Metric numeric to special
// conditions and environments. However, the customization of the type of exception via this
// macro may have an effect on the library. Thus be advised to use the macro with due care!
*/
#ifndef METRIC_NUMERIC_THROW_OUT_OF_RANGE
#define METRIC_NUMERIC_THROW_OUT_OF_RANGE(MESSAGE) METRIC_NUMERIC_THROW(std::out_of_range(MESSAGE))
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\def METRIC_NUMERIC_THROW_RUNTIME_ERROR
// \brief Macro for the emission of a \a std::runtime_error exception.
// \ingroup util
//
// This macro encapsulates the default way of \b Metric numeric to throw a \a std::runtime_error exception.
// Also, since it may be desirable to replace the type of exception by a custom exception type
// this macro provides an opportunity to customize the behavior.
//
// The macro excepts a single argument, which specifies the message of the exception:

   \code
   #define METRIC_NUMERIC_THROW_RUNTIME_ERROR( MESSAGE ) \
	  METRIC_NUMERIC_THROW( std::runtime_error( MESSAGE ) )
   \endcode

// In order to customize the type of exception all that needs to be done is to define the macro
// prior to including any \a Metric numeric header file. This will override the \b Metric numeric default behavior.
// The following example demonstrates this by replacing \a std::runtime_error by a custom
// exception type:

   \code
   class RuntimeError
   {
	public:
	  RuntimeError();
	  explicit RuntimeError( const std::string& message );
	  // ...
   };

   #define METRIC_NUMERIC_THROW_RUNTIME_ERROR( MESSAGE ) \
	  throw RuntimeError( MESSAGE )

   #include <metric/numeric/Numeric.h>
   \endcode

// \note It is recommended to define the macro such that a subsequent semicolon is required!
//
// \warning This macro is provided with the intention to assist in adapting \b Metric numeric to special
// conditions and environments. However, the customization of the type of exception via this
// macro may have an effect on the library. Thus be advised to use the macro with due care!
*/
#ifndef METRIC_NUMERIC_THROW_RUNTIME_ERROR
#define METRIC_NUMERIC_THROW_RUNTIME_ERROR(MESSAGE) METRIC_NUMERIC_THROW(std::runtime_error(MESSAGE))
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\def METRIC_NUMERIC_THROW_OVERFLOW_ERROR
// \brief Macro for the emission of a \a std::overflow_error exception.
// \ingroup util
//
// This macro encapsulates the default way of \b Metric numeric to throw a \a std::overflow_error exception.
// Also, since it may be desirable to replace the type of exception by a custom exception type
// this macro provides an opportunity to customize the behavior.
//
// The macro excepts a single argument, which specifies the message of the exception:

   \code
   #define METRIC_NUMERIC_THROW_OVERFLOW_ERROR( MESSAGE ) \
	  METRIC_NUMERIC_THROW( std::overflow_error( MESSAGE ) )
   \endcode

// In order to customize the type of exception all that needs to be done is to define the macro
// prior to including any \a Metric numeric header file. This will override the \b Metric numeric default behavior.
// The following example demonstrates this by replacing \a std::overflow_error by a custom
// exception type:

   \code
   class OverflowError
   {
	public:
	  OverflowError();
	  explicit OverflowError( const std::string& message );
	  // ...
   };

   #define METRIC_NUMERIC_THROW_OVERFLOW_ERROR( MESSAGE ) \
	  throw OverflowError( MESSAGE )

   #include <metric/numeric/Numeric.h>
   \endcode

// \note It is recommended to define the macro such that a subsequent semicolon is required!
//
// \warning This macro is provided with the intention to assist in adapting \b Metric numeric to special
// conditions and environments. However, the customization of the type of exception via this
// macro may have an effect on the library. Thus be advised to use the macro with due care!
*/
#ifndef METRIC_NUMERIC_THROW_OVERFLOW_ERROR
#define METRIC_NUMERIC_THROW_OVERFLOW_ERROR(MESSAGE) METRIC_NUMERIC_THROW(std::overflow_error(MESSAGE))
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\def METRIC_NUMERIC_THROW_UNDERFLOW_ERROR
// \brief Macro for the emission of a \a std::underflow_error exception.
// \ingroup util
//
// This macro encapsulates the default way of \b Metric numeric to throw a \a std::underflow_error
// exception. Also, since it may be desirable to replace the type of exception by a custom
// exception type this macro provides an opportunity to customize the behavior.
//
// The macro excepts a single argument, which specifies the message of the exception:

   \code
   #define METRIC_NUMERIC_THROW_UNDERFLOW_ERROR( MESSAGE ) \
	  METRIC_NUMERIC_THROW( std::underflow_error( MESSAGE ) )
   \endcode

// In order to customize the type of exception all that needs to be done is to define the macro
// prior to including any \a Metric numeric header file. This will override the \b Metric numeric default behavior.
// The following example demonstrates this by replacing \a std::underflow_error by a custom
// exception type:

   \code
   class UnderflowError
   {
	public:
	  UnderflowError();
	  explicit UnderflowError( const std::string& message );
	  // ...
   };

   #define METRIC_NUMERIC_THROW_UNDERFLOW_ERROR( MESSAGE ) \
	  throw UnderflowError( MESSAGE )

   #include <metric/numeric/Numeric.h>
   \endcode

// \note It is recommended to define the macro such that a subsequent semicolon is required!
//
// \warning This macro is provided with the intention to assist in adapting \b Metric numeric to special
// conditions and environments. However, the customization of the type of exception via this
// macro may have an effect on the library. Thus be advised to use the macro with due care!
*/
#ifndef METRIC_NUMERIC_THROW_UNDERFLOW_ERROR
#define METRIC_NUMERIC_THROW_UNDERFLOW_ERROR(MESSAGE) METRIC_NUMERIC_THROW(std::underflow_error(MESSAGE))
#endif
//*************************************************************************************************

#endif
