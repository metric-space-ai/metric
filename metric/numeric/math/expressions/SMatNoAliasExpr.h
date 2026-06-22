// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_SMATNOALIASEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_SMATNOALIASEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/Aliases.h>
#include <metric/numeric/math/Exception.h>
#include <metric/numeric/math/constraints/RequiresEvaluation.h>
#include <metric/numeric/math/constraints/SparseMatrix.h>
#include <metric/numeric/math/constraints/StorageOrder.h>
#include <metric/numeric/math/expressions/Forward.h>
#include <metric/numeric/math/expressions/MatNoAliasExpr.h>
#include <metric/numeric/math/expressions/Modification.h>
#include <metric/numeric/math/expressions/SparseMatrix.h>
#include <metric/numeric/math/typetraits/IsExpression.h>
#include <metric/numeric/math/typetraits/RequiresEvaluation.h>
#include <metric/numeric/util/Assert.h>
#include <metric/numeric/util/FunctionTrace.h>
#include <metric/numeric/util/InvalidType.h>
#include <metric/numeric/util/MaybeUnused.h>
#include <metric/numeric/util/Types.h>
#include <metric/numeric/util/mpl/If.h>
#include <metric/numeric/util/typetraits/GetMemberType.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS SMATNOALIASEXPR
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Expression object for the non-aliased evaluation of sparse matrices.
// \ingroup sparse_matrix_expression
//
// The SMatNoAliasExpr class represents the compile time expression for the non-aliased evaluation
// of a sparse matrix.
*/
template <typename MT // Type of the sparse matrix
		  ,
		  bool SO> // Storage order
class SMatNoAliasExpr : public MatNoAliasExpr<SparseMatrix<SMatNoAliasExpr<MT, SO>, SO>>, private Modification<MT> {
  private:
	//**Type definitions****************************************************************************
	//! Definition of the GetConstIterator type trait.
	METRIC_NUMERIC_CREATE_GET_TYPE_MEMBER_TYPE_TRAIT(GetConstIterator, ConstIterator, INVALID_TYPE);
	//**********************************************************************************************

  public:
	//**Type definitions****************************************************************************
	//! Type of this SMatNoAliasExpr instance.
	using This = SMatNoAliasExpr<MT, SO>;

	//! Base type of this SMatNoAliasExpr instance.
	using BaseType = MatNoAliasExpr<SparseMatrix<This, SO>>;

	using ResultType = ResultType_t<MT>; //!< Result type for expression template evaluations.
	using OppositeType =
		OppositeType_t<MT>; //!< Result type with opposite storage order for expression template evaluations.
	using TransposeType = TransposeType_t<MT>; //!< Transpose type for expression template evaluations.
	using ElementType = ElementType_t<MT>;	   //!< Resulting element type.
	using ReturnType = ReturnType_t<MT>;	   //!< Return type for expression template evaluations.

	//! Data type for composite expression templates.
	using CompositeType = If_t<RequiresEvaluation_v<MT>, const ResultType, const SMatNoAliasExpr &>;

	//! Iterator over the elements of the sparse matrix.
	using ConstIterator = GetConstIterator_t<MT>;

	//! Composite data type of the sparse matrix expression.
	using Operand = If_t<IsExpression_v<MT>, const MT, const MT &>;
	//**********************************************************************************************

	//**Compilation flags***************************************************************************
	//! Compilation switch for the expression template assignment strategy.
	static constexpr bool smpAssignable = MT::smpAssignable;
	//**********************************************************************************************

	//**Constructor*********************************************************************************
	/*!\brief Constructor for the SMatNoAliasExpr class.
	//
	// \param sm The sparse matrix operand of the no-alias expression.
	*/
	explicit inline SMatNoAliasExpr(const MT &sm) noexcept : sm_(sm) // Sparse matrix of the no-alias expression
	{
	}
	//**********************************************************************************************

	//**Access operator*****************************************************************************
	/*!\brief 2D-access to the matrix elements.
	//
	// \param i Access index for the row. The index has to be in the range \f$[0..M-1]\f$.
	// \param j Access index for the column. The index has to be in the range \f$[0..N-1]\f$.
	// \return The resulting value.
	*/
	inline ReturnType operator()(size_t i, size_t j) const { return sm_(i, j); }
	//**********************************************************************************************

	//**At function*********************************************************************************
	/*!\brief Checked access to the matrix elements.
	//
	// \param i Access index for the row. The index has to be in the range \f$[0..M-1]\f$.
	// \param j Access index for the column. The index has to be in the range \f$[0..N-1]\f$.
	// \return The resulting value.
	// \exception std::out_of_range Invalid matrix access index.
	*/
	inline ReturnType at(size_t i, size_t j) const { return sm_.at(i, j); }
	//**********************************************************************************************

	//**Begin function******************************************************************************
	/*!\brief Returns an iterator to the first non-zero element of row/column \a i.
	//
	// \param i The row/column index.
	// \return Iterator to the first non-zero element of row/column \a i.
	*/
	inline ConstIterator begin(size_t i) const { return sm_.begin(i); }
	//**********************************************************************************************

	//**End function********************************************************************************
	/*!\brief Returns an iterator just past the last non-zero element of row/column \a i.
	//
	// \param i The row/column index.
	// \return Iterator just past the last non-zero element of row/column \a i.
	*/
	inline ConstIterator end(size_t i) const { return sm_.end(i); }
	//**********************************************************************************************

	//**Rows function*******************************************************************************
	/*!\brief Returns the current number of rows of the matrix.
	//
	// \return The number of rows of the matrix.
	*/
	inline size_t rows() const noexcept { return sm_.rows(); }
	//**********************************************************************************************

	//**Columns function****************************************************************************
	/*!\brief Returns the current number of columns of the matrix.
	//
	// \return The number of columns of the matrix.
	*/
	inline size_t columns() const noexcept { return sm_.columns(); }
	//**********************************************************************************************

	//**NonZeros function***************************************************************************
	/*!\brief Returns the number of non-zero elements in the sparse matrix.
	//
	// \return The number of non-zero elements in the sparse matrix.
	*/
	inline size_t nonZeros() const { return sm_.nonZeros(); }
	//**********************************************************************************************

	//**NonZeros function***************************************************************************
	/*!\brief Returns the number of non-zero elements in the specified row.
	//
	// \param i The index of the row.
	// \return The number of non-zero elements of row \a i.
	*/
	inline size_t nonZeros(size_t i) const { return sm_.nonZeros(i); }
	//**********************************************************************************************

	//**Find function*******************************************************************************
	/*!\brief Searches for a specific matrix element.
	//
	// \param i The row index of the search element.
	// \param j The column index of the search element.
	// \return Iterator to the element in case the index is found, end() iterator otherwise.
	*/
	inline ConstIterator find(size_t i, size_t j) const
	{
		METRIC_NUMERIC_CONSTRAINT_MUST_NOT_REQUIRE_EVALUATION(MT);
		return sm_.find(i, j);
	}
	//**********************************************************************************************

	//**LowerBound function*************************************************************************
	/*!\brief Returns an iterator to the first index not less then the given index.
	//
	// \param i The row index of the search element.
	// \param j The column index of the search element.
	// \return Iterator to the first index not less then the given index, end() iterator otherwise.
	*/
	inline ConstIterator lowerBound(size_t i, size_t j) const
	{
		METRIC_NUMERIC_CONSTRAINT_MUST_NOT_REQUIRE_EVALUATION(MT);
		return sm_.lowerBound(i, j);
	}
	//**********************************************************************************************

	//**UpperBound function*************************************************************************
	/*!\brief Returns an iterator to the first index greater then the given index.
	//
	// \param i The row index of the search element.
	// \param j The column index of the search element.
	// \return Iterator to the first index greater then the given index, end() iterator otherwise.
	*/
	inline ConstIterator upperBound(size_t i, size_t j) const
	{
		METRIC_NUMERIC_CONSTRAINT_MUST_NOT_REQUIRE_EVALUATION(MT);
		return sm_.upperBound(i, j);
	}
	//**********************************************************************************************

	//**Operand access******************************************************************************
	/*!\brief Returns the sparse matrix operand.
	//
	// \return The sparse matrix operand.
	*/
	inline Operand operand() const noexcept { return sm_; }
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Returns whether the expression can alias with the given address \a alias.
	//
	// \param alias The alias to be checked.
	// \return \a true in case the expression can alias, \a false otherwise.
	*/
	template <typename T> inline bool canAlias(const T *alias) const noexcept
	{
		MAYBE_UNUSED(alias);
		return false;
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Returns whether the expression is aliased with the given address \a alias.
	//
	// \param alias The alias to be checked.
	// \return \a true in case an alias effect is detected, \a false otherwise.
	*/
	template <typename T> inline bool isAliased(const T *alias) const noexcept
	{
		MAYBE_UNUSED(alias);
		return false;
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Returns whether the expression can be used in SMP assignments.
	//
	// \return \a true in case the expression can be used in SMP assignments, \a false if not.
	*/
	inline bool canSMPAssign() const noexcept { return sm_.canSMPAssign(); }
	//**********************************************************************************************

  private:
	//**Member variables****************************************************************************
	Operand sm_; //!< Sparse matrix of the no-alias expression.
	//**********************************************************************************************

	//**Assignment to dense matrices****************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Assignment of a sparse matrix no-alias expression to a dense matrix.
	// \ingroup sparse_matrix
	//
	// \param lhs The target left-hand side dense matrix.
	// \param rhs The right-hand side no-alias expression to be assigned.
	// \return void
	//
	// This function implements the performance optimized assignment of a sparse matrix no-alias
	// expression to a dense matrix. Due to the explicit application of the SFINAE principle, this
	// function can only be selected by the compiler in case the operand requires an intermediate
	// evaluation.
	*/
	template <typename MT2 // Type of the target dense matrix
			  ,
			  bool SO2> // Storage order of the target dense matrix
	friend inline void assign(DenseMatrix<MT2, SO2> &lhs, const SMatNoAliasExpr &rhs)
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).rows() == rhs.rows(), "Invalid number of rows");
		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).columns() == rhs.columns(), "Invalid number of columns");

		assign(*lhs, rhs.sm_);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**Assignment to sparse matrices***************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Assignment of a sparse matrix no-alias expression to a sparse matrix.
	// \ingroup sparse_matrix
	//
	// \param lhs The target left-hand side sparse matrix.
	// \param rhs The right-hand side no-alias expression to be assigned.
	// \return void
	//
	// This function implements the performance optimized assignment of a sparse matrix no-alias
	// expression to a sparse matrix. Due to the explicit application of the SFINAE principle,
	// this function can only be selected by the compiler in case the operand requires an
	// intermediate evaluation.
	*/
	template <typename MT2 // Type of the target sparse matrix
			  ,
			  bool SO2> // Storage order of the target sparse matrix
	friend inline void assign(SparseMatrix<MT2, SO2> &lhs, const SMatNoAliasExpr &rhs)
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).rows() == rhs.rows(), "Invalid number of rows");
		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).columns() == rhs.columns(), "Invalid number of columns");

		assign(*lhs, rhs.sm_);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**Addition assignment to dense matrices*******************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Addition assignment of a sparse matrix no-alias expression to a dense matrix.
	// \ingroup sparse_matrix
	//
	// \param lhs The target left-hand side dense matrix.
	// \param rhs The right-hand side no-alias expression to be added.
	// \return void
	//
	// This function implements the performance optimized addition assignment of a sparse matrix
	// no-alias expression to a dense matrix. Due to the explicit application of the SFINAE
	// principle, this function can only be selected by the compiler in case the operand
	// requires an intermediate evaluation.
	*/
	template <typename MT2 // Type of the target dense matrix
			  ,
			  bool SO2> // Storage order of the target dense matrix
	friend inline void addAssign(DenseMatrix<MT2, SO2> &lhs, const SMatNoAliasExpr &rhs)
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).rows() == rhs.rows(), "Invalid number of rows");
		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).columns() == rhs.columns(), "Invalid number of columns");

		addAssign(*lhs, rhs.sm_);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**Addition assignment to sparse matrices******************************************************
	// No special implementation for the addition assignment to sparse matrices.
	//**********************************************************************************************

	//**Subtraction assignment to dense matrices****************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Subtraction assignment of a sparse matrix no-alias expression to a dense matrix.
	// \ingroup sparse_matrix
	//
	// \param lhs The target left-hand side dense matrix.
	// \param rhs The right-hand side no-alias expression to be subtracted.
	// \return void
	//
	// This function implements the performance optimized subtraction assignment of a sparse
	// matrix no-alias expression to a dense matrix. Due to the explicit application of the
	// SFINAE principle, this function can only be selected by the compiler in case the
	// operand requires an intermediate evaluation.
	*/
	template <typename MT2 // Type of the target dense matrix
			  ,
			  bool SO2> // Storage order of the target dense matrix
	friend inline void subAssign(DenseMatrix<MT2, SO2> &lhs, const SMatNoAliasExpr &rhs)
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).rows() == rhs.rows(), "Invalid number of rows");
		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).columns() == rhs.columns(), "Invalid number of columns");

		subAssign(*lhs, rhs.sm_);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**Subtraction assignment to sparse matrices***************************************************
	// No special implementation for the subtraction assignment to sparse matrices.
	//**********************************************************************************************

	//**Schur product assignment to dense matrices**************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Schur product assignment of a sparse matrix no-alias expression to a dense matrix.
	// \ingroup sparse_matrix
	//
	// \param lhs The target left-hand side dense matrix.
	// \param rhs The right-hand side no-alias expression for the Schur product.
	// \return void
	//
	// This function implements the performance optimized Schur product assignment of a sparse
	// matrix no-alias expression to a dense matrix. Due to the explicit application of the
	// SFINAE principle, this function can only be selected by the compiler in case the
	// operand requires an intermediate evaluation.
	*/
	template <typename MT2 // Type of the target dense matrix
			  ,
			  bool SO2> // Storage order of the target dense matrix
	friend inline void schurAssign(DenseMatrix<MT2, SO2> &lhs, const SMatNoAliasExpr &rhs)
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).rows() == rhs.rows(), "Invalid number of rows");
		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).columns() == rhs.columns(), "Invalid number of columns");

		schurAssign(*lhs, rhs.sm_);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**Schur product assignment to sparse matrices*************************************************
	// No special implementation for the Schur product assignment to sparse matrices.
	//**********************************************************************************************

	//**Multiplication assignment to dense matrices*************************************************
	// No special implementation for the multiplication assignment to dense matrices.
	//**********************************************************************************************

	//**Multiplication assignment to sparse matrices************************************************
	// No special implementation for the multiplication assignment to sparse matrices.
	//**********************************************************************************************

	//**SMP assignment to dense matrices************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief SMP assignment of a sparse matrix no-alias expression to a dense matrix.
	// \ingroup sparse_matrix
	//
	// \param lhs The target left-hand side dense matrix.
	// \param rhs The right-hand side no-alias expression to be assigned.
	// \return void
	//
	// This function implements the performance optimized SMP assignment of a sparse matrix
	// no-alias expression to a dense matrix. Due to the explicit application of the SFINAE
	// principle, this function can only be selected by the compiler in case the expression
	// specific parallel evaluation strategy is selected.
	*/
	template <typename MT2 // Type of the target dense matrix
			  ,
			  bool SO2> // Storage order of the target dense matrix
	friend inline void smpAssign(DenseMatrix<MT2, SO2> &lhs, const SMatNoAliasExpr &rhs)
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).rows() == rhs.rows(), "Invalid number of rows");
		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).columns() == rhs.columns(), "Invalid number of columns");

		smpAssign(*lhs, rhs.sm_);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**SMP assignment to sparse matrices***********************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief SMP assignment of a sparse matrix no-alias expression to a sparse matrix.
	// \ingroup sparse_matrix
	//
	// \param lhs The target left-hand side sparse matrix.
	// \param rhs The right-hand side no-alias expression to be assigned.
	// \return void
	//
	// This function implements the performance optimized SMP assignment of a sparse matrix
	// no-alias expression to a sparse matrix. Due to the explicit application of the SFINAE
	// principle, this function can only be selected by the compiler in case the expression
	// specific parallel evaluation strategy is selected.
	*/
	template <typename MT2 // Type of the target sparse matrix
			  ,
			  bool SO2> // Storage order of the target sparse matrix
	friend inline void smpAssign(SparseMatrix<MT2, SO2> &lhs, const SMatNoAliasExpr &rhs)
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).rows() == rhs.rows(), "Invalid number of rows");
		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).columns() == rhs.columns(), "Invalid number of columns");

		smpAssign(*lhs, rhs.sm_);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**SMP addition assignment to dense matrices***************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief SMP addition assignment of a sparse matrix no-alias expression to a dense matrix.
	// \ingroup sparse_matrix
	//
	// \param lhs The target left-hand side dense matrix.
	// \param rhs The right-hand side no-alias expression to be added.
	// \return void
	//
	// This function implements the performance optimized SMP addition assignment of a sparse
	// matrix no-alias expression to a dense matrix. Due to the explicit application of the
	// SFINAE principle, this function can only be selected by the compiler in case the
	// expression specific parallel evaluation strategy is selected.
	*/
	template <typename MT2 // Type of the target dense matrix
			  ,
			  bool SO2> // Storage order of the target dense matrix
	friend inline void smpAddAssign(DenseMatrix<MT2, SO2> &lhs, const SMatNoAliasExpr &rhs)
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).rows() == rhs.rows(), "Invalid number of rows");
		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).columns() == rhs.columns(), "Invalid number of columns");

		smpAddAssign(*lhs, rhs.sm_);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**SMP addition assignment to sparse matrices**************************************************
	// No special implementation for the SMP addition assignment to sparse matrices.
	//**********************************************************************************************

	//**SMP subtraction assignment to dense matrices************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief SMP subtraction assignment of a sparse matrix no-alias expression to a dense matrix.
	// \ingroup sparse_matrix
	//
	// \param lhs The target left-hand side dense matrix.
	// \param rhs The right-hand side no-alias expression to be subtracted.
	// \return void
	//
	// This function implements the performance optimized SMP subtraction assignment of a sparse
	// matrix no-alias expression to a dense matrix. Due to the explicit application of the
	// SFINAE principle, this function can only be selected by the compiler in case the
	// expression specific parallel evaluation strategy is selected.
	*/
	template <typename MT2 // Type of the target dense matrix
			  ,
			  bool SO2> // Storage order of the target dense matrix
	friend inline void smpSubAssign(DenseMatrix<MT2, SO2> &lhs, const SMatNoAliasExpr &rhs)
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).rows() == rhs.rows(), "Invalid number of rows");
		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).columns() == rhs.columns(), "Invalid number of columns");

		smpSubAssign(*lhs, rhs.sm_);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**SMP subtraction assignment to sparse matrices***********************************************
	// No special implementation for the SMP subtraction assignment to sparse matrices.
	//**********************************************************************************************

	//**SMP Schur product assignment to dense matrices**********************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief SMP Schur product assignment of a sparse matrix no-alias expression to a dense
	//        matrix.
	// \ingroup sparse_matrix
	//
	// \param lhs The target left-hand side dense matrix.
	// \param rhs The right-hand side no-alias expression for the Schur product.
	// \return void
	//
	// This function implements the performance optimized SMP Schur product assignment of a sparse
	// matrix no-alias expression to a dense matrix. Due to the explicit application of the
	// SFINAE principle, this function can only be selected by the compiler in case the
	// expression specific parallel evaluation strategy is selected.
	*/
	template <typename MT2 // Type of the target dense matrix
			  ,
			  bool SO2> // Storage order of the target dense matrix
	friend inline void smpSchurAssign(DenseMatrix<MT2, SO2> &lhs, const SMatNoAliasExpr &rhs)
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).rows() == rhs.rows(), "Invalid number of rows");
		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).columns() == rhs.columns(), "Invalid number of columns");

		smpSchurAssign(*lhs, rhs.sm_);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**SMP Schur product assignment to sparse matrices*********************************************
	// No special implementation for the SMP Schur product assignment to sparse matrices.
	//**********************************************************************************************

	//**SMP multiplication assignment to dense matrices*********************************************
	// No special implementation for the SMP multiplication assignment to dense matrices.
	//**********************************************************************************************

	//**SMP multiplication assignment to sparse matrices********************************************
	// No special implementation for the SMP multiplication assignment to sparse matrices.
	//**********************************************************************************************

	//**Compile time checks*************************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_SPARSE_MATRIX_TYPE(MT);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_MATRIX_WITH_STORAGE_ORDER(MT, SO);
	/*! \endcond */
	//**********************************************************************************************
};
//*************************************************************************************************

//=================================================================================================
//
//  GLOBAL FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Forces the non-aliased evaluation of the given sparse matrix expression \a sm.
// \ingroup sparse_matrix
//
// \param sm The input matrix.
// \return The non-aliased sparse matrix.
//
// The \a noalias function forces the non-aliased evaluation of the given sparse matrix expression
// \a sm. The function returns an expression representing the operation.\n
// The following example demonstrates the use of the \a noalias function:

   \code
   mtrc::numeric::CompressedMatrix<double> A, B;
   // ... Resizing and initialization
   B = noalias( A );
   \endcode
*/
template <typename MT // Type of the sparse matrix
		  ,
		  bool SO> // Storage order
inline decltype(auto) noalias(const SparseMatrix<MT, SO> &sm)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	using ReturnType = const SMatNoAliasExpr<MT, SO>;
	return ReturnType(*sm);
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
