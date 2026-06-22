// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_MATSCALARMULTEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_MATSCALARMULTEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/Aliases.h>
#include <metric/numeric/math/Exception.h>
#include <metric/numeric/math/expressions/Forward.h>
#include <metric/numeric/math/expressions/MultExpr.h>
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
/*!\brief Base class for all matrix/scalar multiplication expression templates.
// \ingroup math
//
// The MatScalarMultExpr class serves as a tag for all expression templates that implement a
// matrix/scalar multiplication. All classes, that represent a matrix/scalar multiplication
// and that are used within the expression template environment of the Metric numeric library have
// to derive publicly from this class in order to qualify as matrix/scalar multiplication
// expression template. Only in case a class is derived publicly from the MatScalarMultExpr
// base class, the IsMatScalarMultExpr type trait recognizes the class as valid matrix/scalar
// multiplication expression template.
*/
template <typename MT> // Matrix base type of the expression
struct MatScalarMultExpr : public MultExpr<MT> {};
//*************************************************************************************************

//=================================================================================================
//
//  GLOBAL RESTRUCTURING UNARY ARITHMETIC OPERATORS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Unary minus operator for the negation of a matrix-scalar multiplication
//        (\f$ A = -(B*s) \f$).
// \ingroup math
//
// \param mat The matrix-scalar multiplication to be negated.
// \return The negation of the matrix-scalar multiplication.
//
// This operator implements a performance optimized treatment of the negation of a matrix-scalar
// multiplication expression.
*/
template <typename MT> // Matrix base type of the expression
inline decltype(auto) operator-(const MatScalarMultExpr<MT> &mat)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return (*mat).leftOperand() * (-(*mat).rightOperand());
}
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  GLOBAL RESTRUCTURING BINARY ARITHMETIC OPERATORS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Multiplication operator for the multiplication of a matrix-scalar multiplication
//        expression and a scalar value (\f$ A=(B*s1)*s2 \f$).
// \ingroup math
//
// \param mat The left-hand side matrix-scalar multiplication.
// \param scalar The right-hand side scalar value for the multiplication.
// \return The scaled result matrix.
//
// This operator implements a performance optimized treatment of the multiplication of a
// matrix-scalar multiplication expression and a scalar value.
*/
template <typename MT // Matrix base type of the expression
		  ,
		  typename ST // Type of the right-hand side scalar
		  ,
		  EnableIf_t<IsScalar_v<ST>> * = nullptr>
inline decltype(auto) operator*(const MatScalarMultExpr<MT> &mat, ST scalar)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return (*mat).leftOperand() * ((*mat).rightOperand() * scalar);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Multiplication operator for the multiplication of a scalar value and a matrix-scalar
//        multiplication expression (\f$ A=s2*(B*s1) \f$).
// \ingroup math
//
// \param scalar The left-hand side scalar value for the multiplication.
// \param mat The right-hand side matrix-scalar multiplication.
// \return The scaled result matrix.
//
// This operator implements a performance optimized treatment of the multiplication of a
// scalar value and a matrix-scalar multiplication expression.
*/
template <typename ST // Type of the left-hand side scalar
		  ,
		  typename MT // Matrix base type of the expression
		  ,
		  EnableIf_t<IsScalar_v<ST>> * = nullptr>
inline decltype(auto) operator*(ST scalar, const MatScalarMultExpr<MT> &mat)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return (*mat).leftOperand() * (scalar * (*mat).rightOperand());
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Division operator for the division of a matrix-scalar multiplication expression by
//        a scalar value (\f$ A=(B*s1)/s2 \f$).
// \ingroup math
//
// \param mat The left-hand side matrix-scalar multiplication.
// \param scalar The right-hand side scalar value for the division.
// \return The scaled result matrix.
//
// This operator implements a performance optimized treatment of the division of a
// matrix-scalar multiplication expression by a scalar value.
*/
template <
	typename MT // Matrix base type of the expression
	,
	typename ST // Type of the right-hand side scalar
	,
	EnableIf_t<IsScalar_v<ST> && (IsInvertible_v<ST> || IsInvertible_v<RightOperand_t<MatrixType_t<MT>>>)> * = nullptr>
inline decltype(auto) operator/(const MatScalarMultExpr<MT> &mat, ST scalar)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	METRIC_NUMERIC_USER_ASSERT(scalar != ST(0), "Division by zero detected");

	return (*mat).leftOperand() * ((*mat).rightOperand() / scalar);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Multiplication operator for the multiplication of a matrix-scalar multiplication
//        expression and a dense vector (\f$ \vec{a}=(B*s1)*\vec{c} \f$).
// \ingroup math
//
// \param mat The left-hand side matrix-scalar multiplication.
// \param vec The right-hand side dense vector.
// \return The scaled result vector.
//
// This operator implements the performance optimized treatment of the multiplication
// of a matrix-scalar multiplication and a dense vector. It restructures the expression
// \f$ \vec{a}=(B*s1)*\vec{c} \f$ to the expression \f$ \vec{a}=(B*\vec{c})*s1 \f$.
*/
template <typename MT // Matrix base type of the left-hand side expression
		  ,
		  typename VT> // Type of the right-hand side dense vector
inline decltype(auto) operator*(const MatScalarMultExpr<MT> &mat, const DenseVector<VT, false> &vec)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return ((*mat).leftOperand() * (*vec)) * (*mat).rightOperand();
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Multiplication operator for the multiplication of a dense vector and a
//        matrix-scalar multiplication expression (\f$ \vec{a}^T=\vec{c}^T*(B*s1) \f$).
// \ingroup math
//
// \param vec The left-hand side dense vector.
// \param mat The right-hand side matrix-scalar multiplication.
// \return The scaled result vector.
//
// This operator implements the performance optimized treatment of the multiplication
// of a dense vector and a matrix-scalar multiplication. It restructures the expression
// \f$ \vec{a}=\vec{c}^T*(B*s1) \f$ to the expression \f$ \vec{a}^T=(\vec{c}^T*B)*s1 \f$.
*/
template <typename VT // Type of the left-hand side dense vector
		  ,
		  typename MT> // Matrix base type of the right-hand side expression
inline decltype(auto) operator*(const DenseVector<VT, true> &vec, const MatScalarMultExpr<MT> &mat)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return ((*vec) * (*mat).leftOperand()) * (*mat).rightOperand();
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Multiplication operator for the multiplication of a matrix-scalar multiplication
//        expression and a sparse vector (\f$ \vec{a}=(B*s1)*\vec{c} \f$).
// \ingroup math
//
// \param mat The left-hand side matrix-scalar multiplication.
// \param vec The right-hand side sparse vector.
// \return The scaled result vector.
//
// This operator implements the performance optimized treatment of the multiplication
// of a matrix-scalar multiplication and a sparse vector. It restructures the expression
// \f$ \vec{a}=(B*s1)*\vec{c} \f$ to the expression \f$ \vec{a}=(B*\vec{c})*s1 \f$.
*/
template <typename MT // Matrix base type of the left-hand side expression
		  ,
		  typename VT> // Type of the right-hand side sparse vector
inline decltype(auto) operator*(const MatScalarMultExpr<MT> &mat, const SparseVector<VT, false> &vec)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return ((*mat).leftOperand() * (*vec)) * (*mat).rightOperand();
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Multiplication operator for the multiplication of a sparse vector and a
//        matrix-scalar multiplication expression (\f$ \vec{a}^T=\vec{c}^T*(B*s1) \f$).
// \ingroup math
//
// \param vec The left-hand side sparse vector.
// \param mat The right-hand side matrix-scalar multiplication.
// \return The scaled result vector.
//
// This operator implements the performance optimized treatment of the multiplication
// of a sparse vector and a matrix-scalar multiplication. It restructures the expression
// \f$ \vec{a}=\vec{c}^T*(B*s1) \f$ to the expression \f$ \vec{a}^T=(\vec{c}^T*B)*s1 \f$.
*/
template <typename VT // Type of the left-hand side sparse vector
		  ,
		  typename MT> // Matrix base type of the right-hand side expression
inline decltype(auto) operator*(const SparseVector<VT, true> &vec, const MatScalarMultExpr<MT> &mat)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return ((*vec) * (*mat).leftOperand()) * (*mat).rightOperand();
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Multiplication operator for the multiplication of a matrix-scalar multiplication
//        expression and a vector-scalar multiplication expression
//        (\f$ \vec{a}=(B*s1)*(\vec{c}*s2) \f$).
// \ingroup math
//
// \param mat The left-hand side matrix-scalar multiplication.
// \param vec The right-hand side vector-scalar multiplication.
// \return The scaled result vector.
//
// This operator implements the performance optimized treatment of the multiplication of a matrix-
// scalar multiplication and a vector-scalar multiplication. It restructures the expression
// \f$ \vec{a}=(B*s1)*(\vec{c}*s2) \f$ to the expression \f$ \vec{a}=(B*\vec{c})*(s1*s2) \f$.
*/
template <typename MT // Matrix base type of the left-hand side expression
		  ,
		  typename VT> // Vector base type of the right-hand side expression
inline decltype(auto) operator*(const MatScalarMultExpr<MT> &mat, const VecScalarMultExpr<VT> &vec)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return ((*mat).leftOperand() * (*vec).leftOperand()) * ((*mat).rightOperand() * (*vec).rightOperand());
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Multiplication operator for the multiplication of a vector-scalar multiplication
//        expression and a matrix-scalar multiplication expression
//        (\f$ \vec{a}^T=\vec{b}^T*(C*s1) \f$).
// \ingroup math
//
// \param vec The left-hand side vector-scalar multiplication.
// \param mat The right-hand side matrix-scalar multiplication.
// \return The scaled result vector.
//
// This operator implements the performance optimized treatment of the multiplication of a vector-
// scalar multiplication and a matrix-scalar multiplication. It restructures the expression
// \f$ \vec{a}=(\vec{b}^T*s1)*(C*s2) \f$ to the expression \f$ \vec{a}^T=(\vec{b}^T*C)*(s1*s2) \f$.
*/
template <typename VT // Vector base type of the left-hand side expression
		  ,
		  typename MT> // Matrix base type of the right-hand side expression
inline decltype(auto) operator*(const VecScalarMultExpr<VT> &vec, const MatScalarMultExpr<MT> &mat)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return ((*vec).leftOperand() * (*mat).leftOperand()) * ((*vec).rightOperand() * (*mat).rightOperand());
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Multiplication operator for the multiplication of a matrix-scalar multiplication
//        expression and a dense matrix (\f$ A=(B*s1)*C \f$).
// \ingroup math
//
// \param lhs The left-hand side matrix-scalar multiplication.
// \param rhs The right-hand side dense matrix.
// \return The scaled result matrix.
//
// This operator implements the performance optimized treatment of the multiplication
// of a matrix-scalar multiplication and a dense matrix. It restructures the expression
// \f$ A=(B*s1)*C \f$ to the expression \f$ A=(B*C)*s1 \f$.
*/
template <typename MT1 // Matrix base type of the left-hand side expression
		  ,
		  typename MT2 // Type of the right-hand side dense matrix
		  ,
		  bool SO> // Storage order of the right-hand side dense matrix
inline decltype(auto) operator*(const MatScalarMultExpr<MT1> &lhs, const DenseMatrix<MT2, SO> &rhs)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return ((*lhs).leftOperand() * (*rhs)) * (*lhs).rightOperand();
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Multiplication operator for the multiplication of a dense matrix and a matrix-scalar
//        multiplication expression (\f$ A=(B*s1)*C \f$).
// \ingroup math
//
// \param lhs The left-hand side dense matrix.
// \param rhs The right-hand side matrix-scalar multiplication.
// \return The scaled result matrix.
//
// This operator implements the performance optimized treatment of the multiplication
// of a dense matrix and a matrix-scalar multiplication. It restructures the expression
// \f$ A=B*(C*s1) \f$ to the expression \f$ A=(B*C)*s1 \f$.
*/
template <typename MT1 // Type of the left-hand side dense matrix
		  ,
		  bool SO // Storage order of the left-hand side dense matrix
		  ,
		  typename MT2> // Matrix base type of the right-hand side expression
inline decltype(auto) operator*(const DenseMatrix<MT1, SO> &lhs, const MatScalarMultExpr<MT2> &rhs)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return ((*lhs) * (*rhs).leftOperand()) * (*rhs).rightOperand();
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Multiplication operator for the multiplication of a matrix-scalar multiplication
//        expression and a sparse matrix (\f$ A=(B*s1)*C \f$).
// \ingroup math
//
// \param lhs The left-hand side matrix-scalar multiplication.
// \param rhs The right-hand side sparse matrix.
// \return The scaled result matrix.
//
// This operator implements the performance optimized treatment of the multiplication
// of a matrix-scalar multiplication and a sparse matrix. It restructures the expression
// \f$ A=(B*s1)*C \f$ to the expression \f$ A=(B*C)*s1 \f$.
*/
template <typename MT1 // Matrix base type of the left-hand side expression
		  ,
		  typename MT2 // Type of the right-hand side sparse matrix
		  ,
		  bool SO> // Storage order of the right-hand side sparse matrix
inline decltype(auto) operator*(const MatScalarMultExpr<MT1> &lhs, const SparseMatrix<MT2, SO> &rhs)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return ((*lhs).leftOperand() * (*rhs)) * (*lhs).rightOperand();
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Multiplication operator for the multiplication of a sparse matrix and a matrix-scalar
//        multiplication expression (\f$ A=(B*s1)*C \f$).
// \ingroup math
//
// \param lhs The left-hand side sparse  matrix.
// \param rhs The right-hand side matrix-scalar multiplication.
// \return The scaled result matrix.
//
// This operator implements the performance optimized treatment of the multiplication
// of a sparse matrix and a matrix-scalar multiplication. It restructures the expression
// \f$ A=B*(C*s1) \f$ to the expression \f$ A=(B*C)*s1 \f$.
*/
template <typename MT1 // Type of the left-hand side sparse matrix
		  ,
		  bool SO // Storage order of the left-hand side sparse matrix
		  ,
		  typename MT2> // Matrix base type of the right-hand side expression
inline decltype(auto) operator*(const SparseMatrix<MT1, SO> &lhs, const MatScalarMultExpr<MT2> &rhs)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return ((*lhs) * (*rhs).leftOperand()) * (*rhs).rightOperand();
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Multiplication operator for the multiplication of two matrix-scalar multiplication
//        expressions (\f$ A=(B*s1)*(C*s2) \f$).
// \ingroup math
//
// \param lhs The left-hand side matrix-scalar multiplication.
// \param rhs The right-hand side matrix-scalar multiplication.
// \return The scaled result matrix.
//
// This operator implements the performance optimized treatment of the multiplication of
// two matrix-scalar multiplication expressions. It restructures the expression
// \f$ A=(B*s1)*(C*s2) \f$ to the expression \f$ A=(B*C)*(s1*s2) \f$.
*/
template <typename MT1 // Matrix base type of the left-hand side expression
		  ,
		  typename MT2> // Matrix base type of the right-hand side expression
inline decltype(auto) operator*(const MatScalarMultExpr<MT1> &lhs, const MatScalarMultExpr<MT2> &rhs)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return ((*lhs).leftOperand() * (*rhs).leftOperand()) * ((*lhs).rightOperand() * (*rhs).rightOperand());
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
/*!\brief Calculation of the transpose of the given matrix-scalar multiplication.
// \ingroup math
//
// \param matrix The matrix-scalar multiplication expression to be transposed.
// \return The transpose of the expression.
//
// This operator implements the performance optimized treatment of the transpose of a
// matrix-scalar multiplication. It restructures the expression \f$ A=trans(B*s1) \f$ to
// the expression \f$ A=trans(B)*s1 \f$.
*/
template <typename MT> // Matrix base type of the expression
inline decltype(auto) trans(const MatScalarMultExpr<MT> &matrix)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return trans((*matrix).leftOperand()) * (*matrix).rightOperand();
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Calculation of the complex conjugate of the given matrix-scalar multiplication.
// \ingroup math
//
// \param matrix The matrix-scalar multiplication expression to be conjugated.
// \return The complex conjugate of the expression.
//
// This operator implements the performance optimized treatment of the complex conjugate of a
// matrix-scalar multiplication. It restructures the expression \f$ a=conj(b*s1) \f$ to the
// expression \f$ a=conj(b)*s1 \f$.
*/
template <typename MT> // Matrix base type of the expression
inline decltype(auto) conj(const MatScalarMultExpr<MT> &matrix)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return conj((*matrix).leftOperand()) * (*matrix).rightOperand();
}
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
