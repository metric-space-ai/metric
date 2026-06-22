// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_DVECSTDDEVEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_DVECSTDDEVEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/expressions/DVecMapExpr.h>
#include <metric/numeric/math/expressions/DVecVarExpr.h>
#include <metric/numeric/math/expressions/DenseVector.h>
#include <metric/numeric/util/FunctionTrace.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  GLOBAL FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Computes the standard deviation for the given dense vector.
// \ingroup dense_vector
//
// \param dv The given dense vector for the standard deviation computation.
// \return The standard deviation of the given vector.
// \exception std::invalid_argument Invalid input vector.
//
// This function computes the
// <a href="https://en.wikipedia.org/wiki/Standard_deviation">standard deviation</a> for the
// given dense vector \a dv. Example:

   \code
   using mtrc::numeric::DynamicVector;

   DynamicVector<int> v{ 1, 4, 3, 6, 7 };

   const double m = stddev( v );  // Results in 2.38747
   \endcode

// In case the size of the given vector is smaller than 2, a \a std::invalid_argument is thrown.
*/
template <typename VT // Type of the dense vector
		  ,
		  bool TF> // Transpose flag
decltype(auto) stddev(const DenseVector<VT, TF> &dv)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return sqrt(var(*dv));
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
