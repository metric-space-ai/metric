// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_MAYBEUNUSED_H
#define METRIC_NUMERIC_UTIL_MAYBEUNUSED_H
namespace mtrc::numeric {

//=================================================================================================
//
//  MAYBE_UNUSED FUNCTION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Suppression of unused parameter warnings.
// \ingroup util
//
// \return void
//
// The MAYBE_UNUSED function provides the functionality to suppress warnings about any number
// of unused parameters. Usually this problem occurs in case a parameter is given a name but is
// not used within the function:

   \code
   void f( int x )
   {}  // x is not used within f. This may result in an unused parameter warning.
   \endcode

// A possible solution is to keep the parameter unnamed:

   \code
   void f( int )
   {}  // No warning about unused parameter is issued
   \endcode

// However, there are situations where is approach is not possible, as for instance in case the
// variable must be documented via Doxygen. For these cases, the MAYBE_UNUSED class can be used
// to suppress the warnings:

   \code
   void f( int x )
   {
	  MAYBE_UNUSED( x );  // Suppresses the unused parameter warnings
   }
   \endcode
*/
template <typename... Args> constexpr void MAYBE_UNUSED(const Args &...) {}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
