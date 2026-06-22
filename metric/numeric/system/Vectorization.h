// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_SYSTEM_VECTORIZATION_H
#define METRIC_NUMERIC_SYSTEM_VECTORIZATION_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/config/Vectorization.h>
#include <metric/numeric/util/StaticAssert.h>

//=================================================================================================
//
//  SSE/AVX MACRO DEFINITIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
#ifdef __AVX512F__
#ifndef __AVX2__
#define __AVX2__
#endif
#endif
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
#ifdef __AVX2__
#ifndef __AVX__
#define __AVX__
#endif
#endif
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
#ifdef __AVX__
#ifndef __MMX__
#define __MMX__
#endif
#ifndef __SSE__
#define __SSE__
#endif
#ifndef __SSE2__
#define __SSE2__
#endif
#ifndef __SSE3__
#define __SSE3__
#endif
#ifndef __SSSE3__
#define __SSSE3__
#endif
#ifndef __SSE4_1__
#define __SSE4_1__
#endif
#ifndef __SSE4_2__
#define __SSE4_2__
#endif
#endif
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  SSE/AVX/MIC MODE CONFIGURATION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Compilation switch for the SSE mode.
// \ingroup system
//
// This compilation switch enables/disables the SSE mode. In case the SSE mode is enabled
// (i.e. in case SSE functionality is available) the Metric numeric library attempts to vectorize
// the linear algebra operations by SSE intrinsics. In case the SSE mode is disabled, the
// Metric numeric library chooses default, non-vectorized functionality for the operations.
*/
#if METRIC_NUMERIC_USE_VECTORIZATION && (defined(__SSE__) || defined(_M_AMD64) || defined(_M_X64) || (_M_IX86_FP > 0))
#define METRIC_NUMERIC_SSE_MODE 1
#else
#define METRIC_NUMERIC_SSE_MODE 0
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compilation switch for the SSE2 mode.
// \ingroup system
//
// This compilation switch enables/disables the SSE2 mode. In case the SSE2 mode is enabled
// (i.e. in case SSE2 functionality is available) the Metric numeric library attempts to vectorize
// the linear algebra operations by SSE2 intrinsics. In case the SSE2 mode is disabled, the
// Metric numeric library chooses default, non-vectorized functionality for the operations.
*/
#if METRIC_NUMERIC_USE_VECTORIZATION && (defined(__SSE2__) || defined(_M_AMD64) || defined(_M_X64) || (_M_IX86_FP > 1))
#define METRIC_NUMERIC_SSE2_MODE 1
#else
#define METRIC_NUMERIC_SSE2_MODE 0
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compilation switch for the SSE3 mode.
// \ingroup system
//
// This compilation switch enables/disables the SSE3 mode. In case the SSE3 mode is enabled
// (i.e. in case SSE3 functionality is available) the Metric numeric library attempts to vectorize
// the linear algebra operations by SSE3 intrinsics. In case the SSE3 mode is disabled, the
// Metric numeric library chooses default, non-vectorized functionality for the operations.
*/
#if METRIC_NUMERIC_USE_VECTORIZATION && defined(__SSE3__)
#define METRIC_NUMERIC_SSE3_MODE 1
#else
#define METRIC_NUMERIC_SSE3_MODE 0
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compilation switch for the SSSE3 mode.
// \ingroup system
//
// This compilation switch enables/disables the SSSE3 mode. In case the SSSE3 mode is enabled
// (i.e. in case SSSE3 functionality is available) the Metric numeric library attempts to vectorize
// the linear algebra operations by SSSE3 intrinsics. In case the SSSE3 mode is disabled, the
// Metric numeric library chooses default, non-vectorized functionality for the operations.
*/
#if METRIC_NUMERIC_USE_VECTORIZATION && defined(__SSSE3__)
#define METRIC_NUMERIC_SSSE3_MODE 1
#else
#define METRIC_NUMERIC_SSSE3_MODE 0
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compilation switch for the SSE4 mode.
// \ingroup system
//
// This compilation switch enables/disables the SSE4 mode. In case the SSE4 mode is enabled
// (i.e. in case SSE4 functionality is available) the Metric numeric library attempts to vectorize
// the linear algebra operations by SSE4 intrinsics. In case the SSE4 mode is disabled,
// the Metric numeric library chooses default, non-vectorized functionality for the operations.
*/
#if METRIC_NUMERIC_USE_VECTORIZATION && (defined(__SSE4_2__) || defined(__SSE4_1__))
#define METRIC_NUMERIC_SSE4_MODE 1
#else
#define METRIC_NUMERIC_SSE4_MODE 0
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compilation switch for the AVX mode.
// \ingroup system
//
// This compilation switch enables/disables the AVX mode. In case the AVX mode is enabled
// (i.e. in case AVX functionality is available) the Metric numeric library attempts to vectorize
// the linear algebra operations by AVX intrinsics. In case the AVX mode is disabled,
// the Metric numeric library chooses default, non-vectorized functionality for the operations.
*/
#if METRIC_NUMERIC_USE_VECTORIZATION && defined(__AVX__)
#define METRIC_NUMERIC_AVX_MODE 1
#else
#define METRIC_NUMERIC_AVX_MODE 0
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compilation switch for the AVX2 mode.
// \ingroup system
//
// This compilation switch enables/disables the AVX2 mode. In case the AVX2 mode is enabled
// (i.e. in case AVX2 functionality is available) the Metric numeric library attempts to vectorize
// the linear algebra operations by AVX2 intrinsics. In case the AVX2 mode is disabled,
// the Metric numeric library chooses default, non-vectorized functionality for the operations.
*/
#if METRIC_NUMERIC_USE_VECTORIZATION && defined(__AVX2__)
#define METRIC_NUMERIC_AVX2_MODE 1
#else
#define METRIC_NUMERIC_AVX2_MODE 0
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compilation switch for the AVX512F mode.
// \ingroup system
//
// This compilation switch enables/disables the AVX512F mode. In case the AVX512F mode is
// enabled (i.e. in case AVX512F functionality is available) the Metric numeric library attempts to
// vectorize the linear algebra operations by AVX512F intrinsics. In case the AVX512F mode
// is disabled, the Metric numeric library chooses default, non-vectorized functionality for the
// operations.
*/
#if METRIC_NUMERIC_USE_VECTORIZATION && defined(__AVX512F__)
#define METRIC_NUMERIC_AVX512F_MODE 1
#else
#define METRIC_NUMERIC_AVX512F_MODE 0
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compilation switch for the AVX512BW mode.
// \ingroup system
//
// This compilation switch enables/disables the AVX512BW mode. In case the AVX512BW mode is
// enabled (i.e. in case AVX512BW functionality is available) the Metric numeric library attempts to
// vectorize the linear algebra operations by AVX512BW intrinsics. In case the AVX512BW mode
// is disabled, the Metric numeric library chooses default, non-vectorized functionality for the
// operations.
*/
#if METRIC_NUMERIC_USE_VECTORIZATION && defined(__AVX512BW__)
#define METRIC_NUMERIC_AVX512BW_MODE 1
#else
#define METRIC_NUMERIC_AVX512BW_MODE 0
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compilation switch for the AVX512DQ mode.
// \ingroup system
//
// This compilation switch enables/disables the AVX512DQ mode. In case the AVX512DQ mode is
// enabled (i.e. in case AVX512DQ functionality is available) the Metric numeric library attempts to
// vectorize the linear algebra operations by AVX512DQ intrinsics. In case the AVX512DQ mode
// is disabled, the Metric numeric library chooses default, non-vectorized functionality for the
// operations.
*/
#if METRIC_NUMERIC_USE_VECTORIZATION && defined(__AVX512DQ__)
#define METRIC_NUMERIC_AVX512DQ_MODE 1
#else
#define METRIC_NUMERIC_AVX512DQ_MODE 0
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compilation switch for the MIC mode.
// \ingroup system
//
// This compilation switch enables/disables the MIC mode. In case the MIC mode is enabled
// (i.e. in case MIC functionality is available) the Metric numeric library attempts to vectorize
// the linear algebra operations by MIC intrinsics. In case the MIC mode is disabled,
// the Metric numeric library chooses default, non-vectorized functionality for the operations.
*/
#if METRIC_NUMERIC_USE_VECTORIZATION && defined(__MIC__)
#define METRIC_NUMERIC_MIC_MODE 1
#else
#define METRIC_NUMERIC_MIC_MODE 0
#endif
//*************************************************************************************************

//=================================================================================================
//
//  FMA MODE CONFIGURATION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Compilation switch for the FMA mode.
// \ingroup system
//
// This compilation switch enables/disables the FMA mode. In case the FMA mode is enabled
// (i.e. in case FMA functionality is available) the Metric numeric library attempts to vectorize
// the linear algebra operations by FMA intrinsics. In case the FMA mode is disabled,
// the Metric numeric library chooses default, non-vectorized functionality for the operations.
*/
#if METRIC_NUMERIC_USE_VECTORIZATION && defined(__FMA__)
#define METRIC_NUMERIC_FMA_MODE 1
#else
#define METRIC_NUMERIC_FMA_MODE 0
#endif
//*************************************************************************************************

//=================================================================================================
//
//  SVML MODE CONFIGURATION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Compilation switch for the SVML mode.
// \ingroup system
//
// This compilation switch enables/disables the SVML mode. In case the SVML mode is enabled
// (i.e. in case an Intel compiler is used) the Metric numeric library attempts to vectorize several
// linear algebra operations by SVML intrinsics. In case the SVML mode is disabled, the
// Metric numeric library chooses default, non-vectorized functionality for the operations.
*/
#if METRIC_NUMERIC_USE_VECTORIZATION &&                                                                                \
	(defined(__INTEL_COMPILER) || defined(__ICL) || defined(__ICC) || defined(__ECC))
#define METRIC_NUMERIC_SVML_MODE 1
#else
#define METRIC_NUMERIC_SVML_MODE 0
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compilation switch for the Sleef mode.
// \ingroup system
//
// This compilation switch enables/disables the Sleef mode. In case the Sleef mode is enabled
// the Metric numeric library attempts to vectorize several linear algebra operations by Sleef intrinsics.
// In case the Sleef mode is disabled, the Metric numeric library chooses default, non-vectorized
// functionality for the operations.
*/
#if METRIC_NUMERIC_USE_VECTORIZATION && METRIC_NUMERIC_USE_SLEEF
#define METRIC_NUMERIC_SLEEF_MODE 1
#else
#define METRIC_NUMERIC_SLEEF_MODE 0
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compilation switch for the XSIMD mode.
// \ingroup system
//
// This compilation switch enables/disables the XSIMD mode. In case the XSIMD mode is enabled
// the Metric numeric library attempts to vectorize several linear algebra operations by XSIMD intrinsics.
// In case the XSIMD mode is disabled, the Metric numeric library chooses default, non-vectorized
// functionality for the operations.
*/
#if METRIC_NUMERIC_USE_VECTORIZATION && METRIC_NUMERIC_USE_XSIMD
#define METRIC_NUMERIC_XSIMD_MODE 1
#else
#define METRIC_NUMERIC_XSIMD_MODE 0
#endif
//*************************************************************************************************

//=================================================================================================
//
//  COMPILE TIME CONSTRAINTS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
namespace {

METRIC_NUMERIC_STATIC_ASSERT(!METRIC_NUMERIC_SSE2_MODE || METRIC_NUMERIC_SSE_MODE);
METRIC_NUMERIC_STATIC_ASSERT(!METRIC_NUMERIC_SSE3_MODE || METRIC_NUMERIC_SSE2_MODE);
METRIC_NUMERIC_STATIC_ASSERT(!METRIC_NUMERIC_SSSE3_MODE || METRIC_NUMERIC_SSE3_MODE);
METRIC_NUMERIC_STATIC_ASSERT(!METRIC_NUMERIC_SSE4_MODE || METRIC_NUMERIC_SSSE3_MODE);
METRIC_NUMERIC_STATIC_ASSERT(!METRIC_NUMERIC_AVX_MODE || METRIC_NUMERIC_SSE4_MODE);
METRIC_NUMERIC_STATIC_ASSERT(!METRIC_NUMERIC_AVX2_MODE || METRIC_NUMERIC_AVX_MODE);
METRIC_NUMERIC_STATIC_ASSERT(!METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_AVX2_MODE);
METRIC_NUMERIC_STATIC_ASSERT(!METRIC_NUMERIC_AVX512BW_MODE || METRIC_NUMERIC_AVX512F_MODE);
METRIC_NUMERIC_STATIC_ASSERT(!METRIC_NUMERIC_AVX512DQ_MODE || METRIC_NUMERIC_AVX512F_MODE);

} // namespace
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  SSE/AVX/MIC INCLUDE FILE CONFIGURATION
//
//=================================================================================================

#if METRIC_NUMERIC_AVX512F_MODE || METRIC_NUMERIC_MIC_MODE || METRIC_NUMERIC_AVX2_MODE || METRIC_NUMERIC_AVX_MODE
#include <immintrin.h>
#elif METRIC_NUMERIC_SSE4_MODE
#include <smmintrin.h>
#elif METRIC_NUMERIC_SSSE3_MODE
#include <tmmintrin.h>
#elif METRIC_NUMERIC_SSE3_MODE
#include <pmmintrin.h>
#elif METRIC_NUMERIC_SSE2_MODE
#include <emmintrin.h>
#elif METRIC_NUMERIC_SSE_MODE
#include <xmmintrin.h>
#endif

#if METRIC_NUMERIC_XSIMD_MODE
#include <xsimd/xsimd.hpp>
#endif

#endif
