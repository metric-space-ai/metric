// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_SUFFIX_H
#define METRIC_NUMERIC_UTIL_SUFFIX_H
//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Helper macro for macro concatenation.
// \ingroup util
//
// This is the standard token-pasting idiom for macro concatenation. It joins the two arguments
// together, even when one of the arguments is itself a macro (see 16.3.1 in the C++ standard).
// The key is that macro expansion of the macro arguments does not occur in METRIC_NUMERIC_DO_JOIN2
// but does in METRIC_NUMERIC_DO_JOIN.
*/
#define METRIC_NUMERIC_JOIN(X, Y) METRIC_NUMERIC_DO_JOIN(X, Y)
#define METRIC_NUMERIC_DO_JOIN(X, Y) METRIC_NUMERIC_DO_JOIN2(X, Y)
#define METRIC_NUMERIC_DO_JOIN2(X, Y) X##Y
/*! \endcond */
//*************************************************************************************************

#endif
