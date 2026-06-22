// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_DMATSVDEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_DMATSVDEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/Aliases.h>
#include <metric/numeric/math/Exception.h>
#include <metric/numeric/math/constraints/BLASCompatible.h>
#include <metric/numeric/math/constraints/DenseMatrix.h>
#include <metric/numeric/math/constraints/StorageOrder.h>
#include <metric/numeric/math/expressions/Computation.h>
#include <metric/numeric/math/expressions/DenseVector.h>
#include <metric/numeric/math/expressions/Forward.h>
#include <metric/numeric/math/expressions/SVDExpr.h>
#include <metric/numeric/math/shims/Serial.h>
#include <metric/numeric/math/traits/ColumnTrait.h>
#include <metric/numeric/math/typetraits/IsExpression.h>
#include <metric/numeric/math/typetraits/MaxSize.h>
#include <metric/numeric/math/typetraits/Size.h>
#include <metric/numeric/math/typetraits/UnderlyingElement.h>
#include <metric/numeric/util/Assert.h>
#include <metric/numeric/util/FunctionTrace.h>
#include <metric/numeric/util/Types.h>
#include <metric/numeric/util/algorithms/Min.h>
#include <metric/numeric/util/mpl/If.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DMATSVDEXPR
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Expression object for dense matrix singular value solvers.
// \ingroup dense_vector_expression
//
// The DMatSVDExpr class represents the compile time expression for dense matrix singular value
// solvers.
*/
template <typename MT // Type of the dense matrix
		  ,
		  bool SO> // Storage order
class DMatSVDExpr : public SVDExpr<DenseVector<DMatSVDExpr<MT, SO>, false>>, private Computation {
  private:
	//**Type definitions****************************************************************************
	using CRT = ColumnTrait_t<ResultType_t<MT>>;	//!< Column type of the matrix result type.
	using CTT = ColumnTrait_t<TransposeType_t<MT>>; //!< Column type of the matrix transpose type.

	//! Type of the resulting vector.
	using VT = If_t<(Size_v<CRT, 0UL> < Size_v<CTT, 0UL> || MaxSize_v<CRT, 0UL> < MaxSize_v<CTT, 0UL>), CRT, CTT>;

	//! Element type of the resulting vector.
	using ET = UnderlyingElement_t<ElementType_t<MT>>;
	//**********************************************************************************************

  public:
	//**Type definitions****************************************************************************
	//! Type of this DMatSVDExpr instance.
	using This = DMatSVDExpr<MT, SO>;

	//! Base type of this DMatSVDExpr instance.
	using BaseType = SVDExpr<DenseMatrix<This, SO>>;

	using ResultType = Rebind_t<VT, ET>;			   //!< Result type for expression template evaluations.
	using TransposeType = TransposeType_t<ResultType>; //!< Transpose type for expression template evaluations.
	using ElementType = ElementType_t<ResultType>;	   //!< Resulting element type.
	using ReturnType = ReturnType_t<ResultType>;	   //!< Return type for expression template evaluations.

	//! Data type for composite expression templates.
	using CompositeType = const ResultType;

	//! Composite data type of the dense matrix expression.
	using Operand = If_t<IsExpression_v<MT>, const MT, const MT &>;
	//**********************************************************************************************

	//**Compilation flags***************************************************************************
	//! Compilation switch for the expression template evaluation strategy.
	static constexpr bool simdEnabled = false;

	//! Compilation switch for the expression template assignment strategy.
	static constexpr bool smpAssignable = false;
	//**********************************************************************************************

	//**Constructor*********************************************************************************
	/*!\brief Constructor for the DMatSVDExpr class.
	//
	// \param dm The dense matrix operand of the singular value expression.
	*/
	explicit inline DMatSVDExpr(const MT &dm) noexcept : dm_(dm) // Dense matrix of the singular value expression
	{
	}
	//**********************************************************************************************

	//**Size function*******************************************************************************
	/*!\brief Returns the current size/dimension of the vector.
	//
	// \return The size of the vector.
	*/
	inline size_t size() const noexcept { return min(dm_.rows(), dm_.columns()); }
	//**********************************************************************************************

	//**Operand access******************************************************************************
	/*!\brief Returns the dense matrix operand.
	//
	// \return The dense matrix operand.
	*/
	inline Operand operand() const noexcept { return dm_; }
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Returns whether the expression can alias with the given address \a alias.
	//
	// \param alias The alias to be checked.
	// \return \a true in case the expression can alias, \a false otherwise.
	*/
	template <typename T> inline bool canAlias(const T *alias) const noexcept { return dm_.isAliased(alias); }
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Returns whether the expression is aliased with the given address \a alias.
	//
	// \param alias The alias to be checked.
	// \return \a true in case an alias effect is detected, \a false otherwise.
	*/
	template <typename T> inline bool isAliased(const T *alias) const noexcept { return dm_.isAliased(alias); }
	//**********************************************************************************************

  private:
	//**Member variables****************************************************************************
	Operand dm_; //!< Dense matrix of the singular value expression.
	//**********************************************************************************************

	//**Assignment to dense vectors*****************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Assignment of a dense matrix singular value expression to a dense vector.
	// \ingroup dense_vector
	//
	// \param lhs The target left-hand side dense vector.
	// \param rhs The right-hand side singular value expression to be assigned.
	// \return void
	//
	// This function implements the performance optimized assignment of a dense matrix singular
	// value expression to a dense vector.
	*/
	template <typename VT> // Type of the target dense vector
	friend inline void assign(DenseVector<VT, false> &lhs, const DMatSVDExpr &rhs)
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == rhs.size(), "Invalid vector sizes");

		svd(rhs.operand(), *lhs);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**Assignment to sparse vectors****************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Assignment of a dense matrix singular value expression to a sparse vector.
	// \ingroup dense_vector
	//
	// \param lhs The target left-hand side sparse vector.
	// \param rhs The right-hand side singular value expression to be assigned.
	// \return void
	//
	// This function implements the performance optimized assignment of a dense matrix singular
	// value expression to a sparse vector.
	*/
	template <typename VT> // Type of the target sparse vector
	friend inline void assign(SparseVector<VT, false> &lhs, const DMatSVDExpr &rhs)
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == rhs.size(), "Invalid vector sizes");

		const ResultType tmp(serial(rhs));
		assign(*lhs, tmp);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**Addition assignment to dense vectors********************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Addition assignment of a dense matrix singular value expression to a dense vector.
	// \ingroup dense_vector
	//
	// \param lhs The target left-hand side dense vector.
	// \param rhs The right-hand side singular value expression to be added.
	// \return void
	//
	// This function implements the performance optimized addition assignment of a dense matrix
	// singular value expression to a dense vector.
	*/
	template <typename VT> // Type of the target dense vector
	friend inline void addAssign(DenseVector<VT, false> &lhs, const DMatSVDExpr &rhs)
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == rhs.size(), "Invalid vector sizes");

		const ResultType tmp(serial(rhs));
		addAssign(*lhs, tmp);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**Addition assignment to sparse vectors*******************************************************
	// No special implementation for the addition assignment to sparse vectors.
	//**********************************************************************************************

	//**Subtraction assignment to dense vectors*****************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Subtraction assignment of a dense matrix singular value expression to a dense vector.
	// \ingroup dense_vector
	//
	// \param lhs The target left-hand side dense vector.
	// \param rhs The right-hand side singular value expression to be subtracted.
	// \return void
	//
	// This function implements the performance optimized subtraction assignment of a dense matrix
	// singular value expression to a dense vector.
	*/
	template <typename VT> // Type of the target dense vector
	friend inline void subAssign(DenseMatrix<VT, false> &lhs, const DMatSVDExpr &rhs)
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == rhs.size(), "Invalid vector sizes");

		const ResultType tmp(serial(rhs));
		subAssign(*lhs, tmp);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**Subtraction assignment to sparse vectors****************************************************
	// No special implementation for the subtraction assignment to sparse vectors.
	//**********************************************************************************************

	//**Multiplication assignment to dense vectors**************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Multiplication assignment of a dense matrix singular value expression to a dense vector.
	// \ingroup dense_vector
	//
	// \param lhs The target left-hand side dense vector.
	// \param rhs The right-hand side singular value expression to be multiplied.
	// \return void
	//
	// This function implements the performance optimized multiplication assignment of a dense
	// matrix singular value expression to a dense vector.
	*/
	template <typename VT> // Type of the target dense vector
	friend inline void multAssign(DenseMatrix<VT, false> &lhs, const DMatSVDExpr &rhs)
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == rhs.size(), "Invalid vector sizes");

		const ResultType tmp(serial(rhs));
		multAssign(*lhs, tmp);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**Multiplication assignment to sparse vectors*************************************************
	// No special implementation for the multiplication assignment to sparse vectors.
	//**********************************************************************************************

	//**Division assignment to dense vectors********************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Division assignment of a dense matrix singular value expression to a dense vector.
	// \ingroup dense_vector
	//
	// \param lhs The target left-hand side dense vector.
	// \param rhs The right-hand side singular value expression divisor.
	// \return void
	//
	// This function implements the performance optimized division assignment of a dense matrix
	// singular value expression to a dense vector.
	*/
	template <typename VT> // Type of the target dense vector
	friend inline void divAssign(DenseMatrix<VT, false> &lhs, const DMatSVDExpr &rhs)
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).size() == rhs.size(), "Invalid vector sizes");

		const ResultType tmp(serial(rhs));
		divAssign(*lhs, tmp);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**Division assignment to sparse vectors*******************************************************
	// No special implementation for the division assignment to sparse vectors.
	//**********************************************************************************************

	//**Compile time checks*************************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_DENSE_MATRIX_TYPE(MT);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_MATRIX_WITH_STORAGE_ORDER(MT, SO);
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
/*!\brief Calculation of the singular values of the given dense matrix.
// \ingroup dense_vector
//
// \param dm The given general matrix.
// \return The singular values of the matrix.
//
// This function returns an expression representing the singular values of the given dense matrix:

   \code
   using mtrc::numeric::rowMajor;

   mtrc::numeric::DynamicMatrix<double,rowMajor> A, B;
   // ... Resizing and initialization
   B = svd( A );
   \endcode

// \note The \c svd() function can only be used for dense matrices with \c float, \c double,
// \c complex<float> or \c complex<double> element type. The attempt to call the function with
// matrices of any other element type results in a compile time error!
//
// \note It is not possible to use any kind of view on the expression object returned by the
// \c svd() function. Also, it is not possible to access individual elements via the subscript
// operator on the expression object:

   \code
   subvector( svd( A ), 2, 4 );  // Compilation error: Views cannot be used on an svd() expression!
   svd( A )[1];                  // Compilation error: It is not possible to access individual elements!
   \endcode
*/
template <typename MT // Type of the dense matrix
		  ,
		  bool SO> // Storage order
inline decltype(auto) svd(const DenseMatrix<MT, SO> &dm)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	METRIC_NUMERIC_CONSTRAINT_MUST_BE_BLAS_COMPATIBLE_TYPE(ElementType_t<MT>);

	using ReturnType = const DMatSVDExpr<MT, SO>;
	return ReturnType(*dm);
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
