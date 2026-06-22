// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_TRANSPOSEFLAG_H
#define METRIC_NUMERIC_MATH_TRANSPOSEFLAG_H
namespace mtrc::numeric {

//=================================================================================================
//
//  VECTOR TRANSPOSE FLAG TYPES
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Transpose flag for column vectors.
//
// Via this flag it is possible to specify vectors as column vectors. The following example
// demonstrates the setup of a 3-dimensional column vector:

   \code
   using mtrc::numeric::columnVector;
   mtrc::numeric::StaticVector<int,3UL,columnVector> v{ 1, 2, 3 };
   \endcode
*/
constexpr bool columnVector = false;
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Transpose flag for row vectors.
//
// Via this flag it is possible to specify vectors as row vectors. The following example
// demonstrates the setup of a 3-dimensional row vector:

   \code
   using mtrc::numeric::rowVector;
   mtrc::numeric::StaticVector<int,3UL,rowVector> v{ 1, 2, 3 };
   \endcode
*/
constexpr bool rowVector = true;
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
