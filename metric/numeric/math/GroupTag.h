// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_GROUPTAG_H
#define METRIC_NUMERIC_MATH_GROUPTAG_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/Types.h>

namespace mtrc::numeric {

//*************************************************************************************************
/*!\brief Group tag for vectors and matrices.
// \ingroup math
//
// \section grouptag_general General
//
// Via the GroupTag class template it is possible to define distinct groups of vectors and matrices.
// Only vectors and matrices that belong to the same group can be used together. The attempt to
// combine vectors and matrices from different groups results in a compilation error:

   \code
   using mtrc::numeric::columnVector;
   using mtrc::numeric::Group0;
   using mtrc::numeric::Group1;

   mtrc::numeric::DynamicVector<int,columnVector,Group0> a0, b0;
   mtrc::numeric::DynamicVector<int,columnVector,Group1> a1, b1;

   a0 + b0;  // Compiles, a0 and b0 are in the same group (Group0)
   a1 + b1;  // Compiles, a1 and b1 are in the same group (Group1)
   a0 + b1;  // Compilation error: a0 and b1 are not in the same group
   \endcode

// \section grouptag_custom_group Custom Group Tags
//
// The \b Metric numeric library provides 10 different groups based on the GroupTag class template (Group0
// through Group9). In order to create further groups, it is possible to instantiate the GroupTag
// class template with IDs beyond 9:

   \code
   using Group10 = mtrc::numeric::GroupTag<10>;
   \endcode
*/
template <size_t ID> struct GroupTag {};
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Tag for group 0. This is the default tag for vectors and matrices.
// \ingroup math
*/
using Group0 = GroupTag<0UL>;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Tag for group 1.
// \ingroup math
*/
using Group1 = GroupTag<1UL>;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Tag for group 2.
// \ingroup math
*/
using Group2 = GroupTag<2UL>;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Tag for group 3.
// \ingroup math
*/
using Group3 = GroupTag<3UL>;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Tag for group 4.
// \ingroup math
*/
using Group4 = GroupTag<4UL>;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Tag for group 5.
// \ingroup math
*/
using Group5 = GroupTag<5UL>;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Tag for group 6.
// \ingroup math
*/
using Group6 = GroupTag<6UL>;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Tag for group 7.
// \ingroup math
*/
using Group7 = GroupTag<7UL>;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Tag for group 8.
// \ingroup math
*/
using Group8 = GroupTag<8UL>;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Tag for group 9.
// \ingroup math
*/
using Group9 = GroupTag<9UL>;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
