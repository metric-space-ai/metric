// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_SVECMEANEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_SVECMEANEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/Exception.h>
#include <metric/numeric/math/expressions/SparseVector.h>
#include <metric/numeric/math/shims/Invert.h>
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
/*!\brief Backend implementation of the \c mean() function for general sparse vectors.
// \ingroup sparse_vector
//
// \param sv The given general sparse vector for the mean computation.
// \return The mean of the given vector.
*/
template <typename VT // Type of the sparse vector
		  ,
		  bool TF> // Transpose flag
inline decltype(auto) mean_backend(const SparseVector<VT, TF> &sv, FalseType)
{
	using BT = UnderlyingBuiltin_t<VT>;

	METRIC_NUMERIC_INTERNAL_ASSERT(size(*sv) > 0UL, "Invalid vector size detected");

	return sum(*sv) * inv(BT(size(*sv)));
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Backend implementation of the \c mean() function for uniform sparse vectors.
// \ingroup sparse_vector
//
// \param sv The given uniform sparse vector for the mean computation.
// \return The mean of the given vector.
*/
template <typename VT // Type of the sparse vector
		  ,
		  bool TF> // Transpose flag
inline decltype(auto) mean_backend(const SparseVector<VT, TF> &sv, TrueType)
{
	MAYBE_UNUSED(sv);

	METRIC_NUMERIC_INTERNAL_ASSERT(size(*sv) > 0UL, "Invalid vector size detected");

	return ElementType_t<VT>();
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Computes the (arithmetic) mean for the given sparse vector.
// \ingroup sparse_vector
//
// \param sv The given sparse vector for the mean computation.
// \return The mean of the given vector.
// \exception std::invalid_argument Invalid input vector.
//
// This function computes the
// <a href="https://en.wikipedia.org/wiki/Arithmetic_mean">(arithmetic) mean</a> for the given
// sparse vector \a sv. Both the non-zero and zero elements of the sparse vector are taken into
// account. Example:

   \code
   using mtrc::numeric::CompressedVector;

   CompressedVector<int> v{ 1, 0, 4, 0, 3, 0, 6, 0, 7, 0 };

   const double m = mean( v );  // Results in 2.1 (i.e. 21/10 )
   \endcode

// In case the size of the given vector is 0, a \a std::invalid_argument is thrown.
*/
template <typename VT // Type of the sparse vector
		  ,
		  bool TF> // Transpose flag
inline decltype(auto) mean(const SparseVector<VT, TF> &sv)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	if (size(*sv) == 0UL) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid input vector");
	}

	return mean_backend(*sv, IsZero<VT>());
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
