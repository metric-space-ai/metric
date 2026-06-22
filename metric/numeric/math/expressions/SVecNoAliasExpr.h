// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_SVECNOALIASEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_SVECNOALIASEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/Aliases.h>
#include <metric/numeric/math/Exception.h>
#include <metric/numeric/math/constraints/RequiresEvaluation.h>
#include <metric/numeric/math/constraints/SparseVector.h>
#include <metric/numeric/math/constraints/TransposeFlag.h>
#include <metric/numeric/math/expressions/Computation.h>
#include <metric/numeric/math/expressions/Forward.h>
#include <metric/numeric/math/expressions/Modification.h>
#include <metric/numeric/math/expressions/SparseVector.h>
#include <metric/numeric/math/expressions/VecNoAliasExpr.h>
#include <metric/numeric/math/typetraits/IsExpression.h>
#include <metric/numeric/math/typetraits/RequiresEvaluation.h>
#include <metric/numeric/system/MacroDisable.h>
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
//  CLASS SVECNOALIASEXPR
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Expression object for the non-aliased evaluation of sparse vectors.
// \ingroup sparse_vector_expression
//
// The SVecNoAliasExpr class represents the compile time expression for the non-aliased evaluation
// of a sparse vector.
*/
template <typename VT // Type of the sparse vector
		  ,
		  bool TF> // Transpose flag
class SVecNoAliasExpr : public VecNoAliasExpr<SparseVector<SVecNoAliasExpr<VT, TF>, TF>>, private Modification<VT> {
  private:
	//**Type definitions****************************************************************************
	//! Definition of the GetConstIterator type trait.
	METRIC_NUMERIC_CREATE_GET_TYPE_MEMBER_TYPE_TRAIT(GetConstIterator, ConstIterator, INVALID_TYPE);
	//**********************************************************************************************

  public:
	//**Type definitions****************************************************************************
	//! Type of this SVecNoAliasExpr instance.
	using This = SVecNoAliasExpr<VT, TF>;

	//! Base type of this SVecNoAliasExpr instance.
	using BaseType = VecNoAliasExpr<SparseVector<This, TF>>;

	using ResultType = ResultType_t<VT>;	   //!< Result type for expression template evaluations.
	using TransposeType = TransposeType_t<VT>; //!< Transpose type for expression template evaluations.
	using ElementType = ElementType_t<VT>;	   //!< Resulting element type.
	using ReturnType = ReturnType_t<VT>;	   //!< Return type for expression template evaluations.

	//! Data type for composite expression templates.
	using CompositeType = If_t<RequiresEvaluation_v<VT>, const ResultType, const SVecNoAliasExpr &>;

	//! Iterator over the elements of the dense vector.
	using ConstIterator = GetConstIterator_t<VT>;

	//! Composite data type of the sparse vector expression.
	using Operand = If_t<IsExpression_v<VT>, const VT, const VT &>;
	//**********************************************************************************************

	//**Compilation flags***************************************************************************
	//! Compilation switch for the expression template assignment strategy.
	static constexpr bool smpAssignable = VT::smpAssignable;
	//**********************************************************************************************

	//**Constructor*********************************************************************************
	/*!\brief Constructor for the SVecNoAliasExpr class.
	//
	// \param sv The sparse vector operand of the no-alias expression.
	*/
	explicit inline SVecNoAliasExpr(const VT &sv) noexcept : sv_(sv) // Sparse vector of the no-alias expression
	{
	}
	//**********************************************************************************************

	//**Subscript operator**************************************************************************
	/*!\brief Subscript operator for the direct access to the vector elements.
	//
	// \param index Access index. The index has to be in the range \f$[0..N-1]\f$.
	// \return The resulting value.
	*/
	inline ReturnType operator[](size_t index) const { return sv_[index]; }
	//**********************************************************************************************

	//**At function*********************************************************************************
	/*!\brief Checked access to the vector elements.
	//
	// \param index Access index. The index has to be in the range \f$[0..N-1]\f$.
	// \return The resulting value.
	// \exception std::out_of_range Invalid vector access index.
	*/
	inline ReturnType at(size_t index) const { return sv_.at(index); }
	//**********************************************************************************************

	//**Begin function******************************************************************************
	/*!\brief Returns an iterator to the first non-zero element of the sparse vector.
	//
	// \return Iterator to the first non-zero element of the sparse vector.
	*/
	inline ConstIterator begin() const { return ConstIterator(sv_.begin()); }
	//**********************************************************************************************

	//**End function********************************************************************************
	/*!\brief Returns an iterator just past the last non-zero element of the sparse vector.
	//
	// \return Iterator just past the last non-zero element of the sparse vector.
	*/
	inline ConstIterator end() const { return ConstIterator(sv_.end()); }
	//**********************************************************************************************

	//**Size function*******************************************************************************
	/*!\brief Returns the current size/dimension of the vector.
	//
	// \return The size of the vector.
	*/
	inline size_t size() const noexcept { return sv_.size(); }
	//**********************************************************************************************

	//**NonZeros function***************************************************************************
	/*!\brief Returns the number of non-zero elements in the sparse vector.
	//
	// \return The number of non-zero elements in the sparse vector.
	*/
	inline size_t nonZeros() const { return sv_.nonZeros(); }
	//**********************************************************************************************

	//**Find function*******************************************************************************
	/*!\brief Searches for a specific vector element.
	//
	// \param index The index of the search element.
	// \return Iterator to the element in case the index is found, end() iterator otherwise.
	*/
	inline ConstIterator find(size_t index) const
	{
		METRIC_NUMERIC_CONSTRAINT_MUST_NOT_REQUIRE_EVALUATION(VT);
		return ConstIterator(sv_.find(index));
	}
	//**********************************************************************************************

	//**LowerBound function*************************************************************************
	/*!\brief Returns an iterator to the first index not less then the given index.
	//
	// \param index The index of the search element.
	// \return Iterator to the first index not less then the given index, end() iterator otherwise.
	*/
	inline ConstIterator lowerBound(size_t index) const
	{
		METRIC_NUMERIC_CONSTRAINT_MUST_NOT_REQUIRE_EVALUATION(VT);
		return ConstIterator(sv_.lowerBound(index));
	}
	//**********************************************************************************************

	//**UpperBound function*************************************************************************
	/*!\brief Returns an iterator to the first index greater then the given index.
	//
	// \param index The index of the search element.
	// \return Iterator to the first index greater then the given index, end() iterator otherwise.
	*/
	inline ConstIterator upperBound(size_t index) const
	{
		METRIC_NUMERIC_CONSTRAINT_MUST_NOT_REQUIRE_EVALUATION(VT);
		return ConstIterator(sv_.upperBound(index));
	}
	//**********************************************************************************************

	//**Operand access******************************************************************************
	/*!\brief Returns the sparse vector operand.
	//
	// \return The sparse vector operand.
	*/
	inline Operand operand() const noexcept { return sv_; }
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
	inline bool canSMPAssign() const noexcept { return sv_.canSMPAssign(); }
	//**********************************************************************************************

  private:
	//**Member variables****************************************************************************
	Operand sv_; //!< Sparse vector of the no-alias expression.
	//**********************************************************************************************

	//**Assignment to dense vectors*****************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Assignment of a sparse vector no-alias expression to a dense vector.
	// \ingroup sparse_vector
	//
	// \param lhs The target left-hand side dense vector.
	// \param rhs The right-hand side no-alias expression to be assigned.
	// \return void
	//
	// This function implements the performance optimized assignment of a sparse vector no-alias
	// expression to a dense vector. Due to the explicit application of the SFINAE principle, this
	// function can only be selected by the compiler in case the operand requires an intermediate
	// evaluation.
	*/
	template <typename VT2> // Type of the target dense vector
	friend inline void assign(DenseVector<VT2, TF> &lhs, const SVecNoAliasExpr &rhs)
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == rhs.size(), "Invalid vector sizes");

		assign(*lhs, rhs.sv_);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**Assignment to sparse vectors****************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Assignment of a sparse vector no-alias expression to a sparse vector.
	// \ingroup sparse_vector
	//
	// \param lhs The target left-hand side sparse vector.
	// \param rhs The right-hand side no-alias expression to be assigned.
	// \return void
	//
	// This function implements the performance optimized assignment of a sparse vector no-alias
	// expression to a sparse vector. Due to the explicit application of the SFINAE principle, this
	// function can only be selected by the compiler in case the operand requires an intermediate
	// evaluation.
	*/
	template <typename VT2> // Type of the target sparse vector
	friend inline void assign(SparseVector<VT2, TF> &lhs, const SVecNoAliasExpr &rhs)
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == rhs.size(), "Invalid vector sizes");

		assign(*lhs, rhs.sv_);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**Addition assignment to dense vectors********************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Addition assignment of a sparse vector no-alias expression to a dense vector.
	// \ingroup sparse_vector
	//
	// \param lhs The target left-hand side dense vector.
	// \param rhs The right-hand side no-alias expression to be added.
	// \return void
	//
	// This function implements the performance optimized addition assignment of a sparse
	// vector no-alias expression to a dense vector. Due to the explicit application of the SFINAE
	// principle, this function can only be selected by the compiler in case the operand requires
	// an intermediate evaluation.
	*/
	template <typename VT2> // Type of the target dense vector
	friend inline void addAssign(DenseVector<VT2, TF> &lhs, const SVecNoAliasExpr &rhs)
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == rhs.size(), "Invalid vector sizes");

		addAssign(*lhs, rhs.sv_);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**Addition assignment to sparse vectors*******************************************************
	// No special implementation for the addition assignment to sparse vectors.
	//**********************************************************************************************

	//**Subtraction assignment to dense vectors*****************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Subtraction assignment of a sparse vector no-alias expression to a dense vector.
	// \ingroup sparse_vector
	//
	// \param lhs The target left-hand side dense vector.
	// \param rhs The right-hand side no-alias expression to be subtracted.
	// \return void
	//
	// This function implements the performance optimized subtraction assignment of a sparse
	// vector no-alias expression to a dense vector. Due to the explicit application of the SFINAE
	// principle, this function can only be selected by the compiler in case the operand requires
	// an intermediate evaluation.
	*/
	template <typename VT2> // Type of the target dense vector
	friend inline void subAssign(DenseVector<VT2, TF> &lhs, const SVecNoAliasExpr &rhs)
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == rhs.size(), "Invalid vector sizes");

		subAssign(*lhs, rhs.sv_);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**Subtraction assignment to sparse vectors****************************************************
	// No special implementation for the subtraction assignment to sparse vectors.
	//**********************************************************************************************

	//**Multiplication assignment to dense vectors**************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Multiplication assignment of a sparse vector no-alias expression to a dense vector.
	// \ingroup sparse_vector
	//
	// \param lhs The target left-hand side dense vector.
	// \param rhs The right-hand side no-alias expression to be multiplied.
	// \return void
	//
	// This function implements the performance optimized multiplication assignment of a sparse
	// vector no-alias expression to a dense vector. Due to the explicit application of the SFINAE
	// principle, this function can only be selected by the compiler in case the operand requires
	// an intermediate evaluation.
	*/
	template <typename VT2> // Type of the target dense vector
	friend inline void multAssign(DenseVector<VT2, TF> &lhs, const SVecNoAliasExpr &rhs)
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == rhs.size(), "Invalid vector sizes");

		multAssign(*lhs, rhs.sv_);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**Multiplication assignment to sparse vectors*************************************************
	// No special implementation for the multiplication assignment to sparse vectors.
	//**********************************************************************************************

	//**SMP assignment to dense vectors*************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief SMP assignment of a sparse vector no-alias expression to a dense vector.
	// \ingroup sparse_vector
	//
	// \param lhs The target left-hand side dense vector.
	// \param rhs The right-hand side no-alias expression to be assigned.
	// \return void
	//
	// This function implements the performance optimized SMP assignment of a sparse vector
	// no-alias expression to a dense vector. Due to the explicit application of the SFINAE
	// principle, this function can only be selected by the compiler in case the expression
	// specific parallel evaluation strategy is selected.
	*/
	template <typename VT2> // Type of the target dense vector
	friend inline void smpAssign(DenseVector<VT2, TF> &lhs, const SVecNoAliasExpr &rhs)
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == rhs.size(), "Invalid vector sizes");

		smpAssign(*lhs, rhs.sv_);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**SMP assignment to sparse vectors************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief SMP assignment of a sparse vector no-alias expression to a sparse vector.
	// \ingroup sparse_vector
	//
	// \param lhs The target left-hand side sparse vector.
	// \param rhs The right-hand side no-alias expression to be assigned.
	// \return void
	//
	// This function implements the performance optimized SMP assignment of a sparse vector
	// no-alias expression to a sparse vector. Due to the explicit application of the SFINAE
	// principle, this function can only be selected by the compiler in case the expression
	// specific parallel evaluation strategy is selected.
	*/
	template <typename VT2> // Type of the target sparse vector
	friend inline void smpAssign(SparseVector<VT2, TF> &lhs, const SVecNoAliasExpr &rhs)
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == rhs.size(), "Invalid vector sizes");

		smpAssign(*lhs, rhs.sv_);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**SMP addition assignment to dense vectors****************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief SMP addition assignment of a sparse vector no-alias expression to a dense vector.
	// \ingroup sparse_vector
	//
	// \param lhs The target left-hand side dense vector.
	// \param rhs The right-hand side no-alias expression to be added.
	// \return void
	//
	// This function implements the performance optimized SMP addition assignment of a sparse
	// vector no-alias expression to a dense vector. Due to the explicit application of the
	// SFINAE principle, this function can only be selected by the compiler in case the
	// expression specific parallel evaluation strategy is selected.
	*/
	template <typename VT2> // Type of the target dense vector
	friend inline void smpAddAssign(DenseVector<VT2, TF> &lhs, const SVecNoAliasExpr &rhs)
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == rhs.size(), "Invalid vector sizes");

		smpAddAssign(*lhs, rhs.sv_);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**SMP addition assignment to sparse vectors***************************************************
	// No special implementation for the SMP addition assignment to sparse vectors.
	//**********************************************************************************************

	//**SMP subtraction assignment to dense vectors*************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief SMP subtraction assignment of a sparse vector no-alias expression to a dense vector.
	// \ingroup sparse_vector
	//
	// \param lhs The target left-hand side dense vector.
	// \param rhs The right-hand side no-alias expression to be subtracted.
	// \return void
	//
	// This function implements the performance optimized SMP subtraction assignment of a
	// sparse vector no-alias expression to a dense vector. Due to the explicit application
	// of the SFINAE principle, this function can only be selected by the compiler in case
	// the expression specific parallel evaluation strategy is selected.
	*/
	template <typename VT2> // Type of the target dense vector
	friend inline void smpSubAssign(DenseVector<VT2, TF> &lhs, const SVecNoAliasExpr &rhs)
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == rhs.size(), "Invalid vector sizes");

		smpSubAssign(*lhs, rhs.sv_);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**SMP subtraction assignment to sparse vectors************************************************
	// No special implementation for the SMP subtraction assignment to sparse vectors.
	//**********************************************************************************************

	//**SMP multiplication assignment to dense vectors**********************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief SMP multiplication assignment of a sparse vector no-alias expression to a dense vector.
	// \ingroup sparse_vector
	//
	// \param lhs The target left-hand side dense vector.
	// \param rhs The right-hand side no-alias expression to be multiplied.
	// \return void
	//
	// This function implements the performance optimized SMP multiplication assignment of a
	// sparse vector no-alias expression to a dense vector. Due to the explicit application
	// of the SFINAE principle, this function can only be selected by the compiler in case
	// the expression specific parallel evaluation strategy is selected.
	*/
	template <typename VT2> // Type of the target dense vector
	friend inline void smpMultAssign(DenseVector<VT2, TF> &lhs, const SVecNoAliasExpr &rhs)
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == rhs.size(), "Invalid vector sizes");

		smpMultAssign(*lhs, rhs.sv_);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**SMP multiplication assignment to sparse vectors*********************************************
	// No special implementation for the SMP multiplication assignment to sparse vectors.
	//**********************************************************************************************

	//**Compile time checks*************************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_SPARSE_VECTOR_TYPE(VT);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_VECTOR_WITH_TRANSPOSE_FLAG(VT, TF);
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
/*!\brief Forces the non-aliased evaluation of the given sparse vector expression \a sv.
// \ingroup sparse_vector
//
// \param sv The input vector.
// \return The non-aliased sparse vector.
//
// The \a noalias function forces the non-aliased evaluation of the given sparse vector expression
// \a sv. The function returns an expression representing this operation.\n
// The following example demonstrates the use of the \a noalias function:

   \code
   mtrc::numeric::CompressedVector<double> a, b;
   // ... Resizing and initialization
   b = noalias( a );
   \endcode
*/
template <typename VT // Type of the dense vector
		  ,
		  bool TF> // Transpose flag
inline decltype(auto) noalias(const SparseVector<VT, TF> &sv)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	using ReturnType = const SVecNoAliasExpr<VT, TF>;
	return ReturnType(*sv);
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
