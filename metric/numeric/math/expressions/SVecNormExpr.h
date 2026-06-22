// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_SVECNORMEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_SVECNORMEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/Aliases.h>
#include <metric/numeric/math/expressions/SparseVector.h>
#include <metric/numeric/math/functors/Abs.h>
#include <metric/numeric/math/functors/Bind2nd.h>
#include <metric/numeric/math/functors/Cbrt.h>
#include <metric/numeric/math/functors/L1Norm.h>
#include <metric/numeric/math/functors/L2Norm.h>
#include <metric/numeric/math/functors/L3Norm.h>
#include <metric/numeric/math/functors/L4Norm.h>
#include <metric/numeric/math/functors/LpNorm.h>
#include <metric/numeric/math/functors/Noop.h>
#include <metric/numeric/math/functors/Pow.h>
#include <metric/numeric/math/functors/Pow2.h>
#include <metric/numeric/math/functors/Pow3.h>
#include <metric/numeric/math/functors/Qdrt.h>
#include <metric/numeric/math/functors/SqrAbs.h>
#include <metric/numeric/math/functors/Sqrt.h>
#include <metric/numeric/math/shims/Evaluate.h>
#include <metric/numeric/math/shims/Invert.h>
#include <metric/numeric/math/shims/IsZero.h>
#include <metric/numeric/math/traits/MultTrait.h>
#include <metric/numeric/math/typetraits/UnderlyingBuiltin.h>
#include <metric/numeric/util/Assert.h>
#include <metric/numeric/util/FunctionTrace.h>
#include <metric/numeric/util/StaticAssert.h>
#include <metric/numeric/util/TypeList.h>
#include <metric/numeric/util/typetraits/RemoveCVRef.h>
#include <utility>

namespace mtrc::numeric {

//=================================================================================================
//
//  GLOBAL FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Computes a custom norm for the given sparse vector.
// \ingroup sparse_vector
//
// \param sv The given sparse vector for the norm computation.
// \param abs The functor for the abs operation.
// \param power The functor for the power operation.
// \param root The functor for the root operation.
// \return The norm of the given sparse vector.
//
// This function computes a custom norm of the given sparse vector by means of the given functors.
// The following example demonstrates the computation of the L2 norm by means of the mtrc::numeric::Noop,
// mtrc::numeric::Pow2 and mtrc::numeric::Sqrt functors:

   \code
   mtrc::numeric::CompressedVector<double> a;
   // ... Resizing and initialization
   const double l2 = norm( a, mtrc::numeric::Noop(), mtrc::numeric::Pow2(), mtrc::numeric::Sqrt() );
   \endcode
*/
template <typename VT // Type of the sparse vector
		  ,
		  bool TF // Transpose flag
		  ,
		  typename Abs // Type of the abs operation
		  ,
		  typename Power // Type of the power operation
		  ,
		  typename Root> // Type of the root operation
decltype(auto) norm_backend(const SparseVector<VT, TF> &sv, Abs abs, Power power, Root root)
{
	using CT = CompositeType_t<VT>;
	using ET = ElementType_t<VT>;
	using PT = RemoveCVRef_t<decltype(power(abs(std::declval<ET>())))>;
	using RT = RemoveCVRef_t<decltype(evaluate(root(std::declval<PT>())))>;

	if ((*sv).size() == 0UL)
		return RT{};

	CT tmp(*sv);

	const auto end(tmp.end());
	auto element(tmp.begin());

	if (element == end)
		return RT{};

	PT norm(power(abs(element->value())));
	++element;

	for (; element != end; ++element) {
		norm += power(abs(element->value()));
	}

	return evaluate(root(norm));
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Computes the L2 norm for the given sparse vector.
// \ingroup sparse_vector
//
// \param sv The given sparse vector for the norm computation.
// \return The L2 norm of the given sparse vector.
//
// This function computes the L2 norm of the given sparse vector:

   \code
   mtrc::numeric::CompressedVector<double> a;
   // ... Resizing and initialization
   const double l2 = norm( a );
   \endcode
*/
template <typename VT // Type of the sparse vector
		  ,
		  bool TF> // Transpose flag
decltype(auto) norm(const SparseVector<VT, TF> &sv)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return norm_backend(*sv, SqrAbs(), Noop(), Sqrt());
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Computes the squared L2 norm for the given sparse vector.
// \ingroup sparse_vector
//
// \param sv The given sparse vector for the norm computation.
// \return The squared L2 norm of the given sparse vector.
//
// This function computes the squared L2 norm of the given sparse vector:

   \code
   mtrc::numeric::CompressedVector<double> a;
   // ... Resizing and initialization
   const double l2 = sqrNorm( a );
   \endcode
*/
template <typename VT // Type of the sparse vector
		  ,
		  bool TF> // Transpose flag
decltype(auto) sqrNorm(const SparseVector<VT, TF> &sv)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return norm_backend(*sv, SqrAbs(), Noop(), Noop());
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Computes the L1 norm for the given sparse vector.
// \ingroup sparse_vector
//
// \param sv The given sparse vector for the norm computation.
// \return The L1 norm of the given sparse vector.
//
// This function computes the L1 norm of the given sparse vector:

   \code
   mtrc::numeric::CompressedVector<double> a;
   // ... Resizing and initialization
   const double l1 = l1Norm( a );
   \endcode
*/
template <typename VT // Type of the sparse vector
		  ,
		  bool TF> // Transpose flag
decltype(auto) l1Norm(const SparseVector<VT, TF> &sv)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return norm_backend(*sv, Abs(), Noop(), Noop());
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Computes the L2 norm for the given sparse vector.
// \ingroup sparse_vector
//
// \param sv The given sparse vector for the norm computation.
// \return The L2 norm of the given sparse vector.
//
// This function computes the L2 norm of the given sparse vector:

   \code
   mtrc::numeric::CompressedVector<double> a;
   // ... Resizing and initialization
   const double l2 = l2Norm( a );
   \endcode
*/
template <typename VT // Type of the sparse vector
		  ,
		  bool TF> // Transpose flag
decltype(auto) l2Norm(const SparseVector<VT, TF> &sv)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return norm_backend(*sv, SqrAbs(), Noop(), Sqrt());
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Computes the L3 norm for the given sparse vector.
// \ingroup sparse_vector
//
// \param sv The given sparse vector for the norm computation.
// \return The L3 norm of the given sparse vector.
//
// This function computes the L3 norm of the given sparse vector:

   \code
   mtrc::numeric::CompressedVector<double> a;
   // ... Resizing and initialization
   const double l3 = l3Norm( a );
   \endcode
*/
template <typename VT // Type of the sparse vector
		  ,
		  bool TF> // Transpose flag
decltype(auto) l3Norm(const SparseVector<VT, TF> &sv)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return norm_backend(*sv, Abs(), Pow3(), Cbrt());
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Computes the L4 norm for the given sparse vector.
// \ingroup sparse_vector
//
// \param sv The given sparse vector for the norm computation.
// \return The L4 norm of the given sparse vector.
//
// This function computes the L4 norm of the given sparse vector:

   \code
   mtrc::numeric::CompressedVector<double> a;
   // ... Resizing and initialization
   const double l4 = l4Norm( a );
   \endcode
*/
template <typename VT // Type of the sparse vector
		  ,
		  bool TF> // Transpose flag
decltype(auto) l4Norm(const SparseVector<VT, TF> &sv)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return norm_backend(*sv, SqrAbs(), Pow2(), Qdrt());
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Computes the Lp norm for the given sparse vector.
// \ingroup sparse_vector
//
// \param sv The given sparse vector for the norm computation.
// \param p The norm parameter (p > 0).
// \return The Lp norm of the given sparse vector.
//
// This function computes the Lp norm of the given sparse vector, where the norm is specified by
// the runtime argument \a p:

   \code
   mtrc::numeric::CompressedVector<double> a;
   // ... Resizing and initialization
   const double lp = lpNorm( a, 2.3 );
   \endcode

// \note The norm parameter \a p is expected to be larger than 0. This precondition is only checked
// by a user assertion.
*/
template <typename VT // Type of the sparse vector
		  ,
		  bool TF // Transpose flag
		  ,
		  typename ST> // Type of the norm parameter
decltype(auto) lpNorm(const SparseVector<VT, TF> &sv, ST p)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	METRIC_NUMERIC_USER_ASSERT(!isZero(p), "Invalid p for Lp norm detected");

	using ScalarType = MultTrait_t<UnderlyingBuiltin_t<VT>, decltype(inv(p))>;
	using UnaryPow = Bind2nd<Pow, ScalarType>;
	return norm_backend(*sv, Abs(), UnaryPow(Pow(), p), UnaryPow(Pow(), inv(p)));
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Computes the Lp norm for the given sparse vector.
// \ingroup sparse_vector
//
// \param sv The given sparse vector for the norm computation.
// \return The Lp norm of the given sparse vector.
//
// This function computes the Lp norm of the given sparse vector, where the norm is specified by
// the runtime argument \a P:

   \code
   mtrc::numeric::CompressedVector<double> a;
   // ... Resizing and initialization
   const double lp = lpNorm<2>( a );
   \endcode

// \note The norm parameter \a P is expected to be larger than 0. A value of 0 results in a
// compile time error!.
*/
template <size_t P // Compile time norm parameter
		  ,
		  typename VT // Type of the sparse vector
		  ,
		  bool TF> // Transpose flag
inline decltype(auto) lpNorm(const SparseVector<VT, TF> &sv)
{
	METRIC_NUMERIC_STATIC_ASSERT_MSG(P > 0UL, "Invalid norm parameter detected");

	using Norms = TypeList<L1Norm, L2Norm, L3Norm, L4Norm, LpNorm<P>>;
	using Norm = typename TypeAt<Norms, min(P - 1UL, 4UL)>::Type;

	return Norm()(*sv);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Computes the infinity norm for the given sparse vector.
// \ingroup sparse_vector
//
// \param sv The given sparse vector for the norm computation.
// \return The infinity norm of the given sparse vector.
//
// This function computes the infinity norm of the given sparse vector:

   \code
   mtrc::numeric::CompressedVector<double> a;
   // ... Resizing and initialization
   const double linf = linfNorm( a );
   \endcode
*/
template <typename VT // Type of the sparse vector
		  ,
		  bool TF> // Transpose flag
decltype(auto) linfNorm(const SparseVector<VT, TF> &sv)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return max(abs(*sv));
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Computes the maximum norm for the given sparse vector.
// \ingroup sparse_vector
//
// \param sv The given sparse vector for the norm computation.
// \return The maximum norm of the given sparse vector.
//
// This function computes the maximum norm of the given sparse vector:

   \code
   mtrc::numeric::CompressedVector<double> a;
   // ... Resizing and initialization
   const double max = maxNorm( a );
   \endcode
*/
template <typename VT // Type of the sparse vector
		  ,
		  bool TF> // Transpose flag
decltype(auto) maxNorm(const SparseVector<VT, TF> &sv)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return linfNorm(*sv);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Computes the minimum norm for the given sparse vector.
// \ingroup sparse_vector
//
// \param sv The given sparse vector for the norm computation.
// \return The minimum norm of the given sparse vector.
//
// This function computes the minimum norm of the given sparse vector:

   \code
   mtrc::numeric::CompressedVector<double> a;
   // ... Resizing and initialization
   const double min = minNorm( a );
   \endcode
*/
template <typename VT // Type of the sparse vector
		  ,
		  bool TF> // Transpose flag
decltype(auto) minNorm(const SparseVector<VT, TF> &sv)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return min(abs(*sv));
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Calculation of the square length (magnitude) of the sparse vector \f$|\vec{a}|^2\f$.
// \ingroup sparse_vector
//
// \param sv The given sparse vector.
// \return The square length (magnitude) of the sparse vector.
//
// This function calculates the actual square length (magnitude) of the sparse vector. The
// function has the same effect as calling the \a sqrNorm() function on the sparse vector.
*/
template <typename VT // Type of the sparse vector
		  ,
		  bool TF> // Transpose flag
inline decltype(auto) sqrLength(const SparseVector<VT, TF> &sv)
{
	return sqrNorm(*sv);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Calculation of the length (magnitude) of the sparse vector \f$|\vec{a}|\f$.
// \ingroup sparse_vector
//
// \param sv The given sparse vector.
// \return The length (magnitude) of the sparse vector.
//
// This function calculates the actual length (magnitude) of the sparse vector. The function has
// the same effect as calling the \a norm() function on the sparse vector.
*/
template <typename VT // Type of the sparse vector
		  ,
		  bool TF> // Transpose flag
inline decltype(auto) length(const SparseVector<VT, TF> &sv)
{
	return norm(*sv);
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
