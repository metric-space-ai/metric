// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_BLAS_MODE
#define METRIC_NUMERIC_BLAS_MODE 0
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compilation switch for the BLAS 64-bit support.
// \ingroup config
//
// This compilation switch enables/disables 64-bit BLAS and LAPACK support. In case the 64-bit
// BLAS mode is enabled, \c mtrc::numeric::blas_int_t, which is used in the BLAS and LAPACK wrapper
// functions, is a 64-bit signed integral type. In case the 64-bit BLAS mode is disabled,
// \c mtrc::numeric::blas_int_t is a 32-bit signed integral type.
//
// Possible settings for the switch:
//  - 32-bit BLAS/LAPACK: \b 0 (default)
//  - 64-bit BLAS/LAPACK: \b 1
//
// \warning Changing the setting of the BLAS mode requires a recompilation of all code using the
// Metric numeric library!
//
// \note It is possible to (de-)activate the 64-bit BLAS mode via command line or by defining
// this symbol manually before including any Metric numeric header file:

   \code
   g++ ... -DMETRIC_NUMERIC_BLAS_IS_64BIT=1 ...
   \endcode

   \code
   #define METRIC_NUMERIC_BLAS_IS_64BIT 1
   #include <metric/numeric/Numeric.h>
   \endcode
*/
#ifndef METRIC_NUMERIC_BLAS_IS_64BIT
#define METRIC_NUMERIC_BLAS_IS_64BIT 0
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compilation switch for the parallel BLAS mode.
// \ingroup config
//
// This compilation switch specifies whether the used BLAS library is itself parallelized or not.
// In case the given BLAS library is itself parallelized, the Metric numeric library does not perform any
// attempt to parallelize the execution of BLAS kernels. If, however, the given BLAS library is
// not parallelized Metric numeric will attempt to parallelize the execution of BLAS kernels.
//
// Possible settings for the switch:
//  - BLAS library is not parallelized: \b 0 (default)
//  - BLAS library is parallelized    : \b 1
//
// \warning Changing the setting of the BLAS mode requires a recompilation of all code using the
// Metric numeric library!
//
// \note It is possible to (de-)activate the parallel BLAS mode via command line or by defining
// this symbol manually before including any Metric numeric header file:

   \code
   g++ ... -DMETRIC_NUMERIC_BLAS_IS_PARALLEL=1 ...
   \endcode

   \code
   #define METRIC_NUMERIC_BLAS_IS_PARALLEL 1
   #include <metric/numeric/Numeric.h>
   \endcode
*/
#ifndef METRIC_NUMERIC_BLAS_IS_PARALLEL
#define METRIC_NUMERIC_BLAS_IS_PARALLEL 0
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compilation switch for the BLAS matrix/vector multiplication kernels (gemv).
// \ingroup config
//
// This compilation switch enables/disables the BLAS matrix/vector multiplication kernels. If the
// switch is enabled, multiplications between dense matrices and dense vectors are computed by
// BLAS kernels, if it is disabled the multiplications are handled by the default Metric numeric kernels.
//
// Possible settings for the switch:
//  - Disabled: \b 0 (default)
//  - Enabled : \b 1
//
// \warning Changing the setting of this compilation switch requires a recompilation of all code
// using the Metric numeric library!
//
// \note It is possible to (de-)activate the use of the BLAS matrix/vector multiplication kernels
// via command line or by defining this symbol manually before including any Metric numeric header file:

   \code
   g++ ... -DMETRIC_NUMERIC_USE_BLAS_MATRIX_VECTOR_MULTIPLICATION=1 ...
   \endcode

   \code
   #define METRIC_NUMERIC_USE_BLAS_MATRIX_VECTOR_MULTIPLICATION 1
   #include <metric/numeric/Numeric.h>
   \endcode
*/
#ifndef METRIC_NUMERIC_USE_BLAS_MATRIX_VECTOR_MULTIPLICATION
#define METRIC_NUMERIC_USE_BLAS_MATRIX_VECTOR_MULTIPLICATION 0
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compilation switch for the BLAS matrix/matrix multiplication kernels (gemv).
// \ingroup config
//
// This compilation switch enables/disables the BLAS matrix/matrix multiplication kernels. If the
// switch is enabled, multiplications between dense matrices are computed by BLAS kernels, if it
// is disabled the multiplications are handled by the default Metric numeric kernels.
//
// Possible settings for the switch:
//  - Disabled: \b 0
//  - Enabled : \b 1 (default)
//
// \warning Changing the setting of this compilation switch requires a recompilation of all code
// code using the Metric numeric library!
//
// \note It is possible to (de-)activate the use of the BLAS matrix/matrix multiplication kernels
// via command line or by defining this symbol manually before including any Metric numeric header file:

   \code
   g++ ... -DMETRIC_NUMERIC_USE_BLAS_MATRIX_MATRIX_MULTIPLICATION=1 ...
   \endcode

   \code
   #define METRIC_NUMERIC_USE_BLAS_MATRIX_MATRIX_MULTIPLICATION 1
   #include <metric/numeric/Numeric.h>
   \endcode
*/
#ifndef METRIC_NUMERIC_USE_BLAS_MATRIX_MATRIX_MULTIPLICATION
#define METRIC_NUMERIC_USE_BLAS_MATRIX_MATRIX_MULTIPLICATION 1
#endif
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Compilation switch for the BLAS include file.
// \ingroup config
//
// This compilation switch specifies the name of the BLAS include file. By default, the header
// \c <cblas.h> is included when the BLAS mode is activated. In case the name of the include file
// differs (as for instance in case of the MKL the file is called \c <mkl_cblas.h>) this switch
// needs to be adapted accordingly.
//
// \warning Changing the name of the BLAS include file requires a recompilation of all code using
// the Metric numeric library!
//
// \note It is possible to specify the BLAS include file via command line or by defining this
// symbol manually before including any Metric numeric header file:

   \code
   g++ ... -DMETRIC_NUMERIC_BLAS_INCLUDE_FILE="<cblas.h>" ...
   \endcode

   \code
   #define METRIC_NUMERIC_BLAS_INCLUDE_FILE <cblas.h>
   #include <metric/numeric/Numeric.h>
   \endcode
*/
#ifndef METRIC_NUMERIC_BLAS_INCLUDE_FILE
#define METRIC_NUMERIC_BLAS_INCLUDE_FILE <cblas.h>
#endif
//*************************************************************************************************
