// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_SYSTEM_COMPILER_H
#define METRIC_NUMERIC_SYSTEM_COMPILER_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/StaticAssert.h>

//=================================================================================================
//
//  INTEL COMPILER MACRO DEFINITIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
#if defined(__INTEL_COMPILER) || defined(__ICL) || defined(__ICC) || defined(__ECC)
#define METRIC_NUMERIC_INTEL_COMPILER 1
#else
#define METRIC_NUMERIC_INTEL_COMPILER 0
#endif
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  CLANG COMPILER MACRO DEFINITIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
#if defined(__clang__) && !METRIC_NUMERIC_INTEL_COMPILER
#define METRIC_NUMERIC_CLANG_COMPILER 1
#else
#define METRIC_NUMERIC_CLANG_COMPILER 0
#endif
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
#define METRIC_NUMERIC_CLANG_MAJOR_VERSION __clang_major__
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
#define METRIC_NUMERIC_CLANG_MINOR_VERSION __clang_minor__
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
#define METRIC_NUMERIC_CLANG_PATCH_VERSION __clang_patchlevel__
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  GNU COMPILER MACRO DEFINITIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
#if defined(__GNUC__) && !METRIC_NUMERIC_CLANG_COMPILER && !METRIC_NUMERIC_INTEL_COMPILER
#define METRIC_NUMERIC_GNU_COMPILER 1
#else
#define METRIC_NUMERIC_GNU_COMPILER 0
#endif
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
#define METRIC_NUMERIC_GNU_MAJOR_VERSION __GNUC__
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
#define METRIC_NUMERIC_GNU_MINOR_VERSION __GNUC_MINOR__
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  MICROSOFT COMPILER MACRO DEFINITIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
#if defined(_MSC_VER) && !METRIC_NUMERIC_CLANG_COMPILER && !METRIC_NUMERIC_INTEL_COMPILER
#define METRIC_NUMERIC_MSC_COMPILER 1
#else
#define METRIC_NUMERIC_MSC_COMPILER 0
#endif
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  COMPILE TIME CONSTRAINTS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
namespace {

METRIC_NUMERIC_STATIC_ASSERT(!METRIC_NUMERIC_GNU_COMPILER ||
							 (!METRIC_NUMERIC_CLANG_COMPILER && !METRIC_NUMERIC_MSC_COMPILER &&
							  !METRIC_NUMERIC_INTEL_COMPILER));
METRIC_NUMERIC_STATIC_ASSERT(!METRIC_NUMERIC_CLANG_COMPILER ||
							 (!METRIC_NUMERIC_GNU_COMPILER && !METRIC_NUMERIC_MSC_COMPILER &&
							  !METRIC_NUMERIC_INTEL_COMPILER));
METRIC_NUMERIC_STATIC_ASSERT(!METRIC_NUMERIC_MSC_COMPILER ||
							 (!METRIC_NUMERIC_GNU_COMPILER && !METRIC_NUMERIC_CLANG_COMPILER &&
							  !METRIC_NUMERIC_INTEL_COMPILER));
METRIC_NUMERIC_STATIC_ASSERT(!METRIC_NUMERIC_INTEL_COMPILER ||
							 (!METRIC_NUMERIC_GNU_COMPILER && !METRIC_NUMERIC_CLANG_COMPILER &&
							  !METRIC_NUMERIC_MSC_COMPILER));

} // namespace
/*! \endcond */
//*************************************************************************************************

#endif
