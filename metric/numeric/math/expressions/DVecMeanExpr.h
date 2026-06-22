// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_DVECMEANEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_DVECMEANEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/Exception.h>
#include <metric/numeric/math/expressions/DenseVector.h>
#include <metric/numeric/math/shims/Invert.h>
#include <metric/numeric/math/typetraits/IsUniform.h>
#include <metric/numeric/math/typetraits/UnderlyingBuiltin.h>
#include <metric/numeric/util/Assert.h>
#include <metric/numeric/util/FunctionTrace.h>
#include <metric/numeric/util/IntegralConstant.h>
#include <metric/numeric/util/Types.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  GLOBAL FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Backend implementation of the \c mean() function for general dense vectors.
// \ingroup dense_vector
//
// \param dv The given general dense vector for the mean computation.
// \return The mean of the given vector.
*/
template <typename VT // Type of the dense vector
		  ,
		  bool TF> // Transpose flag
inline decltype(auto) mean_backend(const DenseVector<VT, TF> &dv, FalseType)
{
	using BT = UnderlyingBuiltin_t<VT>;

	METRIC_NUMERIC_INTERNAL_ASSERT(size(*dv) > 0UL, "Invalid vector size detected");

	return sum(*dv) * inv(BT(size(*dv)));
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Backend implementation of the \c mean() function for uniform dense vectors.
// \ingroup dense_vector
//
// \param dv The given uniform dense vector for the mean computation.
// \return The mean of the given vector.
*/
template <typename VT // Type of the dense vector
		  ,
		  bool TF> // Transpose flag
inline decltype(auto) mean_backend(const DenseVector<VT, TF> &dv, TrueType)
{
	METRIC_NUMERIC_INTERNAL_ASSERT(size(*dv) > 0UL, "Invalid vector size detected");

	return (*dv)[0];
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Computes the (arithmetic) mean for the given dense vector.
// \ingroup dense_vector
//
// \param dv The given dense vector for the mean computation.
// \return The mean of the given vector.
// \exception std::invalid_argument Invalid input vector.
//
// This function computes the
// <a href="https://en.wikipedia.org/wiki/Arithmetic_mean">(arithmetic) mean</a> for the given
// dense vector \a dv. Example:

   \code
   using mtrc::numeric::DynamicVector;

   DynamicVector<int> v{ 1, 4, 3, 6, 7 };

   const double m = mean( v );  // Results in 4.2 (i.e. 21/5)
   \endcode

// In case the size of the given vector is 0, a \a std::invalid_argument is thrown.
*/
template <typename VT // Type of the dense vector
		  ,
		  bool TF> // Transpose flag
inline decltype(auto) mean(const DenseVector<VT, TF> &dv)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	if (size(*dv) == 0UL) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid input vector");
	}

	return mean_backend(*dv, IsUniform<VT>());
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
