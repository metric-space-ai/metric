// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_SYSTEM_NOUNIQUEADDRESS_H
#define METRIC_NUMERIC_SYSTEM_NOUNIQUEADDRESS_H
//=================================================================================================
//
//  [[NO_UNIQUE_ADDRESS]] ATTRIBUTE
//
//=================================================================================================

//*************************************************************************************************
/*!\def METRIC_NUMERIC_RESTRICT
// \brief Compiler dependent setup of the [[no_unique_address]] attribute.
// \ingroup system
*/
#if (defined(__GNUC__) && __GNUC__ >= 9) ||                                                                            \
	(defined(__clang__) && !defined(__apple_build_version__) && __clang_major__ >= 9)
#define METRIC_NUMERIC_NO_UNIQUE_ADDRESS [[no_unique_address]]
#else
#define METRIC_NUMERIC_NO_UNIQUE_ADDRESS
#endif
//*************************************************************************************************

#endif
