// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_SVECREPEATEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_SVECREPEATEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/Aliases.h>
#include <metric/numeric/math/Exception.h>
#include <metric/numeric/math/constraints/SparseVector.h>
#include <metric/numeric/math/constraints/TransposeFlag.h>
#include <metric/numeric/math/expressions/Computation.h>
#include <metric/numeric/math/expressions/Forward.h>
#include <metric/numeric/math/expressions/RepeatExprData.h>
#include <metric/numeric/math/expressions/SparseVector.h>
#include <metric/numeric/math/expressions/Transformation.h>
#include <metric/numeric/math/expressions/VecRepeatExpr.h>
#include <metric/numeric/math/shims/Serial.h>
#include <metric/numeric/math/traits/RepeatTrait.h>
#include <metric/numeric/math/typetraits/IsExpression.h>
#include <metric/numeric/math/views/Check.h>
#include <metric/numeric/math/views/subvector/Dense.h>
#include <metric/numeric/util/FunctionTrace.h>
#include <metric/numeric/util/Types.h>
#include <metric/numeric/util/mpl/If.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS SVECREPEATEXPR
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Expression object for the sparse vector repeat() function.
// \ingroup sparse_vector_expression
//
// The SVecRepeatExpr class represents the compile time expression for repeating a sparse vector
// via the repeat() function.
*/
template <typename VT // Type of the sparse vector
		  ,
		  bool TF // Transpose flag
		  ,
		  size_t... CRAs> // Compile time repeater arguments
class SVecRepeatExpr : public VecRepeatExpr<SparseVector<SVecRepeatExpr<VT, TF, CRAs...>, TF>, CRAs...>,
					   private If_t<IsComputation_v<VT>, Computation, Transformation>,
					   private RepeatExprData<1UL, CRAs...> {
  private:
	//**Type definitions****************************************************************************
	using DataType = RepeatExprData<1UL, CRAs...>; //!< The type of the RepeatExprData base class.
	//**********************************************************************************************

  public:
	//**Type definitions****************************************************************************
	//! Type of this SVecRepeatExpr instance.
	using This = SVecRepeatExpr<VT, TF, CRAs...>;

	//! Base type of this SVecRepeatExpr instance.
	using BaseType = VecRepeatExpr<SparseVector<This, TF>, CRAs...>;

	using ResultType = RepeatTrait_t<VT, CRAs...>;	   //!< Result type for expression template evaluations.
	using TransposeType = TransposeType_t<ResultType>; //!< Transpose type for expression template evaluations.
	using ElementType = ElementType_t<VT>;			   //!< Resulting element type.
	using ReturnType = ReturnType_t<VT>;			   //!< Return type for expression template evaluations.
	using CompositeType = const ResultType;			   //!< Data type for composite expression templates.

	//! Composite data type of the sparse vector expression.
	using Operand = If_t<IsExpression_v<VT>, const VT, const VT &>;
	//**********************************************************************************************

  public:
	//**Compilation flags***************************************************************************
	//! Compilation switch for the expression template assignment strategy.
	static constexpr bool smpAssignable = false;
	//**********************************************************************************************

	//**Constructor*********************************************************************************
	/*!\brief Constructor for the SVecRepeatExpr class.
	//
	// \param sv The sparse vector operand of the repeater expression.
	// \param args The number of repetitions.
	*/
	template <typename... RRAs> // Runtime repeater arguments
	explicit inline SVecRepeatExpr(const VT &sv, RRAs... args) noexcept
		: DataType(args...) // Base class initialization
		  ,
		  sv_(sv) // Sparse vector of the repeater expression
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
		METRIC_NUMERIC_INTERNAL_ASSERT(index < size(), "Invalid vector access index");
		return sv_[index % sv_.size()];
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
		if (index >= size()) {
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
	inline size_t size() const noexcept { return sv_.size() * this->template repetitions<0UL>(); }
	//**********************************************************************************************

	//**NonZeros function***************************************************************************
	/*!\brief Returns the number of non-zero elements in the sparse vector.
	//
	// \return The number of non-zero elements in the sparse vector.
	*/
	inline size_t nonZeros() const { return sv_.nonZeros() * this->template repetitions<0UL>(); }
	//**********************************************************************************************

	//**Operand access******************************************************************************
	/*!\brief Returns the sparse vector operand.
	//
	// \return The sparse vector operand.
	*/
	inline Operand operand() const noexcept { return sv_; }
	//**********************************************************************************************

	//**********************************************************************************************
	using DataType::repetitions;
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Returns whether the expression can alias with the given address \a alias.
	//
	// \param alias The alias to be checked.
	// \return \a true in case the expression can alias, \a false otherwise.
	*/
	template <typename T> inline bool canAlias(const T *alias) const noexcept
	{
		return IsExpression_v<VT> && sv_.canAlias(alias);
	}
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
	inline bool canSMPAssign() const noexcept { return false; }
	//**********************************************************************************************

  private:
	//**Member variables****************************************************************************
	Operand sv_; //!< Sparse vector of the repeater expression.
	//**********************************************************************************************

	//**Assignment to dense vectors*****************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Assignment of a sparse vector repeater expression to a dense vector.
	// \ingroup sparse_vector
	//
	// \param lhs The target left-hand side dense vector.
	// \param rhs The right-hand side repeater expression to be assigned.
	// \return void
	//
	// This function implements the performance optimized assignment of a sparse vector repeater
	// expression to a dense vector.
	*/
	template <typename VT2> // Type of the target dense vector
	friend inline void assign(DenseVector<VT2, TF> &lhs, const SVecRepeatExpr &rhs)
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == rhs.size(), "Invalid vector sizes");

		CompositeType_t<VT> x(serial(rhs.sv_)); // Evaluation of the sparse vector operand

		const size_t reps(rhs.template repetitions<0UL>());
		const size_t size(x.size());

		for (size_t rep = 0UL; rep < reps; ++rep) {
			subvector(*lhs, rep * size, size, unchecked) = serial(x);
		}
	}
	/*! \endcond */
	//**********************************************************************************************

	//**Assignment to sparse vectors****************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Assignment of a sparse vector repeater expression to a sparse vector.
	// \ingroup sparse_vector
	//
	// \param lhs The target left-hand side sparse vector.
	// \param rhs The right-hand side repeater expression to be assigned.
	// \return void
	//
	// This function implements the performance optimized assignment of a sparse vector repeater
	// expression to a sparse vector.
	*/
	template <typename VT2> // Type of the target sparse vector
	friend inline void assign(SparseVector<VT2, TF> &lhs, const SVecRepeatExpr &rhs)
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == rhs.size(), "Invalid vector sizes");

		CompositeType_t<VT> x(serial(rhs.sv_)); // Evaluation of the sparse vector operand

		const size_t reps(rhs.template repetitions<0UL>());
		const size_t size(x.size());

		(*lhs).reserve(reps * size);

		for (size_t rep = 0UL; rep < reps; ++rep) {
			for (auto element = x.begin(); element != x.end(); ++element) {
				(*lhs).append(rep * size + element->index(), element->value(), true);
			}
		}
	}
	/*! \endcond */
	//**********************************************************************************************

	//**Addition assignment to dense vectors********************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Addition assignment of a sparse vector repeater expression to a dense vector.
	// \ingroup sparse_vector
	//
	// \param lhs The target left-hand side dense vector.
	// \param rhs The right-hand side repeater expression to be added.
	// \return void
	//
	// This function implements the performance optimized addition assignment of a sparse vector
	// repeater expression to a dense vector.
	*/
	template <typename VT2> // Type of the target dense vector
	friend inline void addAssign(DenseVector<VT2, TF> &lhs, const SVecRepeatExpr &rhs)
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == rhs.size(), "Invalid vector sizes");

		CompositeType_t<VT> x(serial(rhs.sv_)); // Evaluation of the sparse vector operand

		const size_t reps(rhs.template repetitions<0UL>());
		const size_t size(x.size());

		for (size_t rep = 0UL; rep < reps; ++rep) {
			subvector(*lhs, rep * size, size, unchecked) += serial(x);
		}
	}
	/*! \endcond */
	//**********************************************************************************************

	//**Addition assignment to sparse vectors*******************************************************
	// No special implementation for the addition assignment to sparse vectors.
	//**********************************************************************************************

	//**Subtraction assignment to dense vectors*****************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Subtraction assignment of a sparse vector repeater expression to a dense vector.
	// \ingroup sparse_vector
	//
	// \param lhs The target left-hand side dense vector.
	// \param rhs The right-hand side repeater expression to be subtracted.
	// \return void
	//
	// This function implements the performance optimized subtraction assignment of a sparse
	// vector repeater expression to a dense vector.
	*/
	template <typename VT2> // Type of the target dense vector
	friend inline void subAssign(DenseVector<VT2, TF> &lhs, const SVecRepeatExpr &rhs)
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == rhs.size(), "Invalid vector sizes");

		CompositeType_t<VT> x(serial(rhs.sv_)); // Evaluation of the sparse vector operand

		const size_t reps(rhs.template repetitions<0UL>());
		const size_t size(x.size());

		for (size_t rep = 0UL; rep < reps; ++rep) {
			subvector(*lhs, rep * size, size, unchecked) -= serial(x);
		}
	}
	/*! \endcond */
	//**********************************************************************************************

	//**Multiplication assignment to sparse vectors*************************************************
	// No special implementation for the multiplication assignment to sparse vectors.
	//**********************************************************************************************

	//**Multiplication assignment to dense vectors**************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Multiplication assignment of a sparse vector repeater expression to a dense vector.
	// \ingroup sparse_vector
	//
	// \param lhs The target left-hand side dense vector.
	// \param rhs The right-hand side repeater expression to be multiplied.
	// \return void
	//
	// This function implements the performance optimized multiplication assignment of a sparse
	// vector repeater expression to a dense vector.
	*/
	template <typename VT2> // Type of the target dense vector
	friend inline void multAssign(DenseVector<VT2, TF> &lhs, const SVecRepeatExpr &rhs)
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == rhs.size(), "Invalid vector sizes");

		CompositeType_t<VT> x(serial(rhs.sv_)); // Evaluation of the sparse vector operand

		const size_t reps(rhs.template repetitions<0UL>());
		const size_t size(x.size());

		for (size_t rep = 0UL; rep < reps; ++rep) {
			subvector(*lhs, rep * size, size, unchecked) *= serial(x);
		}
	}
	/*! \endcond */
	//**********************************************************************************************

	//**Multiplication assignment to sparse vectors*************************************************
	// No special implementation for the multiplication assignment to sparse vectors.
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
/*!\brief Repeats the given sparse vector.
// \ingroup sparse_vector
//
// \param sv The sparse vector to be repeated.
// \param repetitions The number of repetitions.
// \return The repeated sparse vector.
//
// This function returns an expression representing the repeated sparse vector:

   \code
   using mtrc::numeric::columnVector;
   using mtrc::numeric::rowVector;

   mtrc::numeric::CompressedVector<int,columnVector> a1{ 1, 0, -2 };
   mtrc::numeric::CompressedVector<int,rowVector> b1{ 0, -1, 7 };

   mtrc::numeric::CompressedVector<int,columnVector> a2;
   mtrc::numeric::CompressedVector<int,rowVector> b2;

   // ... Resizing and initialization

   a2 = repeat( a1, 3UL );  // Results in ( 1, 0, -2, 1, 0, -2, 1, 0, -2 )
   b2 = repeat( b1, 3UL );  // Results in ( 0, -1, 7, 0, -1, 7, 0, -1, 7 )
   \endcode
*/
template <typename VT // Type of the sparse vector
		  ,
		  bool TF> // Transpose flag
inline decltype(auto) repeat(const SparseVector<VT, TF> &sv, size_t repetitions)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	using ReturnType = const SVecRepeatExpr<VT, TF>;
	return ReturnType(*sv, repetitions);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Repeats the given sparse vector.
// \ingroup sparse_vector
//
// \param sv The sparse vector to be repeated.
// \return The repeated sparse vector.
//
// This function returns an expression representing the repeated sparse vector:

   \code
   using mtrc::numeric::columnVector;
   using mtrc::numeric::rowVector;

   mtrc::numeric::CompressedVector<int,columnVector> a1{ 1, 0, -2 };
   mtrc::numeric::CompressedVector<int,rowVector> b1{ 0, -1, 7 };

   mtrc::numeric::CompressedVector<int,columnVector> a2;
   mtrc::numeric::CompressedVector<int,rowVector> b2;

   // ... Resizing and initialization

   a2 = repeat<3UL>( a1 );  // Results in ( 1, 0, -2, 1, 0, -2, 1, 0, -2 )
   b2 = repeat<3UL>( b1 );  // Results in ( 0, -1, 7, 0, -1, 7, 0, -1, 7 )
   \endcode
*/
template <size_t R // Compile time repetitions
		  ,
		  typename VT // Type of the sparse vector
		  ,
		  bool TF> // Transpose flag
inline decltype(auto) repeat(const SparseVector<VT, TF> &sv)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	using ReturnType = const SVecRepeatExpr<VT, TF, R>;
	return ReturnType(*sv);
}
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Repeating the given sparse vector.
// \ingroup sparse_vector
//
// \param sv The sparse vector to be repeated.
// \param repetitions The number of repetitions.
// \return The repeated sparse vector.
//
// This auxiliary overload of the \c repeat() function accepts both a compile time and a runtime
// expansion. The runtime argument is discarded in favor of the compile time argument.
*/
template <size_t R0 // Compile time repetitions
		  ,
		  typename VT // Type of the sparse vector
		  ,
		  bool TF> // Transpose flag
inline decltype(auto) repeat(const SparseVector<VT, TF> &sv, size_t repetitions)
{
	MAYBE_UNUSED(repetitions);

	METRIC_NUMERIC_FUNCTION_TRACE;

	using ReturnType = const SVecRepeatExpr<VT, TF, R0>;
	return ReturnType(*sv);
}
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
