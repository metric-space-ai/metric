// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_SYSTEM_ALIGNMENT_H
#define METRIC_NUMERIC_SYSTEM_ALIGNMENT_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/config/Alignment.h>
#include <metric/numeric/math/AlignmentFlag.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  ALIGNMENT FLAG
//
//=================================================================================================

//*************************************************************************************************
/*!\brief The default alignment for all vectors and matrices of the Metric numeric library.
// \ingroup system
//
// This value specifies the default alignment for all vectors and matrices of the Metric numeric library.
// In case no explicit alignment is specified with the according vector type, this setting
// is used.

   \code
   // Explicit specification of the alignment => the vector element are aligned
   StaticVector<double,3UL,columnVector,aligned> a;

   // No explicit specification of the alignment => use of the default alignment
   StaticVector<double,3UL> b;
   \endcode

// The default alignment is defined via the METRIC_NUMERIC_DEFAULT_ALIGNMENT_FLAG compilation switch
// (see the \ref alignment section). Valid settings for this value are mtrc::numeric::aligned and
// mtrc::numeric::unaligned.
*/
constexpr AlignmentFlag defaultAlignmentFlag = METRIC_NUMERIC_DEFAULT_ALIGNMENT_FLAG;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
