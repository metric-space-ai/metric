// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_TDVECSMATMULTEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_TDVECSMATMULTEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/Aliases.h>
#include <metric/numeric/math/Exception.h>
#include <metric/numeric/math/constraints/DenseVector.h>
#include <metric/numeric/math/constraints/MatMatMultExpr.h>
#include <metric/numeric/math/constraints/RequiresEvaluation.h>
#include <metric/numeric/math/constraints/RowMajorMatrix.h>
#include <metric/numeric/math/constraints/RowVector.h>
#include <metric/numeric/math/constraints/SparseMatrix.h>
#include <metric/numeric/math/constraints/Symmetric.h>
#include <metric/numeric/math/constraints/TVecMatMultExpr.h>
#include <metric/numeric/math/constraints/Zero.h>
#include <metric/numeric/math/expressions/Computation.h>
#include <metric/numeric/math/expressions/DenseVector.h>
#include <metric/numeric/math/expressions/Forward.h>
#include <metric/numeric/math/expressions/TVecMatMultExpr.h>
#include <metric/numeric/math/shims/IsDefault.h>
#include <metric/numeric/math/shims/Reset.h>
#include <metric/numeric/math/shims/Serial.h>
#include <metric/numeric/math/traits/MultTrait.h>
#include <metric/numeric/math/typetraits/IsAligned.h>
#include <metric/numeric/math/typetraits/IsComputation.h>
#include <metric/numeric/math/typetraits/IsDiagonal.h>
#include <metric/numeric/math/typetraits/IsExpression.h>
#include <metric/numeric/math/typetraits/IsIdentity.h>
#include <metric/numeric/math/typetraits/IsLower.h>
#include <metric/numeric/math/typetraits/IsResizable.h>
#include <metric/numeric/math/typetraits/IsStrictlyLower.h>
#include <metric/numeric/math/typetraits/IsStrictlyUpper.h>
#include <metric/numeric/math/typetraits/IsSymmetric.h>
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
//  CLASS TDVECSMATMULTEXPR
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Expression object for transpose dense vector-sparse matrix multiplications.
// \ingroup dense_vector_expression
//
// The TDVecSMatMultExpr class represents the compile time expression for multiplications
// between transpose dense vectors and row-major sparse matrices.
*/
template <typename VT // Type of the left-hand side dense vector
		  ,
		  typename MT> // Type of the right-hand side sparse matrix
class TDVecSMatMultExpr : public TVecMatMultExpr<DenseVector<TDVecSMatMultExpr<VT, MT>, true>>, private Computation {
  private:
	//**Type definitions****************************************************************************
	using VRT = ResultType_t<VT>;	 //!< Result type of the left-hand side dense vector expression.
	using MRT = ResultType_t<MT>;	 //!< Result type of the right-hand side sparse matrix expression.
	using VCT = CompositeType_t<VT>; //!< Composite type of the left-hand side dense vector expression.
	using MCT = CompositeType_t<MT>; //!< Composite type of the right-hand side sparse matrix expression.
	//**********************************************************************************************

	//**********************************************************************************************
	//! Compilation switch for the composite type of the left-hand side dense vector expression.
	static constexpr bool evaluateVector = (IsComputation_v<VT> || RequiresEvaluation_v<VT>);
	//**********************************************************************************************

	//**********************************************************************************************
	//! Compilation switch for the composite type of the right-hand side sparse matrix expression.
	static constexpr bool evaluateMatrix = RequiresEvaluation_v<MT>;
	//**********************************************************************************************

	//**********************************************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	//! Helper variable template for the explicit application of the SFINAE principle.
	/*! This variable template is a helper for the selection of the parallel evaluation strategy.
		In case either the vector or the matrix operand requires an intermediate evaluation, the
		variable will be set to 1, otherwise it will be 0. */
	template <typename T1> static constexpr bool UseSMPAssign_v = (evaluateVector || evaluateMatrix);
	/*! \endcond */
	//**********************************************************************************************

  public:
	//**Type definitions****************************************************************************
	//! Type of this TDVecSMatMultExpr instance.
	using This = TDVecSMatMultExpr<VT, MT>;

	//! Base type of this TDVecSMatMultExpr instance.
	using BaseType = TVecMatMultExpr<DenseVector<This, true>>;

	using ResultType = MultTrait_t<VRT, MRT>;		   //!< Result type for expression template evaluations.
	using TransposeType = TransposeType_t<ResultType>; //!< Transpose type for expression template evaluations.
	using ElementType = ElementType_t<ResultType>;	   //!< Resulting element type.
	using ReturnType = const ElementType;			   //!< Return type for expression template evaluations.
	using CompositeType = const ResultType;			   //!< Data type for composite expression templates.

	//! Composite type of the left-hand side dense vector expression.
	using LeftOperand = If_t<IsExpression_v<VT>, const VT, const VT &>;

	//! Composite type of the right-hand side sparse matrix expression.
	using RightOperand = If_t<IsExpression_v<MT>, const MT, const MT &>;

	//! Composite type of the left-hand side dense vector expression.
	using LT = If_t<evaluateVector, const VRT, VCT>;

	//! Composite type of the right-hand side sparse matrix expression.
	using RT = If_t<evaluateMatrix, const MRT, MCT>;
	//**********************************************************************************************

	//**Compilation flags***************************************************************************
	//! Compilation switch for the expression template evaluation strategy.
	static constexpr bool simdEnabled = false;

	//! Compilation switch for the expression template assignment strategy.
	static constexpr bool smpAssignable =
		(!evaluateVector && VT::smpAssignable && !evaluateMatrix && MT::smpAssignable);
	//**********************************************************************************************

	//**Constructor*********************************************************************************
	/*!\brief Constructor for the TDVecSMatMultExpr class.
	 */
	inline TDVecSMatMultExpr(const VT &vec, const MT &mat) noexcept
		: vec_(vec) // Left-hand side dense vector of the multiplication expression
		  ,
		  mat_(mat) // Right-hand side sparse matrix of the multiplication expression
	{
		METRIC_NUMERIC_INTERNAL_ASSERT(vec_.size() == mat_.rows(), "Invalid vector and matrix sizes");
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
		METRIC_NUMERIC_INTERNAL_ASSERT(index < mat_.columns(), "Invalid vector access index");

		if (IsDiagonal_v<MT>) {
			return vec_[index] * mat_(index, index);
		} else if (IsLower_v<MT>) {
			const size_t begin(IsStrictlyLower_v<MT> ? index + 1UL : index);
			const size_t n(mat_.rows() - begin);
			return subvector(vec_, begin, n, unchecked) *
				   subvector(column(mat_, index, unchecked), begin, n, unchecked);
		} else if (IsUpper_v<MT>) {
			const size_t n(IsStrictlyUpper_v<MT> ? index : index + 1UL);
			return subvector(vec_, 0UL, n, unchecked) * subvector(column(mat_, index, unchecked), 0UL, n, unchecked);
		} else {
			return vec_ * column(mat_, index, unchecked);
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
		if (index >= mat_.columns()) {
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
	inline size_t size() const noexcept { return mat_.columns(); }
	//**********************************************************************************************

	//**Left operand access*************************************************************************
	/*!\brief Returns the left-hand side dense vector operand.
	//
	// \return The left-hand side dense vector operand.
	*/
	inline LeftOperand leftOperand() const noexcept { return vec_; }
	//**********************************************************************************************

	//**Right operand access************************************************************************
	/*!\brief Returns the right-hand side sparse matrix operand.
	//
	// \return The right-hand side sparse matrix operand.
	*/
	inline RightOperand rightOperand() const noexcept { return mat_; }
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Returns whether the expression can alias with the given address \a alias.
	//
	// \param alias The alias to be checked.
	// \return \a true in case the expression can alias, \a false otherwise.
	*/
	template <typename T> inline bool canAlias(const T *alias) const noexcept
	{
		return (vec_.isAliased(alias) || mat_.isAliased(alias));
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
		return (vec_.isAliased(alias) || mat_.isAliased(alias));
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Returns whether the operands of the expression are properly aligned in memory.
	//
	// \return \a true in case the operands are aligned, \a false if not.
	*/
	inline bool isAligned() const noexcept { return vec_.isAligned(); }
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Returns whether the expression can be used in SMP assignments.
	//
	// \return \a true in case the expression can be used in SMP assignments, \a false if not.
	*/
	inline bool canSMPAssign() const noexcept { return (size() > SMP_TDVECSMATMULT_THRESHOLD); }
	//**********************************************************************************************

  private:
	//**Member variables****************************************************************************
	LeftOperand vec_;  //!< Left-hand side dense vector of the multiplication expression.
	RightOperand mat_; //!< Right-hand side sparse matrix of the multiplication expression.
	//**********************************************************************************************

	//**Assignment to dense vectors*****************************************************************
	/*!\brief Assignment of a transpose dense vector-sparse matrix multiplication to a dense
	//        vector (\f$ \vec{y}^T=\vec{x}^T*A \f$).
	// \ingroup dense_vector
	//
	// \param lhs The target left-hand side dense vector.
	// \param rhs The right-hand side multiplication expression to be assigned.
	// \return void
	//
	// This function implements the performance optimized assignment of a transpose dense vector-
	// sparse matrix multiplication expression to a dense vector.
	*/
	template <typename VT2> // Type of the target dense vector
	friend inline void assign(DenseVector<VT2, true> &lhs, const TDVecSMatMultExpr &rhs)
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == rhs.size(), "Invalid vector sizes");

		reset(*lhs);

		if (rhs.mat_.rows() == 0UL)
			return;

		LT x(serial(rhs.vec_)); // Evaluation of the left-hand side dense vector operator
		RT A(serial(rhs.mat_)); // Evaluation of the right-hand side sparse matrix operator

		METRIC_NUMERIC_INTERNAL_ASSERT(x.size() == rhs.vec_.size(), "Invalid vector size");
		METRIC_NUMERIC_INTERNAL_ASSERT(A.rows() == rhs.mat_.rows(), "Invalid number of rows");
		METRIC_NUMERIC_INTERNAL_ASSERT(A.columns() == rhs.mat_.columns(), "Invalid number of columns");
		METRIC_NUMERIC_INTERNAL_ASSERT(A.columns() == (*lhs).size(), "Invalid vector size");

		TDVecSMatMultExpr::selectAssignKernel(*lhs, x, A);
	}
	//**********************************************************************************************

	//**Optimized assignment to dense vectors*******************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Optimized assignment of a transpose dense vector-sparse matrix multiplication
	//        (\f$ \vec{y}^T=\vec{x}^T*A \f$).
	// \ingroup dense_vector
	//
	// \param y The target left-hand side dense vector.
	// \param x The left-hand side dense vector operand.
	// \param A The right-hand side sparse matrix operand.
	// \return void
	//
	// This function implements the serial assignment kernel for the transpose sparse vector-
	// transpose sparse matrix multiplication.
	*/
	template <typename VT1 // Type of the left-hand side target vector
			  ,
			  typename VT2 // Type of the left-hand side vector operand
			  ,
			  typename MT1> // Type of the right-hand side matrix operand
	static inline void selectAssignKernel(VT1 &y, const VT2 &x, const MT1 &A)
	{
		for (size_t i = 0UL; i < x.size(); ++i) {
			const auto end(A.end(i));
			auto element(A.begin(i));

			for (; element != end; ++element) {
				if (IsResizable_v<ElementType_t<VT1>> && isDefault(y[element->index()]))
					y[element->index()] = x[i] * element->value();
				else
					y[element->index()] += x[i] * element->value();
			}
		}
	}
	/*! \endcond */
	//**********************************************************************************************

	//**Assignment to sparse vectors****************************************************************
	/*!\brief Assignment of a transpose dense vector-sparse matrix multiplication to a sparse
	//        vector (\f$ \vec{y}^T=\vec{x}^T*A \f$).
	// \ingroup dense_vector
	//
	// \param lhs The target left-hand side sparse vector.
	// \param rhs The right-hand side multiplication expression to be assigned.
	// \return void
	//
	// This function implements the performance optimized assignment of a transpose dense vector-
	// sparse matrix multiplication expression to a sparse vector.
	*/
	template <typename VT2> // Type of the target sparse vector
	friend inline void assign(SparseVector<VT2, true> &lhs, const TDVecSMatMultExpr &rhs)
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_CONSTRAINT_MUST_BE_DENSE_VECTOR_TYPE(ResultType);
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_ROW_VECTOR_TYPE(ResultType);
		METRIC_NUMERIC_CONSTRAINT_MUST_NOT_REQUIRE_EVALUATION(ResultType);

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == rhs.size(), "Invalid vector sizes");

		const ResultType tmp(serial(rhs));
		assign(*lhs, tmp);
	}
	//**********************************************************************************************

	//**Addition assignment to dense vectors********************************************************
	/*!\brief Addition assignment of a transpose dense vector-sparse matrix multiplication to a
	//        dense vector (\f$ \vec{y}^T+=\vec{x}^T*A \f$).
	// \ingroup dense_vector
	//
	// \param lhs The target left-hand side dense vector.
	// \param rhs The right-hand side multiplication expression to be added.
	// \return void
	//
	// This function implements the performance optimized addition assignment of a transpose
	// dense vector-sparse matrix multiplication expression to a dense vector.
	*/
	template <typename VT2> // Type of the target dense vector
	friend inline void addAssign(DenseVector<VT2, true> &lhs, const TDVecSMatMultExpr &rhs)
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == rhs.size(), "Invalid vector sizes");

		if (rhs.mat_.rows() == 0UL) {
			return;
		}

		LT x(serial(rhs.vec_)); // Evaluation of the left-hand side dense vector operator
		RT A(serial(rhs.mat_)); // Evaluation of the right-hand side sparse matrix operator

		METRIC_NUMERIC_INTERNAL_ASSERT(x.size() == rhs.vec_.size(), "Invalid vector size");
		METRIC_NUMERIC_INTERNAL_ASSERT(A.rows() == rhs.mat_.rows(), "Invalid number of rows");
		METRIC_NUMERIC_INTERNAL_ASSERT(A.columns() == rhs.mat_.columns(), "Invalid number of columns");
		METRIC_NUMERIC_INTERNAL_ASSERT(A.columns() == (*lhs).size(), "Invalid vector size");

		TDVecSMatMultExpr::selectAddAssignKernel(*lhs, x, A);
	}
	//**********************************************************************************************

	//**Optimized addition assignment to dense vectors*************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Optimized addition assignment of a transpose dense vector-sparse matrix
	//        multiplication (\f$ \vec{y}^T+=\vec{x}^T*A \f$).
	// \ingroup dense_vector
	//
	// \param y The target left-hand side dense vector.
	// \param x The left-hand side dense vector operand.
	// \param A The right-hand side sparse matrix operand.
	// \return void
	//
	// This function implements the serial addition assignment kernel for the transpose sparse
	// vector-transpose sparse matrix multiplication.
	*/
	template <typename VT1 // Type of the left-hand side target vector
			  ,
			  typename VT2 // Type of the left-hand side vector operand
			  ,
			  typename MT1> // Type of the right-hand side matrix operand
	static inline void selectAddAssignKernel(VT1 &y, const VT2 &x, const MT1 &A)
	{
		for (size_t i = 0UL; i < x.size(); ++i) {
			const auto end(A.end(i));
			auto element(A.begin(i));

			for (; element != end; ++element) {
				y[element->index()] += x[i] * element->value();
			}
		}
	}
	/*! \endcond */
	//**********************************************************************************************

	//**Addition assignment to sparse vectors*******************************************************
	// No special implementation for the addition assignment to sparse vectors.
	//**********************************************************************************************

	//**Subtraction assignment to dense vectors*****************************************************
	/*!\brief Subtraction assignment of a transpose dense vector-sparse matrix multiplication
	//        to a dense vector (\f$ \vec{y}^T-=\vec{x}^T*A \f$).
	// \ingroup dense_vector
	//
	// \param lhs The target left-hand side dense vector.
	// \param rhs The right-hand side multiplication expression to be subtracted.
	// \return void
	//
	// This function implements the performance optimized subtraction assignment of a transpose
	// dense vector-sparse matrix multiplication expression to a dense vector.
	*/
	template <typename VT2> // Type of the target dense vector
	friend inline void subAssign(DenseVector<VT2, true> &lhs, const TDVecSMatMultExpr &rhs)
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == rhs.size(), "Invalid vector sizes");

		if (rhs.mat_.rows() == 0UL) {
			return;
		}

		LT x(serial(rhs.vec_)); // Evaluation of the left-hand side dense vector operator
		RT A(serial(rhs.mat_)); // Evaluation of the right-hand side sparse matrix operator

		METRIC_NUMERIC_INTERNAL_ASSERT(x.size() == rhs.vec_.size(), "Invalid vector size");
		METRIC_NUMERIC_INTERNAL_ASSERT(A.rows() == rhs.mat_.rows(), "Invalid number of rows");
		METRIC_NUMERIC_INTERNAL_ASSERT(A.columns() == rhs.mat_.columns(), "Invalid number of columns");
		METRIC_NUMERIC_INTERNAL_ASSERT(A.columns() == (*lhs).size(), "Invalid vector size");

		TDVecSMatMultExpr::selectSubAssignKernel(*lhs, x, A);
	}
	//**********************************************************************************************

	//**Optimized subtraction assignment to dense vectors**********************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Optimized subtraction assignment of a transpose dense vector-sparse matrix
	//        multiplication (\f$ \vec{y}^T-=\vec{x}^T*A \f$).
	// \ingroup dense_vector
	//
	// \param y The target left-hand side dense vector.
	// \param x The left-hand side dense vector operand.
	// \param A The right-hand side sparse matrix operand.
	// \return void
	//
	// This function implements the serial subtraction assignment kernel for the transpose sparse
	// vector-transpose sparse matrix multiplication.
	*/
	template <typename VT1 // Type of the left-hand side target vector
			  ,
			  typename VT2 // Type of the left-hand side vector operand
			  ,
			  typename MT1> // Type of the right-hand side matrix operand
	static inline void selectSubAssignKernel(VT1 &y, const VT2 &x, const MT1 &A)
	{
		for (size_t i = 0UL; i < x.size(); ++i) {
			const auto end(A.end(i));
			auto element(A.begin(i));

			for (; element != end; ++element) {
				y[element->index()] -= x[i] * element->value();
			}
		}
	}
	/*! \endcond */
	//**********************************************************************************************

	//**Subtraction assignment to sparse vectors****************************************************
	// No special implementation for the subtraction assignment to sparse vectors.
	//**********************************************************************************************

	//**Multiplication assignment to dense vectors**************************************************
	/*!\brief Multiplication assignment of a transpose dense vector-sparse matrix multiplication
	//        to a dense vector (\f$ \vec{y}^T*=\vec{x}^T*A \f$).
	// \ingroup dense_vector
	//
	// \param lhs The target left-hand side dense vector.
	// \param rhs The right-hand side multiplication expression to be multiplied.
	// \return void
	//
	// This function implements the performance optimized multiplication assignment of a transpose
	// dense vector-sparse matrix multiplication expression to a dense vector.
	*/
	template <typename VT2> // Type of the target dense vector
	friend inline void multAssign(DenseVector<VT2, true> &lhs, const TDVecSMatMultExpr &rhs)
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_CONSTRAINT_MUST_BE_DENSE_VECTOR_TYPE(ResultType);
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_ROW_VECTOR_TYPE(ResultType);
		METRIC_NUMERIC_CONSTRAINT_MUST_NOT_REQUIRE_EVALUATION(ResultType);

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == rhs.size(), "Invalid vector sizes");

		const ResultType tmp(serial(rhs));
		multAssign(*lhs, tmp);
	}
	//**********************************************************************************************

	//**Multiplication assignment to sparse vectors*************************************************
	// No special implementation for the multiplication assignment to sparse vectors.
	//**********************************************************************************************

	//**Division assignment to dense vectors********************************************************
	/*!\brief Division assignment of a transpose dense vector-sparse matrix multiplication to a
	//        dense vector (\f$ \vec{y}^T/=\vec{x}^T*A \f$).
	// \ingroup dense_vector
	//
	// \param lhs The target left-hand side dense vector.
	// \param rhs The right-hand side multiplication expression divisor.
	// \return void
	//
	// This function implements the performance optimized division assignment of a transpose dense
	// vector-sparse matrix multiplication expression to a dense vector.
	*/
	template <typename VT2> // Type of the target dense vector
	friend inline void divAssign(DenseVector<VT2, true> &lhs, const TDVecSMatMultExpr &rhs)
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_CONSTRAINT_MUST_BE_DENSE_VECTOR_TYPE(ResultType);
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_ROW_VECTOR_TYPE(ResultType);
		METRIC_NUMERIC_CONSTRAINT_MUST_NOT_REQUIRE_EVALUATION(ResultType);

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == rhs.size(), "Invalid vector sizes");

		const ResultType tmp(serial(rhs));
		divAssign(*lhs, tmp);
	}
	//**********************************************************************************************

	//**Division assignment to sparse vectors*******************************************************
	// No special implementation for the division assignment to sparse vectors.
	//**********************************************************************************************

	//**SMP assignment to dense vectors*************************************************************
	/*!\brief SMP assignment of a transpose dense vector-sparse matrix multiplication to a dense
	//        vector (\f$ \vec{y}^T=\vec{x}^T*A \f$).
	// \ingroup dense_vector
	//
	// \param lhs The target left-hand side dense vector.
	// \param rhs The right-hand side multiplication expression to be assigned.
	// \return void
	//
	// This function implements the performance optimized SMP assignment of a transpose dense
	// vector-sparse matrix multiplication expression to a dense vector. Due to the explicit
	// application of the SFINAE principle, this function can only be selected by the compiler
	// in case the expression specific parallel evaluation strategy is selected.
	*/
	template <typename VT2> // Type of the target dense vector
	friend inline auto smpAssign(DenseVector<VT2, true> &lhs, const TDVecSMatMultExpr &rhs)
		-> EnableIf_t<UseSMPAssign_v<VT2>>
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == rhs.size(), "Invalid vector sizes");

		reset(*lhs);

		if (rhs.mat_.rows() == 0UL)
			return;

		LT x(rhs.vec_); // Evaluation of the left-hand side dense vector operator
		RT A(rhs.mat_); // Evaluation of the right-hand side sparse matrix operator

		METRIC_NUMERIC_INTERNAL_ASSERT(x.size() == rhs.vec_.size(), "Invalid vector size");
		METRIC_NUMERIC_INTERNAL_ASSERT(A.rows() == rhs.mat_.rows(), "Invalid number of rows");
		METRIC_NUMERIC_INTERNAL_ASSERT(A.columns() == rhs.mat_.columns(), "Invalid number of columns");
		METRIC_NUMERIC_INTERNAL_ASSERT(A.columns() == (*lhs).size(), "Invalid vector size");

		smpAssign(*lhs, x * A);
	}
	//**********************************************************************************************

	//**SMP assignment to sparse vectors************************************************************
	/*!\brief SMP assignment of a transpose dense vector-sparse matrix multiplication to a sparse
	//        vector (\f$ \vec{y}^T=\vec{x}^T*A \f$).
	// \ingroup dense_vector
	//
	// \param lhs The target left-hand side sparse vector.
	// \param rhs The right-hand side multiplication expression to be assigned.
	// \return void
	//
	// This function implements the performance optimized SMP assignment of a transpose dense
	// vector-sparse matrix multiplication expression to a sparse vector. Due to the explicit
	// application of the SFINAE principle, this function can only be selected by the compiler
	// in case the expression specific parallel evaluation strategy is selected.
	*/
	template <typename VT2> // Type of the target sparse vector
	friend inline auto smpAssign(SparseVector<VT2, true> &lhs, const TDVecSMatMultExpr &rhs)
		-> EnableIf_t<UseSMPAssign_v<VT2>>
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_CONSTRAINT_MUST_BE_DENSE_VECTOR_TYPE(ResultType);
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_ROW_VECTOR_TYPE(ResultType);
		METRIC_NUMERIC_CONSTRAINT_MUST_NOT_REQUIRE_EVALUATION(ResultType);

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == rhs.size(), "Invalid vector sizes");

		const ResultType tmp(rhs);
		smpAssign(*lhs, tmp);
	}
	//**********************************************************************************************

	//**SMP addition assignment to dense vectors****************************************************
	/*!\brief Addition assignment of a transpose dense vector-sparse matrix multiplication to a
	//        dense vector (\f$ \vec{y}^T+=\vec{x}^T*A \f$).
	// \ingroup dense_vector
	//
	// \param lhs The target left-hand side dense vector.
	// \param rhs The right-hand side multiplication expression to be added.
	// \return void
	//
	// This function implements the performance optimized SMP addition assignment of a transpose
	// dense vector-sparse matrix multiplication expression to a dense vector. Due to the explicit
	// application of the SFINAE principle, this function can only be selected by the compiler
	// in case the expression specific parallel evaluation strategy is selected.
	*/
	template <typename VT2> // Type of the target dense vector
	friend inline auto smpAddAssign(DenseVector<VT2, true> &lhs, const TDVecSMatMultExpr &rhs)
		-> EnableIf_t<UseSMPAssign_v<VT2>>
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == rhs.size(), "Invalid vector sizes");

		if (rhs.mat_.rows() == 0UL) {
			return;
		}

		LT x(rhs.vec_); // Evaluation of the left-hand side dense vector operator
		RT A(rhs.mat_); // Evaluation of the right-hand side sparse matrix operator

		METRIC_NUMERIC_INTERNAL_ASSERT(x.size() == rhs.vec_.size(), "Invalid vector size");
		METRIC_NUMERIC_INTERNAL_ASSERT(A.rows() == rhs.mat_.rows(), "Invalid number of rows");
		METRIC_NUMERIC_INTERNAL_ASSERT(A.columns() == rhs.mat_.columns(), "Invalid number of columns");
		METRIC_NUMERIC_INTERNAL_ASSERT(A.columns() == (*lhs).size(), "Invalid vector size");

		smpAddAssign(*lhs, x * A);
	}
	//**********************************************************************************************

	//**SMP addition assignment to sparse vectors***************************************************
	// No special implementation for the SMP addition assignment to sparse vectors.
	//**********************************************************************************************

	//**SMP subtraction assignment to dense vectors*************************************************
	/*!\brief SMP subtraction assignment of a transpose dense vector-sparse matrix multiplication
	//        to a dense vector (\f$ \vec{y}^T-=\vec{x}^T*A \f$).
	// \ingroup dense_vector
	//
	// \param lhs The target left-hand side dense vector.
	// \param rhs The right-hand side multiplication expression to be subtracted.
	// \return void
	//
	// This function implements the performance optimized SMP subtraction assignment of a transpose
	// dense vector-sparse matrix multiplication expression to a dense vector. Due to the explicit
	// application of the SFINAE principle, this function can only be selected by the compiler
	// in case the expression specific parallel evaluation strategy is selected.
	*/
	template <typename VT2> // Type of the target dense vector
	friend inline auto smpSubAssign(DenseVector<VT2, true> &lhs, const TDVecSMatMultExpr &rhs)
		-> EnableIf_t<UseSMPAssign_v<VT2>>
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == rhs.size(), "Invalid vector sizes");

		if (rhs.mat_.rows() == 0UL) {
			return;
		}

		LT x(rhs.vec_); // Evaluation of the left-hand side dense vector operator
		RT A(rhs.mat_); // Evaluation of the right-hand side sparse matrix operator

		METRIC_NUMERIC_INTERNAL_ASSERT(x.size() == rhs.vec_.size(), "Invalid vector size");
		METRIC_NUMERIC_INTERNAL_ASSERT(A.rows() == rhs.mat_.rows(), "Invalid number of rows");
		METRIC_NUMERIC_INTERNAL_ASSERT(A.columns() == rhs.mat_.columns(), "Invalid number of columns");
		METRIC_NUMERIC_INTERNAL_ASSERT(A.columns() == (*lhs).size(), "Invalid vector size");

		smpSubAssign(*lhs, x * A);
	}
	//**********************************************************************************************

	//**SMP subtraction assignment to sparse vectors************************************************
	// No special implementation for the SMP subtraction assignment to sparse vectors.
	//**********************************************************************************************

	//**SMP multiplication assignment to dense vectors**********************************************
	/*!\brief SMP multiplication assignment of a transpose dense vector-sparse matrix multiplication
	//        to a dense vector (\f$ \vec{y}^T*=\vec{x}^T*A \f$).
	// \ingroup dense_vector
	//
	// \param lhs The target left-hand side dense vector.
	// \param rhs The right-hand side multiplication expression to be multiplied.
	// \return void
	//
	// This function implements the performance optimized SMP multiplication assignment of a
	// transpose dense vector-sparse matrix multiplication expression to a dense vector. Due
	// to the explicit application of the SFINAE principle, this function can only be selected
	// by the compiler in case the expression specific parallel evaluation strategy is selected.
	*/
	template <typename VT2> // Type of the target dense vector
	friend inline auto smpMultAssign(DenseVector<VT2, true> &lhs, const TDVecSMatMultExpr &rhs)
		-> EnableIf_t<UseSMPAssign_v<VT2>>
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_CONSTRAINT_MUST_BE_DENSE_VECTOR_TYPE(ResultType);
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_ROW_VECTOR_TYPE(ResultType);
		METRIC_NUMERIC_CONSTRAINT_MUST_NOT_REQUIRE_EVALUATION(ResultType);

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == rhs.size(), "Invalid vector sizes");

		const ResultType tmp(rhs);
		smpMultAssign(*lhs, tmp);
	}
	//**********************************************************************************************

	//**SMP multiplication assignment to sparse vectors*********************************************
	// No special implementation for the SMP multiplication assignment to sparse vectors.
	//**********************************************************************************************

	//**SMP division assignment to dense vectors****************************************************
	/*!\brief SMP division assignment of a transpose dense vector-sparse matrix multiplication to
	//        a dense vector (\f$ \vec{y}^T/=\vec{x}^T*A \f$).
	// \ingroup dense_vector
	//
	// \param lhs The target left-hand side dense vector.
	// \param rhs The right-hand side multiplication expression divisor.
	// \return void
	//
	// This function implements the performance optimized SMP division assignment of a transpose
	// dense vector-sparse matrix multiplication expression to a dense vector. Due to the explicit
	// application of the SFINAE principle, this function can only be selected by the compiler in
	// case the expression specific parallel evaluation strategy is selected.
	*/
	template <typename VT2> // Type of the target dense vector
	friend inline auto smpDivAssign(DenseVector<VT2, true> &lhs, const TDVecSMatMultExpr &rhs)
		-> EnableIf_t<UseSMPAssign_v<VT2>>
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_CONSTRAINT_MUST_BE_DENSE_VECTOR_TYPE(ResultType);
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_ROW_VECTOR_TYPE(ResultType);
		METRIC_NUMERIC_CONSTRAINT_MUST_NOT_REQUIRE_EVALUATION(ResultType);

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == rhs.size(), "Invalid vector sizes");

		const ResultType tmp(rhs);
		smpDivAssign(*lhs, tmp);
	}
	//**********************************************************************************************

	//**SMP division assignment to sparse vectors***************************************************
	// No special implementation for the SMP division assignment to sparse vectors.
	//**********************************************************************************************

	//**Compile time checks*************************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_DENSE_VECTOR_TYPE(VT);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_ROW_VECTOR_TYPE(VT);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_SPARSE_MATRIX_TYPE(MT);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_ROW_MAJOR_MATRIX_TYPE(MT);
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_SYMMETRIC_MATRIX_TYPE(MT);
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_ZERO_TYPE(MT);
	METRIC_NUMERIC_CONSTRAINT_MUST_FORM_VALID_TVECMATMULTEXPR(VT, MT);
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
/*!\brief Backend implementation of the multiplication of a transpose dense vector
//        and a row-major sparse matrix (\f$ \vec{a}=B*\vec{c} \f$).
// \ingroup dense_vector
//
// \param vec The left-hand side transpose dense vector for the multiplication.
// \param mat The right-hand side row-major sparse matrix for the multiplication.
// \return The resulting transpose vector.
//
// This function implements the performance optimized treatment of the multiplication of a
// transpose dense vector and a row-major sparse matrix.
*/
template <typename VT // Type of the left-hand side dense vector
		  ,
		  typename MT // Type of the right-hand side sparse matrix
		  ,
		  DisableIf_t<IsSymmetric_v<MT> || (IsIdentity_v<MT> && IsSame_v<ElementType_t<VT>, ElementType_t<MT>>) ||
					  IsZero_v<MT>> * = nullptr>
inline const TDVecSMatMultExpr<VT, MT> tdvecsmatmult(const DenseVector<VT, true> &vec,
													 const SparseMatrix<MT, false> &mat)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	METRIC_NUMERIC_INTERNAL_ASSERT((*vec).size() == (*mat).rows(), "Invalid vector and matrix sizes");

	return TDVecSMatMultExpr<VT, MT>(*vec, *mat);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Backend implementation of the multiplication of a transpose dense vector and a
//        symmetric row-major sparse matrix (\f$ \vec{a}=B*\vec{c} \f$).
// \ingroup dense_vector
//
// \param vec The left-hand side transpose dense vector for the multiplication.
// \param mat The right-hand side row-major sparse matrix for the multiplication.
// \return The resulting transpose vector.
//
// This function implements the performance optimized treatment of the multiplication of
// a transpose dense vector and a symmetric row-major sparse matrix. It restructures the
// expression \f$ \vec{y}^T=\vec{x}^T*A \f$ to the expression \f$ \vec{y}^T=\vec{x}^T*A^T \f$.
*/
template <typename VT // Type of the left-hand side dense vector
		  ,
		  typename MT // Type of the right-hand side sparse matrix
		  ,
		  EnableIf_t<IsSymmetric_v<MT> && !(IsIdentity_v<MT> && IsSame_v<ElementType_t<VT>, ElementType_t<MT>>) &&
					 !IsZero_v<MT>> * = nullptr>
inline decltype(auto) tdvecsmatmult(const DenseVector<VT, true> &vec, const SparseMatrix<MT, false> &mat)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	METRIC_NUMERIC_INTERNAL_ASSERT((*vec).size() == (*mat).rows(), "Invalid vector and matrix sizes");

	return (*vec) * trans(*mat);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Backend implementation of the multiplication of a transpose dense vector
//        and a row-major identity matrix (\f$ \vec{a}=B*\vec{c} \f$).
// \ingroup dense_vector
//
// \param vec The left-hand side transpose dense vector for the multiplication.
// \param mat The right-hand side row-major identity matrix for the multiplication.
// \return Reference to the given dense vector.
//
// This function implements the performance optimized treatment of the multiplication of a
// transpose dense vector and a row-major identity matrix. It returns a reference to the
// given dense vector.
*/
template <typename VT // Type of the left-hand side dense vector
		  ,
		  typename MT // Type of the right-hand side sparse matrix
		  ,
		  EnableIf_t<IsIdentity_v<MT> && IsSame_v<ElementType_t<VT>, ElementType_t<MT>>> * = nullptr>
inline const VT &tdvecsmatmult(const DenseVector<VT, true> &vec, const SparseMatrix<MT, false> &mat)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	MAYBE_UNUSED(mat);

	METRIC_NUMERIC_INTERNAL_ASSERT((*vec).size() == (*mat).rows(), "Invalid vector and matrix sizes");

	return (*vec);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Backend implementation of the multiplication of a transpose dense vector
//        and a row-major zero matrix (\f$ \vec{a}=B*\vec{c} \f$).
// \ingroup dense_vector
//
// \param vec The left-hand side transpose dense vector for the multiplication.
// \param mat The right-hand side row-major zero matrix for the multiplication.
// \return The resulting zero vector.
//
// This function implements the performance optimized treatment of the multiplication of a
// transpose dense vector and a row-major zero matrix. It returns a zero vector.
*/
template <typename VT // Type of the left-hand side dense vector
		  ,
		  typename MT // Type of the right-hand side sparse matrix
		  ,
		  EnableIf_t<IsZero_v<MT>> * = nullptr>
inline decltype(auto) tdvecsmatmult(const DenseVector<VT, true> &vec, const SparseMatrix<MT, false> &mat)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	MAYBE_UNUSED(vec);

	METRIC_NUMERIC_INTERNAL_ASSERT((*vec).size() == (*mat).rows(), "Invalid vector and matrix sizes");

	using ReturnType = const MultTrait_t<ResultType_t<VT>, ResultType_t<MT>>;

	METRIC_NUMERIC_CONSTRAINT_MUST_BE_ROW_VECTOR_TYPE(ReturnType);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_ZERO_TYPE(ReturnType);

	return ReturnType((*mat).columns());
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Multiplication operator for the multiplication of a transpose dense vector and a
//        row-major sparse matrix (\f$ \vec{y}^T=\vec{x}^T*A \f$).
// \ingroup dense_vector
//
// \param vec The left-hand side transpose dense vector for the multiplication.
// \param mat The right-hand side row-major sparse matrix for the multiplication.
// \return The resulting transpose vector.
// \exception std::invalid_argument Vector and matrix sizes do not match.
//
// This operator represents the multiplication between a transpose dense vector and a row-major
// sparse matrix:

   \code
   using mtrc::numeric::rowVector;
   using mtrc::numeric::rowMajor;

   mtrc::numeric::DynamicVector<double,rowVector> x, y;
   mtrc::numeric::CompressedMatrix<double,rowMajor> A;
   // ... Resizing and initialization
   y = x * A;
   \endcode

// The operator returns an expression representing a transpose dense vector of the higher-order
// element type of the two involved element types \a VT::ElementType and \a MT::ElementType.
// Both the sparse matrix type \a VT and the dense vector type \a MT as well as the two element
// types \a VT::ElementType and \a MT::ElementType have to be supported by the MultTrait class
// template.\n
// In case the current size of the vector \a vec doesn't match the current number of rows of
// the matrix \a mat, a \a std::invalid_argument is thrown.
*/
template <typename VT // Type of the left-hand side dense vector
		  ,
		  typename MT> // Type of the right-hand side sparse matrix
inline decltype(auto) operator*(const DenseVector<VT, true> &vec, const SparseMatrix<MT, false> &mat)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_MATMATMULTEXPR_TYPE(MT);

	if ((*vec).size() != (*mat).rows()) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Vector and matrix sizes do not match");
	}

	return tdvecsmatmult(*vec, *mat);
}
//*************************************************************************************************

//=================================================================================================
//
//  ISALIGNED SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename VT, typename MT> struct IsAligned<TDVecSMatMultExpr<VT, MT>> : public IsAligned<VT> {};
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
