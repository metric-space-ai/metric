// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_SVECTRANSEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_SVECTRANSEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/Aliases.h>
#include <metric/numeric/math/Exception.h>
#include <metric/numeric/math/constraints/RequiresEvaluation.h>
#include <metric/numeric/math/constraints/SparseVector.h>
#include <metric/numeric/math/expressions/Computation.h>
#include <metric/numeric/math/expressions/DVecTransposer.h>
#include <metric/numeric/math/expressions/Forward.h>
#include <metric/numeric/math/expressions/SVecTransposer.h>
#include <metric/numeric/math/expressions/SparseVector.h>
#include <metric/numeric/math/expressions/Transformation.h>
#include <metric/numeric/math/expressions/VecTransExpr.h>
#include <metric/numeric/math/typetraits/IsExpression.h>
#include <metric/numeric/math/typetraits/RequiresEvaluation.h>
#include <metric/numeric/system/MacroDisable.h>
#include <metric/numeric/util/Assert.h>
#include <metric/numeric/util/EnableIf.h>
#include <metric/numeric/util/FunctionTrace.h>
#include <metric/numeric/util/IntegralConstant.h>
#include <metric/numeric/util/InvalidType.h>
#include <metric/numeric/util/Types.h>
#include <metric/numeric/util/mpl/If.h>
#include <metric/numeric/util/typetraits/GetMemberType.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS SVECTRANSEXPR
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Expression object for sparse vector transpositions.
// \ingroup sparse_vector_expression
//
// The SVecTransExpr class represents the compile time expression for transpositions of
// sparse vectors.
*/
template <typename VT // Type of the sparse vector
		  ,
		  bool TF> // Transpose flag
class SVecTransExpr : public VecTransExpr<SparseVector<SVecTransExpr<VT, TF>, TF>>,
					  private If_t<IsComputation_v<VT>, Computation, Transformation> {
  private:
	//**Type definitions****************************************************************************
	//! Definition of the GetConstIterator type trait.
	METRIC_NUMERIC_CREATE_GET_TYPE_MEMBER_TYPE_TRAIT(GetConstIterator, ConstIterator, INVALID_TYPE);
	//**********************************************************************************************

	//**Serial evaluation strategy******************************************************************
	//! Compilation switch for the serial evaluation strategy of the transposition expression.
	/*! The \a useAssign compile time constant expression represents a compilation switch for
		the serial evaluation strategy of the transposition expression. In case the given sparse
		vector expression of type \a VT requires an intermediate evaluation, \a useAssign will be
		set to 1 and the transposition expression will be evaluated via the \a assign function
		family. Otherwise \a useAssign will be set to 0 and the expression will be evaluated via
		the subscript operator. */
	static constexpr bool useAssign = RequiresEvaluation_v<VT>;

	/*! \cond METRIC_NUMERIC_INTERNAL */
	//! Helper variable template for the explicit application of the SFINAE principle.
	template <typename VT2> static constexpr bool UseAssign_v = useAssign;
	/*! \endcond */
	//**********************************************************************************************

	//**Parallel evaluation strategy****************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	//! Helper variable template for the explicit application of the SFINAE principle.
	/*! This variable template is a helper for the selection of the parallel evaluation strategy.
		In case the target vector is SMP assignable and the sparse vector operand requires an
		intermediate evaluation, the variable is set to 1 and the expression specific evaluation
		strategy is selected. Otherwise the variable is set to 0 and the default strategy is
		chosen. */
	template <typename VT2> static constexpr bool UseSMPAssign_v = (VT2::smpAssignable && useAssign);
	/*! \endcond */
	//**********************************************************************************************

  public:
	//**Type definitions****************************************************************************
	//! Type of this SVecTransExpr instance.
	using This = SVecTransExpr<VT, TF>;

	//! Base type of this SVecTransExpr instance.
	using BaseType = VecTransExpr<SparseVector<This, TF>>;

	using ResultType = TransposeType_t<VT>; //!< Result type for expression template evaluations.
	using TransposeType = ResultType_t<VT>; //!< Transpose type for expression template evaluations.
	using ElementType = ElementType_t<VT>;	//!< Resulting element type.
	using ReturnType = ReturnType_t<VT>;	//!< Return type for expression template evaluations.

	//! Data type for composite expression templates.
	using CompositeType = If_t<useAssign, const ResultType, const SVecTransExpr &>;

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
	/*!\brief Constructor for the SVecTransExpr class.
	//
	// \param sv The sparse vector operand of the transposition expression.
	*/
	explicit inline SVecTransExpr(const VT &sv) noexcept : sv_(sv) // Sparse vector of the transposition expression
	{
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
		METRIC_NUMERIC_INTERNAL_ASSERT(index < sv_.size(), "Invalid vector access index");
		return sv_[index];
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
		if (index >= sv_.size()) {
			METRIC_NUMERIC_THROW_OUT_OF_RANGE("Invalid vector access index");
		}
		return (*this)[index];
	}
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
	template <typename T> inline bool canAlias(const T *alias) const noexcept { return sv_.canAlias(alias); }
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Returns whether the expression is aliased with the given address \a alias.
	//
	// \param alias The alias to be checked.
	// \return \a true in case an alias effect is detected, \a false otherwise.
	*/
	template <typename T> inline bool isAliased(const T *alias) const noexcept { return sv_.isAliased(alias); }
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
	Operand sv_; //!< Sparse vector of the transposition expression.
	//**********************************************************************************************

	//**Assignment to dense vectors*****************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Assignment of a sparse vector transposition expression to a dense vector.
	// \ingroup sparse_vector
	//
	// \param lhs The target left-hand side dense vector.
	// \param rhs The right-hand side transposition expression to be assigned.
	// \return void
	//
	// This function implements the performance optimized assignment of a sparse vector
	// transposition expression to a dense vector. Due to the explicit application of
	// the SFINAE principle, this function can only be selected by the compiler in case
	// the operand requires an intermediate evaluation.
	*/
	template <typename VT2> // Type of the target dense vector
	friend inline auto assign(DenseVector<VT2, TF> &lhs, const SVecTransExpr &rhs) -> EnableIf_t<UseAssign_v<VT2>>
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == rhs.size(), "Invalid vector sizes");

		DVecTransposer<VT2, !TF> tmp(*lhs);
		assign(tmp, rhs.sv_);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**Assignment to sparse vectors****************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Assignment of a sparse vector transposition expression to a sparse vector.
	// \ingroup sparse_vector
	//
	// \param lhs The target left-hand side sparse vector.
	// \param rhs The right-hand side transposition expression to be assigned.
	// \return void
	//
	// This function implements the performance optimized assignment of a sparse vector
	// transposition expression to a sparse vector. Due to the explicit application of
	// the SFINAE principle, this function can only be selected by the compiler in case
	// the operand requires an intermediate evaluation.
	*/
	template <typename VT2> // Type of the target sparse vector
	friend inline auto assign(SparseVector<VT2, TF> &lhs, const SVecTransExpr &rhs) -> EnableIf_t<UseAssign_v<VT2>>
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == rhs.size(), "Invalid vector sizes");

		SVecTransposer<VT2, !TF> tmp(*lhs);
		assign(tmp, rhs.sv_);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**Addition assignment to dense vectors********************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Addition assignment of a sparse vector transposition expression to a dense vector.
	// \ingroup sparse_vector
	//
	// \param lhs The target left-hand side dense vector.
	// \param rhs The right-hand side transposition expression to be added.
	// \return void
	//
	// This function implements the performance optimized addition assignment of a sparse
	// vector transposition expression to a dense vector. Due to the explicit application
	// of the SFINAE principle, this function can only be selected by the compiler in case
	// the operand requires an intermediate evaluation.
	*/
	template <typename VT2> // Type of the target dense vector
	friend inline auto addAssign(DenseVector<VT2, TF> &lhs, const SVecTransExpr &rhs) -> EnableIf_t<UseAssign_v<VT2>>
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == rhs.size(), "Invalid vector sizes");

		DVecTransposer<VT2, !TF> tmp(*lhs);
		addAssign(tmp, rhs.sv_);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**Addition assignment to sparse vectors*******************************************************
	// No special implementation for the addition assignment to sparse vectors.
	//**********************************************************************************************

	//**Subtraction assignment to dense vectors*****************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Subtraction assignment of a sparse vector transposition expression to a dense vector.
	// \ingroup sparse_vector
	//
	// \param lhs The target left-hand side dense vector.
	// \param rhs The right-hand side transposition expression to be subtracted.
	// \return void
	//
	// This function implements the performance optimized subtraction assignment of a sparse
	// vector transposition expression to a dense vector. Due to the explicit application of
	// the SFINAE principle, this function can only be selected by the compiler in case the
	// operand requires an intermediate evaluation.
	*/
	template <typename VT2> // Type of the target dense vector
	friend inline auto subAssign(DenseVector<VT2, TF> &lhs, const SVecTransExpr &rhs) -> EnableIf_t<UseAssign_v<VT2>>
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == rhs.size(), "Invalid vector sizes");

		DVecTransposer<VT2, !TF> tmp(*lhs);
		subAssign(tmp, rhs.sv_);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**Subtraction assignment to sparse vectors****************************************************
	// No special implementation for the subtraction assignment to sparse vectors.
	//**********************************************************************************************

	//**Multiplication assignment to dense vectors**************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Multiplication assignment of a sparse vector transposition expression to a dense vector.
	// \ingroup sparse_vector
	//
	// \param lhs The target left-hand side dense vector.
	// \param rhs The right-hand side transposition expression to be multiplied.
	// \return void
	//
	// This function implements the performance optimized multiplication assignment of a sparse
	// vector transposition expression to a dense vector. Due to the explicit application of the
	// SFINAE principle, this function can only be selected by the compiler in case the operand
	// requires an intermediate evaluation.
	*/
	template <typename VT2> // Type of the target dense vector
	friend inline auto multAssign(DenseVector<VT2, TF> &lhs, const SVecTransExpr &rhs) -> EnableIf_t<UseAssign_v<VT2>>
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == rhs.size(), "Invalid vector sizes");

		DVecTransposer<VT2, !TF> tmp(*lhs);
		multAssign(tmp, rhs.sv_);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**Multiplication assignment to sparse vectors*************************************************
	// No special implementation for the multiplication assignment to sparse vectors.
	//**********************************************************************************************

	//**SMP assignment to dense vectors*************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief SMP assignment of a sparse vector transposition expression to a dense vector.
	// \ingroup sparse_vector
	//
	// \param lhs The target left-hand side dense vector.
	// \param rhs The right-hand side transposition expression to be assigned.
	// \return void
	//
	// This function implements the performance optimized SMP assignment of a sparse vector
	// transposition expression to a dense vector. Due to the explicit application of the
	// SFINAE principle, this function can only be selected by the compiler in case the
	// expression specific parallel evaluation strategy is selected.
	*/
	template <typename VT2> // Type of the target dense vector
	friend inline auto smpAssign(DenseVector<VT2, TF> &lhs, const SVecTransExpr &rhs) -> EnableIf_t<UseSMPAssign_v<VT2>>
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == rhs.size(), "Invalid vector sizes");

		DVecTransposer<VT2, !TF> tmp(*lhs);
		smpAssign(tmp, rhs.sv_);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**SMP assignment to sparse vectors************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief SMP assignment of a sparse vector transposition expression to a sparse vector.
	// \ingroup sparse_vector
	//
	// \param lhs The target left-hand side sparse vector.
	// \param rhs The right-hand side transposition expression to be assigned.
	// \return void
	//
	// This function implements the performance optimized SMP assignment of a sparse vector
	// transposition expression to a sparse vector. Due to the explicit application of the
	// SFINAE principle, this function can only be selected by the compiler in case the
	// expression specific parallel evaluation strategy is selected.
	*/
	template <typename VT2> // Type of the target sparse vector
	friend inline auto smpAssign(SparseVector<VT2, TF> &lhs, const SVecTransExpr &rhs)
		-> EnableIf_t<UseSMPAssign_v<VT2>>
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == rhs.size(), "Invalid vector sizes");

		SVecTransposer<VT2, !TF> tmp(*lhs);
		smpAssign(tmp, rhs.sv_);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**SMP addition assignment to dense vectors****************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief SMP addition assignment of a sparse vector transposition expression to a dense vector.
	// \ingroup sparse_vector
	//
	// \param lhs The target left-hand side dense vector.
	// \param rhs The right-hand side transposition expression to be added.
	// \return void
	//
	// This function implements the performance optimized SMP addition assignment of a sparse
	// vector transposition expression to a dense vector. Due to the explicit application of
	// the SFINAE principle, this function can only be selected by the compiler in case the
	// expression specific parallel evaluation strategy is selected.
	*/
	template <typename VT2> // Type of the target dense vector
	friend inline auto smpAddAssign(DenseVector<VT2, TF> &lhs, const SVecTransExpr &rhs)
		-> EnableIf_t<UseSMPAssign_v<VT2>>
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == rhs.size(), "Invalid vector sizes");

		DVecTransposer<VT2, !TF> tmp(*lhs);
		smpAddAssign(tmp, rhs.sv_);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**SMP addition assignment to sparse vectors***************************************************
	// No special implementation for the SMP addition assignment to sparse vectors.
	//**********************************************************************************************

	//**SMP subtraction assignment to dense vectors*************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief SMP subtraction assignment of a sparse vector transposition expression to a dense vector.
	// \ingroup sparse_vector
	//
	// \param lhs The target left-hand side dense vector.
	// \param rhs The right-hand side transposition expression to be subtracted.
	// \return void
	//
	// This function implements the performance optimized SMP subtraction assignment of a sparse
	// vector transposition expression to a dense vector. Due to the explicit application of the
	// SFINAE principle, this function can only be selected by the compiler in case the expression
	// specific parallel evaluation strategy is selected.
	*/
	template <typename VT2> // Type of the target dense vector
	friend inline auto smpSubAssign(DenseVector<VT2, TF> &lhs, const SVecTransExpr &rhs)
		-> EnableIf_t<UseSMPAssign_v<VT2>>
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == rhs.size(), "Invalid vector sizes");

		DVecTransposer<VT2, !TF> tmp(*lhs);
		smpSubAssign(tmp, rhs.sv_);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**SMP subtraction assignment to sparse vectors************************************************
	// No special implementation for the SMP subtraction assignment to sparse vectors.
	//**********************************************************************************************

	//**SMP multiplication assignment to dense vectors**********************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief SMP multiplication assignment of a sparse vector transposition expression to a dense
	//        vector.
	// \ingroup sparse_vector
	//
	// \param lhs The target left-hand side dense vector.
	// \param rhs The right-hand side transposition expression to be multiplied.
	// \return void
	//
	// This function implements the performance optimized SMP multiplication assignment of a sparse
	// vector transposition expression to a dense vector. Due to the explicit application of the
	// SFINAE principle, this function can only be selected by the compiler in case the expression
	// specific parallel evaluation strategy is selected.
	*/
	template <typename VT2> // Type of the target dense vector
	friend inline auto smpMultAssign(DenseVector<VT2, TF> &lhs, const SVecTransExpr &rhs)
		-> EnableIf_t<UseSMPAssign_v<VT2>>
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == rhs.size(), "Invalid vector sizes");

		DVecTransposer<VT2, !TF> tmp(*lhs);
		smpMultAssign(tmp, rhs.sv_);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**SMP multiplication assignment to sparse vectors*********************************************
	// No special implementation for the SMP multiplication assignment to sparse vectors.
	//**********************************************************************************************

	//**Compile time checks*************************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_SPARSE_VECTOR_TYPE(VT);
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
/*!\brief Calculation of the transpose of the given sparse vector.
// \ingroup sparse_vector
//
// \param sv The sparse vector to be transposed.
// \return The transpose of the sparse vector.
//
// This function returns an expression representing the transpose of the given sparse vector:

   \code
   using mtrc::numeric::columnVector;
   using mtrc::numeric::rowVector;

   mtrc::numeric::CompressedVector<double,columnVector> a;
   mtrc::numeric::CompressedVector<double,rowVector> b;
   // ... Resizing and initialization
   b = trans( a );
   \endcode
*/
template <typename VT // Type of the sparse vector
		  ,
		  bool TF> // Transpose flag
inline decltype(auto) trans(const SparseVector<VT, TF> &sv)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	using ReturnType = const SVecTransExpr<VT, !TF>;
	return ReturnType(*sv);
}
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Backend of the transTo() function for sparse vectors with different transpose flag.
// \ingroup sparse_vector
//
// \param sv The sparse vector to be transposed.
// \return The transpose of the sparse vector.
*/
template <typename VT // Type of the sparse vector
		  ,
		  bool TF> // Transpose flag
inline decltype(auto) transTo_backend(const SparseVector<VT, TF> &sv, FalseType)
{
	return trans(*sv);
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Backend of the transTo() function for sparse vectors with matching transpose flag.
// \ingroup sparse_vector
//
// \param sv The sparse vector to be transposed.
// \return The original sparse vector.
*/
template <typename VT // Type of the sparse vector
		  ,
		  bool TF> // Transpose flag
inline const VT &transTo_backend(const SparseVector<VT, TF> &sv, TrueType)
{
	return *sv;
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Conditional calculation of the transpose of the given sparse vector.
// \ingroup sparse_vector
//
// \param sv The sparse vector to be transposed.
// \return The sparse vector with the specified transpose flag.
//
// This function transposes the given sparse vector in case the target transpose flag is different
// from the current transpose flag of the vector and performs no action if the two transpose flags
// match. It returns an expression representing the the given sparse vector with the specified
// transpose flag.
*/
template <bool TTF // Target transpose flag
		  ,
		  typename VT // Type of the sparse vector
		  ,
		  bool TF> // Current transpose flag of the sparse vector
inline decltype(auto) transTo(const SparseVector<VT, TF> &sv)
{
	return transTo_backend(*sv, BoolConstant<TTF == TF>());
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
