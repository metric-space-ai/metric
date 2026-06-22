// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_DMATEIGENEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_DMATEIGENEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/Aliases.h>
#include <metric/numeric/math/Exception.h>
#include <metric/numeric/math/constraints/BLASCompatible.h>
#include <metric/numeric/math/constraints/DenseMatrix.h>
#include <metric/numeric/math/constraints/StorageOrder.h>
#include <metric/numeric/math/expressions/Computation.h>
#include <metric/numeric/math/expressions/DenseVector.h>
#include <metric/numeric/math/expressions/EigenExpr.h>
#include <metric/numeric/math/expressions/Forward.h>
#include <metric/numeric/math/shims/Serial.h>
#include <metric/numeric/math/traits/ColumnTrait.h>
#include <metric/numeric/math/typetraits/IsExpression.h>
#include <metric/numeric/math/typetraits/IsHermitian.h>
#include <metric/numeric/math/typetraits/IsSymmetric.h>
#include <metric/numeric/math/typetraits/MakeComplex.h>
#include <metric/numeric/math/typetraits/UnderlyingElement.h>
#include <metric/numeric/util/Assert.h>
#include <metric/numeric/util/FunctionTrace.h>
#include <metric/numeric/util/Types.h>
#include <metric/numeric/util/mpl/If.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DMATEIGENEXPR
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Expression object for dense matrix eigenvalue solvers.
// \ingroup dense_vector_expression
//
// The DMatEigenExpr class represents the compile time expression for dense matrix eigenvalue
// solvers.
*/
template <typename MT // Type of the dense matrix
		  ,
		  bool SO> // Storage order
class DMatEigenExpr : public EigenExpr<DenseVector<DMatEigenExpr<MT, SO>, false>>, private Computation {
  private:
	//**Type definitions****************************************************************************
	//! Type of the resulting vector.
	using VT = ColumnTrait_t<ResultType_t<MT>>;

	//! Element type of the resulting vector.
	using ET = typename If_t<IsSymmetric_v<MT> || IsHermitian_v<MT>, UnderlyingElement<ElementType_t<MT>>,
							 MakeComplex<ElementType_t<MT>>>::Type;
	//**********************************************************************************************

  public:
	//**Type definitions****************************************************************************
	//! Type of this DMatEigenExpr instance.
	using This = DMatEigenExpr<MT, SO>;

	//! Base type of this DMatEigenExpr instance.
	using BaseType = EigenExpr<DenseVector<This, false>>;

	using ResultType = Rebind_t<VT, ET>;			   //!< Result type for expression template evaluations.
	using TransposeType = TransposeType_t<ResultType>; //!< Transpose type for expression template evaluations.
	using ElementType = ElementType_t<ResultType>;	   //!< Resulting element type.
	using ReturnType = ReturnType_t<ResultType>;	   //!< Return type for expression template evaluations.

	//! Data type for composite expression templates.
	using CompositeType = const ResultType;

	//! Composite data type of the dense matrix expression.
	using Operand = If_t<IsExpression_v<MT>, const MT, const MT &>;
	//**********************************************************************************************

	//**Compilation flags***************************************************************************
	//! Compilation switch for the expression template evaluation strategy.
	static constexpr bool simdEnabled = false;

	//! Compilation switch for the expression template assignment strategy.
	static constexpr bool smpAssignable = false;
	//**********************************************************************************************

	//**Constructor*********************************************************************************
	/*!\brief Constructor for the DMatEigenExpr class.
	//
	// \param dm The dense matrix operand of the eigenvalue expression.
	*/
	explicit inline DMatEigenExpr(const MT &dm) noexcept : dm_(dm) // Dense matrix of the eigenvalue expression
	{
		METRIC_NUMERIC_INTERNAL_ASSERT(isSquare(*dm), "Non-square matrix detected");
	}
	//**********************************************************************************************

	//**Size function*******************************************************************************
	/*!\brief Returns the current size/dimension of the vector.
	//
	// \return The size of the vector.
	*/
	inline size_t size() const noexcept { return dm_.rows(); }
	//**********************************************************************************************

	//**Operand access******************************************************************************
	/*!\brief Returns the dense matrix operand.
	//
	// \return The dense matrix operand.
	*/
	inline Operand operand() const noexcept { return dm_; }
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Returns whether the expression can alias with the given address \a alias.
	//
	// \param alias The alias to be checked.
	// \return \a true in case the expression can alias, \a false otherwise.
	*/
	template <typename T> inline bool canAlias(const T *alias) const noexcept { return dm_.isAliased(alias); }
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Returns whether the expression is aliased with the given address \a alias.
	//
	// \param alias The alias to be checked.
	// \return \a true in case an alias effect is detected, \a false otherwise.
	*/
	template <typename T> inline bool isAliased(const T *alias) const noexcept { return dm_.isAliased(alias); }
	//**********************************************************************************************

  private:
	//**Member variables****************************************************************************
	Operand dm_; //!< Dense matrix of the eigenvalue expression.
	//**********************************************************************************************

	//**Assignment to dense vectors*****************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Assignment of a dense matrix eigenvalue expression to a dense vector.
	// \ingroup dense_vector
	//
	// \param lhs The target left-hand side dense vector.
	// \param rhs The right-hand side eigenvalue expression to be assigned.
	// \return void
	//
	// This function implements the performance optimized assignment of a dense matrix eigenvalue
	// expression to a dense vector.
	*/
	template <typename VT> // Type of the target dense vector
	friend inline void assign(DenseVector<VT, false> &lhs, const DMatEigenExpr &rhs)
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == rhs.size(), "Invalid vector sizes");

		eigen(rhs.operand(), *lhs);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**Assignment to sparse vectors****************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Assignment of a dense matrix eigenvalue expression to a sparse vector.
	// \ingroup dense_vector
	//
	// \param lhs The target left-hand side sparse vector.
	// \param rhs The right-hand side eigenvalue expression to be assigned.
	// \return void
	//
	// This function implements the performance optimized assignment of a dense matrix eigenvalue
	// expression to a sparse vector.
	*/
	template <typename VT> // Type of the target sparse vector
	friend inline void assign(SparseVector<VT, false> &lhs, const DMatEigenExpr &rhs)
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == rhs.size(), "Invalid vector sizes");

		const ResultType tmp(serial(rhs));
		assign(*lhs, tmp);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**Addition assignment to dense vectors********************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Addition assignment of a dense matrix eigenvalue expression to a dense vector.
	// \ingroup dense_vector
	//
	// \param lhs The target left-hand side dense vector.
	// \param rhs The right-hand side eigenvalue expression to be added.
	// \return void
	//
	// This function implements the performance optimized addition assignment of a dense matrix
	// eigenvalue expression to a dense vector.
	*/
	template <typename VT> // Type of the target dense vector
	friend inline void addAssign(DenseVector<VT, false> &lhs, const DMatEigenExpr &rhs)
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == rhs.size(), "Invalid vector sizes");

		const ResultType tmp(serial(rhs));
		addAssign(*lhs, tmp);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**Addition assignment to sparse vectors*******************************************************
	// No special implementation for the addition assignment to sparse vectors.
	//**********************************************************************************************

	//**Subtraction assignment to dense vectors*****************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Subtraction assignment of a dense matrix eigenvalue expression to a dense vector.
	// \ingroup dense_vector
	//
	// \param lhs The target left-hand side dense vector.
	// \param rhs The right-hand side eigenvalue expression to be subtracted.
	// \return void
	//
	// This function implements the performance optimized subtraction assignment of a dense matrix
	// eigenvalue expression to a dense vector.
	*/
	template <typename VT> // Type of the target dense vector
	friend inline void subAssign(DenseMatrix<VT, false> &lhs, const DMatEigenExpr &rhs)
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == rhs.size(), "Invalid vector sizes");

		const ResultType tmp(serial(rhs));
		subAssign(*lhs, tmp);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**Subtraction assignment to sparse vectors****************************************************
	// No special implementation for the subtraction assignment to sparse vectors.
	//**********************************************************************************************

	//**Multiplication assignment to dense vectors**************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Multiplication assignment of a dense matrix eigenvalue expression to a dense vector.
	// \ingroup dense_vector
	//
	// \param lhs The target left-hand side dense vector.
	// \param rhs The right-hand side eigenvalue expression to be multiplied.
	// \return void
	//
	// This function implements the performance optimized multiplication assignment of a dense
	// matrix eigenvalue expression to a dense vector.
	*/
	template <typename VT> // Type of the target dense vector
	friend inline void multAssign(DenseMatrix<VT, false> &lhs, const DMatEigenExpr &rhs)
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == rhs.size(), "Invalid vector sizes");

		const ResultType tmp(serial(rhs));
		multAssign(*lhs, tmp);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**Multiplication assignment to sparse vectors*************************************************
	// No special implementation for the multiplication assignment to sparse vectors.
	//**********************************************************************************************

	//**Division assignment to dense vectors********************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Division assignment of a dense matrix eigenvalue expression to a dense vector.
	// \ingroup dense_vector
	//
	// \param lhs The target left-hand side dense vector.
	// \param rhs The right-hand side eigenvalue expression divisor.
	// \return void
	//
	// This function implements the performance optimized division assignment of a dense matrix
	// eigenvalue expression to a dense vector.
	*/
	template <typename VT> // Type of the target dense vector
	friend inline void divAssign(DenseMatrix<VT, false> &lhs, const DMatEigenExpr &rhs)
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == rhs.size(), "Invalid vector sizes");

		const ResultType tmp(serial(rhs));
		divAssign(*lhs, tmp);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**Division assignment to sparse vectors*******************************************************
	// No special implementation for the division assignment to sparse vectors.
	//**********************************************************************************************

	//**Compile time checks*************************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_DENSE_MATRIX_TYPE(MT);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_MATRIX_WITH_STORAGE_ORDER(MT, SO);
	/*! \endcond */
	//**********************************************************************************************
};
//*************************************************************************************************

//=================================================================================================
//
//  GLOBAL OPERATORS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Calculation of the eigenvalues of the given dense matrix.
// \ingroup dense_vector
//
// \param dm The given general matrix.
// \return The eigenvalues of the matrix.
// \exception std::invalid_argument Invalid non-square matrix provided.
//
// This function returns an expression representing the eigenvalues of the given dense matrix:

   \code
   using mtrc::numeric::rowMajor;

   mtrc::numeric::DynamicMatrix<double,rowMajor> A, B;
   // ... Resizing and initialization
   B = eigen( A );
   \endcode

// \note The \c eigen() function can only be used for dense matrices with \c float, \c double,
// \c complex<float> or \c complex<double> element type. The attempt to call the function with
// matrices of any other element type results in a compile time error!
//
// \note It is not possible to use any kind of view on the expression object returned by the
// \c eigen() function. Also, it is not possible to access individual elements via the subscript
// operator on the expression object:

   \code
   subvector( eigen( A ), 2, 4 );  // Compilation error: Views cannot be used on an eigen() expression!
   eigen( A )[1];                  // Compilation error: It is not possible to access individual elements!
   \endcode
*/
template <typename MT // Type of the dense matrix
		  ,
		  bool SO> // Storage order
inline decltype(auto) eigen(const DenseMatrix<MT, SO> &dm)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	METRIC_NUMERIC_CONSTRAINT_MUST_BE_BLAS_COMPATIBLE_TYPE(ElementType_t<MT>);

	if (!isSquare(*dm)) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid non-square matrix provided");
	}

	using ReturnType = const DMatEigenExpr<MT, SO>;
	return ReturnType(*dm);
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
