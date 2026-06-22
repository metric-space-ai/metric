// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_VECSCALARDIVEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_VECSCALARDIVEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/Aliases.h>
#include <metric/numeric/math/expressions/DivExpr.h>
#include <metric/numeric/math/typetraits/IsInvertible.h>
#include <metric/numeric/math/typetraits/IsScalar.h>
#include <metric/numeric/util/Assert.h>
#include <metric/numeric/util/EnableIf.h>
#include <metric/numeric/util/FunctionTrace.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Base class for all vector/scalar division expression templates.
// \ingroup math
//
// The VecScalarDivExpr class serves as a tag for all expression templates that implement a
// vector/scalar division. All classes, that represent a vector/scalar division and that are
// used within the expression template environment of the Metric numeric library have to derive publicly
// from this class in order to qualify as vector/scalar division expression template. Only in
// case a class is derived publicly from the VecScalarDivExpr base class, the IsVecScalarDivExpr
// type trait recognizes the class as valid vector/scalar division expression template.
*/
template <typename VT> // Vector base type of the expression
struct VecScalarDivExpr : public DivExpr<VT> {};
//*************************************************************************************************

//=================================================================================================
//
//  GLOBAL RESTRUCTURING BINARY ARITHMETIC OPERATORS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Multiplication operator for the multiplication of a vector-scalar division
//        expression and a scalar value (\f$ \vec{a}=(\vec{b}/s1)*s2 \f$).
// \ingroup math
//
// \param vec The left-hand side vector-scalar division.
// \param scalar The right-hand side scalar value for the multiplication.
// \return The scaled result vector.
//
// This operator implements a performance optimized treatment of the multiplication of a
// vector-scalar division expression and a scalar value.
*/
template <
	typename VT // Vector base type of the expression
	,
	typename ST // Type of the right-hand side scalar
	,
	EnableIf_t<IsScalar_v<ST> && (IsInvertible_v<ST> || IsInvertible_v<RightOperand_t<VectorType_t<VT>>>)> * = nullptr>
inline decltype(auto) operator*(const VecScalarDivExpr<VT> &vec, ST scalar)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return (*vec).leftOperand() * (scalar / (*vec).rightOperand());
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Multiplication operator for the multiplication of a scalar value and a vector-scalar
//        division expression (\f$ \vec{a}=s2*(\vec{b}/s1) \f$).
// \ingroup math
//
// \param scalar The left-hand side scalar value for the multiplication.
// \param vec The right-hand side vector-scalar division.
// \return The scaled result vector.
//
// This operator implements a performance optimized treatment of the multiplication of a
// scalar value and a vector-scalar division expression.
*/
template <
	typename ST // Type of the left-hand side scalar
	,
	typename VT // Vector base type of the expression
	,
	EnableIf_t<IsScalar_v<ST> && (IsInvertible_v<ST> || IsInvertible_v<RightOperand_t<VectorType_t<VT>>>)> * = nullptr>
inline decltype(auto) operator*(ST scalar, const VecScalarDivExpr<VT> &vec)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return (*vec).leftOperand() * (scalar / (*vec).rightOperand());
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Division operator for the division of a vector-scalar division expression
//        and a scalar value (\f$ \vec{a}=(\vec{b}/s1)/s2 \f$).
// \ingroup math
//
// \param vec The left-hand side vector-scalar division.
// \param scalar The right-hand side scalar value for the division.
// \return The scaled result vector.
//
// This operator implements a performance optimized treatment of the division of a vector-scalar
// division expression and a scalar value.
*/
template <typename VT // Vector base type of the expression
		  ,
		  typename ST // Type of the right-hand side scalar
		  ,
		  EnableIf_t<IsScalar_v<ST>> * = nullptr>
inline decltype(auto) operator/(const VecScalarDivExpr<VT> &vec, ST scalar)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	METRIC_NUMERIC_USER_ASSERT(scalar != ST(0), "Division by zero detected");

	return (*vec).leftOperand() / ((*vec).rightOperand() * scalar);
}
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  GLOBAL RESTRUCTURING FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Calculation of the transpose of the given vector-scalar division.
// \ingroup math
//
// \param vector The vector-scalar division expression to be transposed.
// \return The transpose of the expression.
//
// This operator implements the performance optimized treatment of the transpose of a
// vector-scalar division. It restructures the expression \f$ a=trans(b/s1) \f$ to
// the expression \f$ a=trans(b)/s1 \f$.
*/
template <typename VT> // Vector base type of the expression
inline decltype(auto) trans(const VecScalarDivExpr<VT> &vector)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return trans((*vector).leftOperand()) / (*vector).rightOperand();
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Calculation of the complex conjugate of the given vector-scalar division.
// \ingroup math
//
// \param vector The vector-scalar division expression to be conjugated.
// \return The complex conjugate of the expression.
//
// This operator implements the performance optimized treatment of the complex conjugate
// of a vector-scalar division. It restructures the expression \f$ a=conj(b/s1) \f$ to the
// expression \f$ a=conj(b)/s1 \f$.
*/
template <typename VT> // Vector base type of the expression
inline decltype(auto) conj(const VecScalarDivExpr<VT> &vector)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return conj((*vector).leftOperand()) / (*vector).rightOperand();
}
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
