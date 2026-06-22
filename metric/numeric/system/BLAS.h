// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_SYSTEM_BLAS_H
#define METRIC_NUMERIC_SYSTEM_BLAS_H
//=================================================================================================
//
//  BLAS MODE CONFIGURATION
//
//=================================================================================================

#include <metric/numeric/config/BLAS.h>

//=================================================================================================
//
//  BLAS INCLUDE FILE CONFIGURATION
//
//=================================================================================================

#if METRIC_NUMERIC_BLAS_MODE
extern "C" {
#include METRIC_NUMERIC_BLAS_INCLUDE_FILE
}
#endif

#endif
