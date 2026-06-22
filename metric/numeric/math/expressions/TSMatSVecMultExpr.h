// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_TSMATSVECMULTEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_TSMATSVECMULTEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/Aliases.h>
#include <metric/numeric/math/Exception.h>
#include <metric/numeric/math/constraints/ColumnMajorMatrix.h>
#include <metric/numeric/math/constraints/ColumnVector.h>
#include <metric/numeric/math/constraints/MatMatMultExpr.h>
#include <metric/numeric/math/constraints/MatVecMultExpr.h>
#include <metric/numeric/math/constraints/RequiresEvaluation.h>
#include <metric/numeric/math/constraints/SparseMatrix.h>
#include <metric/numeric/math/constraints/SparseVector.h>
#include <metric/numeric/math/constraints/Zero.h>
#include <metric/numeric/math/dense/DynamicVector.h>
#include <metric/numeric/math/expressions/Computation.h>
#include <metric/numeric/math/expressions/Forward.h>
#include <metric/numeric/math/expressions/MatVecMultExpr.h>
#include <metric/numeric/math/expressions/SparseVector.h>
#include <metric/numeric/math/shims/IsDefault.h>
#include <metric/numeric/math/shims/Reset.h>
#include <metric/numeric/math/shims/Serial.h>
#include <metric/numeric/math/traits/MultTrait.h>
#include <metric/numeric/math/typetraits/IsComputation.h>
#include <metric/numeric/math/typetraits/IsDiagonal.h>
#include <metric/numeric/math/typetraits/IsExpression.h>
#include <metric/numeric/math/typetraits/IsIdentity.h>
#include <metric/numeric/math/typetraits/IsLower.h>
#include <metric/numeric/math/typetraits/IsResizable.h>
#include <metric/numeric/math/typetraits/IsStrictlyLower.h>
#include <metric/numeric/math/typetraits/IsStrictlyUpper.h>
#include <metric/numeric/math/typetraits/IsUpper.h>
#include <metric/numeric/math/typetraits/IsZero.h>
#include <metric/numeric/math/typetraits/RequiresEvaluation.h>
#include <metric/numeric/math/views/Check.h>
#include <metric/numeric/system/MacroDisable.h>
#include <metric/numeric/system/Thresholds.h>
#include <metric/numeric/util/Assert.h>
#include <metric/numeric/util/EnableIf.h>
#include <metric/numeric/util/FunctionTrace.h>
#include <metric/numeric/util/MaybeUnused.h>
#include <metric/numeric/util/Types.h>
#include <metric/numeric/util/mpl/If.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS SMATDVECMULTEXPR
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Expression object for sparse matrix-sparse vector multiplications.
// \ingroup sparse_vector_expression
//
// The TSMatSVecMultExpr class represents the compile time expression for multiplications
// between column-major sparse matrices and sparse vectors.
*/
template <typename MT // Type of the left-hand side sparse matrix
		  ,
		  typename VT> // Type of the right-hand side sparse vector
class TSMatSVecMultExpr : public MatVecMultExpr<SparseVector<TSMatSVecMultExpr<MT, VT>, false>>, private Computation {
  private:
	//**Type definitions****************************************************************************
	using MRT = ResultType_t<MT>;	 //!< Result type of the left-hand side sparse matrix expression.
	using VRT = ResultType_t<VT>;	 //!< Result type of the right-hand side sparse vector expression.
	using MCT = CompositeType_t<MT>; //!< Composite type of the left-hand side sparse matrix expression.
	using VCT = CompositeType_t<VT>; //!< Composite type of the right-hand side sparse vector expression.
	//**********************************************************************************************

	//**********************************************************************************************
	//! Compilation switch for the composite type of the left-hand side sparse matrix expression.
	static constexpr bool evaluateMatrix = RequiresEvaluation_v<MT>;
	//**********************************************************************************************

	//**********************************************************************************************
	//! Compilation switch for the composite type of the right-hand side sparse vector expression.
	static constexpr bool evaluateVector = (RequiresEvaluation_v<VT> || IsComputation_v<VT>);
	//**********************************************************************************************

	//**********************************************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	//! Helper variable template for the explicit application of the SFINAE principle.
	/*! This variable template is a helper for the selection of the parallel evaluation strategy.
		In case either the matrix or the vector operand requires an intermediate evaluation, the
		variable will be set to 1, otherwise it will be 0. */
	template <typename T1> static constexpr bool UseSMPAssign_v = (evaluateMatrix || evaluateVector);
	/*! \endcond */
	//**********************************************************************************************

  public:
	//**Type definitions****************************************************************************
	//! Type of this TSMatSVecMultExpr instance.
	using This = TSMatSVecMultExpr<MT, VT>;

	//! Base type of this TSMatSVecMultExpr instance.
	using BaseType = MatVecMultExpr<SparseVector<This, false>>;

	using ResultType = MultTrait_t<MRT, VRT>;		   //!< Result type for expression template evaluations.
	using TransposeType = TransposeType_t<ResultType>; //!< Transpose type for expression template evaluations.
	using ElementType = ElementType_t<ResultType>;	   //!< Resulting element type.
	using ReturnType = const ElementType;			   //!< Return type for expression template evaluations.

	//! Data type for composite expression templates.
	using CompositeType = const ResultType;

	//! Composite type of the left-hand side sparse matrix expression.
	using LeftOperand = If_t<IsExpression_v<MT>, const MT, const MT &>;

	//! Composite type of the right-hand side sparse vector expression.
	using RightOperand = If_t<IsExpression_v<VT>, const VT, const VT &>;

	//! Type for the assignment of the left-hand side sparse matrix operand.
	using LT = If_t<evaluateMatrix, const MRT, MCT>;

	//! Type for the assignment of the right-hand side sparse vector operand.
	using RT = If_t<evaluateVector, const VRT, VCT>;
	//**********************************************************************************************

	//**Compilation flags***************************************************************************
	//! Compilation switch for the expression template assignment strategy.
	static constexpr bool smpAssignable =
		(!evaluateMatrix && MT::smpAssignable && !evaluateVector && VT::smpAssignable);
	//**********************************************************************************************

	//**Constructor*********************************************************************************
	/*!\brief Constructor for the TSMatSVecMultExpr class.
	//
	// \param mat The left-hand side sparse matrix operand of the multiplication expression.
	// \param vec The right-hand side sparse vector operand of the multiplication expression.
	*/
	inline TSMatSVecMultExpr(const MT &mat, const VT &vec) noexcept
		: mat_(mat) // Left-hand side sparse matrix of the multiplication expression
		  ,
		  vec_(vec) // Right-hand side sparse vector of the multiplication expression
	{
		METRIC_NUMERIC_INTERNAL_ASSERT(mat_.columns() == vec_.size(), "Invalid matrix and vector sizes");
	}
	//**********************************************************************************************

	//**Subscript operator**************************************************************************
	/*!\brief Subscript operator for the direct access to the vector elements.
	//
	// \param index Access index. The index has to be in the range \f$[0..N-1]\f$.
	// \return The resulting value.
	*/
	inline ReturnType operator[](size_t index) const
	{
		METRIC_NUMERIC_INTERNAL_ASSERT(index < mat_.rows(), "Invalid vector access index");

		if (IsDiagonal_v<MT>) {
			return mat_(index, index) * vec_[index];
		} else if (IsLower_v<MT>) {
			const size_t n(IsStrictlyLower_v<MT> ? index : index + 1UL);
			return subvector(row(mat_, index, unchecked), 0UL, n, unchecked) * subvector(vec_, 0UL, n, unchecked);
		} else if (IsUpper_v<MT>) {
			const size_t begin(IsStrictlyUpper_v<MT> ? index + 1UL : index);
			const size_t n(mat_.columns() - begin);
			return subvector(row(mat_, index, unchecked), begin, n, unchecked) * subvector(vec_, begin, n, unchecked);
		} else {
			return row(mat_, index, unchecked) * vec_;
		}
	}
	//**********************************************************************************************

	//**At function*********************************************************************************
	/*!\brief Checked access to the vector elements.
	//
	// \param index Access index. The index has to be in the range \f$[0..N-1]\f$.
	// \return The resulting value.
	// \exception std::out_of_range Invalid vector access index.
	*/
	inline ReturnType at(size_t index) const
	{
		if (index >= mat_.rows()) {
			METRIC_NUMERIC_THROW_OUT_OF_RANGE("Invalid vector access index");
		}
		return (*this)[index];
	}
	//**********************************************************************************************

	//**Size function*******************************************************************************
	/*!\brief Returns the current size/dimension of the vector.
	//
	// \return The size of the vector.
	*/
	inline size_t size() const noexcept { return mat_.rows(); }
	//**********************************************************************************************

	//**NonZeros function***************************************************************************
	/*!\brief Returns an estimation for the number of non-zero elements in the sparse vector.
	//
	// \return The estimate for the number of non-zero elements in the sparse vector.
	*/
	inline size_t nonZeros() const { return mat_.rows(); }
	//**********************************************************************************************

	//**Left operand access*************************************************************************
	/*!\brief Returns the left-hand side transpose sparse matrix operand.
	//
	// \return The left-hand side transpose sparse matrix operand.
	*/
	inline LeftOperand leftOperand() const noexcept { return mat_; }
	//**********************************************************************************************

	//**Right operand function**********************************************************************
	/*!\brief Returns the right-hand side sparse vector operand.
	//
	// \return The right-hand side sparse vector operand.
	*/
	inline RightOperand rightOperand() const noexcept { return vec_; }
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Returns whether the expression can alias with the given address \a alias.
	//
	// \param alias The alias to be checked.
	// \return \a true in case the expression can alias, \a false otherwise.
	*/
	template <typename T> inline bool canAlias(const T *alias) const noexcept
	{
		return (mat_.isAliased(alias) || vec_.isAliased(alias));
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
		return (mat_.isAliased(alias) || vec_.isAliased(alias));
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Returns whether the expression can be used in SMP assignments.
	//
	// \return \a true in case the expression can be used in SMP assignments, \a false if not.
	*/
	inline bool canSMPAssign() const noexcept { return (size() > SMP_SMATSVECMULT_THRESHOLD); }
	//**********************************************************************************************

  private:
	//**Member variables****************************************************************************
	LeftOperand mat_;  //!< Left-hand side sparse matrix of the multiplication expression.
	RightOperand vec_; //!< Right-hand side sparse vector of the multiplication expression.
	//**********************************************************************************************

	//**Assignment to dense vectors*****************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Assignment of a transpose sparse matrix-sparse vector multiplication to a dense
	//        vector.
	// \ingroup sparse_vector
	//
	// \param lhs The target left-hand side dense vector.
	// \param rhs The right-hand side multiplication expression to be assigned.
	// \return void
	//
	// This function implements the performance optimized assignment of a transpose sparse matrix-
	// sparse vector multiplication expression to a dense vector.
	*/
	template <typename VT1> // Type of the target dense vector
	friend inline void assign(DenseVector<VT1, false> &lhs, const TSMatSVecMultExpr &rhs)
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == rhs.size(), "Invalid vector sizes");

		// Resetting the left-hand side target dense vector
		reset(*lhs);

		// Evaluation of the right-hand side sparse vector operand
		RT x(serial(rhs.vec_));
		if (x.nonZeros() == 0UL)
			return;

		// Evaluation of the left-hand side sparse matrix operand
		LT A(serial(rhs.mat_));

		// Checking the evaluated operators
		METRIC_NUMERIC_INTERNAL_ASSERT(A.rows() == rhs.mat_.rows(), "Invalid number of rows");
		METRIC_NUMERIC_INTERNAL_ASSERT(A.columns() == rhs.mat_.columns(), "Invalid number of columns");
		METRIC_NUMERIC_INTERNAL_ASSERT(x.size() == rhs.vec_.size(), "Invalid vector size");
		METRIC_NUMERIC_INTERNAL_ASSERT(A.rows() == (*lhs).size(), "Invalid vector size");

		// Performing the transpose sparse matrix-sparse vector multiplication
		TSMatSVecMultExpr::selectAssignKernel(*lhs, A, x);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**Default assignment to dense vectors*********************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Default assignment of a transpose sparse matrix-sparse vector multiplication
	//        (\f$ \vec{y}=A*\vec{x} \f$).
	// \ingroup sparse_vector
	//
	// \param y The target left-hand side dense vector.
	// \param A The left-hand side sparse matrix operand.
	// \param x The right-hand side sparse vector operand.
	// \return void
	//
	// This function implements the default assignment kernel for the transpose sparse matrix-
	// sparse vector multiplication.
	*/
	template <typename VT1 // Type of the left-hand side target vector
			  ,
			  typename MT1 // Type of the left-hand side matrix operand
			  ,
			  typename VT2> // Type of the right-hand side vector operand
	static inline void selectAssignKernel(VT1 &y, const MT1 &A, const VT2 &x)
	{
		const auto vend(x.end());
		auto velem(x.begin());

		for (; velem != vend; ++velem) {
			const auto mend(A.end(velem->index()));
			auto melem(A.begin(velem->index()));

			for (; melem != mend; ++melem) {
				if (IsResizable_v<ElementType_t<VT1>> && isDefault(y[melem->index()]))
					y[melem->index()] = melem->value() * velem->value();
				else
					y[melem->index()] += melem->value() * velem->value();
			}
		}
	}
	/*! \endcond */
	//**********************************************************************************************

	//**Assignment to sparse vectors****************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Assignment of a transpose sparse matrix-sparse vector multiplication to a sparse vector.
	// \ingroup sparse_vector
	//
	// \param lhs The target left-hand side sparse vector.
	// \param rhs The right-hand side multiplication expression to be assigned.
	// \return void
	//
	// This function implements the performance optimized assignment of a transpose sparse matrix-
	// sparse vector multiplication expression to a sparse vector.
	*/
	template <typename VT1> // Type of the target sparse vector
	friend inline void assign(SparseVector<VT1, false> &lhs, const TSMatSVecMultExpr &rhs)
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == rhs.size(), "Invalid vector sizes");

		const DynamicVector<ElementType_t<VT1>, false> tmp(serial(rhs));
		assign(*lhs, tmp);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**Addition assignment to dense vectors********************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Addition assignment of a transpose sparse matrix-sparse vector multiplication to a
	//        dense vector.
	// \ingroup sparse_vector
	//
	// \param lhs The target left-hand side dense vector.
	// \param rhs The right-hand side multiplication expression to be added.
	// \return void
	//
	// This function implements the performance optimized addition assignment of a transpose
	// sparse matrix-sparse vector multiplication expression to a dense vector.
	*/
	template <typename VT1> // Type of the target dense vector
	friend inline void addAssign(DenseVector<VT1, false> &lhs, const TSMatSVecMultExpr &rhs)
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == rhs.size(), "Invalid vector sizes");

		// Evaluation of the right-hand side sparse vector operand
		RT x(serial(rhs.vec_));
		if (x.nonZeros() == 0UL)
			return;

		// Evaluation of the left-hand side sparse matrix operand
		LT A(serial(rhs.mat_));

		// Checking the evaluated operators
		METRIC_NUMERIC_INTERNAL_ASSERT(A.rows() == rhs.mat_.rows(), "Invalid number of rows");
		METRIC_NUMERIC_INTERNAL_ASSERT(A.columns() == rhs.mat_.columns(), "Invalid number of columns");
		METRIC_NUMERIC_INTERNAL_ASSERT(x.size() == rhs.vec_.size(), "Invalid vector size");
		METRIC_NUMERIC_INTERNAL_ASSERT(A.rows() == (*lhs).size(), "Invalid vector size");

		// Performing the transpose sparse matrix-sparse vector multiplication
		TSMatSVecMultExpr::selectAddAssignKernel(*lhs, A, x);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**Default addition assignment to dense vectors************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Default addition assignment of a transpose sparse matrix-sparse vector multiplication
	//        (\f$ \vec{y}+=A*\vec{x} \f$).
	// \ingroup sparse_vector
	//
	// \param y The target left-hand side dense vector.
	// \param A The left-hand side sparse matrix operand.
	// \param x The right-hand side sparse vector operand.
	// \return void
	//
	// This function implements the default addition assignment kernel for the sparse matrix-sparse
	// vector multiplication.
	*/
	template <typename VT1 // Type of the left-hand side target vector
			  ,
			  typename MT1 // Type of the left-hand side matrix operand
			  ,
			  typename VT2> // Type of the right-hand side vector operand
	static inline void selectAddAssignKernel(VT1 &y, const MT1 &A, const VT2 &x)
	{
		const auto vend(x.end());
		auto velem(x.begin());

		for (; velem != vend; ++velem) {
			const auto mend(A.end(velem->index()));
			auto melem(A.begin(velem->index()));

			for (; melem != mend; ++melem) {
				y[melem->index()] += melem->value() * velem->value();
			}
		}
	}
	/*! \endcond */
	//**********************************************************************************************

	//**Addition assignment to sparse vectors*******************************************************
	// No special implementation for the addition assignment to sparse vectors.
	//**********************************************************************************************

	//**Subtraction assignment to dense vectors*****************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Subtraction assignment of a transpose sparse matrix-sparse vector multiplication to
	//        a dense vector.
	// \ingroup sparse_vector
	//
	// \param lhs The target left-hand side dense vector.
	// \param rhs The right-hand side multiplication expression to be subtracted.
	// \return void
	//
	// This function implements the performance optimized subtraction assignment of a transpose
	// sparse matrix-sparse vector multiplication expression to a dense vector.
	*/
	template <typename VT1> // Type of the target dense vector
	friend inline void subAssign(DenseVector<VT1, false> &lhs, const TSMatSVecMultExpr &rhs)
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == rhs.size(), "Invalid vector sizes");

		// Evaluation of the right-hand side sparse vector operand
		RT x(serial(rhs.vec_));
		if (x.nonZeros() == 0UL)
			return;

		// Evaluation of the left-hand side sparse matrix operand
		LT A(serial(rhs.mat_));

		// Checking the evaluated operators
		METRIC_NUMERIC_INTERNAL_ASSERT(A.rows() == rhs.mat_.rows(), "Invalid number of rows");
		METRIC_NUMERIC_INTERNAL_ASSERT(A.columns() == rhs.mat_.columns(), "Invalid number of columns");
		METRIC_NUMERIC_INTERNAL_ASSERT(x.size() == rhs.vec_.size(), "Invalid vector size");
		METRIC_NUMERIC_INTERNAL_ASSERT(A.rows() == (*lhs).size(), "Invalid vector size");

		// Performing the transpose sparse matrix-sparse vector multiplication
		TSMatSVecMultExpr::selectSubAssignKernel(*lhs, A, x);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**Default subtraction assignment to dense vectors*********************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Default subtraction assignment of a transpose sparse matrix-sparse vector
	//        multiplication (\f$ \vec{y}-=A*\vec{x} \f$).
	// \ingroup sparse_vector
	//
	// \param y The target left-hand side dense vector.
	// \param A The left-hand side sparse matrix operand.
	// \param x The right-hand side sparse vector operand.
	// \return void
	//
	// This function implements the default subtraction assignment kernel for the sparse matrix-
	// sparse vector multiplication.
	*/
	template <typename VT1 // Type of the left-hand side target vector
			  ,
			  typename MT1 // Type of the left-hand side matrix operand
			  ,
			  typename VT2> // Type of the right-hand side vector operand
	static inline void selectSubAssignKernel(VT1 &y, const MT1 &A, const VT2 &x)
	{
		const auto vend(x.end());
		auto velem(x.begin());

		for (; velem != vend; ++velem) {
			const auto mend(A.end(velem->index()));
			auto melem(A.begin(velem->index()));

			for (; melem != mend; ++melem) {
				y[melem->index()] -= melem->value() * velem->value();
			}
		}
	}
	/*! \endcond */
	//**********************************************************************************************

	//**Subtraction assignment to sparse vectors****************************************************
	// No special implementation for the subtraction assignment to sparse vectors.
	//**********************************************************************************************

	//**Multiplication assignment to dense vectors**************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Multiplication assignment of a transpose sparse matrix-sparse vector multiplication
	//        to a dense vector.
	// \ingroup sparse_vector
	//
	// \param lhs The target left-hand side dense vector.
	// \param rhs The right-hand side multiplication expression to be multiplied.
	// \return void
	//
	// This function implements the performance optimized multiplication assignment of a transpose
	// sparse matrix-sparse vector multiplication expression to a dense vector.
	*/
	template <typename VT1> // Type of the target dense vector
	friend inline void multAssign(DenseVector<VT1, false> &lhs, const TSMatSVecMultExpr &rhs)
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_CONSTRAINT_MUST_BE_SPARSE_VECTOR_TYPE(ResultType);
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_COLUMN_VECTOR_TYPE(ResultType);
		METRIC_NUMERIC_CONSTRAINT_MUST_NOT_REQUIRE_EVALUATION(ResultType);

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == rhs.size(), "Invalid vector sizes");

		const ResultType tmp(serial(rhs));
		multAssign(*lhs, tmp);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**Multiplication assignment to sparse vectors*************************************************
	// No special implementation for the multiplication assignment to sparse vectors.
	//**********************************************************************************************

	//**SMP assignment to dense vectors*************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief SMP assignment of a transpose sparse matrix-sparse vector multiplication to a dense
	//        vector.
	// \ingroup sparse_vector
	//
	// \param lhs The target left-hand side dense vector.
	// \param rhs The right-hand side multiplication expression to be assigned.
	// \return void
	//
	// This function implements the performance optimized SMP assignment of a transpose sparse
	// matrix-sparse vector multiplication expression to a dense vector. Due to the explicit
	// application of the SFINAE principle, this function can only be selected by the compiler
	// in case the expression specific parallel evaluation strategy is selected.
	*/
	template <typename VT1> // Type of the target dense vector
	friend inline auto smpAssign(DenseVector<VT1, false> &lhs, const TSMatSVecMultExpr &rhs)
		-> EnableIf_t<UseSMPAssign_v<VT1>>
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == rhs.size(), "Invalid vector sizes");

		// Resetting the left-hand side target dense vector
		reset(*lhs);

		// Evaluation of the right-hand side sparse vector operand
		RT x(rhs.vec_);
		if (x.nonZeros() == 0UL)
			return;

		// Evaluation of the left-hand side sparse matrix operand
		LT A(rhs.mat_);

		// Checking the evaluated operators
		METRIC_NUMERIC_INTERNAL_ASSERT(A.rows() == rhs.mat_.rows(), "Invalid number of rows");
		METRIC_NUMERIC_INTERNAL_ASSERT(A.columns() == rhs.mat_.columns(), "Invalid number of columns");
		METRIC_NUMERIC_INTERNAL_ASSERT(x.size() == rhs.vec_.size(), "Invalid vector size");
		METRIC_NUMERIC_INTERNAL_ASSERT(A.rows() == (*lhs).size(), "Invalid vector size");

		// Performing the transpose sparse matrix-sparse vector multiplication
		smpAssign(*lhs, A * x);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**SMP assignment to sparse vectors************************************************************
	// No special implementation for the SMP assignment to sparse vectors.
	//**********************************************************************************************

	//**SMP addition assignment to dense vectors****************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief SMP addition assignment of a transpose sparse matrix-sparse vector multiplication
	//        to a dense vector.
	// \ingroup sparse_vector
	//
	// \param lhs The target left-hand side dense vector.
	// \param rhs The right-hand side multiplication expression to be added.
	// \return void
	//
	// This function implements the performance optimized SMP addition assignment of a transpose
	// sparse matrix-sparse vector multiplication expression to a dense vector. Due to the explicit
	// application of the SFINAE principle, this function can only be selected by the compiler
	// in case the expression specific parallel evaluation strategy is selected.
	*/
	template <typename VT1> // Type of the target dense vector
	friend inline auto smpAddAssign(DenseVector<VT1, false> &lhs, const TSMatSVecMultExpr &rhs)
		-> EnableIf_t<UseSMPAssign_v<VT1>>
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == rhs.size(), "Invalid vector sizes");

		// Evaluation of the right-hand side sparse vector operand
		RT x(rhs.vec_);
		if (x.nonZeros() == 0UL)
			return;

		// Evaluation of the left-hand side sparse matrix operand
		LT A(rhs.mat_);

		// Checking the evaluated operators
		METRIC_NUMERIC_INTERNAL_ASSERT(A.rows() == rhs.mat_.rows(), "Invalid number of rows");
		METRIC_NUMERIC_INTERNAL_ASSERT(A.columns() == rhs.mat_.columns(), "Invalid number of columns");
		METRIC_NUMERIC_INTERNAL_ASSERT(x.size() == rhs.vec_.size(), "Invalid vector size");
		METRIC_NUMERIC_INTERNAL_ASSERT(A.rows() == (*lhs).size(), "Invalid vector size");

		// Performing the transpose sparse matrix-sparse vector multiplication
		smpAddAssign(*lhs, A * x);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**SMP addition assignment to sparse vectors***************************************************
	// No special implementation for the SMP addition assignment to sparse vectors.
	//**********************************************************************************************

	//**SMP subtraction assignment to dense vectors*************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief SMP subtraction assignment of a transpose sparse matrix-sparse vector multiplication
	//        to a dense vector.
	// \ingroup sparse_vector
	//
	// \param lhs The target left-hand side dense vector.
	// \param rhs The right-hand side multiplication expression to be subtracted.
	// \return void
	//
	// This function implements the performance optimized SMP subtraction assignment of a transpose
	// sparse matrix-sparse vector multiplication expression to a dense vector. Due to the explicit
	// application of the SFINAE principle, this function can only be selected by the compiler
	// in case the expression specific parallel evaluation strategy is selected.
	*/
	template <typename VT1> // Type of the target dense vector
	friend inline auto smpSubAssign(DenseVector<VT1, false> &lhs, const TSMatSVecMultExpr &rhs)
		-> EnableIf_t<UseSMPAssign_v<VT1>>
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == rhs.size(), "Invalid vector sizes");

		// Evaluation of the right-hand side sparse vector operand
		RT x(rhs.vec_);
		if (x.nonZeros() == 0UL)
			return;

		// Evaluation of the left-hand side sparse matrix operand
		LT A(rhs.mat_);

		// Checking the evaluated operators
		METRIC_NUMERIC_INTERNAL_ASSERT(A.rows() == rhs.mat_.rows(), "Invalid number of rows");
		METRIC_NUMERIC_INTERNAL_ASSERT(A.columns() == rhs.mat_.columns(), "Invalid number of columns");
		METRIC_NUMERIC_INTERNAL_ASSERT(x.size() == rhs.vec_.size(), "Invalid vector size");
		METRIC_NUMERIC_INTERNAL_ASSERT(A.rows() == (*lhs).size(), "Invalid vector size");

		// Performing the transpose sparse matrix-sparse vector multiplication
		smpSubAssign(*lhs, A * x);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**SMP subtraction assignment to sparse vectors************************************************
	// No special implementation for the SMP subtraction assignment to sparse vectors.
	//**********************************************************************************************

	//**SMP multiplication assignment to dense vectors**********************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief SMP multiplication assignment of a transpose sparse matrix-sparse vector multiplication
	//        to a dense vector.
	// \ingroup sparse_vector
	//
	// \param lhs The target left-hand side dense vector.
	// \param rhs The right-hand side multiplication expression to be multiplied.
	// \return void
	//
	// This function implements the performance optimized SMP multiplication assignment of a
	// transpose sparse matrix-sparse vector multiplication expression to a dense vector. Due
	// to the explicit application of the SFINAE principle, this function can only be selected
	// by the compiler in case the expression specific parallel evaluation strategy is selected.
	*/
	template <typename VT1> // Type of the target dense vector
	friend inline auto smpMultAssign(DenseVector<VT1, false> &lhs, const TSMatSVecMultExpr &rhs)
		-> EnableIf_t<UseSMPAssign_v<VT1>>
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_CONSTRAINT_MUST_BE_SPARSE_VECTOR_TYPE(ResultType);
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_COLUMN_VECTOR_TYPE(ResultType);
		METRIC_NUMERIC_CONSTRAINT_MUST_NOT_REQUIRE_EVALUATION(ResultType);

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == rhs.size(), "Invalid vector sizes");

		const ResultType tmp(rhs);
		smpMultAssign(*lhs, tmp);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**SMP multiplication assignment to sparse vectors*********************************************
	// No special implementation for the SMP multiplication assignment to sparse vectors.
	//**********************************************************************************************

	//**Compile time checks*************************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_SPARSE_MATRIX_TYPE(MT);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_COLUMN_MAJOR_MATRIX_TYPE(MT);
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_ZERO_TYPE(MT);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_SPARSE_VECTOR_TYPE(VT);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_COLUMN_VECTOR_TYPE(VT);
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_ZERO_TYPE(VT);
	METRIC_NUMERIC_CONSTRAINT_MUST_FORM_VALID_MATVECMULTEXPR(MT, VT);
	/*! \endcond */
	//**********************************************************************************************
};
//*************************************************************************************************

//=================================================================================================
//
//  GLOBAL BINARY ARITHMETIC OPERATORS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Backend implementation of the multiplication of a column-major sparse matrix and
//        a sparse vector (\f$ \vec{a}=B*\vec{c} \f$).
// \ingroup sparse_vector
//
// \param mat The left-hand side sparse matrix for the multiplication.
// \param vec The right-hand side sparse vector for the multiplication.
// \return The resulting vector.
//
// This function implements the performance optimized treatment of the multiplication of a
// column-major sparse matrix and a sparse vector.
*/
template <typename MT // Type of the left-hand side sparse matrix
		  ,
		  typename VT // Type of the right-hand side sparse vector
		  ,
		  DisableIf_t<(IsIdentity_v<MT> && IsSame_v<ElementType_t<MT>, ElementType_t<VT>>) ||
					  (IsZero_v<MT> || IsZero_v<VT>)> * = nullptr>
inline const TSMatSVecMultExpr<MT, VT> tsmatsvecmult(const SparseMatrix<MT, true> &mat,
													 const SparseVector<VT, false> &vec)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	METRIC_NUMERIC_INTERNAL_ASSERT((*mat).columns() == (*vec).size(), "Invalid matrix and vector sizes");

	return TSMatSVecMultExpr<MT, VT>(*mat, *vec);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Backend implementation of the multiplication of a column-major identity matrix and a
//        sparse vector (\f$ \vec{y}=A*\vec{x} \f$).
// \ingroup sparse_vector
//
// \param mat The left-hand side column-major identity matrix for the multiplication.
// \param vec The right-hand side sparse vector for the multiplication.
// \return Reference to the given sparse vector.
//
// This function implements the performance optimized treatment of the multiplication of a
// column-major identity matrix and a sparse vector. It returns a reference to the given sparse
// vector.
*/
template <typename MT // Type of the left-hand side sparse matrix
		  ,
		  typename VT // Type of the right-hand side sparse vector
		  ,
		  EnableIf_t<(IsIdentity_v<MT> && IsSame_v<ElementType_t<MT>, ElementType_t<VT>>) && !IsZero_v<VT>> * = nullptr>
inline const VT &tsmatsvecmult(const SparseMatrix<MT, true> &mat, const SparseVector<VT, false> &vec)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	MAYBE_UNUSED(mat);

	METRIC_NUMERIC_INTERNAL_ASSERT((*mat).columns() == (*vec).size(), "Invalid matrix and vector sizes");

	return (*vec);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Backend implementation of the multiplication of a column-major (zero) sparse matrix and
// a (zero) sparse vector (\f$ \vec{y}=A*\vec{x} \f$).
// \ingroup sparse_vector
//
// \param mat The left-hand side column-major sparse matrix for the multiplication.
// \param vec The right-hand side sparse vector for the multiplication.
// \return The resulting zero vector.
//
// This function implements the performance optimized treatment of the multiplication of a
// column-major (zero) sparse matrix and a (zero) sparse vector. It returns a zero vector.
*/
template <typename MT // Type of the left-hand side sparse matrix
		  ,
		  typename VT // Type of the right-hand side sparse vector
		  ,
		  EnableIf_t<IsZero_v<MT> || IsZero_v<VT>> * = nullptr>
inline decltype(auto) tsmatsvecmult(const SparseMatrix<MT, true> &mat, const SparseVector<VT, false> &vec)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	MAYBE_UNUSED(vec);

	METRIC_NUMERIC_INTERNAL_ASSERT((*mat).columns() == (*vec).size(), "Invalid matrix and vector sizes");

	using ReturnType = const MultTrait_t<ResultType_t<MT>, ResultType_t<VT>>;

	METRIC_NUMERIC_CONSTRAINT_MUST_BE_COLUMN_VECTOR_TYPE(ReturnType);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_ZERO_TYPE(ReturnType);

	return ReturnType((*mat).rows());
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Multiplication operator for the multiplication of a transpose sparse matrix and a
//        sparse vector (\f$ \vec{y}=A*\vec{x} \f$).
// \ingroup sparse_vector
//
// \param mat The left-hand side sparse matrix for the multiplication.
// \param vec The right-hand side sparse vector for the multiplication.
// \return The resulting vector.
// \exception std::invalid_argument Matrix and vector sizes do not match.
//
// This operator represents the multiplication between a transpose sparse matrix and a sparse
// vector:

   \code
   using mtrc::numeric::columnMajor;
   using mtrc::numeric::columnVector;

   mtrc::numeric::CompressedMatrix<double,columnMajor> A;
   mtrc::numeric::CompressedVector<double,columnVector> x, y;
   // ... Resizing and initialization
   y = A * x;
   \endcode

// The operator returns an expression representing a sparse vector of the higher-order element
// type of the two involved element types \a MT::ElementType and \a VT::ElementType. Both the
// sparse matrix type \a MT and the sparse vector type \a VT as well as the two element types
// \a MT::ElementType and \a VT::ElementType have to be supported by the MultTrait class
// template.\n
// In case the current size of the vector \a vec doesn't match the current number of columns
// of the matrix \a mat, a \a std::invalid_argument is thrown.
*/
template <typename MT // Type of the left-hand side sparse matrix
		  ,
		  typename VT> // Type of the right-hand side sparse vector
inline decltype(auto) operator*(const SparseMatrix<MT, true> &mat, const SparseVector<VT, false> &vec)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_MATMATMULTEXPR_TYPE(MT);

	if ((*mat).columns() != (*vec).size()) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Matrix and vector sizes do not match");
	}

	return tsmatsvecmult(*mat, *vec);
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
