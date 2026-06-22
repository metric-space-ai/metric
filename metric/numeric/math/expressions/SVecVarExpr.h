// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_SVECVAREXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_SVECVAREXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/Exception.h>
#include <metric/numeric/math/expressions/SparseVector.h>
#include <metric/numeric/math/shims/Invert.h>
#include <metric/numeric/math/shims/Pow2.h>
#include <metric/numeric/math/typetraits/IsZero.h>
#include <metric/numeric/math/typetraits/UnderlyingBuiltin.h>
#include <metric/numeric/util/Assert.h>
#include <metric/numeric/util/FunctionTrace.h>
#include <metric/numeric/util/IntegralConstant.h>
#include <metric/numeric/util/MaybeUnused.h>
#include <metric/numeric/util/Types.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  GLOBAL FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Backend implementation of the \c var() function for general sparse vectors.
// \ingroup sparse_vector
//
// \param sv The given general sparse vector for the variance computation.
// \return The variance of the given sparse.
*/
template <typename VT // Type of the sparse vector
		  ,
		  bool TF> // Transpose flag
decltype(auto) var_backend(const SparseVector<VT, TF> &sv, FalseType)
{
	using BT = UnderlyingBuiltin_t<VT>;

	const size_t n(size(*sv));
	const size_t nz(nonZeros(*sv));

	METRIC_NUMERIC_INTERNAL_ASSERT(n > 1UL, "Invalid vector size detected");
	METRIC_NUMERIC_INTERNAL_ASSERT(n >= nz, "Invalid number of non-zero elements detected");

	const auto meanValue(mean(*sv));
	auto variance((n - nz) * pow2(meanValue));

	const auto end((*sv).end());
	for (auto element = (*sv).begin(); element != end; ++element) {
		variance += pow2(element->value() - meanValue);
	}

	return variance * inv(BT(n - 1UL));
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Backend implementation of the \c var() function for uniform sparse vectors.
// \ingroup sparse_vector
//
// \param sv The given uniform sparse vector for the variance computation.
// \return The variance of the given vector.
*/
template <typename VT // Type of the sparse vector
		  ,
		  bool TF> // Transpose flag
decltype(auto) var_backend(const SparseVector<VT, TF> &sv, TrueType)
{
	MAYBE_UNUSED(sv);

	METRIC_NUMERIC_INTERNAL_ASSERT(size(*sv) > 1UL, "Invalid vector size detected");

	return ElementType_t<VT>();
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Computes the variance for the given sparse vector.
// \ingroup sparse_vector
//
// \param sv The given sparse vector for the variance computation.
// \return The variance of the given vector.
// \exception std::invalid_argument Invalid input vector.
//
// This function computes the <a href="https://en.wikipedia.org/wiki/Variance">variance</a> for
// the given sparse vector \a sv. Both the non-zero and zero elements of the sparse vector are
// taken into account. Example:

   \code
   using mtrc::numeric::CompressedVector;

   CompressedVector<int> v{ 1, 4, 3, 6, 7 };

   const double m = var( v );  // Results in 5.7
   \endcode

// In case the size of the given vector is smaller than 2, a \a std::invalid_argument is thrown.
*/
template <typename VT // Type of the sparse vector
		  ,
		  bool TF> // Transpose flag
decltype(auto) var(const SparseVector<VT, TF> &sv)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	if ((*sv).size() < 2UL) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid input vector");
	}

	return var_backend(*sv, IsZero<VT>());
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
