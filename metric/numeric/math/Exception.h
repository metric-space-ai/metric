// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXCEPTION_H
#define METRIC_NUMERIC_MATH_EXCEPTION_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/Exception.h>

//=================================================================================================
//
//  EXCEPTION MACROS
//
//=================================================================================================

//*************************************************************************************************
/*!\def METRIC_NUMERIC_THROW_DIVISION_BY_ZERO
// \brief Macro for the emission of an exception on detection of a division by zero.
// \ingroup math
//
// This macro encapsulates the default way of \b Metric numeric to throw an exception on detection of
// a division by zero. Also, since it may be desirable to replace the type of exception by a
// custom exception type this macro provides an opportunity to customize the behavior.
//
// The macro excepts a single argument, which specifies the message of the exception:

   \code
   #define METRIC_NUMERIC_THROW_DIVISION_BY_ZERO( MESSAGE ) \
	  METRIC_NUMERIC_THROW_RUNTIME_ERROR( MESSAGE )
   \endcode

// In order to customize the type of exception all that needs to be done is to define the macro
// prior to including any \a Metric numeric header file. This will override the \b Metric numeric default behavior.
// The following example demonstrates this by replacing \a std::runtime_error by a custom
// exception type:

   \code
   class DivisionByZero
   {
	public:
	  DivisionByZero();
	  explicit DivisionByZero( const std::string& message );
	  // ...
   };

   #define METRIC_NUMERIC_THROW_DIVISION_BY_ZERO( MESSAGE ) \
	  throw DivisionByZero( MESSAGE )

   #include <metric/numeric/Numeric.h>
   \endcode

// \note It is recommended to define the macro such that a subsequent semicolon is required!
//
// \warning This macro is provided with the intention to assist in adapting \b Metric numeric to special
// conditions and environments. However, the customization of the type of exception via this
// macro may have an effect on the library. Thus be advised to use the macro with due care!
*/
#ifndef METRIC_NUMERIC_THROW_DIVISION_BY_ZERO
#define METRIC_NUMERIC_THROW_DIVISION_BY_ZERO(MESSAGE) METRIC_NUMERIC_THROW_RUNTIME_ERROR(MESSAGE)
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\def METRIC_NUMERIC_THROW_LAPACK_ERROR
// \brief Macro for the emission of an exception on detection of a LAPACK error.
// \ingroup math
//
// This macro encapsulates the default way of \b Metric numeric to throw an exception when encountering
// a LAPACK error (for instance when trying to invert a singular matrix). Also, since it may be
// desirable to replace the type of exception by a custom exception type this macro provides an
// opportunity to customize the behavior.
//
// The macro excepts a single argument, which specifies the message of the exception:

   \code
   #define METRIC_NUMERIC_THROW_LAPACK_ERROR( MESSAGE ) \
	  METRIC_NUMERIC_THROW_RUNTIME_ERROR( MESSAGE )
   \endcode

// In order to customize the type of exception all that needs to be done is to define the macro
// prior to including any \a Metric numeric header file. This will override the \b Metric numeric default behavior.
// The following example demonstrates this by replacing \a std::runtime_error by a custom
// exception type:

   \code
   class LapackError
   {
	public:
	  LapackError();
	  explicit LapackError( const std::string& message );
	  // ...
   };

   #define METRIC_NUMERIC_THROW_LAPACK_ERROR( MESSAGE ) \
	  throw LapackError( MESSAGE )

   #include <metric/numeric/Numeric.h>
   \endcode

// \note It is recommended to define the macro such that a subsequent semicolon is required!
//
// \warning This macro is provided with the intention to assist in adapting \b Metric numeric to special
// conditions and environments. However, the customization of the type of exception via this
// macro may have an effect on the library. Thus be advised to use the macro with due care!
*/
#ifndef METRIC_NUMERIC_THROW_LAPACK_ERROR
#define METRIC_NUMERIC_THROW_LAPACK_ERROR(MESSAGE) METRIC_NUMERIC_THROW_RUNTIME_ERROR(MESSAGE)
#endif
//*************************************************************************************************

#endif
