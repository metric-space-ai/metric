// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_SIMD_REDUCE_H
#define METRIC_NUMERIC_MATH_SIMD_REDUCE_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/functors/Forward.h>
#include <metric/numeric/math/simd/Prod.h>
#include <metric/numeric/math/simd/SIMDPack.h>
#include <metric/numeric/math/simd/Storea.h>
#include <metric/numeric/math/simd/Sum.h>
#include <metric/numeric/util/typetraits/AlignmentOf.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  SIMD REDUCTION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Reduces the elements in the given SIMD vector.
// \ingroup simd
//
// \param a The SIMD vector to be reduced.
// \param op The reduction operation.
// \return The result of the reduction operation.
*/
template <typename T, typename OP> inline decltype(auto) reduce(const SIMDPack<T> &a, OP op)
{
	using ValueType = typename T::ValueType;

	alignas(AlignmentOf_v<ValueType>) ValueType array[T::size];
	storea(array, *a);

	ValueType redux(array[0UL]);
	for (size_t k = 1UL; k < T::size; ++k) {
		redux = op(redux, array[k]);
	}

	return redux;
}
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Reduces the elements in the given SIMD vector by means of addition.
// \ingroup simd
//
// \param a The vector to be summed up.
// \return The sum of all vector elements.
*/
template <typename T> inline decltype(auto) reduce(const SIMDPack<T> &a, const Add & /*op*/) { return sum(*a); }
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Reduces the elements in the given SIMD vector by means of multiplication.
// \ingroup simd
//
// \param a The vector to be reduced by multiplication.
// \return The produdct of all vector elements.
*/
template <typename T> inline decltype(auto) reduce(const SIMDPack<T> &a, const Mult & /*op*/) { return prod(*a); }
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
