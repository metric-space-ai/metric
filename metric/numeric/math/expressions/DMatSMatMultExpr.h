// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_DMATSMATMULTEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_DMATSMATMULTEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/Aliases.h>
#include <metric/numeric/math/Exception.h>
#include <metric/numeric/math/constraints/ColumnMajorMatrix.h>
#include <metric/numeric/math/constraints/DenseMatrix.h>
#include <metric/numeric/math/constraints/MatMatMultExpr.h>
#include <metric/numeric/math/constraints/RequiresEvaluation.h>
#include <metric/numeric/math/constraints/RowMajorMatrix.h>
#include <metric/numeric/math/constraints/SparseMatrix.h>
#include <metric/numeric/math/constraints/StorageOrder.h>
#include <metric/numeric/math/constraints/Symmetric.h>
#include <metric/numeric/math/constraints/Zero.h>
#include <metric/numeric/math/expressions/Computation.h>
#include <metric/numeric/math/expressions/DenseMatrix.h>
#include <metric/numeric/math/expressions/Forward.h>
#include <metric/numeric/math/expressions/MatMatMultExpr.h>
#include <metric/numeric/math/functors/DeclDiag.h>
#include <metric/numeric/math/functors/DeclHerm.h>
#include <metric/numeric/math/functors/DeclLow.h>
#include <metric/numeric/math/functors/DeclSym.h>
#include <metric/numeric/math/functors/DeclUpp.h>
#include <metric/numeric/math/functors/Noop.h>
#include <metric/numeric/math/shims/Conjugate.h>
#include <metric/numeric/math/shims/IsDefault.h>
#include <metric/numeric/math/shims/Reset.h>
#include <metric/numeric/math/shims/Serial.h>
#include <metric/numeric/math/traits/DeclDiagTrait.h>
#include <metric/numeric/math/traits/DeclHermTrait.h>
#include <metric/numeric/math/traits/DeclLowTrait.h>
#include <metric/numeric/math/traits/DeclSymTrait.h>
#include <metric/numeric/math/traits/DeclUppTrait.h>
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
#include <metric/numeric/math/typetraits/IsTriangular.h>
#include <metric/numeric/math/typetraits/IsUpper.h>
#include <metric/numeric/math/typetraits/IsZero.h>
#include <metric/numeric/math/typetraits/RequiresEvaluation.h>
#include <metric/numeric/math/typetraits/Size.h>
#include <metric/numeric/math/views/Check.h>
#include <metric/numeric/system/Optimizations.h>
#include <metric/numeric/system/Thresholds.h>
#include <metric/numeric/util/Assert.h>
#include <metric/numeric/util/EnableIf.h>
#include <metric/numeric/util/FunctionTrace.h>
#include <metric/numeric/util/IntegralConstant.h>
#include <metric/numeric/util/MaybeUnused.h>
#include <metric/numeric/util/Types.h>
#include <metric/numeric/util/algorithms/Max.h>
#include <metric/numeric/util/algorithms/Min.h>
#include <metric/numeric/util/mpl/If.h>
#include <metric/numeric/util/typetraits/IsBuiltin.h>
#include <metric/numeric/util/typetraits/IsSame.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DMATSMATMULTEXPR
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Expression object for dense matrix-sparse matrix multiplications.
// \ingroup dense_matrix_expression
//
// The DMatSMatMultExpr class represents the compile time expression for multiplications between
// a row-major dense matrix and a row-major sparse matrix.
*/
template <typename MT1 // Type of the left-hand side dense matrix
		  ,
		  typename MT2 // Type of the right-hand side sparse matrix
		  ,
		  bool SF // Symmetry flag
		  ,
		  bool HF // Hermitian flag
		  ,
		  bool LF // Lower flag
		  ,
		  bool UF> // Upper flag
class DMatSMatMultExpr : public MatMatMultExpr<DenseMatrix<DMatSMatMultExpr<MT1, MT2, SF, HF, LF, UF>, false>>,
						 private Computation {
  private:
	//**Type definitions****************************************************************************
	using RT1 = ResultType_t<MT1>;	  //!< Result type of the left-hand side dense matrix expression.
	using RT2 = ResultType_t<MT2>;	  //!< Result type of the right-hand side sparse matrix expression.
	using ET1 = ElementType_t<RT1>;	  //!< Element type of the left-hand side dense matrix expression.
	using ET2 = ElementType_t<RT2>;	  //!< Element type of the right-hand side sparse matrix expression.
	using CT1 = CompositeType_t<MT1>; //!< Composite type of the left-hand side dense matrix expression.
	using CT2 = CompositeType_t<MT2>; //!< Composite type of the right-hand side sparse matrix expression.
	//**********************************************************************************************

	//**********************************************************************************************
	//! Compilation switch for the composite type of the left-hand side dense matrix expression.
	static constexpr bool evaluateLeft = (IsComputation_v<MT1> || RequiresEvaluation_v<MT1>);
	//**********************************************************************************************

	//**********************************************************************************************
	//! Compilation switch for the composite type of the right-hand side sparse matrix expression.
	static constexpr bool evaluateRight = (IsComputation_v<MT2> || RequiresEvaluation_v<MT2>);
	//**********************************************************************************************

	//**********************************************************************************************
	static constexpr bool SYM = (SF && !(HF || LF || UF));	//!< Flag for symmetric matrices.
	static constexpr bool HERM = (HF && !(LF || UF));		//!< Flag for Hermitian matrices.
	static constexpr bool LOW = (LF || ((SF || HF) && UF)); //!< Flag for lower matrices.
	static constexpr bool UPP = (UF || ((SF || HF) && LF)); //!< Flag for upper matrices.
	//**********************************************************************************************

	//**********************************************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	//! Helper variable template for the explicit application of the SFINAE principle.
	/*! This variable template is a helper for the selection of the optimal evaluation strategy.
		In case either of the two matrix operands is symmetric, the variable is set to 1 and an
		optimized evaluation strategy is selected. Otherwise the variable is set to 0 and the
		default strategy is chosen. */
	template <typename T1, typename T2, typename T3>
	static constexpr bool CanExploitSymmetry_v = (IsSymmetric_v<T2> || IsSymmetric_v<T3>);
	/*! \endcond */
	//**********************************************************************************************

	//**********************************************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	//! Helper variable template for the explicit application of the SFINAE principle.
	/*! This variable template is a helper for the selection of the parallel evaluation strategy.
		In case either of the two matrix operands requires an intermediate evaluation, the variable
		will be set to 1, otherwise it will be 0. */
	template <typename T1, typename T2, typename T3>
	static constexpr bool IsEvaluationRequired_v = (evaluateLeft || evaluateRight) && !CanExploitSymmetry_v<T1, T2, T3>;
	/*! \endcond */
	//**********************************************************************************************

	//**********************************************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	//! Helper variable template for the explicit application of the SFINAE principle.
	/*! In case the element type of the target matrix has a fixed size (i.e. is not resizable),
		the variable will be set to 1, otherwise it will be 0. */
	template <typename T1, typename T2, typename T3>
	static constexpr bool UseOptimizedKernel_v =
		useOptimizedKernels && !IsDiagonal_v<T2> && !IsResizable_v<ElementType_t<T1>>;
	/*! \endcond */
	//**********************************************************************************************

	//**********************************************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	//! Helper variable template for the explicit application of the SFINAE principle.
	/*! In case the element type of the target matrix is resizable, the variable will be set to 1,
		otherwise it will be 0. */
	template <typename T1, typename T2, typename T3>
	static constexpr bool UseDefaultKernel_v = !UseOptimizedKernel_v<T1, T2, T3>;
	/*! \endcond */
	//**********************************************************************************************

	//**********************************************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	//! Type of the functor for forwarding an expression to another assign kernel.
	/*! In case a temporary matrix needs to be created, this functor is used to forward the
		resulting expression to another assign kernel. */
	using ForwardFunctor =
		If_t<HERM, DeclHerm, If_t<SYM, DeclSym, If_t<LOW, If_t<UPP, DeclDiag, DeclLow>, If_t<UPP, DeclUpp, Noop>>>>;
	/*! \endcond */
	//**********************************************************************************************

  public:
	//**Type definitions****************************************************************************
	//! Type of this DMatSMatMultExpr instance.
	using This = DMatSMatMultExpr<MT1, MT2, SF, HF, LF, UF>;

	//! Base type of this DMatSMatMultExpr instance.
	using BaseType = MatMatMultExpr<DenseMatrix<This, false>>;

	//! Result type for expression template evaluations.
	using ResultType = typename If_t<
		HERM, DeclHermTrait<MultTrait_t<RT1, RT2>>,
		If_t<SYM, DeclSymTrait<MultTrait_t<RT1, RT2>>,
			 If_t<LOW, If_t<UPP, DeclDiagTrait<MultTrait_t<RT1, RT2>>, DeclLowTrait<MultTrait_t<RT1, RT2>>>,
				  If_t<UPP, DeclUppTrait<MultTrait_t<RT1, RT2>>, MultTrait<RT1, RT2>>>>>::Type;

	using OppositeType =
		OppositeType_t<ResultType>; //!< Result type with opposite storage order for expression template evaluations.
	using TransposeType = TransposeType_t<ResultType>; //!< Transpose type for expression template evaluations.
	using ElementType = ElementType_t<ResultType>;	   //!< Resulting element type.
	using ReturnType = const ElementType;			   //!< Return type for expression template evaluations.
	using CompositeType = const ResultType;			   //!< Data type for composite expression templates.

	//! Composite type of the left-hand side dense matrix expression.
	using LeftOperand = If_t<IsExpression_v<MT1>, const MT1, const MT1 &>;

	//! Composite type of the right-hand side sparse matrix expression.
	using RightOperand = If_t<IsExpression_v<MT2>, const MT2, const MT2 &>;

	//! Type for the assignment of the left-hand side dense matrix operand.
	using LT = If_t<evaluateLeft, const RT1, CT1>;

	//! Type for the assignment of the right-hand side sparse matrix operand.
	using RT = If_t<evaluateRight, const RT2, CT2>;
	//**********************************************************************************************

	//**Compilation flags***************************************************************************
	//! Compilation switch for the expression template evaluation strategy.
	static constexpr bool simdEnabled = false;

	//! Compilation switch for the expression template assignment strategy.
	static constexpr bool smpAssignable = (!evaluateLeft && MT1::smpAssignable && !evaluateRight && MT2::smpAssignable);
	//**********************************************************************************************

	//**Constructor*********************************************************************************
	/*!\brief Constructor for the DMatSMatMultExpr class.
	//
	// \param lhs The left-hand side dense matrix operand of the multiplication expression.
	// \param rhs The right-hand side sparse matrix operand of the multiplication expression.
	*/
	inline DMatSMatMultExpr(const MT1 &lhs, const MT2 &rhs) noexcept
		: lhs_(lhs) // Left-hand side dense matrix of the multiplication expression
		  ,
		  rhs_(rhs) // Right-hand side sparse matrix of the multiplication expression
	{
		METRIC_NUMERIC_INTERNAL_ASSERT(lhs.columns() == rhs.rows(), "Invalid matrix sizes");
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
		METRIC_NUMERIC_INTERNAL_ASSERT(i < lhs_.rows(), "Invalid row access index");
		METRIC_NUMERIC_INTERNAL_ASSERT(j < rhs_.columns(), "Invalid column access index");

		if (IsDiagonal_v<MT1>) {
			return lhs_(i, i) * rhs_(i, j);
		} else if (IsDiagonal_v<MT2>) {
			return lhs_(i, j) * rhs_(j, j);
		} else if (IsTriangular_v<MT1> || IsTriangular_v<MT2>) {
			const size_t begin((IsUpper_v<MT1>) ? ((IsLower_v<MT2>) ? (max((IsStrictlyUpper_v<MT1> ? i + 1UL : i),
																		   (IsStrictlyLower_v<MT2> ? j + 1UL : j)))
																	: (IsStrictlyUpper_v<MT1> ? i + 1UL : i))
												: ((IsLower_v<MT2>) ? (IsStrictlyLower_v<MT2> ? j + 1UL : j) : (0UL)));
			const size_t end((IsLower_v<MT1>)
								 ? ((IsUpper_v<MT2>) ? (min((IsStrictlyLower_v<MT1> ? i : i + 1UL),
															(IsStrictlyUpper_v<MT2> ? j : j + 1UL)))
													 : (IsStrictlyLower_v<MT1> ? i : i + 1UL))
								 : ((IsUpper_v<MT2>) ? (IsStrictlyUpper_v<MT2> ? j : j + 1UL) : (lhs_.columns())));

			if (begin >= end)
				return ElementType();

			const size_t n(end - begin);

			return subvector(row(lhs_, i, unchecked), begin, n, unchecked) *
				   subvector(column(rhs_, j, unchecked), begin, n, unchecked);
		} else {
			return row(lhs_, i, unchecked) * column(rhs_, j, unchecked);
		}
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
		if (i >= lhs_.rows()) {
			METRIC_NUMERIC_THROW_OUT_OF_RANGE("Invalid row access index");
		}
		if (j >= rhs_.columns()) {
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
	inline size_t rows() const noexcept { return lhs_.rows(); }
	//**********************************************************************************************

	//**Columns function****************************************************************************
	/*!\brief Returns the current number of columns of the matrix.
	//
	// \return The number of columns of the matrix.
	*/
	inline size_t columns() const noexcept { return rhs_.columns(); }
	//**********************************************************************************************

	//**Left operand access*************************************************************************
	/*!\brief Returns the left-hand side dense matrix operand.
	//
	// \return The left-hand side dense matrix operand.
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
	// \return \a true in case the expression can alias, \a false otherwise.
	*/
	template <typename T> inline bool canAlias(const T *alias) const noexcept
	{
		return (lhs_.isAliased(alias) || rhs_.isAliased(alias));
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

	//**********************************************************************************************
	/*!\brief Returns whether the operands of the expression are properly aligned in memory.
	//
	// \return \a true in case the operands are aligned, \a false if not.
	*/
	inline bool isAligned() const noexcept { return lhs_.isAligned(); }
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Returns whether the expression can be used in SMP assignments.
	//
	// \return \a true in case the expression can be used in SMP assignments, \a false if not.
	*/
	inline bool canSMPAssign() const noexcept
	{
		return (rows() * columns() >= SMP_DMATSMATMULT_THRESHOLD) && !IsDiagonal_v<MT1>;
	}
	//**********************************************************************************************

  private:
	//**Member variables****************************************************************************
	LeftOperand lhs_;  //!< Left-hand side dense matrix of the multiplication expression.
	RightOperand rhs_; //!< Right-hand side sparse matrix of the multiplication expression.
	//**********************************************************************************************

	//**Assignment to dense matrices****************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Assignment of a dense matrix-sparse matrix multiplication to a dense matrix
	//        (\f$ C=A*B \f$).
	// \ingroup dense_matrix
	//
	// \param lhs The target left-hand side dense matrix.
	// \param rhs The right-hand side multiplication expression to be assigned.
	// \return void
	//
	// This function implements the performance optimized assignment of a dense matrix-sparse
	// matrix multiplication expression to a dense matrix.
	*/
	template <typename MT // Type of the target dense matrix
			  ,
			  bool SO> // Storage order of the target dense matrix
	friend inline auto assign(DenseMatrix<MT, SO> &lhs, const DMatSMatMultExpr &rhs)
		-> DisableIf_t<CanExploitSymmetry_v<MT, MT1, MT2>>
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).rows() == rhs.rows(), "Invalid number of rows");
		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).columns() == rhs.columns(), "Invalid number of columns");

		LT A(serial(rhs.lhs_)); // Evaluation of the left-hand side dense matrix operand
		RT B(serial(rhs.rhs_)); // Evaluation of the right-hand side sparse matrix operand

		METRIC_NUMERIC_INTERNAL_ASSERT(A.rows() == rhs.lhs_.rows(), "Invalid number of rows");
		METRIC_NUMERIC_INTERNAL_ASSERT(A.columns() == rhs.lhs_.columns(), "Invalid number of columns");
		METRIC_NUMERIC_INTERNAL_ASSERT(B.rows() == rhs.rhs_.rows(), "Invalid number of rows");
		METRIC_NUMERIC_INTERNAL_ASSERT(B.columns() == rhs.rhs_.columns(), "Invalid number of columns");
		METRIC_NUMERIC_INTERNAL_ASSERT(A.rows() == (*lhs).rows(), "Invalid number of rows");
		METRIC_NUMERIC_INTERNAL_ASSERT(B.columns() == (*lhs).columns(), "Invalid number of columns");

		DMatSMatMultExpr::selectAssignKernel(*lhs, A, B);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**Assignment to dense matrices (kernel selection)*********************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Selection of the kernel for an assignment of a dense matrix-sparse matrix
	//        multiplication to a dense matrix (\f$ C=A*B \f$).
	// \ingroup dense_matrix
	//
	// \param C The target left-hand side dense matrix.
	// \param A The left-hand side multiplication operand.
	// \param B The right-hand side multiplication operand.
	// \return void
	*/
	template <typename MT3 // Type of the left-hand side target matrix
			  ,
			  typename MT4 // Type of the left-hand side matrix operand
			  ,
			  typename MT5> // Type of the right-hand side matrix operand
	static inline void selectAssignKernel(MT3 &C, const MT4 &A, const MT5 &B)
	{
		if (C.rows() * C.columns() < DMATSMATMULT_THRESHOLD)
			selectSmallAssignKernel(C, A, B);
		else
			selectLargeAssignKernel(C, A, B);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**Default assignment to dense matrices********************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Default assignment of a dense matrix-sparse matrix multiplication (\f$ C=A*B \f$).
	// \ingroup dense_matrix
	//
	// \param C The target left-hand side dense matrix.
	// \param A The left-hand side multiplication operand.
	// \param B The right-hand side multiplication operand.
	// \return void
	//
	// This function implements the default assignment of a dense matrix-sparse matrix
	// multiplication expression to a dense matrix. This assign function is used in case
	// the element type of the target matrix is resizable.
	*/
	template <typename MT3 // Type of the left-hand side target matrix
			  ,
			  typename MT4 // Type of the left-hand side matrix operand
			  ,
			  typename MT5> // Type of the right-hand side matrix operand
	static inline void selectDefaultAssignKernel(MT3 &C, const MT4 &A, const MT5 &B)
	{
		reset(C);

		for (size_t k = 0UL; k < B.rows(); ++k) {
			if (IsDiagonal_v<MT4>) {
				auto element(B.begin(k));
				const auto end(B.end(k));

				for (; element != end; ++element) {
					C(k, element->index()) = A(k, k) * element->value();
				}
			} else {
				const size_t iibegin((IsLower_v<MT4>) ? (IsStrictlyLower_v<MT4> ? k + 1UL : k) : (0UL));
				const size_t iiend((IsUpper_v<MT4>) ? (IsStrictlyUpper_v<MT4> ? k : k + 1UL) : (A.rows()));
				METRIC_NUMERIC_INTERNAL_ASSERT(iibegin <= iiend, "Invalid loop indices detected");

				for (size_t ii = iibegin; ii < iiend; ii += 8UL) {
					auto element(B.begin(k));
					const auto end(B.end(k));
					const size_t itmp((ii + 8UL > iiend) ? (iiend) : (ii + 8UL));

					for (; element != end; ++element) {
						const size_t j1(element->index());
						const size_t ibegin(SYM || HERM || LOW ? max(j1, ii) : ii);
						const size_t iend(UPP ? min(j1 + 1UL, itmp) : itmp);

						for (size_t i = ibegin; i < iend; ++i) {
							if (isDefault(C(i, element->index())))
								C(i, j1) = A(i, k) * element->value();
							else
								C(i, j1) += A(i, k) * element->value();
						}
					}
				}
			}
		}

		if (SYM || HERM) {
			for (size_t i = 0UL; i < A.rows(); ++i) {
				for (size_t j = i + 1UL; j < B.columns(); ++j) {
					C(i, j) = HERM ? conj(C(j, i)) : C(j, i);
				}
			}
		}
	}
	/*! \endcond */
	//**********************************************************************************************

	//**Default assignment to dense matrices (small matrices)***************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Default assignment of a small dense matrix-sparse matrix multiplication (\f$ C=A*B \f$).
	// \ingroup dense_matrix
	//
	// \param C The target left-hand side dense matrix.
	// \param A The left-hand side multiplication operand.
	// \param B The right-hand side multiplication operand.
	// \return void
	//
	// This function relays to the default implementation of the assignment of a dense matrix-
	// sparse matrix multiplication expression to a dense matrix.
	*/
	template <typename MT3 // Type of the left-hand side target matrix
			  ,
			  typename MT4 // Type of the left-hand side matrix operand
			  ,
			  typename MT5> // Type of the right-hand side matrix operand
	static inline auto selectSmallAssignKernel(MT3 &C, const MT4 &A, const MT5 &B)
		-> EnableIf_t<UseDefaultKernel_v<MT3, MT4, MT5>>
	{
		selectDefaultAssignKernel(C, A, B);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**Optimized assignment to dense matrices (small matrices)*************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Optimized assignment of a small dense matrix-sparse matrix multiplication
	//        (\f$ C=A*B \f$).
	// \ingroup dense_matrix
	//
	// \param C The target left-hand side dense matrix.
	// \param A The left-hand side multiplication operand.
	// \param B The right-hand side multiplication operand.
	// \return void
	//
	// This function implements the performance optimized assignment of a dense matrix-sparse
	// matrix multiplication expression to a dense matrix. This assign function is used in case
	// the element type of the target matrix is not resizable.
	*/
	template <typename MT3 // Type of the left-hand side target matrix
			  ,
			  typename MT4 // Type of the left-hand side matrix operand
			  ,
			  typename MT5> // Type of the right-hand side matrix operand
	static inline auto selectSmallAssignKernel(MT3 &C, const MT4 &A, const MT5 &B)
		-> EnableIf_t<UseOptimizedKernel_v<MT3, MT4, MT5>>
	{
		reset(C);

		for (size_t j = 0UL; j < B.rows(); ++j) {
			const size_t iibegin((IsLower_v<MT4>) ? (IsStrictlyLower_v<MT4> ? j + 1UL : j) : (0UL));
			const size_t iiend((IsUpper_v<MT4>) ? (IsStrictlyUpper_v<MT4> ? j : j + 1UL) : (A.rows()));
			METRIC_NUMERIC_INTERNAL_ASSERT(iibegin <= iiend, "Invalid loop indices detected");

			for (size_t ii = iibegin; ii < iiend; ii += 8UL) {
				auto element(B.begin(j));
				const auto end(B.end(j));
				const size_t itmp((ii + 8UL > iiend) ? (iiend) : (ii + 8UL));

				for (; element != end; ++element) {
					const size_t j1(element->index());
					const size_t ibegin(SYM || HERM || LOW ? max(j1, ii) : ii);
					const size_t iend(UPP ? min(j1 + 1UL, itmp) : itmp);

					for (size_t i = ibegin; i < iend; ++i) {
						C(i, j1) += A(i, j) * element->value();
					}
				}
			}
		}

		if (SYM || HERM) {
			for (size_t i = 0UL; i < A.rows(); ++i) {
				for (size_t j = i + 1UL; j < B.columns(); ++j) {
					C(i, j) = HERM ? conj(C(j, i)) : C(j, i);
				}
			}
		}
	}
	/*! \endcond */
	//**********************************************************************************************

	//**Default assignment to dense matrices (large matrices)***************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Default assignment of a large dense matrix-sparse matrix multiplication (\f$ C=A*B \f$).
	// \ingroup dense_matrix
	//
	// \param C The target left-hand side dense matrix.
	// \param A The left-hand side multiplication operand.
	// \param B The right-hand side multiplication operand.
	// \return void
	//
	// This function relays to the default implementation of the assignment of a dense matrix-
	// sparse matrix multiplication expression to a dense matrix.
	*/
	template <typename MT3 // Type of the left-hand side target matrix
			  ,
			  typename MT4 // Type of the left-hand side matrix operand
			  ,
			  typename MT5> // Type of the right-hand side matrix operand
	static inline auto selectLargeAssignKernel(MT3 &C, const MT4 &A, const MT5 &B)
		-> EnableIf_t<UseDefaultKernel_v<MT3, MT4, MT5>>
	{
		selectDefaultAssignKernel(C, A, B);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**Optimized assignment to dense matrices (large matrices)*************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Default assignment of a large dense matrix-sparse matrix multiplication
	//        (\f$ C=A*B \f$).
	// \ingroup dense_matrix
	//
	// \param C The target left-hand side dense matrix.
	// \param A The left-hand side multiplication operand.
	// \param B The right-hand side multiplication operand.
	// \return void
	//
	// This function implements the performance optimized assignment of a dense matrix-sparse
	// matrix multiplication expression to a dense matrix. This assign function is used in case
	// the element type of the target matrix is not resizable.
	*/
	template <typename MT3 // Type of the left-hand side target matrix
			  ,
			  typename MT4 // Type of the left-hand side matrix operand
			  ,
			  typename MT5> // Type of the right-hand side matrix operand
	static inline auto selectLargeAssignKernel(MT3 &C, const MT4 &A, const MT5 &B)
		-> EnableIf_t<UseOptimizedKernel_v<MT3, MT4, MT5>>
	{
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_COLUMN_MAJOR_MATRIX_TYPE(OppositeType_t<MT5>);

		const ForwardFunctor fwd;

		const OppositeType_t<MT5> tmp(serial(B));
		assign(C, fwd(A * tmp));
	}
	/*! \endcond */
	//**********************************************************************************************

	//**Assignment to sparse matrices***************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Assignment of a dense matrix-sparse matrix multiplication to a sparse matrix
	//        (\f$ C=A*B \f$).
	// \ingroup dense_matrix
	//
	// \param lhs The target left-hand side sparse matrix.
	// \param rhs The right-hand side multiplication expression to be assigned.
	// \return void
	//
	// This function implements the performance optimized assignment of a dense matrix-sparse
	// matrix multiplication expression to a sparse matrix.
	*/
	template <typename MT // Type of the target sparse matrix
			  ,
			  bool SO> // Storage order of the target sparse matrix
	friend inline auto assign(SparseMatrix<MT, SO> &lhs, const DMatSMatMultExpr &rhs)
		-> DisableIf_t<CanExploitSymmetry_v<MT, MT1, MT2>>
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		using TmpType = If_t<SO, OppositeType, ResultType>;

		METRIC_NUMERIC_CONSTRAINT_MUST_BE_DENSE_MATRIX_TYPE(ResultType);
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_DENSE_MATRIX_TYPE(OppositeType);
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_ROW_MAJOR_MATRIX_TYPE(ResultType);
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_COLUMN_MAJOR_MATRIX_TYPE(OppositeType);
		METRIC_NUMERIC_CONSTRAINT_MATRICES_MUST_HAVE_SAME_STORAGE_ORDER(MT, TmpType);
		METRIC_NUMERIC_CONSTRAINT_MUST_NOT_REQUIRE_EVALUATION(TmpType);

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).rows() == rhs.rows(), "Invalid number of rows");
		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).columns() == rhs.columns(), "Invalid number of columns");

		const ForwardFunctor fwd;

		const TmpType tmp(serial(rhs));
		assign(*lhs, fwd(tmp));
	}
	/*! \endcond */
	//**********************************************************************************************

	//**Restructuring assignment********************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Restructuring assignment of a dense matrix-sparse matrix multiplication
	//        (\f$ C=A*B \f$).
	// \ingroup dense_matrix
	//
	// \param lhs The target left-hand side matrix.
	// \param rhs The right-hand side multiplication expression to be assigned.
	// \return void
	//
	// This function implements the symmetry-based restructuring assignment of a dense matrix-
	// sparse matrix multiplication expression. Due to the explicit application of the SFINAE
	// principle this function can only be selected by the compiler in case the symmetry of
	// either of the two matrix operands can be exploited.
	*/
	template <typename MT // Type of the target matrix
			  ,
			  bool SO> // Storage order of the target matrix
	friend inline auto assign(Matrix<MT, SO> &lhs, const DMatSMatMultExpr &rhs)
		-> EnableIf_t<CanExploitSymmetry_v<MT, MT1, MT2>>
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_SYMMETRIC_MATRIX_TYPE(MT);

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).rows() == rhs.rows(), "Invalid number of rows");
		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).columns() == rhs.columns(), "Invalid number of columns");

		const ForwardFunctor fwd;

		decltype(auto) A(transIf<IsSymmetric_v<MT1>>(rhs.lhs_));
		decltype(auto) B(transIf<IsSymmetric_v<MT2>>(rhs.rhs_));

		assign(*lhs, fwd(A * B));
	}
	/*! \endcond */
	//**********************************************************************************************

	//**Addition assignment to dense matrices*******************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Addition assignment of a dense matrix-sparse matrix multiplication to a dense matrix
	//        (\f$ C+=A*B \f$).
	// \ingroup dense_matrix
	//
	// \param lhs The target left-hand side dense matrix.
	// \param rhs The right-hand side multiplication expression to be added.
	// \return void
	//
	// This function implements the performance optimized addition assignment of a dense matrix-
	// sparse matrix multiplication expression to a dense matrix.
	*/
	template <typename MT // Type of the target dense matrix
			  ,
			  bool SO> // Storage order of the target dense matrix
	friend inline auto addAssign(DenseMatrix<MT, SO> &lhs, const DMatSMatMultExpr &rhs)
		-> DisableIf_t<CanExploitSymmetry_v<MT, MT1, MT2>>
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).rows() == rhs.rows(), "Invalid number of rows");
		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).columns() == rhs.columns(), "Invalid number of columns");

		LT A(serial(rhs.lhs_)); // Evaluation of the left-hand side dense matrix operand
		RT B(serial(rhs.rhs_)); // Evaluation of the right-hand side sparse matrix operand

		METRIC_NUMERIC_INTERNAL_ASSERT(A.rows() == rhs.lhs_.rows(), "Invalid number of rows");
		METRIC_NUMERIC_INTERNAL_ASSERT(A.columns() == rhs.lhs_.columns(), "Invalid number of columns");
		METRIC_NUMERIC_INTERNAL_ASSERT(B.rows() == rhs.rhs_.rows(), "Invalid number of rows");
		METRIC_NUMERIC_INTERNAL_ASSERT(B.columns() == rhs.rhs_.columns(), "Invalid number of columns");
		METRIC_NUMERIC_INTERNAL_ASSERT(A.rows() == (*lhs).rows(), "Invalid number of rows");
		METRIC_NUMERIC_INTERNAL_ASSERT(B.columns() == (*lhs).columns(), "Invalid number of columns");

		DMatSMatMultExpr::selectAddAssignKernel(*lhs, A, B);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**Addition assignment to dense matrices (kernel selection)************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Selection of the kernel for an addition assignment of a dense matrix-sparse matrix
	//        multiplication to a dense matrix (\f$ C+=A*B \f$).
	// \ingroup dense_matrix
	//
	// \param C The target left-hand side dense matrix.
	// \param A The left-hand side multiplication operand.
	// \param B The right-hand side multiplication operand.
	// \return void
	*/
	template <typename MT3 // Type of the left-hand side target matrix
			  ,
			  typename MT4 // Type of the left-hand side matrix operand
			  ,
			  typename MT5> // Type of the right-hand side matrix operand
	static inline void selectAddAssignKernel(MT3 &C, const MT4 &A, const MT5 &B)
	{
		if (C.rows() * C.columns() < DMATSMATMULT_THRESHOLD)
			selectSmallAddAssignKernel(C, A, B);
		else
			selectLargeAddAssignKernel(C, A, B);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**Optimized addition assignment to dense matrices (small matrices)****************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Optimized addition assignment of a small dense matrix-sparse matrix multiplication
	//        (\f$ C+=A*B \f$).
	// \ingroup dense_matrix
	//
	// \param C The target left-hand side dense matrix.
	// \param A The left-hand side multiplication operand.
	// \param B The right-hand side multiplication operand.
	// \return void
	//
	// This function implements the performance optimized addition assignment of a dense matrix-
	// sparse matrix multiplication expression to a dense matrix.
	*/
	template <typename MT3 // Type of the left-hand side target matrix
			  ,
			  typename MT4 // Type of the left-hand side matrix operand
			  ,
			  typename MT5> // Type of the right-hand side matrix operand
	static inline void selectSmallAddAssignKernel(MT3 &C, const MT4 &A, const MT5 &B)
	{
		for (size_t j = 0UL; j < B.rows(); ++j) {
			if (IsDiagonal_v<MT4>) {
				auto element(B.begin(j));
				const auto end(B.end(j));

				for (; element != end; ++element) {
					C(j, element->index()) += A(j, j) * element->value();
				}
			} else {
				const size_t iibegin((IsLower_v<MT4>) ? (IsStrictlyLower_v<MT4> ? j + 1UL : j) : (0UL));
				const size_t iiend((IsUpper_v<MT4>) ? (IsStrictlyUpper_v<MT4> ? j : j + 1UL) : (A.rows()));
				METRIC_NUMERIC_INTERNAL_ASSERT(iibegin <= iiend, "Invalid loop indices detected");

				for (size_t ii = iibegin; ii < iiend; ii += 8UL) {
					auto element(B.begin(j));
					const auto end(B.end(j));
					const size_t itmp((ii + 8UL > iiend) ? (iiend) : (ii + 8UL));

					for (; element != end; ++element) {
						const size_t j1(element->index());
						const size_t ibegin(LOW ? max(j1, ii) : ii);
						const size_t iend(UPP ? min(j1 + 1UL, itmp) : itmp);

						for (size_t i = ibegin; i < iend; ++i) {
							C(i, j1) += A(i, j) * element->value();
						}
					}
				}
			}
		}
	}
	/*! \endcond */
	//**********************************************************************************************

	//**Optimized addition assignment to dense matrices (large matrices)****************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Default addition assignment of a large dense matrix-sparse matrix multiplication
	//        (\f$ C+=A*B \f$).
	// \ingroup dense_matrix
	//
	// \param C The target left-hand side dense matrix.
	// \param A The left-hand side multiplication operand.
	// \param B The right-hand side multiplication operand.
	// \return void
	//
	// This function implements the performance optimized addition assignment of a dense matrix-
	// sparse matrix multiplication expression to a dense matrix.
	*/
	template <typename MT3 // Type of the left-hand side target matrix
			  ,
			  typename MT4 // Type of the left-hand side matrix operand
			  ,
			  typename MT5> // Type of the right-hand side matrix operand
	static inline void selectLargeAddAssignKernel(MT3 &C, const MT4 &A, const MT5 &B)
	{
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_COLUMN_MAJOR_MATRIX_TYPE(OppositeType_t<MT5>);

		const ForwardFunctor fwd;

		const OppositeType_t<MT5> tmp(serial(B));
		addAssign(C, fwd(A * tmp));
	}
	/*! \endcond */
	//**********************************************************************************************

	//**Restructuring addition assignment***********************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Restructuring addition assignment of a dense matrix-sparse matrix multiplication
	//        (\f$ C+=A*B \f$).
	// \ingroup dense_matrix
	//
	// \param lhs The target left-hand side matrix.
	// \param rhs The right-hand side multiplication expression to be added.
	// \return void
	//
	// This function implements the symmetry-based restructuring addition assignment of a dense
	// matrix-sparse matrix multiplication expression. Due to the explicit application of the
	// SFINAE principle this function can only be selected by the compiler in case the symmetry
	// of either of the two matrix operands can be exploited.
	*/
	template <typename MT // Type of the target matrix
			  ,
			  bool SO> // Storage order of the target matrix
	friend inline auto addAssign(Matrix<MT, SO> &lhs, const DMatSMatMultExpr &rhs)
		-> EnableIf_t<CanExploitSymmetry_v<MT, MT1, MT2>>
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_SYMMETRIC_MATRIX_TYPE(MT);

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).rows() == rhs.rows(), "Invalid number of rows");
		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).columns() == rhs.columns(), "Invalid number of columns");

		const ForwardFunctor fwd;

		decltype(auto) A(transIf<IsSymmetric_v<MT1>>(rhs.lhs_));
		decltype(auto) B(transIf<IsSymmetric_v<MT2>>(rhs.rhs_));

		addAssign(*lhs, fwd(A * B));
	}
	/*! \endcond */
	//**********************************************************************************************

	//**Addition assignment to sparse matrices******************************************************
	// No special implementation for the addition assignment to sparse matrices.
	//**********************************************************************************************

	//**Subtraction assignment to dense matrices****************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Subtraction assignment of a dense matrix-sparse matrix multiplication to a dense
	//        dense matrix (\f$ C-=A*B \f$).
	// \ingroup dense_matrix
	//
	// \param lhs The target left-hand side dense matrix.
	// \param rhs The right-hand side multiplication expression to be subtracted.
	// \return void
	//
	// This function implements the performance optimized subtraction assignment of a dense matrix-
	// sparse matrix multiplication expression to a dense matrix.
	*/
	template <typename MT // Type of the target dense matrix
			  ,
			  bool SO> // Storage order of the target dense matrix
	friend inline auto subAssign(DenseMatrix<MT, SO> &lhs, const DMatSMatMultExpr &rhs)
		-> DisableIf_t<CanExploitSymmetry_v<MT, MT1, MT2>>
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).rows() == rhs.rows(), "Invalid number of rows");
		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).columns() == rhs.columns(), "Invalid number of columns");

		LT A(serial(rhs.lhs_)); // Evaluation of the left-hand side dense matrix operand
		RT B(serial(rhs.rhs_)); // Evaluation of the right-hand side sparse matrix operand

		METRIC_NUMERIC_INTERNAL_ASSERT(A.rows() == rhs.lhs_.rows(), "Invalid number of rows");
		METRIC_NUMERIC_INTERNAL_ASSERT(A.columns() == rhs.lhs_.columns(), "Invalid number of columns");
		METRIC_NUMERIC_INTERNAL_ASSERT(B.rows() == rhs.rhs_.rows(), "Invalid number of rows");
		METRIC_NUMERIC_INTERNAL_ASSERT(B.columns() == rhs.rhs_.columns(), "Invalid number of columns");
		METRIC_NUMERIC_INTERNAL_ASSERT(A.rows() == (*lhs).rows(), "Invalid number of rows");
		METRIC_NUMERIC_INTERNAL_ASSERT(B.columns() == (*lhs).columns(), "Invalid number of columns");

		DMatSMatMultExpr::selectSubAssignKernel(*lhs, A, B);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**Subtraction assignment to dense matrices (kernel selection)*********************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Selection of the kernel for a subtraction assignment of a dense matrix-sparse matrix
	//        multiplication to a dense matrix (\f$ C-=A*B \f$).
	// \ingroup dense_matrix
	//
	// \param C The target left-hand side dense matrix.
	// \param A The left-hand side multiplication operand.
	// \param B The right-hand side multiplication operand.
	// \return void
	*/
	template <typename MT3 // Type of the left-hand side target matrix
			  ,
			  typename MT4 // Type of the left-hand side matrix operand
			  ,
			  typename MT5> // Type of the right-hand side matrix operand
	static inline void selectSubAssignKernel(MT3 &C, const MT4 &A, const MT5 &B)
	{
		if (C.rows() * C.columns() < DMATSMATMULT_THRESHOLD)
			selectSmallSubAssignKernel(C, A, B);
		else
			selectLargeSubAssignKernel(C, A, B);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**Optimized subtraction assignment to dense matrices (small matrices)*************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Optimized subtraction assignment of a small dense matrix-sparse matrix multiplication
	//        (\f$ C-=A*B \f$).
	// \ingroup dense_matrix
	//
	// \param C The target left-hand side dense matrix.
	// \param A The left-hand side multiplication operand.
	// \param B The right-hand side multiplication operand.
	// \return void
	//
	// This function implements the performance optimized subtraction assignment of a dense matrix-
	// sparse matrix multiplication expression to a column-major dense matrix.
	*/
	template <typename MT3 // Type of the left-hand side target matrix
			  ,
			  typename MT4 // Type of the left-hand side matrix operand
			  ,
			  typename MT5> // Type of the right-hand side matrix operand
	static inline void selectSmallSubAssignKernel(MT3 &C, const MT4 &A, const MT5 &B)
	{
		for (size_t j = 0UL; j < B.rows(); ++j) {
			if (IsDiagonal_v<MT4>) {
				auto element(B.begin(j));
				const auto end(B.end(j));

				for (; element != end; ++element) {
					C(j, element->index()) -= A(j, j) * element->value();
				}
			} else {
				const size_t iibegin((IsLower_v<MT4>) ? (IsStrictlyLower_v<MT4> ? j + 1UL : j) : (0UL));
				const size_t iiend((IsUpper_v<MT4>) ? (IsStrictlyUpper_v<MT4> ? j : j + 1UL) : (A.rows()));
				METRIC_NUMERIC_INTERNAL_ASSERT(iibegin <= iiend, "Invalid loop indices detected");

				for (size_t ii = iibegin; ii < iiend; ii += 8UL) {
					auto element(B.begin(j));
					const auto end(B.end(j));
					const size_t itmp((ii + 8UL > iiend) ? (iiend) : (ii + 8UL));

					for (; element != end; ++element) {
						const size_t j1(element->index());
						const size_t ibegin(LOW ? max(j1, ii) : ii);
						const size_t iend(UPP ? min(j1 + 1UL, itmp) : itmp);

						for (size_t i = ibegin; i < iend; ++i) {
							C(i, j1) -= A(i, j) * element->value();
						}
					}
				}
			}
		}
	}
	/*! \endcond */
	//**********************************************************************************************

	//**Optimized subtraction assignment to dense matrices (large matrices)*************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Default subtraction assignment of a large dense matrix-sparse matrix multiplication
	//        (\f$ C-=A*B \f$).
	// \ingroup dense_matrix
	//
	// \param C The target left-hand side dense matrix.
	// \param A The left-hand side multiplication operand.
	// \param B The right-hand side multiplication operand.
	// \return void
	//
	// This function implements the performance optimized subtraction assignment of a dense matrix-
	// sparse matrix multiplication expression to a column-major dense matrix.
	*/
	template <typename MT3 // Type of the left-hand side target matrix
			  ,
			  typename MT4 // Type of the left-hand side matrix operand
			  ,
			  typename MT5> // Type of the right-hand side matrix operand
	static inline void selectLargeSubAssignKernel(MT3 &C, const MT4 &A, const MT5 &B)
	{
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_COLUMN_MAJOR_MATRIX_TYPE(OppositeType_t<MT5>);

		const ForwardFunctor fwd;

		const OppositeType_t<MT5> tmp(serial(B));
		subAssign(C, fwd(A * tmp));
	}
	/*! \endcond */
	//**********************************************************************************************

	//**Restructuring subtraction assignment********************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Restructuring subtraction assignment of a dense matrix-sparse matrix multiplication
	//        (\f$ C-=A*B \f$).
	// \ingroup dense_matrix
	//
	// \param lhs The target left-hand side matrix.
	// \param rhs The right-hand side multiplication expression to be subtracted.
	// \return void
	//
	// This function implements the symmetry-based restructuring subtraction assignment of a
	// dense matrix-sparse matrix multiplication expression. Due to the explicit application
	// of the SFINAE principle this function can only be selected by the compiler in case the
	// symmetry of either of the two matrix operands can be exploited.
	*/
	template <typename MT // Type of the target matrix
			  ,
			  bool SO> // Storage order of the target matrix
	friend inline auto subAssign(Matrix<MT, SO> &lhs, const DMatSMatMultExpr &rhs)
		-> EnableIf_t<CanExploitSymmetry_v<MT, MT1, MT2>>
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_SYMMETRIC_MATRIX_TYPE(MT);

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).rows() == rhs.rows(), "Invalid number of rows");
		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).columns() == rhs.columns(), "Invalid number of columns");

		const ForwardFunctor fwd;

		decltype(auto) A(transIf<IsSymmetric_v<MT1>>(rhs.lhs_));
		decltype(auto) B(transIf<IsSymmetric_v<MT2>>(rhs.rhs_));

		subAssign(*lhs, fwd(A * B));
	}
	/*! \endcond */
	//**********************************************************************************************

	//**Subtraction assignment to sparse matrices***************************************************
	// No special implementation for the subtraction assignment to sparse matrices.
	//**********************************************************************************************

	//**Schur product assignment to dense matrices**************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Schur product assignment of a dense matrix-sparse matrix multiplication to a dense
	//        dense matrix (\f$ C\circ=A*B \f$).
	// \ingroup dense_matrix
	//
	// \param lhs The target left-hand side dense matrix.
	// \param rhs The right-hand side multiplication expression for the Schur product.
	// \return void
	//
	// This function implements the performance optimized Schur product assignment of a dense
	// matrix-sparse matrix multiplication expression to a dense matrix.
	*/
	template <typename MT // Type of the target dense matrix
			  ,
			  bool SO> // Storage order of the target dense matrix
	friend inline void schurAssign(DenseMatrix<MT, SO> &lhs, const DMatSMatMultExpr &rhs)
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_CONSTRAINT_MUST_BE_DENSE_MATRIX_TYPE(ResultType);
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_ROW_MAJOR_MATRIX_TYPE(ResultType);
		METRIC_NUMERIC_CONSTRAINT_MUST_NOT_REQUIRE_EVALUATION(ResultType);

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).rows() == rhs.rows(), "Invalid number of rows");
		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).columns() == rhs.columns(), "Invalid number of columns");

		const ResultType tmp(serial(rhs));
		schurAssign(*lhs, tmp);
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
	/*!\brief SMP assignment of a dense matrix-sparse matrix multiplication to a dense matrix
	//        (\f$ C=A*B \f$).
	// \ingroup dense_matrix
	//
	// \param lhs The target left-hand side dense matrix.
	// \param rhs The right-hand side multiplication expression to be assigned.
	// \return void
	//
	// This function implements the performance optimized SMP assignment of a dense matrix-sparse
	// matrix multiplication expression to a dense matrix. Due to the explicit application of the
	// SFINAE principle, this function can only be selected by the compiler in case either of the
	// two matrix operands requires an intermediate evaluation and no symmetry can be exploited.
	*/
	template <typename MT // Type of the target dense matrix
			  ,
			  bool SO> // Storage order of the target dense matrix
	friend inline auto smpAssign(DenseMatrix<MT, SO> &lhs, const DMatSMatMultExpr &rhs)
		-> EnableIf_t<IsEvaluationRequired_v<MT, MT1, MT2>>
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).rows() == rhs.rows(), "Invalid number of rows");
		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).columns() == rhs.columns(), "Invalid number of columns");

		LT A(rhs.lhs_); // Evaluation of the left-hand side dense matrix operand
		RT B(rhs.rhs_); // Evaluation of the right-hand side sparse matrix operand

		METRIC_NUMERIC_INTERNAL_ASSERT(A.rows() == rhs.lhs_.rows(), "Invalid number of rows");
		METRIC_NUMERIC_INTERNAL_ASSERT(A.columns() == rhs.lhs_.columns(), "Invalid number of columns");
		METRIC_NUMERIC_INTERNAL_ASSERT(B.rows() == rhs.rhs_.rows(), "Invalid number of rows");
		METRIC_NUMERIC_INTERNAL_ASSERT(B.columns() == rhs.rhs_.columns(), "Invalid number of columns");
		METRIC_NUMERIC_INTERNAL_ASSERT(A.rows() == (*lhs).rows(), "Invalid number of rows");
		METRIC_NUMERIC_INTERNAL_ASSERT(B.columns() == (*lhs).columns(), "Invalid number of columns");

		smpAssign(*lhs, A * B);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**SMP assignment to sparse matrices***********************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief SMP assignment of a dense matrix-sparse matrix multiplication to a sparse matrix
	//        (\f$ C=A*B \f$).
	// \ingroup dense_matrix
	//
	// \param lhs The target left-hand side sparse matrix.
	// \param rhs The right-hand side multiplication expression to be assigned.
	// \return void
	//
	// This function implements the performance optimized SMP assignment of a dense matrix-sparse
	// matrix multiplication expression to a sparse matrix. Due to the explicit application of the
	// SFINAE principle, this function can only be selected by the compiler in case either of the
	// two matrix operands requires an intermediate evaluation and no symmetry can be exploited.
	*/
	template <typename MT // Type of the target sparse matrix
			  ,
			  bool SO> // Storage order of the target sparse matrix
	friend inline auto smpAssign(SparseMatrix<MT, SO> &lhs, const DMatSMatMultExpr &rhs)
		-> EnableIf_t<IsEvaluationRequired_v<MT, MT1, MT2>>
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		using TmpType = If_t<SO, OppositeType, ResultType>;

		METRIC_NUMERIC_CONSTRAINT_MUST_BE_DENSE_MATRIX_TYPE(ResultType);
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_DENSE_MATRIX_TYPE(OppositeType);
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_ROW_MAJOR_MATRIX_TYPE(ResultType);
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_COLUMN_MAJOR_MATRIX_TYPE(OppositeType);
		METRIC_NUMERIC_CONSTRAINT_MATRICES_MUST_HAVE_SAME_STORAGE_ORDER(MT, TmpType);
		METRIC_NUMERIC_CONSTRAINT_MUST_NOT_REQUIRE_EVALUATION(TmpType);

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).rows() == rhs.rows(), "Invalid number of rows");
		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).columns() == rhs.columns(), "Invalid number of columns");

		const ForwardFunctor fwd;

		const TmpType tmp(rhs);
		smpAssign(*lhs, fwd(tmp));
	}
	/*! \endcond */
	//**********************************************************************************************

	//**Restructuring SMP assignment****************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Restructuring SMP assignment of a dense matrix-sparse matrix multiplication
	//        (\f$ C=A*B \f$).
	// \ingroup dense_matrix
	//
	// \param lhs The target left-hand side matrix.
	// \param rhs The right-hand side multiplication expression to be assigned.
	// \return void
	//
	// This function implements the symmetry-based restructuring SMP assignment of a dense matrix-
	// sparse matrix multiplication expression. Due to the explicit application of the SFINAE
	// principle this function can only be selected by the compiler in case the symmetry of either
	// of the two matrix operands can be exploited.
	*/
	template <typename MT // Type of the target matrix
			  ,
			  bool SO> // Storage order of the target matrix
	friend inline auto smpAssign(Matrix<MT, SO> &lhs, const DMatSMatMultExpr &rhs)
		-> EnableIf_t<CanExploitSymmetry_v<MT, MT1, MT2>>
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_SYMMETRIC_MATRIX_TYPE(MT);

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).rows() == rhs.rows(), "Invalid number of rows");
		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).columns() == rhs.columns(), "Invalid number of columns");

		const ForwardFunctor fwd;

		decltype(auto) A(transIf<IsSymmetric_v<MT1>>(rhs.lhs_));
		decltype(auto) B(transIf<IsSymmetric_v<MT2>>(rhs.rhs_));

		smpAssign(*lhs, fwd(A * B));
	}
	/*! \endcond */
	//**********************************************************************************************

	//**SMP addition assignment to dense matrices***************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief SMP addition assignment of a dense matrix-sparse matrix multiplication to a dense
	//        matrix (\f$ C+=A*B \f$).
	// \ingroup dense_matrix
	//
	// \param lhs The target left-hand side dense matrix.
	// \param rhs The right-hand side multiplication expression to be added.
	// \return void
	//
	// This function implements the performance optimized SMP addition assignment of a dense matrix-
	// sparse matrix multiplication expression to a dense matrix. Due to the explicit application of
	// the SFINAE principle, this function can only be selected by the compiler in case either of
	// the two matrix operands requires an intermediate evaluation and no symmetry can be exploited.
	*/
	template <typename MT // Type of the target dense matrix
			  ,
			  bool SO> // Storage order of the target sparse matrix
	friend inline auto smpAddAssign(DenseMatrix<MT, SO> &lhs, const DMatSMatMultExpr &rhs)
		-> EnableIf_t<IsEvaluationRequired_v<MT, MT1, MT2>>
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).rows() == rhs.rows(), "Invalid number of rows");
		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).columns() == rhs.columns(), "Invalid number of columns");

		LT A(rhs.lhs_); // Evaluation of the left-hand side dense matrix operand
		RT B(rhs.rhs_); // Evaluation of the right-hand side sparse matrix operand

		METRIC_NUMERIC_INTERNAL_ASSERT(A.rows() == rhs.lhs_.rows(), "Invalid number of rows");
		METRIC_NUMERIC_INTERNAL_ASSERT(A.columns() == rhs.lhs_.columns(), "Invalid number of columns");
		METRIC_NUMERIC_INTERNAL_ASSERT(B.rows() == rhs.rhs_.rows(), "Invalid number of rows");
		METRIC_NUMERIC_INTERNAL_ASSERT(B.columns() == rhs.rhs_.columns(), "Invalid number of columns");
		METRIC_NUMERIC_INTERNAL_ASSERT(A.rows() == (*lhs).rows(), "Invalid number of rows");
		METRIC_NUMERIC_INTERNAL_ASSERT(B.columns() == (*lhs).columns(), "Invalid number of columns");

		smpAddAssign(*lhs, A * B);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**Restructuring SMP addition assignment*******************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Restructuring SMP addition assignment of a dense matrix-sparse matrix multiplication
	//        (\f$ C+=A*B \f$).
	// \ingroup dense_matrix
	//
	// \param lhs The target left-hand side matrix.
	// \param rhs The right-hand side multiplication expression to be added.
	// \return void
	//
	// This function implements the symmetry-based restructuring SMP addition assignment of a
	// dense matrix-sparse matrix multiplication expression. Due to the explicit application
	// of the SFINAE principle this function can only be selected by the compiler in case the
	// symmetry of either of the two matrix operands can be exploited.
	*/
	template <typename MT // Type of the target matrix
			  ,
			  bool SO> // Storage order of the target matrix
	friend inline auto smpAddAssign(Matrix<MT, SO> &lhs, const DMatSMatMultExpr &rhs)
		-> EnableIf_t<CanExploitSymmetry_v<MT, MT1, MT2>>
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_SYMMETRIC_MATRIX_TYPE(MT);

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).rows() == rhs.rows(), "Invalid number of rows");
		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).columns() == rhs.columns(), "Invalid number of columns");

		const ForwardFunctor fwd;

		decltype(auto) A(transIf<IsSymmetric_v<MT1>>(rhs.lhs_));
		decltype(auto) B(transIf<IsSymmetric_v<MT2>>(rhs.rhs_));

		smpAddAssign(*lhs, fwd(A * B));
	}
	/*! \endcond */
	//**********************************************************************************************

	//**SMP addition assignment to sparse matrices**************************************************
	// No special implementation for the SMP addition assignment to sparse matrices.
	//**********************************************************************************************

	//**SMP subtraction assignment to dense matrices************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief SMP subtraction assignment of a dense matrix-sparse matrix multiplication to a dense
	//        matrix (\f$ C-=A*B \f$).
	// \ingroup dense_matrix
	//
	// \param lhs The target left-hand side dense matrix.
	// \param rhs The right-hand side multiplication expression to be subtracted.
	// \return void
	//
	// This function implements the performance optimized SMP subtraction assignment of a dense
	// matrix-sparse matrix multiplication expression to a dense matrix. Due to the explicit
	// application of the SFINAE principle, this function can only be selected by the compiler
	// in case either of the two matrix operands requires an intermediate evaluation and no
	// symmetry can be exploited.
	*/
	template <typename MT // Type of the target dense matrix
			  ,
			  bool SO> // Storage order of the target sparse matrix
	friend inline auto smpSubAssign(DenseMatrix<MT, SO> &lhs, const DMatSMatMultExpr &rhs)
		-> EnableIf_t<IsEvaluationRequired_v<MT, MT1, MT2>>
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).rows() == rhs.rows(), "Invalid number of rows");
		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).columns() == rhs.columns(), "Invalid number of columns");

		LT A(rhs.lhs_); // Evaluation of the left-hand side dense matrix operand
		RT B(rhs.rhs_); // Evaluation of the right-hand side sparse matrix operand

		METRIC_NUMERIC_INTERNAL_ASSERT(A.rows() == rhs.lhs_.rows(), "Invalid number of rows");
		METRIC_NUMERIC_INTERNAL_ASSERT(A.columns() == rhs.lhs_.columns(), "Invalid number of columns");
		METRIC_NUMERIC_INTERNAL_ASSERT(B.rows() == rhs.rhs_.rows(), "Invalid number of rows");
		METRIC_NUMERIC_INTERNAL_ASSERT(B.columns() == rhs.rhs_.columns(), "Invalid number of columns");
		METRIC_NUMERIC_INTERNAL_ASSERT(A.rows() == (*lhs).rows(), "Invalid number of rows");
		METRIC_NUMERIC_INTERNAL_ASSERT(B.columns() == (*lhs).columns(), "Invalid number of columns");

		smpSubAssign(*lhs, A * B);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**Restructuring SMP subtraction assignment****************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Restructuring SMP subtraction assignment of a dense matrix-sparse matrix multiplication
	//        (\f$ C-=A*B \f$).
	// \ingroup dense_matrix
	//
	// \param lhs The target left-hand side matrix.
	// \param rhs The right-hand side multiplication expression to be subtracted.
	// \return void
	//
	// This function implements the symmetry-based restructuring SMP subtraction assignment of
	// a dense matrix-sparse matrix multiplication expression. Due to the explicit application
	// of the SFINAE principle this function can only be selected by the compiler in case the
	// symmetry of either of the two matrix operands can be exploited.
	*/
	template <typename MT // Type of the target matrix
			  ,
			  bool SO> // Storage order of the target matrix
	friend inline auto smpSubAssign(Matrix<MT, SO> &lhs, const DMatSMatMultExpr &rhs)
		-> EnableIf_t<CanExploitSymmetry_v<MT, MT1, MT2>>
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_SYMMETRIC_MATRIX_TYPE(MT);

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).rows() == rhs.rows(), "Invalid number of rows");
		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).columns() == rhs.columns(), "Invalid number of columns");

		const ForwardFunctor fwd;

		decltype(auto) A(transIf<IsSymmetric_v<MT1>>(rhs.lhs_));
		decltype(auto) B(transIf<IsSymmetric_v<MT2>>(rhs.rhs_));

		smpSubAssign(*lhs, fwd(A * B));
	}
	/*! \endcond */
	//**********************************************************************************************

	//**SMP subtraction assignment to sparse matrices***********************************************
	// No special implementation for the SMP subtraction assignment to sparse matrices.
	//**********************************************************************************************

	//**SMP Schur product assignment to dense matrices**********************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief SMP Schur product assignment of a dense matrix-sparse matrix multiplication to a
	//        dense matrix (\f$ C\circ=A*B \f$).
	// \ingroup dense_matrix
	//
	// \param lhs The target left-hand side dense matrix.
	// \param rhs The right-hand side multiplication expression for the Schur product.
	// \return void
	//
	// This function implements the performance optimized SMP Schur product assignment of a
	// dense matrix-sparse matrix multiplication expression to a dense matrix.
	*/
	template <typename MT // Type of the target dense matrix
			  ,
			  bool SO> // Storage order of the target sparse matrix
	friend inline void smpSchurAssign(DenseMatrix<MT, SO> &lhs, const DMatSMatMultExpr &rhs)
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_CONSTRAINT_MUST_BE_DENSE_MATRIX_TYPE(ResultType);
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_ROW_MAJOR_MATRIX_TYPE(ResultType);
		METRIC_NUMERIC_CONSTRAINT_MUST_NOT_REQUIRE_EVALUATION(ResultType);

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).rows() == rhs.rows(), "Invalid number of rows");
		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).columns() == rhs.columns(), "Invalid number of columns");

		const ResultType tmp(rhs);
		smpSchurAssign(*lhs, tmp);
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
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_DENSE_MATRIX_TYPE(MT1);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_ROW_MAJOR_MATRIX_TYPE(MT1);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_SPARSE_MATRIX_TYPE(MT2);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_ROW_MAJOR_MATRIX_TYPE(MT2);
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_ZERO_TYPE(MT2);
	METRIC_NUMERIC_CONSTRAINT_MUST_FORM_VALID_MATMATMULTEXPR(MT1, MT2);
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
/*!\brief Backend implementation of the multiplication between a row-major dense matrix and a
//        row-major sparse matrix (\f$ A=B*C \f$).
// \ingroup dense_matrix
//
// \param lhs The left-hand side dense matrix for the multiplication.
// \param rhs The right-hand side sparse matrix for the multiplication.
// \return The product of the two matrices.
//
// This function implements a performance optimized treatment of the multiplication between a
// row-major dense matrix and a row-major sparse matrix.
*/
template <
	typename MT1 // Type of the left-hand side dense matrix
	,
	typename MT2 // Type of the right-hand side sparse matrix
	,
	DisableIf_t<(IsIdentity_v<MT2> && IsSame_v<ElementType_t<MT1>, ElementType_t<MT2>>) || IsZero_v<MT2>> * = nullptr>
inline const DMatSMatMultExpr<MT1, MT2, false, false, false, false> dmatsmatmult(const DenseMatrix<MT1, false> &lhs,
																				 const SparseMatrix<MT2, false> &rhs)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).columns() == (*rhs).rows(), "Invalid matrix sizes");

	return DMatSMatMultExpr<MT1, MT2, false, false, false, false>(*lhs, *rhs);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Backend implementation of the multiplication between a row-major dense matrix and a
//        row-major identity matrix (\f$ A=B*C \f$).
// \ingroup dense_matrix
//
// \param lhs The left-hand side dense matrix for the multiplication.
// \param rhs The right-hand side identity matrix for the multiplication.
// \return Reference to the left-hand side dense matrix.
//
// This function implements a performance optimized treatment of the multiplication between
// a row-major dense matrix and a row-major identity matrix. It returns a reference to the
// left-hand side dense matrix.
*/
template <typename MT1 // Type of the left-hand side dense matrix
		  ,
		  typename MT2 // Type of the right-hand side sparse matrix
		  ,
		  EnableIf_t<IsIdentity_v<MT2> && IsSame_v<ElementType_t<MT1>, ElementType_t<MT2>>> * = nullptr>
inline const MT1 &dmatsmatmult(const DenseMatrix<MT1, false> &lhs, const SparseMatrix<MT2, false> &rhs)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	MAYBE_UNUSED(rhs);

	METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).columns() == (*rhs).rows(), "Invalid matrix sizes");

	return (*lhs);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Backend implementation of the multiplication between a row-major dense matrix and a
//        row-major zero matrix (\f$ A=B*C \f$).
// \ingroup dense_matrix
//
// \param lhs The left-hand side dense matrix for the multiplication.
// \param rhs The right-hand side zero matrix for the multiplication.
// \return The resulting zero matrix.
//
// This function implements a performance optimized treatment of the multiplication between a
// row-major dense matrix and a row-major zero matrix. It returns a zero matrix.
*/
template <typename MT1 // Type of the left-hand side dense matrix
		  ,
		  typename MT2 // Type of the right-hand side sparse matrix
		  ,
		  EnableIf_t<IsZero_v<MT2>> * = nullptr>
inline decltype(auto) dmatsmatmult(const DenseMatrix<MT1, false> &lhs, const SparseMatrix<MT2, false> &rhs)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).columns() == (*rhs).rows(), "Invalid matrix sizes");

	using ReturnType = const MultTrait_t<ResultType_t<MT1>, ResultType_t<MT2>>;

	METRIC_NUMERIC_CONSTRAINT_MUST_BE_ROW_MAJOR_MATRIX_TYPE(ReturnType);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_ZERO_TYPE(ReturnType);

	return ReturnType((*lhs).rows(), (*rhs).columns());
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Multiplication operator for the multiplication of a row-major dense matrix and a
//        row-major sparse matrix (\f$ A=B*C \f$).
// \ingroup dense_matrix
//
// \param lhs The left-hand side dense matrix for the multiplication.
// \param rhs The right-hand side sparse matrix for the multiplication.
// \return The resulting matrix.
// \exception std::invalid_argument Matrix sizes do not match.
//
// This operator represents the multiplication of a row-major dense matrix and a row-major
// sparse matrix:

   \code
   using mtrc::numeric::rowMajor;

   mtrc::numeric::DynamicMatrix<double,rowMajor> A, C;
   mtrc::numeric::CompressedMatrix<double,rowMajor> B;
   // ... Resizing and initialization
   C = A * B;
   \endcode

// The operator returns an expression representing a dense matrix of the higher-order element
// type of the two involved matrix element types \a MT1::ElementType and \a MT2::ElementType.
// Both matrix types \a MT1 and \a MT2 as well as the two element types \a MT1::ElementType
// and \a MT2::ElementType have to be supported by the MultTrait class template.\n
// In case the current sizes of the two given matrices don't match, a \a std::invalid_argument
// is thrown.
*/
template <typename MT1 // Type of the left-hand side dense matrix
		  ,
		  typename MT2> // Type of the right-hand side sparse matrix
inline decltype(auto) operator*(const DenseMatrix<MT1, false> &lhs, const SparseMatrix<MT2, false> &rhs)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	if ((*lhs).columns() != (*rhs).rows()) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Matrix sizes do not match");
	}

	return dmatsmatmult(*lhs, *rhs);
}
//*************************************************************************************************

//=================================================================================================
//
//  GLOBAL FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Declares the given non-symmetric matrix multiplication expression as symmetric.
// \ingroup dense_matrix
//
// \param dm The input matrix multiplication expression.
// \return The redeclared matrix multiplication expression.
// \exception std::invalid_argument Invalid symmetric matrix specification.
//
// The \a declsym function declares the given non-symmetric matrix multiplication expression
// \a dm as symmetric. The function returns an expression representing the operation. In case
// the given expression does not represent a square matrix, a \a std::invalid_argument exception
// is thrown.\n
// The following example demonstrates the use of the \a declsym function:

   \code
   using mtrc::numeric::rowMajor;

   mtrc::numeric::DynamicMatrix<double,rowMajor> A, C;
   mtrc::numeric::CompressedMatrix<double,rowMajor> B;
   // ... Resizing and initialization
   C = declsym( A * B );
   \endcode
*/
template <typename MT1 // Type of the left-hand side dense matrix
		  ,
		  typename MT2 // Type of the right-hand side sparse matrix
		  ,
		  bool SF // Symmetry flag
		  ,
		  bool HF // Hermitian flag
		  ,
		  bool LF // Lower flag
		  ,
		  bool UF> // Upper flag
inline decltype(auto) declsym(const DMatSMatMultExpr<MT1, MT2, SF, HF, LF, UF> &dm)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	if (!isSquare(dm)) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid symmetric matrix specification");
	}

	using ReturnType = const DMatSMatMultExpr<MT1, MT2, true, HF, LF, UF>;
	return ReturnType(dm.leftOperand(), dm.rightOperand());
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Declares the given non-Hermitian matrix multiplication expression as Hermitian.
// \ingroup dense_matrix
//
// \param dm The input matrix multiplication expression.
// \return The redeclared matrix multiplication expression.
// \exception std::invalid_argument Invalid Hermitian matrix specification.
//
// The \a declherm function declares the given non-Hermitian matrix multiplication expression
// \a dm as Hermitian. The function returns an expression representing the operation. In case
// the given expression does not represent a square matrix, a \a std::invalid_argument exception
// is thrown.\n
// The following example demonstrates the use of the \a declherm function:

   \code
   using mtrc::numeric::rowMajor;

   mtrc::numeric::DynamicMatrix<double,rowMajor> A, C;
   mtrc::numeric::CompressedMatrix<double,rowMajor> B;
   // ... Resizing and initialization
   C = declherm( A * B );
   \endcode
*/
template <typename MT1 // Type of the left-hand side dense matrix
		  ,
		  typename MT2 // Type of the right-hand side sparse matrix
		  ,
		  bool SF // Symmetry flag
		  ,
		  bool HF // Hermitian flag
		  ,
		  bool LF // Lower flag
		  ,
		  bool UF> // Upper flag
inline decltype(auto) declherm(const DMatSMatMultExpr<MT1, MT2, SF, HF, LF, UF> &dm)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	if (!isSquare(dm)) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid Hermitian matrix specification");
	}

	using ReturnType = const DMatSMatMultExpr<MT1, MT2, SF, true, LF, UF>;
	return ReturnType(dm.leftOperand(), dm.rightOperand());
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Declares the given non-lower matrix multiplication expression as lower.
// \ingroup dense_matrix
//
// \param dm The input matrix multiplication expression.
// \return The redeclared matrix multiplication expression.
// \exception std::invalid_argument Invalid lower matrix specification.
//
// The \a decllow function declares the given non-lower matrix multiplication expression
// \a dm as lower. The function returns an expression representing the operation. In case
// the given expression does not represent a square matrix, a \a std::invalid_argument
// exception is thrown.\n
// The following example demonstrates the use of the \a decllow function:

   \code
   using mtrc::numeric::rowMajor;

   mtrc::numeric::DynamicMatrix<double,rowMajor> A, C;
   mtrc::numeric::CompressedMatrix<double,rowMajor> B;
   // ... Resizing and initialization
   C = decllow( A * B );
   \endcode
*/
template <typename MT1 // Type of the left-hand side dense matrix
		  ,
		  typename MT2 // Type of the right-hand side sparse matrix
		  ,
		  bool SF // Symmetry flag
		  ,
		  bool HF // Hermitian flag
		  ,
		  bool LF // Lower flag
		  ,
		  bool UF> // Upper flag
inline decltype(auto) decllow(const DMatSMatMultExpr<MT1, MT2, SF, HF, LF, UF> &dm)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	if (!isSquare(dm)) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid lower matrix specification");
	}

	using ReturnType = const DMatSMatMultExpr<MT1, MT2, SF, HF, true, UF>;
	return ReturnType(dm.leftOperand(), dm.rightOperand());
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Declares the given non-unilower matrix multiplication expression as unilower.
// \ingroup dense_matrix
//
// \param dm The input matrix multiplication expression.
// \return The redeclared matrix multiplication expression.
// \exception std::invalid_argument Invalid unilower matrix specification.
//
// The \a declunilow function declares the given non-unilower matrix multiplication expression
// \a dm as unilower. The function returns an expression representing the operation. In case
// the given expression does not represent a square matrix, a \a std::invalid_argument
// exception is thrown.\n
// The following example demonstrates the use of the \a declunilow function:

   \code
   using mtrc::numeric::rowMajor;

   mtrc::numeric::DynamicMatrix<double,rowMajor> A, C;
   mtrc::numeric::CompressedMatrix<double,rowMajor> B;
   // ... Resizing and initialization
   C = declunilow( A * B );
   \endcode
*/
template <typename MT1 // Type of the left-hand side dense matrix
		  ,
		  typename MT2 // Type of the right-hand side sparse matrix
		  ,
		  bool SF // Symmetry flag
		  ,
		  bool HF // Hermitian flag
		  ,
		  bool UF> // Upper flag
inline decltype(auto) declunilow(const DMatSMatMultExpr<MT1, MT2, SF, HF, false, UF> &dm)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	if (!isSquare(dm)) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid unilower matrix specification");
	}

	return declunilow(decllow(*dm));
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Declares the given non-strictly-lower matrix multiplication expression as strictly lower.
// \ingroup dense_matrix
//
// \param dm The input matrix multiplication expression.
// \return The redeclared matrix multiplication expression.
// \exception std::invalid_argument Invalid strictly lower matrix specification.
//
// The \a declstrlow function declares the given non-strictly-lower matrix multiplication
// expression \a dm as strictly lower. The function returns an expression representing the
// operation. In case the given expression does not represent a square matrix, a
// \a std::invalid_argument exception is thrown.\n
// The following example demonstrates the use of the \a declstrlow function:

   \code
   using mtrc::numeric::rowMajor;

   mtrc::numeric::DynamicMatrix<double,rowMajor> A, C;
   mtrc::numeric::CompressedMatrix<double,rowMajor> B;
   // ... Resizing and initialization
   C = declstrlow( A * B );
   \endcode
*/
template <typename MT1 // Type of the left-hand side dense matrix
		  ,
		  typename MT2 // Type of the right-hand side sparse matrix
		  ,
		  bool SF // Symmetry flag
		  ,
		  bool HF // Hermitian flag
		  ,
		  bool UF> // Upper flag
inline decltype(auto) declstrlow(const DMatSMatMultExpr<MT1, MT2, SF, HF, false, UF> &dm)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	if (!isSquare(dm)) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid strictly lower matrix specification");
	}

	return declstrlow(decllow(*dm));
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Declares the given non-upper matrix multiplication expression as upper.
// \ingroup dense_matrix
//
// \param dm The input matrix multiplication expression.
// \return The redeclared matrix multiplication expression.
// \exception std::invalid_argument Invalid upper matrix specification.
//
// The \a declupp function declares the given non-upper matrix multiplication expression
// \a dm as upper. The function returns an expression representing the operation. In case
// the given expression does not represent a square matrix, a \a std::invalid_argument
// exception is thrown.\n
// The following example demonstrates the use of the \a declupp function:

   \code
   using mtrc::numeric::rowMajor;

   mtrc::numeric::DynamicMatrix<double,rowMajor> A, C;
   mtrc::numeric::CompressedMatrix<double,rowMajor> B;
   // ... Resizing and initialization
   C = declupp( A * B );
   \endcode
*/
template <typename MT1 // Type of the left-hand side dense matrix
		  ,
		  typename MT2 // Type of the right-hand side sparse matrix
		  ,
		  bool SF // Symmetry flag
		  ,
		  bool HF // Hermitian flag
		  ,
		  bool LF // Lower flag
		  ,
		  bool UF> // Upper flag
inline decltype(auto) declupp(const DMatSMatMultExpr<MT1, MT2, SF, HF, LF, UF> &dm)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	if (!isSquare(dm)) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid upper matrix specification");
	}

	using ReturnType = const DMatSMatMultExpr<MT1, MT2, SF, HF, LF, true>;
	return ReturnType(dm.leftOperand(), dm.rightOperand());
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Declares the given non-uniupper matrix multiplication expression as uniupper.
// \ingroup dense_matrix
//
// \param dm The input matrix multiplication expression.
// \return The redeclared matrix multiplication expression.
// \exception std::invalid_argument Invalid uniupper matrix specification.
//
// The \a decluniupp function declares the given non-uniupper matrix multiplication expression
// \a dm as uniupper. The function returns an expression representing the operation. In case
// the given expression does not represent a square matrix, a \a std::invalid_argument
// exception is thrown.\n
// The following example demonstrates the use of the \a decluniupp function:

   \code
   using mtrc::numeric::rowMajor;

   mtrc::numeric::DynamicMatrix<double,rowMajor> A, C;
   mtrc::numeric::CompressedMatrix<double,rowMajor> B;
   // ... Resizing and initialization
   C = decluniupp( A * B );
   \endcode
*/
template <typename MT1 // Type of the left-hand side dense matrix
		  ,
		  typename MT2 // Type of the right-hand side sparse matrix
		  ,
		  bool SF // Symmetry flag
		  ,
		  bool HF // Hermitian flag
		  ,
		  bool LF> // Lower flag
inline decltype(auto) decluniupp(const DMatSMatMultExpr<MT1, MT2, SF, HF, LF, false> &dm)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	if (!isSquare(dm)) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid uniupper matrix specification");
	}

	return decluniupp(declupp(*dm));
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Declares the given non-strictly-upper matrix multiplication expression as strictly upper.
// \ingroup dense_matrix
//
// \param dm The input matrix multiplication expression.
// \return The redeclared matrix multiplication expression.
// \exception std::invalid_argument Invalid strictly upper matrix specification.
//
// The \a declstrupp function declares the given non-strictly-upper matrix multiplication
// expression \a dm as strictly upper. The function returns an expression representing the
// operation. In case the given expression does not represent a square matrix, a
// \a std::invalid_argument exception is thrown.\n
// The following example demonstrates the use of the \a declstrupp function:

   \code
   using mtrc::numeric::rowMajor;

   mtrc::numeric::DynamicMatrix<double,rowMajor> A, C;
   mtrc::numeric::CompressedMatrix<double,rowMajor> B;
   // ... Resizing and initialization
   C = declstrupp( A * B );
   \endcode
*/
template <typename MT1 // Type of the left-hand side dense matrix
		  ,
		  typename MT2 // Type of the right-hand side sparse matrix
		  ,
		  bool SF // Symmetry flag
		  ,
		  bool HF // Hermitian flag
		  ,
		  bool LF> // Lower flag
inline decltype(auto) declstrupp(const DMatSMatMultExpr<MT1, MT2, SF, HF, LF, false> &dm)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	if (!isSquare(dm)) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid strictly upper matrix specification");
	}

	return declstrupp(declupp(*dm));
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Declares the given non-diagonal matrix multiplication expression as diagonal.
// \ingroup dense_matrix
//
// \param dm The input matrix multiplication expression.
// \return The redeclared matrix multiplication expression.
// \exception std::invalid_argument Invalid diagonal matrix specification.
//
// The \a decldiag function declares the given non-diagonal matrix multiplication expression
// \a dm as diagonal. The function returns an expression representing the operation. In case
// the given expression does not represent a square matrix, a \a std::invalid_argument exception
// is thrown.\n
// The following example demonstrates the use of the \a decldiag function:

   \code
   using mtrc::numeric::rowMajor;

   mtrc::numeric::DynamicMatrix<double,rowMajor> A, C;
   mtrc::numeric::CompressedMatrix<double,rowMajor> B;
   // ... Resizing and initialization
   C = decldiag( A * B );
   \endcode
*/
template <typename MT1 // Type of the left-hand side dense matrix
		  ,
		  typename MT2 // Type of the right-hand side sparse matrix
		  ,
		  bool SF // Symmetry flag
		  ,
		  bool HF // Hermitian flag
		  ,
		  bool LF // Lower flag
		  ,
		  bool UF> // Upper flag
inline decltype(auto) decldiag(const DMatSMatMultExpr<MT1, MT2, SF, HF, LF, UF> &dm)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	if (!isSquare(dm)) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid diagonal matrix specification");
	}

	using ReturnType = const DMatSMatMultExpr<MT1, MT2, SF, HF, true, true>;
	return ReturnType(dm.leftOperand(), dm.rightOperand());
}
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  SIZE SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename MT1, typename MT2, bool SF, bool HF, bool LF, bool UF>
struct Size<DMatSMatMultExpr<MT1, MT2, SF, HF, LF, UF>, 0UL> : public Size<MT1, 0UL> {};

template <typename MT1, typename MT2, bool SF, bool HF, bool LF, bool UF>
struct Size<DMatSMatMultExpr<MT1, MT2, SF, HF, LF, UF>, 1UL> : public Size<MT2, 1UL> {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  ISALIGNED SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename MT1, typename MT2, bool SF, bool HF, bool LF, bool UF>
struct IsAligned<DMatSMatMultExpr<MT1, MT2, SF, HF, LF, UF>> : public IsAligned<MT1> {};
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
