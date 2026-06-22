// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_SYSTEM_TRANSPOSEFLAG_H
#define METRIC_NUMERIC_SYSTEM_TRANSPOSEFLAG_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/config/TransposeFlag.h>
#include <metric/numeric/math/TransposeFlag.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  TRANSPOSE FLAG
//
//=================================================================================================

//*************************************************************************************************
/*!\brief The default transpose flag for all vectors of the Metric numeric library.
// \ingroup system
//
// This value specifies the default transpose flag for all vectors of the Metric numeric library.
// In case no explicit transpose flag is specified with the according vector type, this
// setting is used.

   \code
   // Explicit specification of the transpose flag => column vector
   StaticVector<double,3UL,columnVector> a;

   // No explicit specification of the transpose flag => use of the default transpose flag
   StaticVector<double,3UL> b;
   \endcode

// The default transpose flag is defined via the METRIC_NUMERIC_DEFAULT_TRANSPOSE_FLAG compilation switch
// (see the \ref transpose_flag section). Valid settings for this value are mtrc::numeric::rowVector and
// mtrc::numeric::columnVector.
*/
constexpr bool defaultTransposeFlag = METRIC_NUMERIC_DEFAULT_TRANSPOSE_FLAG;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
