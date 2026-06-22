// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_MATSCALARDIVEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_MATSCALARDIVEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/Aliases.h>
#include <metric/numeric/math/Exception.h>
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
/*!\brief Base class for all matrix/scalar division expression templates.
// \ingroup math
//
// The MatScalarDivExpr class serves as a tag for all expression templates that implement a
// matrix/scalar division. All classes, that represent a matrix/scalar division and that are
// used within the expression template environment of the Metric numeric library have to derive publicly
// from this class in order to qualify as matrix/scalar division expression template. Only in
// case a class is derived publicly from the MatScalarDivExpr base class, the IsMatScalarDivExpr
// type trait recognizes the class as valid matrix/scalar division expression template.
*/
template <typename MT> // Matrix base type of the expression
struct MatScalarDivExpr : public DivExpr<MT> {};
//*************************************************************************************************

//=================================================================================================
//
//  GLOBAL RESTRUCTURING BINARY ARITHMETIC OPERATORS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Multiplication operator for the multiplication of a matrix-scalar division expression
//        and a scalar value (\f$ A=(B/s1)*s2 \f$).
// \ingroup math
//
// \param mat The left-hand side matrix-scalar division.
// \param scalar The right-hand side scalar value for the multiplication.
// \return The scaled result matrix.
//
// This operator implements a performance optimized treatment of the multiplication of a
// matrix-scalar division expression and a scalar value.
*/
template <
	typename MT // Matrix base type of the expression
	,
	typename ST // Type of the right-hand side scalar
	,
	EnableIf_t<IsScalar_v<ST> && (IsInvertible_v<ST> || IsInvertible_v<RightOperand_t<MatrixType_t<MT>>>)> * = nullptr>
inline decltype(auto) operator*(const MatScalarDivExpr<MT> &mat, ST scalar)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return (*mat).leftOperand() * (scalar / (*mat).rightOperand());
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Multiplication operator for the multiplication of a scalar value and a matrix-scalar
//        division expression (\f$ A=s2*(B/s1) \f$).
// \ingroup math
//
// \param scalar The left-hand side scalar value for the multiplication.
// \param mat The right-hand side matrix-scalar division.
// \return The scaled result matrix.
//
// This operator implements a performance optimized treatment of the multiplication of a
// scalar value and a matrix-scalar division expression.
*/
template <
	typename ST // Type of the left-hand side scalar
	,
	typename MT // Matrix base type of the expression
	,
	EnableIf_t<IsScalar_v<ST> && (IsInvertible_v<ST> || IsInvertible_v<RightOperand_t<MatrixType_t<MT>>>)> * = nullptr>
inline decltype(auto) operator*(ST scalar, const MatScalarDivExpr<MT> &mat)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return (*mat).leftOperand() * (scalar / (*mat).rightOperand());
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Division operator for the division of a dense matrix-scalar division expression
//        and a scalar value (\f$ A=(B/s1)/s2 \f$).
// \ingroup dense_matrix
//
// \param mat The left-hand side dense matrix-scalar division.
// \param scalar The right-hand side scalar value for the division.
// \return The scaled result matrix.
//
// This operator implements a performance optimized treatment of the division of a dense
// matrix-scalar division expression and a scalar value.
*/
template <typename MT // Matrix base type of the expression
		  ,
		  typename ST // Type of the right-hand side scalar
		  ,
		  EnableIf_t<IsScalar_v<ST>> * = nullptr>
inline decltype(auto) operator/(const MatScalarDivExpr<MT> &mat, ST scalar)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	METRIC_NUMERIC_USER_ASSERT(scalar != ST(0), "Division by zero detected");

	return (*mat).leftOperand() / ((*mat).rightOperand() * scalar);
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
/*!\brief Calculation of the transpose of the given matrix-scalar division.
// \ingroup math
//
// \param matrix The matrix-scalar division expression to be transposed.
// \return The transpose of the expression.
//
// This operator implements the performance optimized treatment of the transpose of a
// matrix-scalar division. It restructures the expression \f$ A=trans(B/s1) \f$ to
// the expression \f$ A=trans(B)/s1 \f$.
*/
template <typename MT> // Matrix base type of the expression
inline decltype(auto) trans(const MatScalarDivExpr<MT> &matrix)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return trans((*matrix).leftOperand()) / (*matrix).rightOperand();
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Calculation of the complex conjugate of the given matrix-scalar division.
// \ingroup math
//
// \param matrix The matrix-scalar division expression to be conjugated.
// \return The complex conjugate of the expression.
//
// This operator implements the performance optimized treatment of the complex conjugate
// of a matrix-scalar division. It restructures the expression \f$ a=conj(b/s1) \f$ to the
// expression \f$ a=conj(b)/s1 \f$.
*/
template <typename MT> // Matrix base type of the expression
inline decltype(auto) conj(const MatScalarDivExpr<MT> &matrix)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return conj((*matrix).leftOperand()) / (*matrix).rightOperand();
}
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
