// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_BLAS_TYPES_H
#define METRIC_NUMERIC_MATH_BLAS_TYPES_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/system/BLAS.h>
#include <metric/numeric/util/Types.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  TYPE DEFINITIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Signed integer type used in the BLAS/LAPACK wrapper functions.
// \ingroup math
*/
#if METRIC_NUMERIC_BLAS_IS_64BIT && (defined(INTEL_MKL_VERSION) || defined(MKL_ILP64))
using blas_int_t = long long int;
#elif METRIC_NUMERIC_BLAS_IS_64BIT
using blas_int_t = int64_t;
#else
using blas_int_t = int32_t;
#endif
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
