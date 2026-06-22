// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_SYSTEM_PADDING_H
#define METRIC_NUMERIC_SYSTEM_PADDING_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/config/Padding.h>
#include <metric/numeric/math/PaddingFlag.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  PADDING FLAG
//
//=================================================================================================

//*************************************************************************************************
/*!\brief The default padding flag for all vectors and matrices of the Metric numeric library.
// \ingroup system
//
// This value specifies the default padding flag for all vectors and matrices of the Metric numeric
// library. In case no explicit padding flag is specified with the according vector type, this
// setting is used.

   \code
   // Explicit specification of the padding flag => the vector is padded
   StaticVector<double,3UL,columnVector,aligned,padded> a;

   // No explicit specification of the padding flag => use of the default padding flag
   StaticVector<double,3UL,columnVector,aligned> b;
   \endcode

// The default alignment is defined via the METRIC_NUMERIC_DEFAULT_PADDING_FLAG compilation switch (see
// the \ref padding section). Valid settings for this value are mtrc::numeric::padded and mtrc::numeric::unpadded.
*/
constexpr PaddingFlag defaultPaddingFlag = METRIC_NUMERIC_DEFAULT_PADDING_FLAG;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
