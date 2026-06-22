// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_DIMENSIONOF_H
#define METRIC_NUMERIC_UTIL_DIMENSIONOF_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/MaybeUnused.h>
#include <metric/numeric/util/Types.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  DIMENSIONOF FUNCTIONALITY
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Static evaluation of array dimensions.
// \ingroup util
//
// \param a Reference to a static array of type T and size N.
// \return Dimension of the static array.
//
// The dimensionof function is a safe way to evaluate the size of an array. The function only
// works for array arguments and fails for pointers and user-defined class types.

   \code
   int              ai[ 42 ];
   int*             pi( ai );
   std::vector<int> vi( 42 );

   dimensionof( ai );  // Returns the size of the integer array (42)
   dimensionof( pi );  // Fails to compile!
   dimensionof( vi );  // Fails to compile!
   \endcode
*/
template <typename T, size_t N> constexpr size_t dimensionof(T (&a)[N])
{
	MAYBE_UNUSED(a);
	return N;
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
