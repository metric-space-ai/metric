// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_SVECSTDDEVEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_SVECSTDDEVEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/expressions/SVecMapExpr.h>
#include <metric/numeric/math/expressions/SVecVarExpr.h>
#include <metric/numeric/math/expressions/SparseVector.h>
#include <metric/numeric/util/FunctionTrace.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  GLOBAL FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Computes the standard deviation for the given sparse vector.
// \ingroup sparse_vector
//
// \param sv The given sparse vector for the standard deviation computation.
// \return The standard deviation of the given vector.
// \exception std::invalid_argument Invalid input vector.
//
// This function computes the
// <a href="https://en.wikipedia.org/wiki/Standard_deviation">standard deviation</a> for the
// given sparse vector \a sv. Both the non-zero and zero elements of the sparse vector are taken
// into account. Example:

   \code
   using mtrc::numeric::CompressedVector;

   CompressedVector<int> v{ 1, 4, 3, 6, 7 };

   const double m = stddev( v );  // Results in 2.38747
   \endcode

// In case the size of the given vector is smaller than 2, a \a std::invalid_argument is thrown.
*/
template <typename VT // Type of the sparse vector
		  ,
		  bool TF> // Transpose flag
decltype(auto) stddev(const SparseVector<VT, TF> &sv)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return sqrt(var(*sv));
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
