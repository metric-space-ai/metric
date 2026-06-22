// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_SVECDVECINNEREXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_SVECDVECINNEREXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/Aliases.h>
#include <metric/numeric/math/Exception.h>
#include <metric/numeric/math/constraints/ColumnVector.h>
#include <metric/numeric/math/constraints/DenseVector.h>
#include <metric/numeric/math/constraints/RowVector.h>
#include <metric/numeric/math/constraints/SparseVector.h>
#include <metric/numeric/math/expressions/DenseVector.h>
#include <metric/numeric/math/expressions/SparseVector.h>
#include <metric/numeric/math/traits/MultTrait.h>
#include <metric/numeric/system/MacroDisable.h>
#include <metric/numeric/util/FunctionTrace.h>
#include <metric/numeric/util/Types.h>
#include <metric/numeric/util/typetraits/RemoveReference.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  GLOBAL BINARY ARITHMETIC OPERATORS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Multiplication operator for the scalar product (inner product) of a sparse and a
//        dense vector (\f$ s=\vec{a}*\vec{b} \f$).
// \ingroup sparse_vector
//
// \param lhs The left-hand side sparse vector for the inner product.
// \param rhs The right-hand side dense vector for the inner product.
// \return The scalar product.
// \exception std::invalid_argument Vector sizes do not match.
//
// This operator represents the scalar product (inner product) of a sparse vector and a dense
// vector:

   \code
   using mtrc::numeric::columnVector;
   using mtrc::numeric::rowVector;

   mtrc::numeric::CompressedVector<double,rowVector> a;
   mtrc::numeric::DynamicVector<double,columnVector> b;
   mtrc::numeric::real res;
   // ... Resizing and initialization
   res = a * b;
   \endcode

// The operator returns a scalar value of the higher-order element type of the two involved
// vector element types \a VT1::ElementType and \a VT2::ElementType. Both vector types \a VT1
// and \a VT2 as well as the two element types \a VT1::ElementType and \a VT2::ElementType
// have to be supported by the MultTrait class template.\n
// In case the current sizes of the two given vectors don't match, a \a std::invalid_argument
// is thrown.
*/
template <typename VT1 // Type of the left-hand side sparse vector
		  ,
		  typename VT2> // Type of the right-hand side dense vector
inline decltype(auto) operator*(const SparseVector<VT1, true> &lhs, const DenseVector<VT2, false> &rhs)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	using CT1 = CompositeType_t<VT1>;		// Composite type of the left-hand side sparse vector expression
	using CT2 = CompositeType_t<VT2>;		// Composite type of the right-hand side dense vector expression
	using XT1 = RemoveReference_t<CT1>;		// Auxiliary type for the left-hand side composite type
	using XT2 = RemoveReference_t<CT2>;		// Auxiliary type for the right-hand side composite type
	using ET1 = ElementType_t<XT1>;			// Element type of the left-hand side sparse vector expression
	using ET2 = ElementType_t<XT2>;			// Element type of the right-hand side dense vector expression
	using MultType = MultTrait_t<ET1, ET2>; // Multiplication result type

	METRIC_NUMERIC_CONSTRAINT_MUST_BE_SPARSE_VECTOR_TYPE(VT1);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_DENSE_VECTOR_TYPE(VT2);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_ROW_VECTOR_TYPE(VT1);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_COLUMN_VECTOR_TYPE(VT2);

	if ((*lhs).size() != (*rhs).size()) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Vector sizes do not match");
	}

	CT1 left(*lhs);
	CT2 right(*rhs);

	auto element(left.begin());
	auto end(left.end());

	MultType sp{};

	if (element != end) {
		sp = element->value() * right[element->index()];
		++element;
		for (; element != end; ++element)
			sp += element->value() * right[element->index()];
	}

	return sp;
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
