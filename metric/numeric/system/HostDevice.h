// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_SYSTEM_HOSTDEVICE_H
#define METRIC_NUMERIC_SYSTEM_HOSTDEVICE_H
//=================================================================================================
//
//  CUDA MACRO DEFINITIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\def METRIC_NUMERIC_GLOBAL
// \brief Conditional macro that sets __global__ attribute when compiled with CUDA.
// \ingroup system
*/
#ifdef __CUDACC__
#define METRIC_NUMERIC_GLOBAL __global__
#else
#define METRIC_NUMERIC_GLOBAL
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\def METRIC_NUMERIC_DEVICE
// \brief Conditional macro that sets __device__ attribute when compiled with CUDA.
// \ingroup system
*/
#ifdef __CUDACC__
#define METRIC_NUMERIC_DEVICE __device__
#else
#define METRIC_NUMERIC_DEVICE
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\def METRIC_NUMERIC_HOST
// \brief Conditional macro that sets __host__ attribute when compiled with CUDA.
// \ingroup system
*/
#ifdef __CUDACC__
#define METRIC_NUMERIC_HOST __host__
#else
#define METRIC_NUMERIC_HOST
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\def METRIC_NUMERIC_DEVICE_CALLABLE
// \brief Conditional macro that sets __host__ and __device__ attributes when compiled with CUDA.
// \ingroup system
*/
#ifdef __CUDACC__
#define METRIC_NUMERIC_DEVICE_CALLABLE __host__ __device__
#else
#define METRIC_NUMERIC_DEVICE_CALLABLE
#endif
//*************************************************************************************************

#endif
