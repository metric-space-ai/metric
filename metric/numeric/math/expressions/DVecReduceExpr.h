// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_DVECREDUCEEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_DVECREDUCEEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/Aliases.h>
#include <metric/numeric/math/SIMD.h>
#include <metric/numeric/math/expressions/DenseVector.h>
#include <metric/numeric/math/functors/Add.h>
#include <metric/numeric/math/functors/Max.h>
#include <metric/numeric/math/functors/Min.h>
#include <metric/numeric/math/functors/Mult.h>
#include <metric/numeric/math/shims/PrevMultiple.h>
#include <metric/numeric/math/traits/ReduceTrait.h>
#include <metric/numeric/math/typetraits/HasLoad.h>
#include <metric/numeric/math/typetraits/IsPadded.h>
#include <metric/numeric/math/typetraits/IsSIMDEnabled.h>
#include <metric/numeric/math/typetraits/IsUniform.h>
#include <metric/numeric/util/Assert.h>
#include <metric/numeric/util/EnableIf.h>
#include <metric/numeric/util/FunctionTrace.h>
#include <metric/numeric/util/Types.h>
#include <metric/numeric/util/typetraits/HasMember.h>
#include <metric/numeric/util/typetraits/RemoveCV.h>
#include <metric/numeric/util/typetraits/RemoveReference.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Auxiliary helper struct for the dense vector reduction operation.
// \ingroup dense_vector
*/
template <typename VT // Type of the dense vector
		  ,
		  typename OP> // Type of the reduction operation
struct DVecReduceExprHelper {
	//**Type definitions****************************************************************************
	//! Composite type of the dense vector expression.
	using CT = RemoveReference_t<CompositeType_t<VT>>;

	//! Element type of the dense vector expression.
	using ET = RemoveCV_t<ElementType_t<CT>>;
	//**********************************************************************************************

	//**********************************************************************************************
	static constexpr bool value =
		(CT::simdEnabled && If_t<HasSIMDEnabled_v<OP>, GetSIMDEnabled<OP, ET, ET>, HasLoad<OP>>::value);
	//**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  GLOBAL FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Default backend implementation of the reduction of a dense vector.
// \ingroup dense_vector
//
// \param dv The given dense vector for the reduction computation.
// \param op The reduction operation.
// \return The result of the reduction operation.
//
// This function implements the performance optimized reduction operation for a dense vector. Due
// to the explicit application of the SFINAE principle, this function can only be selected by the
// compiler in case vectorization cannot be applied.
*/
template <typename VT // Type of the dense vector
		  ,
		  bool TF // Transpose flag
		  ,
		  typename OP> // Type of the reduction operation
inline auto dvecreduce(const DenseVector<VT, TF> &dv, OP op)
	-> DisableIf_t<DVecReduceExprHelper<VT, OP>::value, RemoveCV_t<ReduceTrait_t<VT, OP>>>
{
	using CT = CompositeType_t<VT>;
	using RT = RemoveCV_t<ReduceTrait_t<VT, OP>>;

	const size_t N((*dv).size());

	if (N == 0UL)
		return RT{};
	if (N == 1UL)
		return (*dv)[0UL];

	CT tmp(*dv);

	METRIC_NUMERIC_INTERNAL_ASSERT(tmp.size() == N, "Invalid vector size");

	RT redux1(tmp[0UL]);
	RT redux2(tmp[1UL]);
	size_t i(2UL);

	for (; (i + 4UL) <= N; i += 4UL) {
		redux1 = op(op(redux1, tmp[i]), tmp[i + 1UL]);
		redux2 = op(op(redux2, tmp[i + 2UL]), tmp[i + 3UL]);
	}
	for (; (i + 2UL) <= N; i += 2UL) {
		redux1 = op(redux1, tmp[i]);
		redux2 = op(redux2, tmp[i + 1UL]);
	}
	for (; i < N; ++i) {
		redux1 = op(redux1, tmp[i]);
	}

	return op(redux1, redux2);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief SIMD optimized backend implementation of the reduction of a dense vector.
// \ingroup dense_vector
//
// \param dv The given dense vector for the reduction computation.
// \param op The reduction operation.
// \return The result of the reduction operation.
//
// This function implements the performance optimized reduction operation for a dense vector. Due
// to the explicit application of the SFINAE principle, this function can only be selected by the
// compiler in case vectorization can be applied.
*/
template <typename VT // Type of the dense vector
		  ,
		  bool TF // Transpose flag
		  ,
		  typename OP> // Type of the reduction operation
inline auto dvecreduce(const DenseVector<VT, TF> &dv, OP op)
	-> EnableIf_t<DVecReduceExprHelper<VT, OP>::value, RemoveCV_t<ElementType_t<VT>>>
{
	using CT = CompositeType_t<VT>;
	using ET = RemoveCV_t<ElementType_t<VT>>;

	const size_t N((*dv).size());

	if (N == 0UL)
		return ET{};

	CT tmp(*dv);

	METRIC_NUMERIC_INTERNAL_ASSERT(tmp.size() == N, "Invalid vector size");

	constexpr size_t SIMDSIZE = SIMDTrait<ET>::size;

	ET redux{};

	if (N >= SIMDSIZE) {
		const size_t ipos(prevMultiple(N, SIMDSIZE));
		METRIC_NUMERIC_INTERNAL_ASSERT(ipos <= N, "Invalid end calculation");

		SIMDTrait_t<ET> xmm1(tmp.load(0UL));

		if (N >= SIMDSIZE * 2UL) {
			SIMDTrait_t<ET> xmm2(tmp.load(SIMDSIZE));
			size_t i(SIMDSIZE * 2UL);

			for (; (i + SIMDSIZE) < ipos; i += SIMDSIZE * 2UL) {
				xmm1 = op(xmm1, tmp.load(i));
				xmm2 = op(xmm2, tmp.load(i + SIMDSIZE));
			}
			for (; i < ipos; i += SIMDSIZE) {
				xmm1 = op(xmm1, tmp.load(i));
			}

			xmm1 = op(xmm1, xmm2);
		}

		redux = reduce(xmm1, op);

		for (size_t i = ipos; i < N; ++i) {
			redux = op(redux, tmp[i]);
		}
	} else {
		redux = tmp[0UL];
		for (size_t i = 1UL; i < N; ++i) {
			redux = op(redux, tmp[i]);
		}
	}

	return redux;
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief SIMD optimized backend implementation of the summation of a dense vector.
// \ingroup dense_vector
//
// \param dv The given dense vector for the summation.
// \return The result of the summation.
//
// This function implements the performance optimized summation for a dense vector. Due to
// the explicit application of the SFINAE principle, this function can only be selected by
// the compiler in case vectorization can be applied.
*/
template <typename VT // Type of the dense vector
		  ,
		  bool TF> // Transpose flag
inline auto dvecreduce(const DenseVector<VT, TF> &dv, Add /*op*/)
	-> EnableIf_t<DVecReduceExprHelper<VT, Add>::value, RemoveCV_t<ElementType_t<VT>>>
{
	using CT = CompositeType_t<VT>;
	using ET = RemoveCV_t<ElementType_t<VT>>;

	const size_t N((*dv).size());

	if (N == 0UL)
		return ET{};

	CT tmp(*dv);

	METRIC_NUMERIC_INTERNAL_ASSERT(tmp.size() == N, "Invalid vector size");

	constexpr bool remainder(!IsPadded_v<RemoveReference_t<CT>>);
	constexpr size_t SIMDSIZE = SIMDTrait<ET>::size;

	const size_t ipos(remainder ? prevMultiple(N, SIMDSIZE) : N);
	METRIC_NUMERIC_INTERNAL_ASSERT(ipos <= N, "Invalid end calculation");

	size_t i(0UL);
	ET redux{};

	if (SIMDSIZE * 3UL < ipos) {
		SIMDTrait_t<ET> xmm1{}, xmm2{}, xmm3{}, xmm4{};

		for (; (i + SIMDSIZE * 3UL) < ipos; i += SIMDSIZE * 4UL) {
			xmm1 += tmp.load(i);
			xmm2 += tmp.load(i + SIMDSIZE);
			xmm3 += tmp.load(i + SIMDSIZE * 2UL);
			xmm4 += tmp.load(i + SIMDSIZE * 3UL);
		}
		for (; (i + SIMDSIZE) < ipos; i += SIMDSIZE * 2UL) {
			xmm1 += tmp.load(i);
			xmm2 += tmp.load(i + SIMDSIZE);
		}
		for (; i < ipos; i += SIMDSIZE) {
			xmm1 += tmp.load(i);
		}

		redux = sum(xmm1 + xmm2 + xmm3 + xmm4);
	} else if (SIMDSIZE < ipos) {
		SIMDTrait_t<ET> xmm1{}, xmm2{};

		for (; (i + SIMDSIZE) < ipos; i += SIMDSIZE * 2UL) {
			xmm1 += tmp.load(i);
			xmm2 += tmp.load(i + SIMDSIZE);
		}
		for (; i < ipos; i += SIMDSIZE) {
			xmm1 += tmp.load(i);
		}

		redux = sum(xmm1 + xmm2);
	} else {
		SIMDTrait_t<ET> xmm1{};

		for (; i < ipos; i += SIMDSIZE) {
			xmm1 += tmp.load(i);
		}

		redux = sum(xmm1);
	}

	for (; remainder && i < N; ++i) {
		redux += tmp[i];
	}

	return redux;
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Optimized backend implementation of the minimum evaluation of a uniform dense vector.
// \ingroup dense_vector
//
// \param dv The given dense vector.
// \return The smallest dense vector element.
//
// This function implements the performance optimized minimum evaluation for a given uniform
// dense vector.
*/
template <typename VT // Type of the dense vector
		  ,
		  bool TF> // Transpose flag
inline auto dvecreduce(const DenseVector<VT, TF> &dv, Min /*op*/)
	-> EnableIf_t<IsUniform_v<VT>, RemoveCV_t<ElementType_t<VT>>>
{
	return (*dv)[0UL];
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Optimized backend implementation of the maximum evaluation of a uniform dense vector.
// \ingroup dense_vector
//
// \param dv The given dense vector.
// \return The smallest dense vector element.
//
// This function implements the performance optimized maximum evaluation for a given uniform
// dense vector.
*/
template <typename VT // Type of the dense vector
		  ,
		  bool TF> // Transpose flag
inline auto dvecreduce(const DenseVector<VT, TF> &dv, Max /*op*/)
	-> EnableIf_t<IsUniform_v<VT>, RemoveCV_t<ElementType_t<VT>>>
{
	return (*dv)[0UL];
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Performs a custom reduction operation on the given dense vector.
// \ingroup dense_vector
//
// \param dv The given dense vector for the reduction computation.
// \param op The reduction operation.
// \return The result of the reduction operation.
//
// This function reduces the given dense vector \a dv by means of the given reduction operation
// \a op:

   \code
   mtrc::numeric::DynamicVector<double> a;
   // ... Resizing and initialization

   const double totalsum1 = reduce( a, mtrc::numeric::Add() );
   const double totalsum2 = reduce( a, []( double a, double b ){ return a + b; } );
   \endcode

// As demonstrated in the example it is possible to pass any binary callable as custom reduction
// operation. However, for instance in the case of lambdas the vectorization of the reduction
// operation is compiler dependent and might not perform at peak performance. However, it is also
// possible to create vectorized custom operations. See \ref custom_operations for a detailed
// overview of the possibilities of custom operations.
//
// Please note that the evaluation order of the reduction operation is unspecified. Thus the
// behavior is non-deterministic if \a op is not associative or not commutative. Also, the
// operation is undefined if the given reduction operation modifies the values.
*/
template <typename VT // Type of the dense vector
		  ,
		  bool TF // Transpose flag
		  ,
		  typename OP> // Type of the reduction operation
inline decltype(auto) reduce(const DenseVector<VT, TF> &dv, OP op)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return dvecreduce(*dv, std::move(op));
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Reduces the given dense vector by means of addition.
// \ingroup dense_vector
//
// \param dv The given dense vector for the reduction operation.
// \return The result of the reduction operation.
//
// This function reduces the given dense vector \a dv by means of addition:

   \code
   mtrc::numeric::DynamicVector<int> a{ 1, 2, 3, 4 };

   const int totalsum = sum( a );  // Results in 10
   \endcode

// Please note that the evaluation order of the reduction operation is unspecified.
*/
template <typename VT // Type of the dense vector
		  ,
		  bool TF> // Transpose flag
inline decltype(auto) sum(const DenseVector<VT, TF> &dv)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return reduce(*dv, Add());
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Reduces the given dense vector by means of multiplication.
// \ingroup dense_vector
//
// \param dv The given dense vector for the reduction operation.
// \return The result of the reduction operation.
//
// This function reduces the given dense vector \a dv by means of multiplication:

   \code
   mtrc::numeric::DynamicVector<int> a{ 1, 2, 3, 4 };
   const int totalprod = prod( a );  // Results in 24
   \endcode

// Please note that the evaluation order of the reduction operation is unspecified.
*/
template <typename VT // Type of the dense vector
		  ,
		  bool TF> // Transpose flag
inline decltype(auto) prod(const DenseVector<VT, TF> &dv)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return reduce(*dv, Mult());
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Returns the smallest element of the dense vector.
// \ingroup dense_vector
//
// \param dv The given dense vector.
// \return The smallest dense vector element.
//
// This function returns the smallest element of the given dense vector. This function can
// only be used for element types that support the smaller-than relationship. In case the given
// vector currently has a size of 0, the returned value is the default value (e.g. 0 in case
// of fundamental data types).

   \code
   mtrc::numeric::DynamicVector<int> a{ 1, -2, 3, 0 };
   const int totalmin = min( a );  // Results in -2
   \endcode
*/
template <typename VT // Type of the dense vector
		  ,
		  bool TF> // Transpose flag
inline decltype(auto) min(const DenseVector<VT, TF> &dv)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return reduce(*dv, Min());
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Returns the largest element of the dense vector.
// \ingroup dense_vector
//
// \param dv The given dense vector.
// \return The largest dense vector element.
//
// This function returns the largest element of the given dense vector. This function can
// only be used for element types that support the smaller-than relationship. In case the given
// vector currently has a size of 0, the returned value is the default value (e.g. 0 in case
// of fundamental data types).

   \code
   mtrc::numeric::DynamicVector<int> a{ 1, -2, 3, 0 };
   const int totalmax = max( a );  // Results in 3
   \endcode
*/
template <typename VT // Type of the dense vector
		  ,
		  bool TF> // Transpose flag
inline decltype(auto) max(const DenseVector<VT, TF> &dv)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return reduce(*dv, Max());
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Returns the index of the first smallest element of the dense vector.
// \ingroup dense_vector
//
// \param dv The given dense vector.
// \return The index of the first smallest dense vector element.
//
// This function returns the index of the first smallest element of the given dense vector. This
// function can only be used for element types that support the smaller-than relationship. In
// case the given vector currently has a size of 0, the returned index is 0.

   \code
   mtrc::numeric::DynamicVector<int> a{ 1, -2, 3, 0 };
   const size_t minindex = argmin( a );  // Results in 1
   \endcode
*/
template <typename VT // Type of the dense vector
		  ,
		  bool TF> // Transpose flag
inline size_t argmin(const DenseVector<VT, TF> &dv)
{
	if ((*dv).size() < 2UL)
		return 0UL;

	CompositeType_t<VT> a(*dv); // Evaluation of the dense vector operand

	const size_t size(a.size());
	size_t index(0UL);
	auto min(a[0UL]);

	for (size_t i = 1UL; i < size; ++i) {
		auto cur(a[i]);
		if (cur < min) {
			index = i;
			min = std::move(cur);
		}
	}

	return index;
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Returns the index of the first largest element of the dense vector.
// \ingroup dense_vector
//
// \param dv The given dense vector.
// \return The index of the first largest dense vector element.
//
// This function returns the index of the first largest element of the given dense vector. This
// function can only be used for element types that support the smaller-than relationship. In
// case the given vector currently has a size of 0, the returned index is 0.

   \code
   mtrc::numeric::DynamicVector<int> a{ 1, -2, 3, 0 };
   const size_t maxindex = argmax( a );  // Results in 2
   \endcode
*/
template <typename VT // Type of the dense vector
		  ,
		  bool TF> // Transpose flag
inline size_t argmax(const DenseVector<VT, TF> &dv)
{
	if ((*dv).size() < 2UL)
		return 0UL;

	CompositeType_t<VT> a(*dv); // Evaluation of the dense vector operand

	const size_t size(a.size());
	size_t index(0UL);
	auto max(a[0UL]);

	for (size_t i = 1UL; i < size; ++i) {
		auto cur(a[i]);
		if (max < cur) {
			index = i;
			max = std::move(cur);
		}
	}

	return index;
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
