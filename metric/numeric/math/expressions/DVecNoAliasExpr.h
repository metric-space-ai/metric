// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_DVECNOALIASEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_DVECNOALIASEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/Aliases.h>
#include <metric/numeric/math/Exception.h>
#include <metric/numeric/math/constraints/DenseVector.h>
#include <metric/numeric/math/constraints/TransposeFlag.h>
#include <metric/numeric/math/expressions/DenseVector.h>
#include <metric/numeric/math/expressions/Forward.h>
#include <metric/numeric/math/expressions/Modification.h>
#include <metric/numeric/math/expressions/VecNoAliasExpr.h>
#include <metric/numeric/math/simd/SIMDTrait.h>
#include <metric/numeric/math/typetraits/HasConstDataAccess.h>
#include <metric/numeric/math/typetraits/IsAligned.h>
#include <metric/numeric/math/typetraits/IsExpression.h>
#include <metric/numeric/math/typetraits/IsPadded.h>
#include <metric/numeric/math/typetraits/RequiresEvaluation.h>
#include <metric/numeric/system/Inline.h>
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
//  CLASS DVECNOALIASEXPR
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Expression object for the non-aliased evaluation of dense vectors.
// \ingroup dense_vector_expression
//
// The DVecNoAliasExpr class represents the compile time expression for the non-alias evaluation
// of a dense vector.
*/
template <typename VT // Type of the dense vector
		  ,
		  bool TF> // Transpose flag
class DVecNoAliasExpr : public VecNoAliasExpr<DenseVector<DVecNoAliasExpr<VT, TF>, TF>>, private Modification<VT> {
  private:
	//**Type definitions****************************************************************************
	//! Definition of the GetConstIterator type trait.
	METRIC_NUMERIC_CREATE_GET_TYPE_MEMBER_TYPE_TRAIT(GetConstIterator, ConstIterator, INVALID_TYPE);
	//**********************************************************************************************

  public:
	//**Type definitions****************************************************************************
	//! Type of this DVecNoAliasExpr instance.
	using This = DVecNoAliasExpr<VT, TF>;

	//! Base type of this DVecNoAliasExpr instance.
	using BaseType = VecNoAliasExpr<DenseVector<This, TF>>;

	using ResultType = ResultType_t<VT>;	   //!< Result type for expression template evaluations.
	using TransposeType = TransposeType_t<VT>; //!< Transpose type for expression template evaluations.
	using ElementType = ElementType_t<VT>;	   //!< Resulting element type.
	using ReturnType = ReturnType_t<VT>;	   //!< Return type for expression template evaluations.

	//! Data type for composite expression templates.
	using CompositeType = If_t<RequiresEvaluation_v<VT>, const ResultType, const DVecNoAliasExpr &>;

	//! Iterator over the elements of the dense vector.
	using ConstIterator = GetConstIterator_t<VT>;

	//! Composite data type of the dense vector expression.
	using Operand = If_t<IsExpression_v<VT>, const VT, const VT &>;
	//**********************************************************************************************

	//**Compilation flags***************************************************************************
	//! Compilation switch for the expression template evaluation strategy.
	static constexpr bool simdEnabled = VT::simdEnabled;

	//! Compilation switch for the expression template assignment strategy.
	static constexpr bool smpAssignable = VT::smpAssignable;
	//**********************************************************************************************

	//**SIMD properties*****************************************************************************
	//! The number of elements packed within a single SIMD element.
	static constexpr size_t SIMDSIZE = SIMDTrait<ElementType>::size;
	//**********************************************************************************************

	//**Constructor*********************************************************************************
	/*!\brief Constructor for the DVecNoAliasExpr class.
	//
	// \param dv The dense vector operand of the no-alias expression.
	*/
	explicit inline DVecNoAliasExpr(const VT &dv) noexcept : dv_(dv) // Dense vector of the no-alias expression
	{
	}
	//**********************************************************************************************

	//**Subscript operator**************************************************************************
	/*!\brief Subscript operator for the direct access to the vector elements.
	//
	// \param index Access index. The index has to be in the range \f$[0..N-1]\f$.
	// \return The resulting value.
	*/
	inline ReturnType operator[](size_t index) const { return dv_[index]; }
	//**********************************************************************************************

	//**At function*********************************************************************************
	/*!\brief Checked access to the vector elements.
	//
	// \param index Access index. The index has to be in the range \f$[0..N-1]\f$.
	// \return The resulting value.
	// \exception std::out_of_range Invalid vector access index.
	*/
	inline ReturnType at(size_t index) const { return dv_.at(index); }
	//**********************************************************************************************

	//**Load function*******************************************************************************
	/*!\brief Access to the SIMD elements of the vector.
	//
	// \param index Access index. The index has to be in the range \f$[0..N-1]\f$.
	// \return Reference to the accessed values.
	*/
	METRIC_NUMERIC_ALWAYS_INLINE auto load(size_t index) const noexcept { return dv_.load(index); }
	//**********************************************************************************************

	//**Low-level data access***********************************************************************
	/*!\brief Low-level data access to the vector elements.
	//
	// \return Pointer to the internal element storage.
	*/
	inline const ElementType *data() const noexcept { return dv_.data(); }
	//**********************************************************************************************

	//**Begin function******************************************************************************
	/*!\brief Returns an iterator to the first non-zero element of the dense vector.
	//
	// \return Iterator to the first non-zero element of the dense vector.
	*/
	inline ConstIterator begin() const { return ConstIterator(dv_.begin()); }
	//**********************************************************************************************

	//**End function********************************************************************************
	/*!\brief Returns an iterator just past the last non-zero element of the dense vector.
	//
	// \return Iterator just past the last non-zero element of the dense vector.
	*/
	inline ConstIterator end() const { return ConstIterator(dv_.end()); }
	//**********************************************************************************************

	//**Size function*******************************************************************************
	/*!\brief Returns the current size/dimension of the vector.
	//
	// \return The size of the vector.
	*/
	inline size_t size() const noexcept { return dv_.size(); }
	//**********************************************************************************************

	//**Operand access******************************************************************************
	/*!\brief Returns the dense vector operand.
	//
	// \return The dense vector operand.
	*/
	inline Operand operand() const noexcept { return dv_; }
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
	/*!\brief Returns whether the operands of the expression are properly aligned in memory.
	//
	// \return \a true in case the operands are aligned, \a false if not.
	*/
	inline bool isAligned() const noexcept { return dv_.isAligned(); }
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Returns whether the expression can be used in SMP assignments.
	//
	// \return \a true in case the expression can be used in SMP assignments, \a false if not.
	*/
	inline bool canSMPAssign() const noexcept { return dv_.canSMPAssign(); }
	//**********************************************************************************************

  private:
	//**Member variables****************************************************************************
	Operand dv_; //!< Dense vector of the no-alias expression.
	//**********************************************************************************************

	//**Assignment to dense vectors*****************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Assignment of a dense vector no-alias expression to a dense vector.
	// \ingroup dense_vector
	//
	// \param lhs The target left-hand side dense vector.
	// \param rhs The right-hand side no-alias expression to be assigned.
	// \return void
	//
	// This function implements the performance optimized assignment of a dense vector no-alias
	// expression to a dense vector. Due to the explicit application of the SFINAE principle, this
	// function can only be selected by the compiler in case the operand requires an intermediate
	// evaluation.
	*/
	template <typename VT2> // Type of the target dense vector
	friend inline void assign(DenseVector<VT2, TF> &lhs, const DVecNoAliasExpr &rhs)
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == rhs.size(), "Invalid vector sizes");

		assign(*lhs, rhs.dv_);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**Assignment to sparse vectors****************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Assignment of a dense vector no-alias expression to a sparse vector.
	// \ingroup dense_vector
	//
	// \param lhs The target left-hand side sparse vector.
	// \param rhs The right-hand side no-alias expression to be assigned.
	// \return void
	//
	// This function implements the performance optimized assignment of a dense vector no-alias
	// expression to a sparse vector. Due to the explicit application of the SFINAE principle,
	// this function can only be selected by the compiler in case the operand requires an
	// intermediate evaluation.
	*/
	template <typename VT2> // Type of the target sparse vector
	friend inline void assign(SparseVector<VT2, TF> &lhs, const DVecNoAliasExpr &rhs)
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == rhs.size(), "Invalid vector sizes");

		assign(*lhs, rhs.dv_);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**Addition assignment to dense vectors********************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Addition assignment of a dense vector no-alias expression to a dense vector.
	// \ingroup dense_vector
	//
	// \param lhs The target left-hand side dense vector.
	// \param rhs The right-hand side no-alias expression to be added.
	// \return void
	//
	// This function implements the performance optimized addition assignment of a dense vector
	// no-alias expression to a dense vector. Due to the explicit application of the SFINAE
	// principle, this function can only be selected by the compiler in case the operand
	// requires an intermediate evaluation.
	*/
	template <typename VT2> // Type of the target dense vector
	friend inline void addAssign(DenseVector<VT2, TF> &lhs, const DVecNoAliasExpr &rhs)
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == rhs.size(), "Invalid vector sizes");

		addAssign(*lhs, rhs.dv_);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**Addition assignment to sparse vectors*******************************************************
	// No special implementation for the addition assignment to sparse vectors.
	//**********************************************************************************************

	//**Subtraction assignment to dense vectors*****************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Subtraction assignment of a dense vector no-alias expression to a dense vector.
	// \ingroup dense_vector
	//
	// \param lhs The target left-hand side dense vector.
	// \param rhs The right-hand side no-alias expression to be subtracted.
	// \return void
	//
	// This function implements the performance optimized subtraction assignment of a dense
	// vector no-alias expression to a dense vector. Due to the explicit application of the
	// SFINAE principle, this function can only be selected by the compiler in case the operand
	// requires an intermediate evaluation.
	*/
	template <typename VT2> // Type of the target dense vector
	friend inline void subAssign(DenseVector<VT2, TF> &lhs, const DVecNoAliasExpr &rhs)
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == rhs.size(), "Invalid vector sizes");

		subAssign(*lhs, rhs.dv_);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**Subtraction assignment to sparse vectors****************************************************
	// No special implementation for the subtraction assignment to sparse vectors.
	//**********************************************************************************************

	//**Multiplication assignment to dense vectors**************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Multiplication assignment of a dense vector no-alias expression to a dense vector.
	// \ingroup dense_vector
	//
	// \param lhs The target left-hand side dense vector.
	// \param rhs The right-hand side no-alias expression to be multiplied.
	// \return void
	//
	// This function implements the performance optimized multiplication assignment of a dense
	// vector no-alias expression to a dense vector. Due to the explicit application of the
	// SFINAE principle, this function can only be selected by the compiler in case the operand
	// requires an intermediate evaluation.
	*/
	template <typename VT2> // Type of the target dense vector
	friend inline void multAssign(DenseVector<VT2, TF> &lhs, const DVecNoAliasExpr &rhs)
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == rhs.size(), "Invalid vector sizes");

		multAssign(*lhs, rhs.dv_);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**Multiplication assignment to sparse vectors*************************************************
	// No special implementation for the multiplication assignment to sparse vectors.
	//**********************************************************************************************

	//**Division assignment to dense vectors********************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Division assignment of a dense vector no-alias expression to a dense vector.
	// \ingroup dense_vector
	//
	// \param lhs The target left-hand side dense vector.
	// \param rhs The right-hand side no-alias expression divisor.
	// \return void
	//
	// This function implements the performance optimized division assignment of a dense vector
	// no-alias expression to a dense vector. Due to the explicit application of the SFINAE
	// principle, this function can only be selected by the compiler in case the operand
	// requires an intermediate evaluation.
	*/
	template <typename VT2> // Type of the target dense vector
	friend inline void divAssign(DenseVector<VT2, TF> &lhs, const DVecNoAliasExpr &rhs)
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == rhs.size(), "Invalid vector sizes");

		divAssign(*lhs, rhs.dv_);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**Division assignment to sparse vectors*******************************************************
	// No special implementation for the division assignment to sparse vectors.
	//**********************************************************************************************

	//**SMP assignment to dense vectors*************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief SMP assignment of a dense vector no-alias expression to a dense vector.
	// \ingroup dense_vector
	//
	// \param lhs The target left-hand side dense vector.
	// \param rhs The right-hand side no-alias expression to be assigned.
	// \return void
	//
	// This function implements the performance optimized SMP assignment of a dense vector
	// no-alias expression to a dense vector. Due to the explicit application of the SFINAE
	// principle, this function can only be selected by the compiler in case the expression
	// specific parallel evaluation strategy is selected.
	*/
	template <typename VT2> // Type of the target dense vector
	friend inline void smpAssign(DenseVector<VT2, TF> &lhs, const DVecNoAliasExpr &rhs)
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == rhs.size(), "Invalid vector sizes");

		smpAssign(*lhs, rhs.dv_);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**SMP assignment to sparse vectors************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief SMP assignment of a dense vector no-alias expression to a sparse vector.
	// \ingroup dense_vector
	//
	// \param lhs The target left-hand side sparse vector.
	// \param rhs The right-hand side no-alias expression to be assigned.
	// \return void
	//
	// This function implements the performance optimized SMP assignment of a dense vector
	// no-alias expression to a sparse vector. Due to the explicit application of the SFINAE
	// principle, this function can only be selected by the compiler in case the expression
	// specific parallel evaluation strategy is selected.
	*/
	template <typename VT2> // Type of the target sparse vector
	friend inline void smpAssign(SparseVector<VT2, TF> &lhs, const DVecNoAliasExpr &rhs)
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == rhs.size(), "Invalid vector sizes");

		smpAssign(*lhs, rhs.dv_);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**SMP addition assignment to dense vectors****************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief SMP addition assignment of a dense vector no-alias expression to a dense vector.
	// \ingroup dense_vector
	//
	// \param lhs The target left-hand side dense vector.
	// \param rhs The right-hand side no-alias expression to be added.
	// \return void
	//
	// This function implements the performance optimized SMP addition assignment of a dense
	// vector no-alias expression to a dense vector. Due to the explicit application of the
	// SFINAE principle, this function can only be selected by the compiler in case the
	// expression specific parallel evaluation strategy is selected.
	*/
	template <typename VT2> // Type of the target dense vector
	friend inline void smpAddAssign(DenseVector<VT2, TF> &lhs, const DVecNoAliasExpr &rhs)
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == rhs.size(), "Invalid vector sizes");

		smpAddAssign(*lhs, rhs.dv_);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**SMP addition assignment to sparse vectors***************************************************
	// No special implementation for the SMP addition assignment to sparse vectors.
	//**********************************************************************************************

	//**SMP subtraction assignment to dense vectors*************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief SMP subtraction assignment of a dense vector no-alias expression to a dense vector.
	// \ingroup dense_vector
	//
	// \param lhs The target left-hand side dense vector.
	// \param rhs The right-hand side no-alias expression to be subtracted.
	// \return void
	//
	// This function implements the performance optimized SMP subtraction assignment of a
	// dense vector no-alias expression to a dense vector. Due to the explicit application
	// of the SFINAE principle, this function can only be selected by the compiler in case
	// the expression specific parallel evaluation strategy is selected.
	*/
	template <typename VT2> // Type of the target dense vector
	friend inline void smpSubAssign(DenseVector<VT2, TF> &lhs, const DVecNoAliasExpr &rhs)
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == rhs.size(), "Invalid vector sizes");

		smpSubAssign(*lhs, rhs.dv_);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**SMP subtraction assignment to sparse vectors************************************************
	// No special implementation for the SMP subtraction assignment to sparse vectors.
	//**********************************************************************************************

	//**SMP multiplication assignment to dense vectors**********************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief SMP multiplication assignment of a dense vector no-alias expression to a dense vector.
	// \ingroup dense_vector
	//
	// \param lhs The target left-hand side dense vector.
	// \param rhs The right-hand side no-alias expression to be multiplied.
	// \return void
	//
	// This function implements the performance optimized SMP multiplication assignment of a
	// dense vector no-alias expression to a dense vector. Due to the explicit application of
	// the SFINAE principle, this function can only be selected by the compiler in case the
	// expression specific parallel evaluation strategy is selected.
	*/
	template <typename VT2> // Type of the target dense vector
	friend inline void smpMultAssign(DenseVector<VT2, TF> &lhs, const DVecNoAliasExpr &rhs)
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == rhs.size(), "Invalid vector sizes");

		smpMultAssign(*lhs, rhs.dv_);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**SMP multiplication assignment to sparse vectors*********************************************
	// No special implementation for the SMP multiplication assignment to sparse vectors.
	//**********************************************************************************************

	//**SMP division assignment to dense vectors****************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief SMP division assignment of a dense vector no-alias expression to a dense vector.
	// \ingroup dense_vector
	//
	// \param lhs The target left-hand side dense vector.
	// \param rhs The right-hand side no-alias expression divisor.
	// \return void
	//
	// This function implements the performance optimized SMP division assignment of a dense
	// vector no-alias expression to a dense vector. Due to the explicit application of the
	// SFINAE principle, this function can only be selected by the compiler in case the
	// expression specific parallel evaluation strategy is selected.
	*/
	template <typename VT2> // Type of the target dense vector
	friend inline void smpDivAssign(DenseVector<VT2, TF> &lhs, const DVecNoAliasExpr &rhs)
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == rhs.size(), "Invalid vector sizes");

		smpDivAssign(*lhs, rhs.dv_);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**SMP division assignment to sparse vectors***************************************************
	// No special implementation for the SMP division assignment to sparse vectors.
	//**********************************************************************************************

	//**Compile time checks*************************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_DENSE_VECTOR_TYPE(VT);
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
/*!\brief Forces the non-aliased evaluation of the given dense vector expression \a dv.
// \ingroup dense_vector
//
// \param dv The input vector.
// \return The non-aliased dense vector.
//
// The \a noalias function forces the non-aliased evaluation of the given dense vector expression
// \a dv. The function returns an expression representing this operation.\n
// The following example demonstrates the use of the \a noalias function:

   \code
   mtrc::numeric::DynamicVector<double> a, b;
   // ... Resizing and initialization
   b = noalias( a );
   \endcode
*/
template <typename VT // Type of the dense vector
		  ,
		  bool TF> // Transpose flag
inline decltype(auto) noalias(const DenseVector<VT, TF> &dv)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	using ReturnType = const DVecNoAliasExpr<VT, TF>;
	return ReturnType(*dv);
}
//*************************************************************************************************

//=================================================================================================
//
//  ISALIGNED SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename VT, bool TF> struct HasConstDataAccess<DVecNoAliasExpr<VT, TF>> : public HasConstDataAccess<VT> {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  ISALIGNED SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename VT, bool TF> struct IsAligned<DVecNoAliasExpr<VT, TF>> : public IsAligned<VT> {};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  ISPADDED SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
template <typename VT, bool TF> struct IsPadded<DVecNoAliasExpr<VT, TF>> : public IsPadded<VT> {};
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
