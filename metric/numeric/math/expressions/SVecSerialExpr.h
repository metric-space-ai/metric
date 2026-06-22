// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_SVECSERIALEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_SVECSERIALEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/Aliases.h>
#include <metric/numeric/math/Exception.h>
#include <metric/numeric/math/constraints/SparseVector.h>
#include <metric/numeric/math/constraints/TransposeFlag.h>
#include <metric/numeric/math/expressions/Computation.h>
#include <metric/numeric/math/expressions/Forward.h>
#include <metric/numeric/math/expressions/SparseVector.h>
#include <metric/numeric/math/expressions/VecSerialExpr.h>
#include <metric/numeric/math/typetraits/IsExpression.h>
#include <metric/numeric/system/MacroDisable.h>
#include <metric/numeric/util/Assert.h>
#include <metric/numeric/util/FunctionTrace.h>
#include <metric/numeric/util/Types.h>
#include <metric/numeric/util/mpl/If.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS SVECSERIALEXPR
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Expression object for the forced serial evaluation of sparse vectors.
// \ingroup sparse_vector_expression
//
// The SVecSerialExpr class represents the compile time expression for the forced serial
// evaluation of a sparse vector.
*/
template <typename VT // Type of the sparse vector
		  ,
		  bool TF> // Transpose flag
class SVecSerialExpr : public VecSerialExpr<SparseVector<SVecSerialExpr<VT, TF>, TF>>, private Computation {
  public:
	//**Type definitions****************************************************************************
	//! Type of this SVecSerialExpr instance.
	using This = SVecSerialExpr<VT, TF>;

	//! Base type of this SVecSerialExpr instance.
	using BaseType = VecSerialExpr<SparseVector<This, TF>>;

	using ResultType = ResultType_t<VT>;	   //!< Result type for expression template evaluations.
	using TransposeType = TransposeType_t<VT>; //!< Transpose type for expression template evaluations.
	using ElementType = ElementType_t<VT>;	   //!< Resulting element type.
	using ReturnType = ReturnType_t<VT>;	   //!< Return type for expression template evaluations.

	//! Data type for composite expression templates.
	using CompositeType = const ResultType;

	//! Composite data type of the sparse vector expression.
	using Operand = If_t<IsExpression_v<VT>, const VT, const VT &>;
	//**********************************************************************************************

	//**Compilation flags***************************************************************************
	//! Compilation switch for the expression template assignment strategy.
	static constexpr bool smpAssignable = VT::smpAssignable;
	//**********************************************************************************************

	//**Constructor*********************************************************************************
	/*!\brief Constructor for the SVecSerialExpr class.
	//
	// \param sv The sparse vector operand of the serial evaluation expression.
	*/
	explicit inline SVecSerialExpr(const VT &sv) noexcept : sv_(sv) // Sparse vector of the serial evaluation expression
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

	//**Operand access******************************************************************************
	/*!\brief Returns the sparse vector operand.
	//
	// \return The sparse vector operand.
	*/
	inline Operand operand() const noexcept { return sv_; }
	//**********************************************************************************************

	//**Conversion operator*************************************************************************
	/*!\brief Conversion to the type of the sparse vector operand.
	//
	// \return The sparse vector operand.
	*/
	inline operator Operand() const noexcept { return sv_; }
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
	Operand sv_; //!< Sparse vector of the serial evaluation expression.
	//**********************************************************************************************

	//**Assignment to dense vectors*****************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Assignment of a sparse vector serial evaluation expression to a dense vector.
	// \ingroup sparse_vector
	//
	// \param lhs The target left-hand side dense vector.
	// \param rhs The right-hand side serial evaluation expression to be assigned.
	// \return void
	//
	// This function implements the performance optimized assignment of a sparse vector serial
	// evaluation expression to a dense vector.
	*/
	template <typename VT2> // Type of the target dense vector
	friend inline void assign(DenseVector<VT2, TF> &lhs, const SVecSerialExpr &rhs)
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == rhs.size(), "Invalid vector sizes");

		assign(*lhs, rhs.sv_);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**Assignment to sparse vectors****************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Assignment of a sparse vector serial evaluation expression to a sparse vector.
	// \ingroup sparse_vector
	//
	// \param lhs The target left-hand side sparse vector.
	// \param rhs The right-hand side serial evaluation expression to be assigned.
	// \return void
	//
	// This function implements the performance optimized assignment of a sparse vector serial
	// evaluation expression to a sparse vector.
	*/
	template <typename VT2> // Type of the target sparse vector
	friend inline void assign(SparseVector<VT2, TF> &lhs, const SVecSerialExpr &rhs)
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == rhs.size(), "Invalid vector sizes");

		assign(*lhs, rhs.sv_);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**Addition assignment to dense vectors********************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Addition assignment of a sparse vector serial evaluation expression to a dense vector.
	// \ingroup sparse_vector
	//
	// \param lhs The target left-hand side dense vector.
	// \param rhs The right-hand side serial evaluation expression to be added.
	// \return void
	//
	// This function implements the performance optimized addition assignment of a sparse vector
	// serial evaluation expression to a dense vector.
	*/
	template <typename VT2> // Type of the target dense vector
	friend inline void addAssign(DenseVector<VT2, TF> &lhs, const SVecSerialExpr &rhs)
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == rhs.size(), "Invalid vector sizes");

		addAssign(*lhs, rhs.sv_);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**Addition assignment to sparse vectors*******************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Addition assignment of a sparse vector serial evaluation expression to a sparse vector.
	// \ingroup sparse_vector
	//
	// \param lhs The target left-hand side sparse vector.
	// \param rhs The right-hand side serial evaluation expression to be added.
	// \return void
	//
	// This function implements the performance optimized addition assignment of a sparse vector
	// serial evaluation expression to a sparse vector.
	*/
	template <typename VT2> // Type of the target sparse vector
	friend inline void addAssign(SparseVector<VT2, TF> &lhs, const SVecSerialExpr &rhs)
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == rhs.size(), "Invalid vector sizes");

		addAssign(*lhs, rhs.sv_);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**Subtraction assignment to dense vectors*****************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Subtraction assignment of a sparse vector serial evaluation expression to a dense
	//        vector.
	// \ingroup sparse_vector
	//
	// \param lhs The target left-hand side dense vector.
	// \param rhs The right-hand side serial evaluation expression to be subtracted.
	// \return void
	//
	// This function implements the performance optimized subtraction assignment of a sparse
	// vector serial evaluation expression to a dense vector.
	*/
	template <typename VT2> // Type of the target dense vector
	friend inline void subAssign(DenseVector<VT2, TF> &lhs, const SVecSerialExpr &rhs)
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == rhs.size(), "Invalid vector sizes");

		subAssign(*lhs, rhs.sv_);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**Subtraction assignment to sparse vectors****************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Subtraction assignment of a sparse vector serial evaluation expression to a sparse
	//        vector.
	// \ingroup sparse_vector
	//
	// \param lhs The target left-hand side sparse vector.
	// \param rhs The right-hand side serial evaluation expression to be subtracted.
	// \return void
	//
	// This function implements the performance optimized subtraction assignment of a sparse
	// vector serial evaluation expression to a sparse vector.
	*/
	template <typename VT2> // Type of the target sparse vector
	friend inline void subAssign(SparseVector<VT2, TF> &lhs, const SVecSerialExpr &rhs)
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == rhs.size(), "Invalid vector sizes");

		subAssign(*lhs, rhs.sv_);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**Multiplication assignment to dense vectors**************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Multiplication assignment of a sparse vector serial evaluation expression to a dense
	//        vector.
	// \ingroup sparse_vector
	//
	// \param lhs The target left-hand side dense vector.
	// \param rhs The right-hand side serial evaluation expression to be multiplied.
	// \return void
	//
	// This function implements the performance optimized multiplication assignment of a sparse
	// vector serial evaluation expression to a dense vector.
	*/
	template <typename VT2> // Type of the target dense vector
	friend inline void multAssign(DenseVector<VT2, TF> &lhs, const SVecSerialExpr &rhs)
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == rhs.size(), "Invalid vector sizes");

		multAssign(*lhs, rhs.sv_);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**Multiplication assignment to sparse vectors*************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Multiplication assignment of a sparse vector serial evaluation expression to a sparse
	//        vector.
	// \ingroup sparse_vector
	//
	// \param lhs The target left-hand side sparse vector.
	// \param rhs The right-hand side serial evaluation expression to be multiplied.
	// \return void
	//
	// This function implements the performance optimized multiplication assignment of a sparse
	// vector serial evaluation expression to a sparse vector.
	*/
	template <typename VT2> // Type of the target sparse vector
	friend inline void multAssign(SparseVector<VT2, TF> &lhs, const SVecSerialExpr &rhs)
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == rhs.size(), "Invalid vector sizes");

		multAssign(*lhs, rhs.sv_);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**SMP assignment to dense vectors*************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief SMP assignment of a sparse vector serial evaluation expression to a dense vector.
	// \ingroup sparse_vector
	//
	// \param lhs The target left-hand side dense vector.
	// \param rhs The right-hand side serial evaluation expression to be assigned.
	// \return void
	//
	// This function implements the performance optimized SMP assignment of a sparse vector
	// serial evaluation expression to a dense vector.
	*/
	template <typename VT2> // Type of the target dense vector
	friend inline void smpAssign(DenseVector<VT2, TF> &lhs, const SVecSerialExpr &rhs)
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == rhs.size(), "Invalid vector sizes");

		assign(*lhs, rhs.sv_);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**SMP assignment to sparse vectors************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief SMP assignment of a sparse vector serial evaluation expression to a sparse vector.
	// \ingroup sparse_vector
	//
	// \param lhs The target left-hand side sparse vector.
	// \param rhs The right-hand side serial evaluation expression to be assigned.
	// \return void
	//
	// This function implements the performance optimized SMP assignment of a sparse vector
	// serial evaluation expression to a sparse vector.
	*/
	template <typename VT2> // Type of the target sparse vector
	friend inline void smpAssign(SparseVector<VT2, TF> &lhs, const SVecSerialExpr &rhs)
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == rhs.size(), "Invalid vector sizes");

		assign(*lhs, rhs.sv_);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**SMP addition assignment to dense vectors****************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief SMP addition assignment of a sparse vector serial evaluation expression to a dense
	//        vector.
	// \ingroup sparse_vector
	//
	// \param lhs The target left-hand side dense vector.
	// \param rhs The right-hand side serial evaluation expression to be added.
	// \return void
	//
	// This function implements the performance optimized SMP addition assignment of a sparse
	// vector serial evaluation expression to a dense vector.
	*/
	template <typename VT2> // Type of the target dense vector
	friend inline void smpAddAssign(DenseVector<VT2, TF> &lhs, const SVecSerialExpr &rhs)
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == rhs.size(), "Invalid vector sizes");

		addAssign(*lhs, rhs.sv_);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**SMP addition assignment to sparse vectors***************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief SMP addition assignment of a sparse vector serial evaluation expression to a sparse
	//        vector.
	// \ingroup sparse_vector
	//
	// \param lhs The target left-hand side sparse vector.
	// \param rhs The right-hand side serial evaluation expression to be added.
	// \return void
	//
	// This function implements the performance optimized SMP addition assignment of a sparse
	// vector serial evaluation expression to a sparse vector.
	*/
	template <typename VT2> // Type of the target sparse vector
	friend inline void smpAddAssign(SparseVector<VT2, TF> &lhs, const SVecSerialExpr &rhs)
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == rhs.size(), "Invalid vector sizes");

		addAssign(*lhs, rhs.sv_);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**SMP subtraction assignment to dense vectors*************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief SMP subtraction assignment of a sparse vector serial evaluation expression to a dense
	//        vector.
	// \ingroup sparse_vector
	//
	// \param lhs The target left-hand side dense vector.
	// \param rhs The right-hand side serial evaluation expression to be subtracted.
	// \return void
	//
	// This function implements the performance optimized SMP subtraction assignment of a sparse
	// vector serial evaluation expression to a dense vector.
	*/
	template <typename VT2> // Type of the target dense vector
	friend inline void smpSubAssign(DenseVector<VT2, TF> &lhs, const SVecSerialExpr &rhs)
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == rhs.size(), "Invalid vector sizes");

		subAssign(*lhs, rhs.sv_);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**SMP subtraction assignment to sparse vectors************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief SMP subtraction assignment of a sparse vector serial evaluation expression to a sparse
	//        vector.
	// \ingroup sparse_vector
	//
	// \param lhs The target left-hand side sparse vector.
	// \param rhs The right-hand side serial evaluation expression to be subtracted.
	// \return void
	//
	// This function implements the performance optimized SMP subtraction assignment of a sparse
	// vector serial evaluation expression to a sparse vector.
	*/
	template <typename VT2> // Type of the target sparse vector
	friend inline void smpSubAssign(SparseVector<VT2, TF> &lhs, const SVecSerialExpr &rhs)
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == rhs.size(), "Invalid vector sizes");

		subAssign(*lhs, rhs.sv_);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**SMP multiplication assignment to dense vectors**********************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief SMP multiplication assignment of a sparse vector serial evaluation expression to a
	//        dense vector.
	// \ingroup sparse_vector
	//
	// \param lhs The target left-hand side dense vector.
	// \param rhs The right-hand side serial evaluation expression to be multiplied.
	// \return void
	//
	// This function implements the performance optimized SMP multiplication assignment of a sparse
	// vector serial evaluation expression to a dense vector.
	*/
	template <typename VT2> // Type of the target dense vector
	friend inline void smpMultAssign(DenseVector<VT2, TF> &lhs, const SVecSerialExpr &rhs)
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == rhs.size(), "Invalid vector sizes");

		multAssign(*lhs, rhs.sv_);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**SMP multiplication assignment to sparse vectors*********************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief SMP multiplication assignment of a sparse vector serial evaluation expression to a
	//        sparse vector.
	// \ingroup sparse_vector
	//
	// \param lhs The target left-hand side sparse vector.
	// \param rhs The right-hand side serial evaluation expression to be multiplied.
	// \return void
	//
	// This function implements the performance optimized SMP multiplication assignment of a sparse
	// vector serial evaluation expression to a sparse vector.
	*/
	template <typename VT2> // Type of the target sparse vector
	friend inline void smpMultAssign(SparseVector<VT2, TF> &lhs, const SVecSerialExpr &rhs)
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == rhs.size(), "Invalid vector sizes");

		multAssign(*lhs, rhs.sv_);
	}
	/*! \endcond */
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
/*!\brief Forces the serial evaluation of the given sparse vector expression \a sv.
// \ingroup sparse_vector
//
// \param sv The input vector.
// \return The evaluated sparse vector.
//
// The \a serial function forces the serial evaluation of the given sparse vector expression
// \a sv. The function returns an expression representing this operation.\n
// The following example demonstrates the use of the \a serial function:

   \code
   mtrc::numeric::CompressedVector<double> a, b;
   // ... Resizing and initialization
   b = serial( a );
   \endcode
*/
template <typename VT // Type of the dense vector
		  ,
		  bool TF> // Transpose flag
inline decltype(auto) serial(const SparseVector<VT, TF> &sv)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	using ReturnType = const SVecSerialExpr<VT, TF>;
	return ReturnType(*sv);
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
