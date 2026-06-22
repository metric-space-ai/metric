// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_DVECSOFTMAXEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_DVECSOFTMAXEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/expressions/DenseVector.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  GLOBAL FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Computes the softmax function for the given dense vector.
// \ingroup dense_vector
//
// \param dv The given dense vector for the softmax computation.
// \return The resulting dense vector.
//
// This function computes the softmax function (i.e. the normalized exponential function) for
// the given dense vector \a dv (see also https://en.wikipedia.org/wiki/Softmax_function). The
// resulting dense vector consists of real values in the range (0..1], which add up to 1.
*/
template <typename VT // Type of the dense vector
		  ,
		  bool TF> // Transpose flag
auto softmax(const DenseVector<VT, TF> &dv)
{
	auto tmp(evaluate(exp(*dv - max(*dv))));
	const auto scalar(sum(*tmp));
	tmp /= scalar;
	return tmp;
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
