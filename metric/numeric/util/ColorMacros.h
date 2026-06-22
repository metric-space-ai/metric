// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_COLORMACROS_H
#define METRIC_NUMERIC_UTIL_COLORMACROS_H
//=================================================================================================
//
//  COLOR MACRO SWITCH
//
//=================================================================================================

//! pe color output mode.
/*! This mode triggers the color output macros. */
#define METRIC_NUMERIC_COLOR_OUTPUT 0

//=================================================================================================
//
//  COLOR MACRO DEFINITIONS
//
//=================================================================================================

#if METRIC_NUMERIC_COLOR_OUTPUT

//! Switches the text color to black in case the METRIC_NUMERIC_COLOR_OUTPUT macro is set.
#define METRIC_NUMERIC_BLACK "\033[0;30m"

//! Switches the text color to red in case the METRIC_NUMERIC_COLOR_OUTPUT macro is set.
#define METRIC_NUMERIC_RED "\033[0;31m"

//! Switches the text color to green in case the METRIC_NUMERIC_COLOR_OUTPUT macro is set.
#define METRIC_NUMERIC_GREEN "\033[0;32m"

//! Switches the text color to brown in case the METRIC_NUMERIC_COLOR_OUTPUT macro is set.
#define METRIC_NUMERIC_BROWN "\033[0;33m"

//! Switches the text color to blue in case the METRIC_NUMERIC_COLOR_OUTPUT macro is set.
#define METRIC_NUMERIC_BLUE "\033[0;34m"

//! Switches the text color to magenta in case the METRIC_NUMERIC_COLOR_OUTPUT macro is set.
#define METRIC_NUMERIC_MAGENTA "\033[0;35m"

//! Switches the text color to cyan in case the METRIC_NUMERIC_COLOR_OUTPUT macro is set.
#define METRIC_NUMERIC_CYAN "\033[0;36m"

//! Switches the text color to white in case the METRIC_NUMERIC_COLOR_OUTPUT macro is set.
#define METRIC_NUMERIC_WHITE "\033[0;37m"

//! Switches the text color to a light black in case the METRIC_NUMERIC_COLOR_OUTPUT macro is set.
#define METRIC_NUMERIC_LIGHTBLACK "\033[1;30m"

//! Switches the text color to a light red in case the METRIC_NUMERIC_COLOR_OUTPUT macro is set.
#define METRIC_NUMERIC_LIGHTRED "\033[1;31m"

//! Switches the text color to a light green in case the METRIC_NUMERIC_COLOR_OUTPUT macro is set.
#define METRIC_NUMERIC_LIGHTGREEN "\033[1;32m"

//! Switches the text color to yellow in case the METRIC_NUMERIC_COLOR_OUTPUT macro is set.
#define METRIC_NUMERIC_YELLOW "\033[1;33m"

//! Switches the text color to a light blue in case the METRIC_NUMERIC_COLOR_OUTPUT macro is set.
#define METRIC_NUMERIC_LIGHTBLUE "\033[1;34m"

//! Switches the text color to a light magenta in case the METRIC_NUMERIC_COLOR_OUTPUT macro is set.
#define METRIC_NUMERIC_LIGHTMAGENTA "\033[1;35m"

//! Switches the text color to a light cyan in case the METRIC_NUMERIC_COLOR_OUTPUT macro is set.
#define METRIC_NUMERIC_LIGHTCYAN "\033[1;36m"

//! Switches the text color to a light white in case the METRIC_NUMERIC_COLOR_OUTPUT macro is set.
#define METRIC_NUMERIC_LIGHTWHITE "\033[1;37m"

//! Switches the text color back to the default color.
#define METRIC_NUMERIC_OLDCOLOR "\033[0m"

#else

//! Switches the text color to black in case the METRIC_NUMERIC_COLOR_OUTPUT macro is set.
#define METRIC_NUMERIC_BLACK ""

//! Switches the text color to red in case the METRIC_NUMERIC_COLOR_OUTPUT macro is set.
#define METRIC_NUMERIC_RED ""

//! Switches the text color to green in case the METRIC_NUMERIC_COLOR_OUTPUT macro is set.
#define METRIC_NUMERIC_GREEN ""

//! Switches the text color to brown in case the METRIC_NUMERIC_COLOR_OUTPUT macro is set.
#define METRIC_NUMERIC_BROWN ""

//! Switches the text color to blue in case the METRIC_NUMERIC_COLOR_OUTPUT macro is set.
#define METRIC_NUMERIC_BLUE ""

//! Switches the text color to magenta in case the METRIC_NUMERIC_COLOR_OUTPUT macro is set.
#define METRIC_NUMERIC_MAGENTA ""

//! Switches the text color to cyan in case the METRIC_NUMERIC_COLOR_OUTPUT macro is set.
#define METRIC_NUMERIC_CYAN ""

//! Switches the text color to white in case the METRIC_NUMERIC_COLOR_OUTPUT macro is set.
#define METRIC_NUMERIC_WHITE ""

//! Switches the text color to a light black in case the METRIC_NUMERIC_COLOR_OUTPUT macro is set.
#define METRIC_NUMERIC_LIGHTBLACK ""

//! Switches the text color to a light red in case the METRIC_NUMERIC_COLOR_OUTPUT macro is set.
#define METRIC_NUMERIC_LIGHTRED ""

//! Switches the text color to a light green in case the METRIC_NUMERIC_COLOR_OUTPUT macro is set.
#define METRIC_NUMERIC_LIGHTGREEN ""

//! Switches the text color to yellow in case the METRIC_NUMERIC_COLOR_OUTPUT macro is set.
#define METRIC_NUMERIC_YELLOW ""

//! Switches the text color to a light blue in case the METRIC_NUMERIC_COLOR_OUTPUT macro is set.
#define METRIC_NUMERIC_LIGHTBLUE ""

//! Switches the text color to a light magenta in case the METRIC_NUMERIC_COLOR_OUTPUT macro is set.
#define METRIC_NUMERIC_LIGHTMAGENTA ""

//! Switches the text color to a light cyan in case the METRIC_NUMERIC_COLOR_OUTPUT macro is set.
#define METRIC_NUMERIC_LIGHTCYAN ""

//! Switches the text color to a light white in case the METRIC_NUMERIC_COLOR_OUTPUT macro is set.
#define METRIC_NUMERIC_LIGHTWHITE ""

//! Switches the text color back to the default color.
#define METRIC_NUMERIC_OLDCOLOR ""

#endif

#endif
