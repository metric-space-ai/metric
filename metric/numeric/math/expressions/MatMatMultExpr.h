// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_MATMATMULTEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_MATMATMULTEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/expressions/Forward.h>
#include <metric/numeric/math/expressions/MultExpr.h>
#include <metric/numeric/util/FunctionTrace.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Base class for all matrix/matrix multiplication expression templates.
// \ingroup math
//
// The MatMatMultExpr class serves as a tag for all expression templates that implement a
// matrix/matrix multiplication. All classes, that represent a matrix multiplication and
// that are used within the expression template environment of the Metric numeric library have to
// derive publicly from this class in order to qualify as matrix multiplication expression
// template. Only in case a class is derived publicly from the MatMatMultExpr base class,
// the IsMatMatMultExpr type trait recognizes the class as valid matrix multiplication
// expression template.
*/
template <typename MT> // Matrix base type of the expression
struct MatMatMultExpr : public MultExpr<MT> {};
//*************************************************************************************************

//=================================================================================================
//
//  GLOBAL RESTRUCTURING BINARY ARITHMETIC OPERATORS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Multiplication operator for the multiplication of a matrix-matrix multiplication
//        expression and a dense vector (\f$ \vec{y}=(A*B)*\vec{x} \f$).
// \ingroup math
//
// \param mat The left-hand side matrix-matrix multiplication.
// \param vec The right-hand side dense vector for the multiplication.
// \return The resulting vector.
//
// This operator implements a performance optimized treatment of the multiplication of a
// matrix-matrix multiplication expression and a dense vector. It restructures the expression
// \f$ \vec{x}=(A*B)*\vec{x} \f$ to the expression \f$ \vec{y}=A*(B*\vec{x}) \f$.
*/
template <typename MT // Matrix base type of the left-hand side expression
		  ,
		  typename VT> // Type of the right-hand side dense vector
inline decltype(auto) operator*(const MatMatMultExpr<MT> &mat, const DenseVector<VT, false> &vec)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return (*mat).leftOperand() * ((*mat).rightOperand() * vec);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Multiplication operator for the multiplication of a matrix-matrix multiplication
//        expression and a sparse vector (\f$ \vec{y}=(A*B)*\vec{x} \f$).
// \ingroup math
//
// \param mat The left-hand side matrix-matrix multiplication.
// \param vec The right-hand side sparse vector for the multiplication.
// \return The resulting vector.
//
// This operator implements a performance optimized treatment of the multiplication of a
// matrix-matrix multiplication expression and a sparse vector. It restructures the expression
// \f$ \vec{y}=(A*B)*\vec{x} \f$ to the expression \f$ \vec{y}=A*(B*\vec{x}) \f$.
*/
template <typename MT // Matrix base type of the left-hand side expression
		  ,
		  typename VT> // Type of the right-hand side sparse vector
inline decltype(auto) operator*(const MatMatMultExpr<MT> &mat, const SparseVector<VT, false> &vec)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return (*mat).leftOperand() * ((*mat).rightOperand() * vec);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Multiplication operator for the multiplication of a transpose dense vector and a
//        matrix-matrix multiplication expression (\f$ \vec{y}^T=\vec{x}^T*(A*B) \f$).
// \ingroup math
//
// \param vec The left-hand side dense vector for the multiplication.
// \param mat The right-hand side matrix-matrix multiplication.
// \return The resulting vector.
//
// This operator implements a performance optimized treatment of the multiplication of a dense
// vector and a matrix-matrix multiplication expression. It restructures the expression
// \f$ \vec{y}^T=\vec{x}^T*(A*B) \f$ to the expression \f$ \vec{y}^T=(\vec{x}^T*A)*B \f$.
*/
template <typename VT // Type of the left-hand side dense vector
		  ,
		  typename MT> // Matrix base type of the right-hand side expression
inline decltype(auto) operator*(const DenseVector<VT, true> &vec, const MatMatMultExpr<MT> &mat)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return (vec * (*mat).leftOperand()) * (*mat).rightOperand();
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Multiplication operator for the multiplication of a transpose sparse vector and a
//        matrix-matrix multiplication expression (\f$ \vec{y}^T=\vec{x}^T*(A*B) \f$).
// \ingroup math
//
// \param vec The left-hand side sparse vector for the multiplication.
// \param mat The right-hand side matrix-matrix multiplication.
// \return The resulting vector.
//
// This operator implements a performance optimized treatment of the multiplication of a sparse
// vector and a matrix-matrix multiplication expression. It restructures the expression
// \f$ \vec{y}^T=\vec{x}^T*(A*B) \f$ to the expression \f$ \vec{y}^T=(\vec{x}^T*A)*B \f$.
*/
template <typename VT // Type of the left-hand side sparse vector
		  ,
		  typename MT> // Matrix base type of the right-hand side expression
inline decltype(auto) operator*(const SparseVector<VT, true> &vec, const MatMatMultExpr<MT> &mat)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return (vec * (*mat).leftOperand()) * (*mat).rightOperand();
}
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
