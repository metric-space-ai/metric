// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_DVECVAREXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_DVECVAREXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/Exception.h>
#include <metric/numeric/math/dense/UniformVector.h>
#include <metric/numeric/math/expressions/DenseVector.h>
#include <metric/numeric/math/functors/Pow2.h>
#include <metric/numeric/math/shims/Invert.h>
#include <metric/numeric/math/typetraits/IsUniform.h>
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
/*!\brief Backend implementation of the \c var() function for general dense vectors.
// \ingroup dense_vector
//
// \param dv The given general dense vector for the variance computation.
// \return The variance of the given vector.
*/
template <typename VT // Type of the dense vector
		  ,
		  bool TF> // Transpose flag
decltype(auto) var_backend(const DenseVector<VT, TF> &dv, FalseType)
{
	using BT = UnderlyingBuiltin_t<VT>;

	const size_t n(size(*dv));

	METRIC_NUMERIC_INTERNAL_ASSERT(n > 1UL, "Invalid vector size detected");

	const auto m(uniform<TF>(n, mean(*dv)));

	return sum(map((*dv) - m, Pow2())) * inv(BT(n - 1UL));
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Backend implementation of the \c var() function for uniform dense vectors.
// \ingroup dense_vector
//
// \param dv The given uniform dense vector for the variance computation.
// \return The variance of the given vector.
*/
template <typename VT // Type of the dense vector
		  ,
		  bool TF> // Transpose flag
decltype(auto) var_backend(const DenseVector<VT, TF> &dv, TrueType)
{
	MAYBE_UNUSED(dv);

	METRIC_NUMERIC_INTERNAL_ASSERT(size(*dv) > 1UL, "Invalid vector size detected");

	return ElementType_t<VT>();
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Computes the variance for the given dense vector.
// \ingroup dense_vector
//
// \param dv The given dense vector for the variance computation.
// \return The variance of the given vector.
// \exception std::invalid_argument Invalid input vector.
//
// This function computes the <a href="https://en.wikipedia.org/wiki/Variance">variance</a> for
// the given dense vector \a dv. Example:

   \code
   using mtrc::numeric::DynamicVector;

   DynamicVector<int> v{ 1, 4, 3, 6, 7 };

   const double m = var( v );  // Results in 5.7
   \endcode

// In case the size of the given vector is smaller than 2, a \a std::invalid_argument is thrown.
*/
template <typename VT // Type of the dense vector
		  ,
		  bool TF> // Transpose flag
decltype(auto) var(const DenseVector<VT, TF> &dv)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	const size_t n(size(*dv));

	if (n < 2UL) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid input vector");
	}

	return var_backend(*dv, IsUniform<VT>());
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
