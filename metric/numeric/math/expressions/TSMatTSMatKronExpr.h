// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_TSMATTSMATKRONEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_TSMATTSMATKRONEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/Aliases.h>
#include <metric/numeric/math/Exception.h>
#include <metric/numeric/math/constraints/ColumnMajorMatrix.h>
#include <metric/numeric/math/constraints/Identity.h>
#include <metric/numeric/math/constraints/MatMatKronExpr.h>
#include <metric/numeric/math/constraints/SparseMatrix.h>
#include <metric/numeric/math/constraints/StorageOrder.h>
#include <metric/numeric/math/constraints/Zero.h>
#include <metric/numeric/math/expressions/Computation.h>
#include <metric/numeric/math/expressions/MatMatKronExpr.h>
#include <metric/numeric/math/expressions/SparseMatrix.h>
#include <metric/numeric/math/shims/Reset.h>
#include <metric/numeric/math/shims/Serial.h>
#include <metric/numeric/math/traits/KronTrait.h>
#include <metric/numeric/math/typetraits/IsExpression.h>
#include <metric/numeric/math/typetraits/IsIdentity.h>
#include <metric/numeric/math/typetraits/IsTemporary.h>
#include <metric/numeric/math/typetraits/IsZero.h>
#include <metric/numeric/util/Assert.h>
#include <metric/numeric/util/EnableIf.h>
#include <metric/numeric/util/FunctionTrace.h>
#include <metric/numeric/util/Types.h>
#include <metric/numeric/util/mpl/If.h>
#include <utility>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS TSMATTSMATKRONEXPR
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Expression object for transpose sparse matrix-transpose sparse matrix Kronecker product.
// \ingroup sparse_matrix_expression
//
// The TSMatTSMatKronExpr class represents the compile time expression for Kronecker products
// between two column-major sparse matrices.
*/
template <typename MT1 // Type of the left-hand side sparse matrix
		  ,
		  typename MT2> // Type of the right-hand side sparse matrix
class TSMatTSMatKronExpr : public MatMatKronExpr<SparseMatrix<TSMatTSMatKronExpr<MT1, MT2>, true>>,
						   private Computation {
  private:
	//**Type definitions****************************************************************************
	using RT1 = ResultType_t<MT1>;	  //!< Result type of the left-hand side sparse matrix expression.
	using RT2 = ResultType_t<MT2>;	  //!< Result type of the right-hand side sparse matrix expression.
	using RN1 = ReturnType_t<MT1>;	  //!< Return type of the left-hand side sparse matrix expression.
	using RN2 = ReturnType_t<MT2>;	  //!< Return type of the right-hand side sparse matrix expression.
	using CT1 = CompositeType_t<MT1>; //!< Composite type of the left-hand side sparse matrix expression.
	using CT2 = CompositeType_t<MT2>; //!< Composite type of the right-hand side sparse matrix expression.
	//**********************************************************************************************

	//**Return type evaluation**********************************************************************
	//! Compilation switch for the selection of the subscript operator return type.
	/*! The \a returnExpr compile time constant expression is a compilation switch for the
		selection of the \a ReturnType. If either matrix operand returns a temporary vector
		or matrix, \a returnExpr will be set to \a false and the subscript operator will
		return it's result by value. Otherwise \a returnExpr will be set to \a true and
		the subscript operator may return it's result as an expression. */
	static constexpr bool returnExpr = (!IsTemporary_v<RN1> && !IsTemporary_v<RN2>);

	//! Expression return type for the subscript operator.
	using ExprReturnType = decltype(std::declval<RN1>() * std::declval<RN2>());
	//**********************************************************************************************

  public:
	//**Type definitions****************************************************************************
	//! Type of this TSMatTSMatKronExpr instance.
	using This = TSMatTSMatKronExpr<MT1, MT2>;

	//! Base type of this TSMatTSMatKronExpr instance.
	using BaseType = MatMatKronExpr<SparseMatrix<This, true>>;

	using ResultType = KronTrait_t<RT1, RT2>; //!< Result type for expression template evaluations.
	using OppositeType =
		OppositeType_t<ResultType>; //!< Result type with opposite storage order for expression template evaluations.
	using TransposeType = TransposeType_t<ResultType>; //!< Transpose type for expression template evaluations.
	using ElementType = ElementType_t<ResultType>;	   //!< Resulting element type.

	//! Return type for expression template evaluations.
	using ReturnType = const If_t<returnExpr, ExprReturnType, ElementType>;

	//! Data type for composite expression templates.
	using CompositeType = const ResultType;

	//! Composite type of the left-hand side sparse matrix expression.
	using LeftOperand = If_t<IsExpression_v<MT1>, const MT1, const MT1 &>;

	//! Composite type of the right-hand side sparse matrix expression.
	using RightOperand = If_t<IsExpression_v<MT2>, const MT2, const MT2 &>;
	//**********************************************************************************************

	//**Compilation flags***************************************************************************
	//! Compilation switch for the expression template assignment strategy.
	static constexpr bool smpAssignable = false;
	//**********************************************************************************************

	//**Constructor*********************************************************************************
	/*!\brief Constructor for the TSMatTSMatKronExpr class.
	//
	// \param lhs The left-hand side sparse matrix operand of the Kronecker product expression.
	// \param rhs The right-hand side sparse matrix operand of the Kronecker product expression.
	*/
	inline TSMatTSMatKronExpr(const MT1 &lhs, const MT2 &rhs) noexcept
		: lhs_(lhs) // Left-hand side sparse matrix of the Kronecker product expression
		  ,
		  rhs_(rhs) // Right-hand side sparse matrix of the Kronecker product expression
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
	inline ReturnType operator()(size_t i, size_t j) const
	{
		METRIC_NUMERIC_INTERNAL_ASSERT(i < rows(), "Invalid row access index");
		METRIC_NUMERIC_INTERNAL_ASSERT(j < columns(), "Invalid column access index");
		return lhs_(i / rhs_.rows(), j / rhs_.columns()) * rhs_(i % rhs_.rows(), j % rhs_.columns());
	}
	//**********************************************************************************************

	//**At function*********************************************************************************
	/*!\brief Checked access to the matrix elements.
	//
	// \param i Access index for the row. The index has to be in the range \f$[0..M-1]\f$.
	// \param j Access index for the column. The index has to be in the range \f$[0..N-1]\f$.
	// \return The resulting value.
	// \exception std::out_of_range Invalid matrix access index.
	*/
	inline ReturnType at(size_t i, size_t j) const
	{
		if (i >= rows()) {
			METRIC_NUMERIC_THROW_OUT_OF_RANGE("Invalid row access index");
		}
		if (j >= columns()) {
			METRIC_NUMERIC_THROW_OUT_OF_RANGE("Invalid column access index");
		}
		return (*this)(i, j);
	}
	//**********************************************************************************************

	//**Rows function*******************************************************************************
	/*!\brief Returns the current number of rows of the matrix.
	//
	// \return The number of rows of the matrix.
	*/
	inline size_t rows() const noexcept { return lhs_.rows() * rhs_.rows(); }
	//**********************************************************************************************

	//**Columns function****************************************************************************
	/*!\brief Returns the current number of columns of the matrix.
	//
	// \return The number of columns of the matrix.
	*/
	inline size_t columns() const noexcept { return lhs_.columns() * rhs_.columns(); }
	//**********************************************************************************************

	//**NonZeros function***************************************************************************
	/*!\brief Returns the number of non-zero elements in the sparse matrix.
	//
	// \return The number of non-zero elements in the sparse matrix.
	*/
	inline size_t nonZeros() const { return lhs_.nonZeros() * rhs_.nonZeros(); }
	//**********************************************************************************************

	//**NonZeros function***************************************************************************
	/*!\brief Returns the number of non-zero elements in the specified row.
	//
	// \param i The index of the row.
	// \return The number of non-zero elements of row \a i.
	*/
	inline size_t nonZeros(size_t i) const
	{
		return lhs_.nonZeros(i / rhs_.columns()) * rhs_.nonZeros(i % rhs_.columns());
	}
	//**********************************************************************************************

	//**Left operand access*************************************************************************
	/*!\brief Returns the left-hand side sparse matrix operand.
	//
	// \return The left-hand side sparse matrix operand.
	*/
	inline LeftOperand leftOperand() const noexcept { return lhs_; }
	//**********************************************************************************************

	//**Right operand access************************************************************************
	/*!\brief Returns the right-hand side sparse matrix operand.
	//
	// \return The right-hand side sparse matrix operand.
	*/
	inline RightOperand rightOperand() const noexcept { return rhs_; }
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Returns whether the expression can alias with the given address \a alias.
	//
	// \param alias The alias to be checked.
	// \return \a true in case the expression can alias effect is detected, \a false otherwise.
	*/
	template <typename T> inline bool canAlias(const T *alias) const noexcept
	{
		return (lhs_.canAlias(alias) || rhs_.canAlias(alias));
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
		return (lhs_.isAliased(alias) || rhs_.isAliased(alias));
	}
	//**********************************************************************************************

  private:
	//**Member variables****************************************************************************
	LeftOperand lhs_;  //!< Left-hand side sparse matrix of the Kronecker product expression.
	RightOperand rhs_; //!< Right-hand side sparse matrix of the Kronecker product expression.
	//**********************************************************************************************

	//**Assignment to dense matrices****************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Assignment of a transpose sparse matrix-transpose sparse matrix Kronecker product to
	//        a dense matrix.
	// \ingroup sparse_matrix
	//
	// \param lhs The target left-hand side dense matrix.
	// \param rhs The right-hand side Kronecker product expression to be assigned.
	// \return void
	//
	// This function implements the performance optimized assignment of a transpose sparse matrix-
	// transpose sparse matrix Kronecker product expression to a dense matrix.
	*/
	template <typename MT // Type of the target dense matrix
			  ,
			  bool SO2> // Storage order of the target dense matrix
	friend inline void assign(DenseMatrix<MT, SO2> &lhs, const TSMatTSMatKronExpr &rhs)
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).rows() == rhs.rows(), "Invalid number of rows");
		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).columns() == rhs.columns(), "Invalid number of columns");

		if (rhs.rows() == 0UL || rhs.columns() == 0UL) {
			return;
		}

		CT1 A(serial(rhs.lhs_)); // Evaluation of the left-hand side sparse matrix operand
		CT2 B(serial(rhs.rhs_)); // Evaluation of the right-hand side sparse matrix operand

		METRIC_NUMERIC_INTERNAL_ASSERT(A.rows() == rhs.lhs_.rows(), "Invalid number of rows");
		METRIC_NUMERIC_INTERNAL_ASSERT(A.columns() == rhs.lhs_.columns(), "Invalid number of columns");
		METRIC_NUMERIC_INTERNAL_ASSERT(B.rows() == rhs.rhs_.rows(), "Invalid number of rows");
		METRIC_NUMERIC_INTERNAL_ASSERT(B.columns() == rhs.rhs_.columns(), "Invalid number of columns");

		const size_t M(B.rows());
		const size_t N(B.columns());

		for (size_t j = 0UL; j < A.columns(); ++j)
			for (size_t l = 0UL; l < N; ++l)
				for (auto aelem = A.begin(j); aelem != A.end(j); ++aelem)
					for (auto belem = B.begin(l); belem != B.end(l); ++belem)
						(*lhs)(aelem->index() * M + belem->index(), j * N + l) = aelem->value() * belem->value();
	}
	/*! \endcond */
	//**********************************************************************************************

	//**Assignment to row-major sparse matrices*****************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Assignment of a transpose sparse matrix-transpose sparse matrix Kronecker product to
	//        a row-major sparse matrix.
	// \ingroup sparse_matrix
	//
	// \param lhs The target left-hand side sparse matrix.
	// \param rhs The right-hand side Kronecker product expression to be assigned.
	// \return void
	//
	// This function implements the performance optimized assignment of a transpose sparse matrix-
	// transpose sparse matrix Kronecker product expression to a row-major sparse matrix.
	*/
	template <typename MT> // Type of the target sparse matrix
	friend inline void assign(SparseMatrix<MT, false> &lhs, const TSMatTSMatKronExpr &rhs)
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).rows() == rhs.rows(), "Invalid number of rows");
		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).columns() == rhs.columns(), "Invalid number of columns");

		if (rhs.rows() == 0UL || rhs.columns() == 0UL) {
			return;
		}

		CT1 A(serial(rhs.lhs_)); // Evaluation of the left-hand side sparse matrix operand
		CT2 B(serial(rhs.rhs_)); // Evaluation of the right-hand side sparse matrix operand

		METRIC_NUMERIC_INTERNAL_ASSERT(A.rows() == rhs.lhs_.rows(), "Invalid number of rows");
		METRIC_NUMERIC_INTERNAL_ASSERT(A.columns() == rhs.lhs_.columns(), "Invalid number of columns");
		METRIC_NUMERIC_INTERNAL_ASSERT(B.rows() == rhs.rhs_.rows(), "Invalid number of rows");
		METRIC_NUMERIC_INTERNAL_ASSERT(B.columns() == rhs.rhs_.columns(), "Invalid number of columns");

		const size_t M(B.rows());
		const size_t N(B.columns());

		// Counting the number of elements per row in A
		std::vector<size_t> lnonzeros(A.rows(), 0UL);
		for (size_t j = 0UL; j < A.columns(); ++j) {
			const auto end(A.end(j));
			for (auto aelem = A.begin(j); aelem != end; ++aelem) {
				++lnonzeros[aelem->index()];
			}
		}

		// Counting the number of elements per row in B
		std::vector<size_t> rnonzeros(M, 0UL);
		for (size_t j = 0UL; j < N; ++j) {
			const auto end(B.end(j));
			for (auto belem = B.begin(j); belem != end; ++belem) {
				++rnonzeros[belem->index()];
			}
		}

		// Resizing the left-hand side sparse matrix
		for (size_t i = 0UL; i < A.rows(); ++i) {
			for (size_t j = 0UL; j < M; ++j) {
				(*lhs).reserve(i * M + j, lnonzeros[i] * rnonzeros[j]);
			}
		}

		// Performing the Kronecker product
		for (size_t j = 0UL; j < A.columns(); ++j)
			for (size_t l = 0UL; l < N; ++l)
				for (auto aelem = A.begin(j); aelem != A.end(j); ++aelem)
					for (auto belem = B.begin(l); belem != B.end(l); ++belem)
						(*lhs).append(aelem->index() * M + belem->index(), j * N + l, aelem->value() * belem->value(),
									  true);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**Assignment to column-major sparse matrices**************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Assignment of a transpose sparse matrix-transpose sparse matrix Kronecker product
	//        to a column-major sparse matrix.
	// \ingroup sparse_matrix
	//
	// \param lhs The target left-hand side sparse matrix.
	// \param rhs The right-hand side Kronecker product expression to be assigned.
	// \return void
	//
	// This function implements the performance optimized assignment of a transpose sparse matrix-
	// transpose sparse matrix Kronecker product expression to a column-major sparse matrix.
	*/
	template <typename MT> // Type of the target sparse matrix
	friend inline void assign(SparseMatrix<MT, true> &lhs, const TSMatTSMatKronExpr &rhs)
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).rows() == rhs.rows(), "Invalid number of rows");
		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).columns() == rhs.columns(), "Invalid number of columns");

		if (rhs.rows() == 0UL || rhs.columns() == 0UL) {
			return;
		}

		CT1 A(serial(rhs.lhs_)); // Evaluation of the left-hand side sparse matrix operand
		CT2 B(serial(rhs.rhs_)); // Evaluation of the right-hand side sparse matrix operand

		METRIC_NUMERIC_INTERNAL_ASSERT(A.rows() == rhs.lhs_.rows(), "Invalid number of rows");
		METRIC_NUMERIC_INTERNAL_ASSERT(A.columns() == rhs.lhs_.columns(), "Invalid number of columns");
		METRIC_NUMERIC_INTERNAL_ASSERT(B.rows() == rhs.rhs_.rows(), "Invalid number of rows");
		METRIC_NUMERIC_INTERNAL_ASSERT(B.columns() == rhs.rhs_.columns(), "Invalid number of columns");

		const size_t M(B.rows());
		const size_t N(B.columns());

		for (size_t j = 0UL; j < A.columns(); ++j) {
			for (size_t l = 0UL; l < N; ++l) {
				for (auto aelem = A.begin(j); aelem != A.end(j); ++aelem)
					for (auto belem = B.begin(l); belem != B.end(l); ++belem)
						(*lhs).append(aelem->index() * M + belem->index(), j * N + l, aelem->value() * belem->value(),
									  true);
				(*lhs).finalize(j * N + l);
			}
		}
	}
	/*! \endcond */
	//**********************************************************************************************

	//**Addition assignment to dense matrices*******************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Addition assignment of a transpose sparse matrix-transpose sparse matrix Kronecker
	//        product to a dense matrix.
	// \ingroup sparse_matrix
	//
	// \param lhs The target left-hand side dense matrix.
	// \param rhs The right-hand side Kronecker product expression to be added.
	// \return void
	//
	// This function implements the performance optimized addition assignment of a transpose sparse
	// matrix-transpose sparse matrix Kronecker product expression to a dense matrix.
	*/
	template <typename MT // Type of the target dense matrix
			  ,
			  bool SO2> // Storage order of the target dense matrix
	friend inline void addAssign(DenseMatrix<MT, SO2> &lhs, const TSMatTSMatKronExpr &rhs)
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).rows() == rhs.rows(), "Invalid number of rows");
		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).columns() == rhs.columns(), "Invalid number of columns");

		if (rhs.rows() == 0UL || rhs.columns() == 0UL) {
			return;
		}

		CT1 A(serial(rhs.lhs_)); // Evaluation of the left-hand side sparse matrix operand
		CT2 B(serial(rhs.rhs_)); // Evaluation of the right-hand side sparse matrix operand

		METRIC_NUMERIC_INTERNAL_ASSERT(A.rows() == rhs.lhs_.rows(), "Invalid number of rows");
		METRIC_NUMERIC_INTERNAL_ASSERT(A.columns() == rhs.lhs_.columns(), "Invalid number of columns");
		METRIC_NUMERIC_INTERNAL_ASSERT(B.rows() == rhs.rhs_.rows(), "Invalid number of rows");
		METRIC_NUMERIC_INTERNAL_ASSERT(B.columns() == rhs.rhs_.columns(), "Invalid number of columns");

		const size_t M(B.rows());
		const size_t N(B.columns());

		for (size_t j = 0UL; j < A.columns(); ++j)
			for (size_t l = 0UL; l < N; ++l)
				for (auto aelem = A.begin(j); aelem != A.end(j); ++aelem)
					for (auto belem = B.begin(l); belem != B.end(l); ++belem)
						(*lhs)(aelem->index() * M + belem->index(), j * N + l) += aelem->value() * belem->value();
	}
	/*! \endcond */
	//**********************************************************************************************

	//**Addition assignment to sparse matrices******************************************************
	// No special implementation for the addition assignment to sparse matrices.
	//**********************************************************************************************

	//**Subtraction assignment to dense matrices****************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Subtraction assignment of a transpose sparse matrix-transpose sparse matrix Kronecker
	//        product to a dense matrix.
	// \ingroup sparse_matrix
	//
	// \param lhs The target left-hand side dense matrix.
	// \param rhs The right-hand side Kronecker product expression to be subtracted.
	// \return void
	//
	// This function implements the performance optimized subtraction assignment of a transpose
	// sparse matrix-transpose sparse matrix Kronecker product expression to a dense matrix.
	*/
	template <typename MT // Type of the target dense matrix
			  ,
			  bool SO2> // Storage order of the target dense matrix
	friend inline void subAssign(DenseMatrix<MT, SO2> &lhs, const TSMatTSMatKronExpr &rhs)
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).rows() == rhs.rows(), "Invalid number of rows");
		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).columns() == rhs.columns(), "Invalid number of columns");

		if (rhs.rows() == 0UL || rhs.columns() == 0UL) {
			return;
		}

		CT1 A(serial(rhs.lhs_)); // Evaluation of the left-hand side sparse matrix operand
		CT2 B(serial(rhs.rhs_)); // Evaluation of the right-hand side sparse matrix operand

		METRIC_NUMERIC_INTERNAL_ASSERT(A.rows() == rhs.lhs_.rows(), "Invalid number of rows");
		METRIC_NUMERIC_INTERNAL_ASSERT(A.columns() == rhs.lhs_.columns(), "Invalid number of columns");
		METRIC_NUMERIC_INTERNAL_ASSERT(B.rows() == rhs.rhs_.rows(), "Invalid number of rows");
		METRIC_NUMERIC_INTERNAL_ASSERT(B.columns() == rhs.rhs_.columns(), "Invalid number of columns");

		const size_t M(B.rows());
		const size_t N(B.columns());

		for (size_t j = 0UL; j < A.columns(); ++j)
			for (size_t l = 0UL; l < N; ++l)
				for (auto aelem = A.begin(j); aelem != A.end(j); ++aelem)
					for (auto belem = B.begin(l); belem != B.end(l); ++belem)
						(*lhs)(aelem->index() * M + belem->index(), j * N + l) -= aelem->value() * belem->value();
	}
	/*! \endcond */
	//**********************************************************************************************

	//**Subtraction assignment to sparse matrices***************************************************
	// No special implementation for the subtraction assignment to sparse matrices.
	//**********************************************************************************************

	//**Schur product assignment to dense matrices**************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Schur product assignment of a transpose sparse matrix-transpose sparse matrix Kronecker
	//        product to a dense matrix.
	// \ingroup sparse_matrix
	//
	// \param lhs The target left-hand side dense matrix.
	// \param rhs The right-hand side Kronecker product expression for the Schur product.
	// \return void
	//
	// This function implements the performance optimized Schur product assignment of a transpose
	// sparse matrix-transpose sparse matrix Kronecker product expression to a dense matrix.
	*/
	template <typename MT // Type of the target dense matrix
			  ,
			  bool SO2> // Storage order of the target dense matrix
	friend inline void schurAssign(DenseMatrix<MT, SO2> &lhs, const TSMatTSMatKronExpr &rhs)
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).rows() == rhs.rows(), "Invalid number of rows");
		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).columns() == rhs.columns(), "Invalid number of columns");

		if (rhs.rows() == 0UL || rhs.columns() == 0UL) {
			return;
		}

		CT1 A(serial(rhs.lhs_)); // Evaluation of the left-hand side sparse matrix operand
		CT2 B(serial(rhs.rhs_)); // Evaluation of the right-hand side sparse matrix operand

		METRIC_NUMERIC_INTERNAL_ASSERT(A.rows() == rhs.lhs_.rows(), "Invalid number of rows");
		METRIC_NUMERIC_INTERNAL_ASSERT(A.columns() == rhs.lhs_.columns(), "Invalid number of columns");
		METRIC_NUMERIC_INTERNAL_ASSERT(B.rows() == rhs.rhs_.rows(), "Invalid number of rows");
		METRIC_NUMERIC_INTERNAL_ASSERT(B.columns() == rhs.rhs_.columns(), "Invalid number of columns");

		const size_t M(B.rows());
		const size_t N(B.columns());

		for (size_t j = 0UL; j < A.columns(); ++j) {
			for (size_t l = 0UL; l < N; ++l) {
				size_t i(0UL);

				for (auto aelem = A.begin(j); aelem != A.end(j); ++aelem) {
					for (auto belem = B.begin(l); belem != B.end(l); ++belem, ++i) {
						const size_t index(aelem->index() * M + belem->index());
						for (; i < index; ++i)
							reset((*lhs)(i, j * N + l));
						(*lhs)(i, j *N + l) *= aelem->value() * belem->value();
					}
				}

				for (; i < (*lhs).rows(); ++i)
					reset((*lhs)(i, j * N + l));
			}
		}
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

	//**Compile time checks*************************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_SPARSE_MATRIX_TYPE(MT1);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_COLUMN_MAJOR_MATRIX_TYPE(MT1);
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_ZERO_TYPE(MT1);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_SPARSE_MATRIX_TYPE(MT2);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_COLUMN_MAJOR_MATRIX_TYPE(MT2);
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_ZERO_TYPE(MT2);
	METRIC_NUMERIC_CONSTRAINT_MUST_FORM_VALID_MATMATKRONEXPR(MT1, MT2);
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
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Backend implementation of the Kronecker product between two column-major sparse matrices
//        (\f$ A=B \otimes C \f$).
// \ingroup sparse_matrix
//
// \param lhs The left-hand side sparse matrix for the Kronecker product.
// \param rhs The right-hand side sparse matrix for the Kronecker product.
// \return The Kronecker product of the two matrices.
//
// This function implements a performance optimized treatment of the Kronecker product between
// two column-major sparse matrices.
*/
template <typename MT1 // Type of the left-hand side sparse matrix
		  ,
		  typename MT2 // Type of the right-hand side sparse matrix
		  ,
		  DisableIf_t<(IsIdentity_v<MT1> && IsIdentity_v<MT2>) || (IsZero_v<MT1> || IsZero_v<MT2>)> * = nullptr>
inline const TSMatTSMatKronExpr<MT1, MT2> tsmattsmatkron(const SparseMatrix<MT1, true> &lhs,
														 const SparseMatrix<MT2, true> &rhs)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return TSMatTSMatKronExpr<MT1, MT2>(*lhs, *rhs);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Backend implementation of the Kronecker product between a two identity matrices
//        (\f$ A=B \otimes C \f$).
// \ingroup sparse_matrix
//
// \param lhs The left-hand side sparse matrix for the Kronecker product.
// \param rhs The right-hand side sparse matrix for the Kronecker product.
// \return The Kronecker product of the two matrices.
//
// This function implements a performance optimized treatment of the Kronecker product between
// two identity matrices. It returns an identity matrix.
*/
template <typename MT1 // Type of the left-hand side sparse matrix
		  ,
		  typename MT2 // Type of the right-hand side sparse matrix
		  ,
		  EnableIf_t<IsIdentity_v<MT1> && IsIdentity_v<MT2>> * = nullptr>
inline decltype(auto) tsmattsmatkron(const SparseMatrix<MT1, true> &lhs, const SparseMatrix<MT2, true> &rhs)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	using ReturnType = const KronTrait_t<ResultType_t<MT1>, ResultType_t<MT2>>;

	METRIC_NUMERIC_CONSTRAINT_MUST_BE_COLUMN_MAJOR_MATRIX_TYPE(ReturnType);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_IDENTITY_MATRIX_TYPE(ReturnType);

	return ReturnType((*lhs).rows() * (*rhs).rows());
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Backend implementation of the Kronecker product between a zero matrix and sparse matrix
//        (\f$ A=B \otimes C \f$).
// \ingroup sparse_matrix
//
// \param lhs The left-hand side sparse matrix for the Kronecker product.
// \param rhs The right-hand side sparse matrix for the Kronecker product.
// \return The Kronecker product of the two matrices.
//
// This function implements a performance optimized treatment of the Kronecker product between a
// zero matrix and a sparse matrix. It returns a zero matrix.
*/
template <typename MT1 // Type of the left-hand side sparse matrix
		  ,
		  typename MT2 // Type of the right-hand side sparse matrix
		  ,
		  EnableIf_t<IsZero_v<MT1> || IsZero_v<MT2>> * = nullptr>
inline decltype(auto) tsmattsmatkron(const SparseMatrix<MT1, true> &lhs, const SparseMatrix<MT2, true> &rhs)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	using ReturnType = const KronTrait_t<ResultType_t<MT1>, ResultType_t<MT2>>;

	METRIC_NUMERIC_CONSTRAINT_MUST_BE_COLUMN_MAJOR_MATRIX_TYPE(ReturnType);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_ZERO_TYPE(ReturnType);

	return ReturnType((*lhs).rows() * (*rhs).rows(), (*lhs).columns() * (*rhs).columns());
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Computes the Kronecker product of two column-major sparse matrices (\f$ A=B \otimes C \f$).
// \ingroup sparse_matrix
//
// \param lhs The left-hand side sparse matrix for the Kronecker product.
// \param rhs The right-hand side sparse matrix for the Kronecker product.
// \return The Kronecker product of the two matrices.
//
// The kron() function computes the Kronecker product of the two given column-major sparse matrices:

   \code
   using mtrc::numeric::columnMajor;

   mtrc::numeric::CompressedMatrix<double,columnMajor> A, B, C;
   // ... Resizing and initialization
   C = kron( A, B );
   \endcode

// The function returns an expression representing a sparse matrix of the higher-order element
// type of the two involved matrix element types \a MT1::ElementType and \a MT2::ElementType.
// Both matrix types \a MT1 and \a MT2 as well as the two element types \a MT1::ElementType
// and \a MT2::ElementType have to be supported by the MultTrait class template.
*/
template <typename MT1 // Type of the left-hand side sparse matrix
		  ,
		  typename MT2> // Type of the right-hand side sparse matrix
inline decltype(auto) kron(const SparseMatrix<MT1, true> &lhs, const SparseMatrix<MT2, true> &rhs)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return tsmattsmatkron(*lhs, *rhs);
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
