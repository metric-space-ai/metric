// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_MPL_BOOLS_H
#define METRIC_NUMERIC_UTIL_MPL_BOOLS_H
namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Auxiliary tool for unwrapping a pack of variadic boolean values.
// \ingroup mpl
//
// The Bools class template represents an auxiliary tool for unwrapping a pack of variadic
// boolean values.
*/
template <bool...> struct Bools {};
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
