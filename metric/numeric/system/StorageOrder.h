// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_SYSTEM_STORAGEORDER_H
#define METRIC_NUMERIC_SYSTEM_STORAGEORDER_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/config/StorageOrder.h>
#include <metric/numeric/math/StorageOrder.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  STORAGE ORDER
//
//=================================================================================================

//*************************************************************************************************
/*!\brief The default storage order for all matrices of the Metric numeric library.
// \ingroup system
//
// This value specifies the default storage order for all matrices of the Metric numeric library.
// In case no explicit storage order is specified with the according matrix type, this
// setting is used.

   \code
   // Explicit specification of the storage order => row-major matrix
   StaticMatrix<double,3UL,3UL,rowMajor> A;

   // No explicit specification of the storage order => use of the default storage order
   StaticMatrix<double,3UL,3UL> B;
   \endcode

// The default storage order is defined via the METRIC_NUMERIC_DEFAULT_STORAGE_ORDER compilation switch
// (see the \ref storage_order section). Valid settings for this value are mtrc::numeric::rowMajor and
// mtrc::numeric::columnMajor.
*/
constexpr bool defaultStorageOrder = METRIC_NUMERIC_DEFAULT_STORAGE_ORDER;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
