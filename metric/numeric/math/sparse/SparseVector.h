// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_SPARSE_SPARSEVECTOR_H
#define METRIC_NUMERIC_MATH_SPARSE_SPARSEVECTOR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/Aliases.h>
#include <metric/numeric/math/Exception.h>
#include <metric/numeric/math/RelaxationFlag.h>
#include <metric/numeric/math/expressions/SparseVector.h>
#include <metric/numeric/math/shims/Equal.h>
#include <metric/numeric/math/shims/IsDefault.h>
#include <metric/numeric/math/shims/IsFinite.h>
#include <metric/numeric/math/shims/IsInf.h>
#include <metric/numeric/math/shims/IsNaN.h>
#include <metric/numeric/math/shims/IsZero.h>
#include <metric/numeric/math/shims/Pow2.h>
#include <metric/numeric/math/shims/Sqrt.h>
#include <metric/numeric/math/traits/DivTrait.h>
#include <metric/numeric/math/typetraits/IsInvertible.h>
#include <metric/numeric/math/typetraits/IsResizable.h>
#include <metric/numeric/math/typetraits/IsRestricted.h>
#include <metric/numeric/math/typetraits/IsScalar.h>
#include <metric/numeric/math/typetraits/IsUniform.h>
#include <metric/numeric/math/typetraits/IsZero.h>
#include <metric/numeric/math/typetraits/UnderlyingBuiltin.h>
#include <metric/numeric/math/typetraits/UnderlyingScalar.h>
#include <metric/numeric/util/Assert.h>
#include <metric/numeric/util/EnableIf.h>
#include <metric/numeric/util/Types.h>
#include <metric/numeric/util/constraints/Numeric.h>
#include <metric/numeric/util/mpl/If.h>
#include <metric/numeric/util/typetraits/IsBuiltin.h>
#include <metric/numeric/util/typetraits/IsComplex.h>
#include <metric/numeric/util/typetraits/IsFloatingPoint.h>
#include <metric/numeric/util/typetraits/RemoveReference.h>
#include <utility>

namespace mtrc::numeric {

//=================================================================================================
//
//  GLOBAL OPERATORS
//
//=================================================================================================

//*************************************************************************************************
/*!\name SparseVector operators */
//@{
template <typename VT, bool TF, typename ST>
auto operator*=(SparseVector<VT, TF> &vec, ST scalar) -> EnableIf_t<IsScalar_v<ST>, VT &>;

template <typename VT, bool TF, typename ST>
auto operator*=(SparseVector<VT, TF> &&vec, ST scalar) -> EnableIf_t<IsScalar_v<ST>, VT &>;

template <typename VT, bool TF, typename ST>
auto operator/=(SparseVector<VT, TF> &vec, ST scalar) -> EnableIf_t<IsScalar_v<ST>, VT &>;

template <typename VT, bool TF, typename ST>
auto operator/=(SparseVector<VT, TF> &&vec, ST scalar) -> EnableIf_t<IsScalar_v<ST>, VT &>;
//@}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Multiplication assignment operator for the multiplication of a sparse vector and
//        a scalar value (\f$ \vec{a}*=s \f$).
// \ingroup sparse_vector
//
// \param vec The left-hand side sparse vector for the multiplication.
// \param scalar The right-hand side scalar value for the multiplication.
// \return Reference to the left-hand side sparse vector.
// \exception std::invalid_argument Invalid scaling of restricted vector.
//
// In case the vector \a VT is restricted and the assignment would violate an invariant of the
// vector, a \a std::invalid_argument exception is thrown.
*/
template <typename VT // Type of the left-hand side sparse vector
		  ,
		  bool TF // Transpose flag
		  ,
		  typename ST> // Data type of the right-hand side scalar
inline auto operator*=(SparseVector<VT, TF> &vec, ST scalar) -> EnableIf_t<IsScalar_v<ST>, VT &>
{
	if (IsRestricted_v<VT>) {
		if (!tryMult(*vec, 0UL, (*vec).size(), scalar)) {
			METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid scaling of restricted vector");
		}
	}

	if (!IsResizable_v<ElementType_t<VT>> && isZero(scalar)) {
		reset(*vec);
	} else {
		decltype(auto) left(derestrict(*vec));

		const auto last(left.end());
		for (auto element = left.begin(); element != last; ++element) {
			element->value() *= scalar;
		}
	}

	METRIC_NUMERIC_INTERNAL_ASSERT(isIntact(*vec), "Invariant violation detected");

	return *vec;
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Multiplication assignment operator for the multiplication of a temporary sparse vector
//        and a scalar (\f$ v*=s \f$).
// \ingroup sparse_vector
//
// \param vec The left-hand side temporary sparse vector for the multiplication.
// \param scalar The right-hand side scalar value for the multiplication.
// \return Reference to the left-hand side sparse vector.
// \exception std::invalid_argument Invalid scaling of restricted vector.
//
// In case the vector \a VT is restricted and the assignment would violate an invariant of the
// vector, a \a std::invalid_argument exception is thrown.
*/
template <typename VT // Type of the left-hand side sparse vector
		  ,
		  bool TF // Transpose flag
		  ,
		  typename ST> // Data type of the right-hand side scalar
inline auto operator*=(SparseVector<VT, TF> &&vec, ST scalar) -> EnableIf_t<IsScalar_v<ST>, VT &>
{
	return operator*=(*vec, scalar);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Division assignment operator for the division of a sparse vector by a scalar value
//        (\f$ \vec{a}/=s \f$).
// \ingroup sparse_vector
//
// \param vec The left-hand side sparse vector for the division.
// \param scalar The right-hand side scalar value for the division.
// \return Reference to the left-hand side sparse vector.
// \exception std::invalid_argument Invalid scaling of restricted vector.
//
// In case the vector \a VT is restricted and the assignment would violate an invariant of the
// vector, a \a std::invalid_argument exception is thrown.
//
// \note A division by zero is only checked by an user assert.
*/
template <typename VT // Type of the left-hand side sparse vector
		  ,
		  bool TF // Transpose flag
		  ,
		  typename ST> // Data type of the right-hand side scalar
inline auto operator/=(SparseVector<VT, TF> &vec, ST scalar) -> EnableIf_t<IsScalar_v<ST>, VT &>
{
	METRIC_NUMERIC_USER_ASSERT(!isZero(scalar), "Division by zero detected");

	if (IsRestricted_v<VT>) {
		if (!tryDiv(*vec, 0UL, (*vec).size(), scalar)) {
			METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid scaling of restricted vector");
		}
	}

	using ScalarType = If_t<IsFloatingPoint_v<UnderlyingBuiltin_t<VT>> || IsFloatingPoint_v<UnderlyingBuiltin_t<ST>>,
							If_t<IsComplex_v<UnderlyingScalar_t<VT>> && IsBuiltin_v<ST>,
								 DivTrait_t<UnderlyingBuiltin_t<VT>, ST>, DivTrait_t<UnderlyingScalar_t<VT>, ST>>,
							ST>;

	decltype(auto) left(derestrict(*vec));

	if (IsInvertible_v<ScalarType>) {
		const ScalarType tmp(ScalarType(1) / static_cast<ScalarType>(scalar));
		for (auto element = left.begin(); element != left.end(); ++element) {
			element->value() *= tmp;
		}
	} else {
		for (auto element = left.begin(); element != left.end(); ++element) {
			element->value() /= scalar;
		}
	}

	METRIC_NUMERIC_INTERNAL_ASSERT(isIntact(*vec), "Invariant violation detected");

	return *vec;
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Division assignment operator for the division of a temporary sparse vector by a scalar
//        value (\f$ \vec{a}/=s \f$).
// \ingroup sparse_vector
//
// \param vec The left-hand side temporary sparse vector for the division.
// \param scalar The right-hand side scalar value for the division.
// \return Reference to the left-hand side sparse vector.
// \exception std::invalid_argument Invalid scaling of restricted vector.
//
// In case the vector \a VT is restricted and the assignment would violate an invariant of the
// vector, a \a std::invalid_argument exception is thrown.
//
// \note A division by zero is only checked by an user assert.
*/
template <typename VT // Type of the left-hand side sparse vector
		  ,
		  bool TF // Transpose flag
		  ,
		  typename ST> // Data type of the right-hand side scalar
inline auto operator/=(SparseVector<VT, TF> &&vec, ST scalar) -> EnableIf_t<IsScalar_v<ST>, VT &>
{
	return operator/=(*vec, scalar);
}
//*************************************************************************************************

//=================================================================================================
//
//  GLOBAL FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\name SparseVector functions */
//@{
template <typename VT, bool TF> bool isnan(const SparseVector<VT, TF> &sv);

template <typename VT, bool TF> bool isinf(const SparseVector<VT, TF> &sv);

template <typename VT, bool TF> bool isfinite(const SparseVector<VT, TF> &sv);

template <RelaxationFlag RF, typename VT, bool TF> bool isUniform(const SparseVector<VT, TF> &sv);

template <RelaxationFlag RF, typename VT, bool TF> bool isZero(const SparseVector<VT, TF> &sv);
//@}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Checks the given sparse vector for not-a-number elements.
// \ingroup sparse_vector
//
// \param sv The sparse vector to be checked for not-a-number elements.
// \return \a true if at least one element of the vector is not-a-number, \a false otherwise.
//
// This function checks the N-dimensional sparse vector for not-a-number (NaN) elements. If
// at least one element of the vector is not-a-number, the function returns \a true, otherwise
// it returns \a false.

   \code
   mtrc::numeric::CompressedVector<double> a;
   // ... Resizing and initialization
   if( isnan( a ) ) { ... }
   \endcode
*/
template <typename VT // Type of the sparse vector
		  ,
		  bool TF> // Transpose flag
inline bool isnan(const SparseVector<VT, TF> &sv)
{
	if (!IsFloatingPoint_v<UnderlyingBuiltin_t<VT>>)
		return false;

	using CT = CompositeType_t<VT>;

	CT a(*sv); // Evaluation of the sparse vector operand

	const auto end(a.end());
	for (auto element = a.begin(); element != end; ++element) {
		if (isnan(element->value()))
			return true;
	}
	return false;
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Checks the given sparse vector for infinite elements.
// \ingroup sparse_vector
//
// \param sv The sparse vector to be checked for infinite elements.
// \return \a true if at least one element of the vector is infinite, \a false otherwise.
//
// This function checks the N-dimensional sparse vector for infinite (inf) elements. If at
// least one element of the vector is infinite, the function returns \a true, otherwise
// it returns \a false.

   \code
   mtrc::numeric::CompressedVector<double> a;
   // ... Resizing and initialization
   if( isinf( a ) ) { ... }
   \endcode
*/
template <typename VT // Type of the sparse vector
		  ,
		  bool TF> // Transpose flag
inline bool isinf(const SparseVector<VT, TF> &sv)
{
	if (!IsFloatingPoint_v<UnderlyingBuiltin_t<VT>>)
		return false;

	using CT = CompositeType_t<VT>;

	CT a(*sv); // Evaluation of the sparse vector operand

	const auto end(a.end());
	for (auto element = a.begin(); element != end; ++element) {
		if (isinf(element->value()))
			return true;
	}
	return false;
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Checks the given sparse vector for finite elements.
// \ingroup sparse_vector
//
// \param sv The sparse vector to be checked for finite elements.
// \return \a true if all elements of the vector are finite, \a false otherwise.
//
// This function checks if all elements of the N-dimensional sparse vector are finite elements
// (i.e. normal, subnormal or zero elements, but not infinite or NaN). If all elements of the
// vector are finite, the function returns \a true, otherwise it returns \a false.

   \code
   mtrc::numeric::CompressedVector<double> a;
   // ... Resizing and initialization
   if( isfinite( a ) ) { ... }
   \endcode
*/
template <typename VT // Type of the sparse vector
		  ,
		  bool TF> // Transpose flag
inline bool isfinite(const SparseVector<VT, TF> &sv)
{
	if (!IsFloatingPoint_v<UnderlyingBuiltin_t<VT>>)
		return true;

	using CT = CompositeType_t<VT>;

	CT a(*sv); // Evaluation of the sparse vector operand

	const auto end(a.end());
	for (auto element = a.begin(); element != end; ++element) {
		if (!isfinite(element->value()))
			return false;
	}
	return true;
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Checks if the given sparse vector is a uniform vector.
// \ingroup sparse_vector
//
// \param sv The sparse vector to be checked.
// \return \a true if the vector is a uniform vector, \a false if not.
//
// This function checks if the given sparse vector is a uniform vector. The vector is considered
// to be uniform if all its elements are identical. The following code example demonstrates the
// use of the function:

   \code
   mtrc::numeric::CompressedVector<int,mtrc::numeric::columnVector> a, b;
   // ... Initialization
   if( isUniform( a ) ) { ... }
   \endcode

// Optionally, it is possible to switch between strict semantics (mtrc::numeric::strict) and relaxed
// semantics (mtrc::numeric::relaxed):

   \code
   if( isUniform<relaxed>( a ) ) { ... }
   \endcode

// It is also possible to check if a vector expression results is a uniform vector:

   \code
   if( isUniform( a + b ) ) { ... }
   \endcode

// However, note that this might require the complete evaluation of the expression, including
// the generation of a temporary vector.
*/
template <RelaxationFlag RF // Relaxation flag
		  ,
		  typename VT // Type of the sparse vector
		  ,
		  bool TF> // Transpose flag
bool isUniform(const SparseVector<VT, TF> &sv)
{
	using CT = CompositeType_t<VT>;

	if (IsUniform_v<VT> || (*sv).size() < 2UL)
		return true;

	CT a(*sv); // Evaluation of the sparse vector operand

	if (a.nonZeros() != a.size()) {
		const auto end(a.end());
		for (auto element = a.begin(); element != end; ++element) {
			if (!isDefault<RF>(element->value()))
				return false;
		}
	} else {
		const auto &cmp(a[0]);
		auto element(a.begin());
		const auto end(a.end());

		++element;

		for (; element != end; ++element) {
			if (!equal<RF>(element->value(), cmp))
				return false;
		}
	}

	return true;
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Checks if the given sparse vector is a zero vector.
// \ingroup sparse_vector
//
// \param sv The sparse vector to be checked.
// \return \a true if the vector is a zero vector, \a false if not.
//
// This function checks if the given sparse vector is a zero vector. The vector is considered
// to be zero if all its elements are zero. The following code example demonstrates the use
// of the function:

   \code
   mtrc::numeric::CompressedVector<int,mtrc::numeric::columnVector> a, b;
   // ... Initialization
   if( isZero( a ) ) { ... }
   \endcode

// Optionally, it is possible to switch between strict semantics (mtrc::numeric::strict) and relaxed
// semantics (mtrc::numeric::relaxed):

   \code
   if( isZero<relaxed>( a ) ) { ... }
   \endcode

// It is also possible to check if a vector expression results is a zero vector:

   \code
   if( isZero( a + b ) ) { ... }
   \endcode

// However, note that this might require the complete evaluation of the expression, including
// the generation of a temporary vector.
*/
template <RelaxationFlag RF // Relaxation flag
		  ,
		  typename VT // Type of the sparse vector
		  ,
		  bool TF> // Transpose flag
bool isZero(const SparseVector<VT, TF> &sv)
{
	if (IsZero_v<VT> || (*sv).nonZeros() == 0UL)
		return true;

	CompositeType_t<VT> a(*sv); // Evaluation of the sparse vector operand

	const auto end(a.end());
	for (auto element = a.begin(); element != end; ++element) {
		if (!isZero<RF>(element->value()))
			return false;
	}

	return true;
}
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Erasing a single element, a range or selection of elements from the given sparse vector.
// \ingroup sparse_vector
//
// \param sv The given sparse vector.
// \param args The runtime arguments for the erase call.
// \return The result of the according erase member function.
//
// This function represents an abstract interface for erasing a single element, a range of
// elements or a selection of elements from the given sparse vector. It forwards the given
// arguments to the according \a erase() member function of the sparse vector and returns
// the result of the function call.
*/
template <typename VT // Type of the sparse vector
		  ,
		  bool TF // Transpose flag
		  ,
		  typename... Args> // Type of the erase arguments
auto erase(SparseVector<VT, TF> &sv, Args &&...args) -> decltype((*sv).erase(std::forward<Args>(args)...))
{
	return (*sv).erase(std::forward<Args>(args)...);
}
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
