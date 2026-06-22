// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_SVECREDUCEEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_SVECREDUCEEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/Aliases.h>
#include <metric/numeric/math/expressions/SparseVector.h>
#include <metric/numeric/math/functors/Add.h>
#include <metric/numeric/math/functors/Max.h>
#include <metric/numeric/math/functors/Min.h>
#include <metric/numeric/math/functors/Mult.h>
#include <metric/numeric/util/Assert.h>
#include <metric/numeric/util/FunctionTrace.h>
#include <metric/numeric/util/Types.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  GLOBAL FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Performs a custom reduction operation on the given sparse vector.
// \ingroup sparse_vector
//
// \param sv The given sparse vector for the reduction computation.
// \param op The reduction operation.
// \return The result of the reduction operation.
//
// This function reduces the non-zero elements of the given sparse vector \a sv by means of the
// given reduction operation \a op:

   \code
   mtrc::numeric::CompressedVector<double> a;
   // ... Resizing and initialization

   const double totalsum1 = reduce( a, mtrc::numeric::Add() );
   const double totalsum2 = reduce( a, []( double a, double b ){ return a + b; } );
   \endcode

// As demonstrated in the example it is possible to pass any binary callable as custom reduction
// operation. See \ref custom_operations for a detailed overview of the possibilities of custom
// operations.
//
// Please note that the evaluation order of the reduction operation is unspecified. Thus the
// behavior is non-deterministic if \a op is not associative or not commutative. Also, the
// operation is undefined if the given reduction operation modifies the values.
*/
template <typename VT // Type of the sparse vector
		  ,
		  bool TF // Transpose flag
		  ,
		  typename OP> // Type of the reduction operation
inline decltype(auto) reduce(const SparseVector<VT, TF> &sv, OP op)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	using CT = CompositeType_t<VT>;
	using RT = RemoveCV_t<ElementType_t<VT>>;

	if ((*sv).size() == 0UL)
		return RT{};

	CT tmp(*sv);

	METRIC_NUMERIC_INTERNAL_ASSERT(tmp.size() == (*sv).size(), "Invalid vector size");

	const auto end(tmp.end());
	auto element(tmp.begin());

	if (element == end)
		return RT{};

	RT redux(element->value());
	++element;

	for (; element != end; ++element) {
		redux = op(redux, element->value());
	}

	return redux;
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Reduces the given sparse vector by means of addition.
// \ingroup sparse_vector
//
// \param sv The given sparse vector for the reduction operation.
// \return The result of the reduction operation.
//
// This function reduces the non-zero elements of the given sparse vector \a sv by means of
// addition:

   \code
   mtrc::numeric::CompressedVector<int> a{ 1, 2, 3, 4 };

   const int totalsum = sum( a );  // Results in 10
   \endcode

// Please note that the evaluation order of the reduction operation is unspecified.
*/
template <typename VT // Type of the sparse vector
		  ,
		  bool TF> // Transpose flag
inline decltype(auto) sum(const SparseVector<VT, TF> &sv)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return reduce(*sv, Add());
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Reduces the given sparse vector by means of multiplication.
// \ingroup sparse_vector
//
// \param sv The given sparse vector for the reduction operation.
// \return The result of the reduction operation.
//
// This function reduces the non-zero elements of the given sparse vector \a sv by means of
// multiplication:

   \code
   mtrc::numeric::CompressedVector<int> a{ 1, 2, 3, 4 };
   const int totalprod = prod( a );  // Results in 24
   \endcode

// Please note that the evaluation order of the reduction operation is unspecified.
*/
template <typename VT // Type of the sparse vector
		  ,
		  bool TF> // Transpose flag
inline decltype(auto) prod(const SparseVector<VT, TF> &sv)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return reduce(*sv, Mult());
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Returns the smallest element of the sparse vector.
// \ingroup sparse_vector
//
// \param sv The given sparse vector.
// \return The smallest sparse vector element.
//
// This function returns the smallest non-zero element of the given sparse vector. This function
// can only be used for element types that support the smaller-than relationship. In case the
// given vector currently has a size of 0, the returned value is the default value (e.g. 0 in
// case of fundamental data types).
//
// \note In case the sparse vector is not completely filled, the implicit zero elements are NOT
// taken into account. Example: the following compressed vector has only 2 non-zero elements.
// However, the minimum of this vector is 1:

   \code
   mtrc::numeric::CompressedVector<int> a{ 1, 0, 3, 0 };
   const int totalmin = min( a );  // Results in 1
   \endcode
*/
template <typename VT // Type of the sparse vector
		  ,
		  bool TF> // Transpose flag
inline decltype(auto) min(const SparseVector<VT, TF> &sv)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return reduce(*sv, Min());
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Returns the largest element of the sparse vector.
// \ingroup sparse_vector
//
// \param sv The given sparse vector.
// \return The largest sparse vector element.
//
// This function returns the largest non-zero element of the given sparse vector. This function
// can only be used for element types that support the smaller-than relationship. In case the
// given vector currently has a size of 0, the returned value is the default value (e.g. 0 in
// case of fundamental data types).
//
// \note In case the sparse vector is not completely filled, the implicit zero elements are NOT
// taken into account. Example: the following compressed vector has only 2 non-zero elements.
// However, the maximum of this vector is -1:

   \code
   mtrc::numeric::CompressedVector<int> a{ -1, 0, -3, 0 };
   const int totalmin = max( a );  // Results in -1
   \endcode
*/
template <typename VT // Type of the sparse vector
		  ,
		  bool TF> // Transpose flag
inline decltype(auto) max(const SparseVector<VT, TF> &sv)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return reduce(*sv, Max());
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Returns the index of the first smallest non-zero element of the sparse vector.
// \ingroup sparse_vector
//
// \param sv The given sparse vector.
// \return The index of the first smallest sparse vector element.
//
// This function returns the index of the first smallest non-zero element of the given sparse
// vector. This function can only be used for element types that support the smaller-than
// relationship. I case the given vector currently has a size of 0 or no non-zero elements,
// the returned index is 0.

   \code
   mtrc::numeric::CompressedVector<int> a{ 1, -2, 3, 0 };
   const size_t minindex = argmin( a );  // Results in 1
   \endcode
*/
template <typename VT // Type of the sparse vector
		  ,
		  bool TF> // Transpose flag
inline size_t argmin(const SparseVector<VT, TF> &sv)
{
	if ((*sv).size() < 2UL)
		return 0UL;

	CompositeType_t<VT> a(*sv); // Evaluation of the sparse vector operand

	if (a.nonZeros() == 0UL)
		return 0UL;

	const auto end(a.end());
	auto element(a.begin());
	size_t index(element->index());
	auto min(element->value());

	++element;

	for (; element != end; ++element) {
		auto cur(element->value());
		if (cur < min) {
			index = element->index();
			min = std::move(cur);
		}
	}

	return index;
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Returns the index of the first largest non-zero element of the sparse vector.
// \ingroup sparse_vector
//
// \param sv The given sparse vector.
// \return The index of the first largest sparse vector element.
//
// This function returns the index of the first largest non-zero element of the given sparse
// vector. This function can only be used for element types that support the largest-than
// relationship. I case the given vector currently has a size of 0 or no non-zero elements,
// the returned index is 0.

   \code
   mtrc::numeric::CompressedVector<int> a{ 1, -2, 3, 0 };
   const size_t maxindex = argmax( a );  // Results in 2
   \endcode
*/
template <typename VT // Type of the sparse vector
		  ,
		  bool TF> // Transpose flag
inline size_t argmax(const SparseVector<VT, TF> &sv)
{
	if ((*sv).size() < 2UL)
		return 0UL;

	CompositeType_t<VT> a(*sv); // Evaluation of the sparse vector operand

	if (a.nonZeros() == 0UL)
		return 0UL;

	const auto end(a.end());
	auto element(a.begin());
	size_t index(element->index());
	auto max(element->value());

	++element;

	for (; element != end; ++element) {
		auto cur(element->value());
		if (max < cur) {
			index = element->index();
			max = std::move(cur);
		}
	}

	return index;
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
