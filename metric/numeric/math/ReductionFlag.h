// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_REDUCTIONFLAG_H
#define METRIC_NUMERIC_MATH_REDUCTIONFLAG_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/Types.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  REDUCTION FLAGS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Type of the reduction flags.
 */
using ReductionFlag = size_t;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Reduction flag for row-wise reduction operations.
//
// This flag can be used to perform row-wise reduction operations on matrices. The following
// example shows the row-wise summation of a row-major matrix:

   \code
   using mtrc::numeric::rowMajor;
   using mtrc::numeric::columnVector;

   mtrc::numeric::DynamicMatrix<int,rowMajor> A{ { 4, 1, 2 }, { -2, 0, 3 } };
   mtrc::numeric::DynamicVector<int,columnVector> v;

   v = sum<rowwise>( A );  // Results in ( 7, 1 )
   \endcode
*/
constexpr ReductionFlag rowwise = 1UL;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Reduction flag for column-wise reduction operations.
//
// This flag can be used to perform column-wise reduction operations on matrices. The following
// example shows the column-wise summation of a column-major matrix:

   \code
   using mtrc::numeric::columnMajor;
   using mtrc::numeric::rowVector;

   mtrc::numeric::DynamicMatrix<int,columnMajor> A{ { 4, 1, 2 }, { -2, 0, 3 } };
   mtrc::numeric::DynamicVector<int,rowVector> v;

   v = sum<columnwise>( A );  // Results in ( 2, 1, 5 )
   \endcode
*/
constexpr ReductionFlag columnwise = 0UL;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
