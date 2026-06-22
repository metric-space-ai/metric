// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_DMATEXPEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_DMATEXPEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/Aliases.h>
#include <metric/numeric/math/Exception.h>
#include <metric/numeric/math/constraints/BLASCompatible.h>
#include <metric/numeric/math/constraints/DenseMatrix.h>
#include <metric/numeric/math/constraints/RequiresEvaluation.h>
#include <metric/numeric/math/constraints/StorageOrder.h>
#include <metric/numeric/math/expressions/Computation.h>
#include <metric/numeric/math/expressions/DenseMatrix.h>
#include <metric/numeric/math/expressions/Forward.h>
#include <metric/numeric/math/expressions/MatExpExpr.h>
#include <metric/numeric/math/shims/Exp.h>
#include <metric/numeric/math/shims/Frexp.h>
#include <metric/numeric/math/shims/Pow.h>
#include <metric/numeric/math/shims/Serial.h>
#include <metric/numeric/math/typetraits/IsDiagonal.h>
#include <metric/numeric/math/typetraits/IsExpression.h>
#include <metric/numeric/math/typetraits/RemoveAdaptor.h>
#include <metric/numeric/math/typetraits/UnderlyingBuiltin.h>
#include <metric/numeric/util/Assert.h>
#include <metric/numeric/util/FunctionTrace.h>
#include <metric/numeric/util/Types.h>
#include <metric/numeric/util/algorithms/Max.h>
#include <metric/numeric/util/mpl/If.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DMATEXPEXPR
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Expression object for dense matrix exponential.
// \ingroup dense_matrix_expression
//
// The DMatExpExpr class represents the compile time expression for the dense matrices exponential
// operation (see https://en.wikipedia.org/wiki/Matrix_exponential).
*/
template <typename MT // Type of the dense matrix
		  ,
		  bool SO> // Storage order
class DMatExpExpr : public MatExpExpr<DenseMatrix<DMatExpExpr<MT, SO>, SO>>, private Computation {
  private:
	//**Type definitions****************************************************************************
	using RT = ResultType_t<MT>;  //!< Result type of the dense matrix expression.
	using ET = ElementType_t<MT>; //!< Element type of the dense matrix expression.
	//**********************************************************************************************

	//**********************************************************************************************
	static constexpr size_t K = 18; //!< The approximation limit for the exponential computation.
	//**********************************************************************************************

  public:
	//**Type definitions****************************************************************************
	//! Type of this DMatExpExpr instance.
	using This = DMatExpExpr<MT, SO>;

	//! Base type of this DMatExpExpr instance.
	using BaseType = MatExpExpr<DenseMatrix<This, SO>>;

	using ResultType = RemoveAdaptor_t<RT>; //!< Result type for expression template evaluations.
	using OppositeType =
		OppositeType_t<MT>; //!< Result type with opposite storage order for expression template evaluations.
	using TransposeType = TransposeType_t<MT>; //!< Transpose type for expression template evaluations.
	using ElementType = ElementType_t<MT>;	   //!< Resulting element type.
	using ReturnType = ReturnType_t<MT>;	   //!< Return type for expression template evaluations.

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
	/*!\brief Constructor for the DMatExpExpr class.
	//
	// \param dm The dense matrix operand of the exponential expression.
	*/
	explicit inline DMatExpExpr(const MT &dm) noexcept : dm_(dm) // Dense matrix of the exponential expression
	{
		METRIC_NUMERIC_INTERNAL_ASSERT(isSquare(*dm), "Non-square matrix detected");
	}
	//**********************************************************************************************

	//**Rows function*******************************************************************************
	/*!\brief Returns the current number of rows of the matrix.
	//
	// \return The number of rows of the matrix.
	*/
	inline size_t rows() const noexcept { return dm_.columns(); }
	//**********************************************************************************************

	//**Columns function****************************************************************************
	/*!\brief Returns the current number of columns of the matrix.
	//
	// \return The number of columns of the matrix.
	*/
	inline size_t columns() const noexcept { return dm_.rows(); }
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
	Operand dm_; //!< Dense matrix of the exponential expression.
	//**********************************************************************************************

	//**Assignment to dense matrices****************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Assignment of a dense matrix exponential expression to a dense matrix.
	// \ingroup dense_matrix
	//
	// \param lhs The target left-hand side dense matrix.
	// \param rhs The right-hand side exponential expression to be assigned.
	// \return void
	//
	// This function implements the performance optimized assignment of a dense matrix exponential
	// expression to a dense matrix.
	*/
	template <typename MT2 // Type of the target dense matrix
			  ,
			  bool SO2> // Storage order of the target dense matrix
	friend inline void assign(DenseMatrix<MT2, SO2> &lhs, const DMatExpExpr &rhs)
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).rows() == rhs.rows(), "Invalid number of rows");
		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).columns() == rhs.columns(), "Invalid number of columns");

		const size_t N(rhs.rows());

		if (IsDiagonal_v<MT> || N < 2UL) {
			assign(*lhs, rhs.dm_);
			for (size_t i = 0UL; i < N; ++i) {
				(*lhs)(i, i) = exp((*lhs)(i, i));
			}
		} else {
			using BT = UnderlyingBuiltin_t<ET>;

			const BT norm(maxNorm(rhs.dm_));

			int exponent(0);
			frexp(norm, &exponent);
			exponent = max(0, exponent);

			ResultType R(rhs.dm_ / pow(2.0, double(exponent)));
			ResultType A(R);
			ResultType B(R);

			for (size_t i = 0UL; i < N; ++i) {
				B(i, i) += BT(1);
			}

			BT factor(1);
			for (size_t k = 2UL; k < K; ++k) {
				factor *= BT(k);
				A *= R;
				addAssign(B, (A / factor));
			}

			for (int i = 0; i < exponent; ++i) {
				B *= B;
			}

			assign(*lhs, B);
		}
	}
	/*! \endcond */
	//**********************************************************************************************

	//**Assignment to sparse matrices***************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Assignment of a dense matrix exponential expression to a sparse matrix.
	// \ingroup dense_matrix
	//
	// \param lhs The target left-hand side sparse matrix.
	// \param rhs The right-hand side exponential expression to be assigned.
	// \return void
	//
	// This function implements the performance optimized assignment of a dense matrix exponential
	// expression to a sparse matrix.
	*/
	template <typename MT2 // Type of the target sparse matrix
			  ,
			  bool SO2> // Storage order of the target sparse matrix
	friend inline void assign(SparseMatrix<MT2, SO2> &lhs, const DMatExpExpr &rhs)
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_CONSTRAINT_MUST_BE_DENSE_MATRIX_TYPE(ResultType);
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_MATRIX_WITH_STORAGE_ORDER(ResultType, SO);
		METRIC_NUMERIC_CONSTRAINT_MUST_NOT_REQUIRE_EVALUATION(ResultType);

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).rows() == rhs.rows(), "Invalid number of rows");
		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).columns() == rhs.columns(), "Invalid number of columns");

		const ResultType tmp(serial(rhs));
		assign(*lhs, tmp);
	}
	/*! \endcond */
	//**********************************************************************************************

	//**Addition assignment to dense matrices*******************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Addition assignment of a dense matrix exponential expression to a dense matrix.
	// \ingroup dense_matrix
	//
	// \param lhs The target left-hand side dense matrix.
	// \param rhs The right-hand side exponential expression to be added.
	// \return void
	//
	// This function implements the performance optimized addition assignment of a dense matrix
	// exponential expression to a dense matrix.
	*/
	template <typename MT2 // Type of the target dense matrix
			  ,
			  bool SO2> // Storage order of the target dense matrix
	friend inline void addAssign(DenseMatrix<MT2, SO2> &lhs, const DMatExpExpr &rhs)
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).rows() == rhs.rows(), "Invalid number of rows");
		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).columns() == rhs.columns(), "Invalid number of columns");

		const size_t N(rhs.rows());

		if (IsDiagonal_v<MT> || N < 2UL) {
			CompositeType_t<MT> tmp(rhs.dm_);
			for (size_t i = 0UL; i < N; ++i) {
				(*lhs)(i, i) += exp(tmp(i, i));
			}
		} else {
			using BT = UnderlyingBuiltin_t<ET>;

			const BT norm(maxNorm(rhs.dm_));

			int exponent(0);
			frexp(norm, &exponent);
			exponent = max(0, exponent);

			ResultType R(rhs.dm_ / pow(2.0, double(exponent)));
			ResultType A(R);
			ResultType B(R);

			for (size_t i = 0UL; i < N; ++i) {
				B(i, i) += BT(1);
			}

			BT factor(1);
			for (size_t k = 2UL; k < K; ++k) {
				factor *= BT(k);
				A *= R;
				addAssign(B, (A / factor));
			}

			for (int i = 0; i < exponent; ++i) {
				B *= B;
			}

			addAssign(*lhs, B);
		}
	}
	/*! \endcond */
	//**********************************************************************************************

	//**Addition assignment to sparse matrices******************************************************
	// No special implementation for the addition assignment to sparse matrices.
	//**********************************************************************************************

	//**Subtraction assignment to dense matrices****************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Subtraction assignment of a dense matrix exponential expression to a dense matrix.
	// \ingroup dense_matrix
	//
	// \param lhs The target left-hand side dense matrix.
	// \param rhs The right-hand side exponential expression to be subtracted.
	// \return void
	//
	// This function implements the performance optimized subtraction assignment of a dense matrix
	// exponential expression to a dense matrix.
	*/
	template <typename MT2 // Type of the target dense matrix
			  ,
			  bool SO2> // Storage order of the target dense matrix
	friend inline void subAssign(DenseMatrix<MT2, SO2> &lhs, const DMatExpExpr &rhs)
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).rows() == rhs.rows(), "Invalid number of rows");
		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).columns() == rhs.columns(), "Invalid number of columns");

		const size_t N(rhs.rows());

		if (IsDiagonal_v<MT> || N < 2UL) {
			CompositeType_t<MT> tmp(rhs.dm_);
			for (size_t i = 0UL; i < N; ++i) {
				(*lhs)(i, i) -= exp(tmp(i, i));
			}
		} else {
			using BT = UnderlyingBuiltin_t<ET>;

			const BT norm(maxNorm(rhs.dm_));

			int exponent(0);
			frexp(norm, &exponent);
			exponent = max(0, exponent);

			ResultType R(rhs.dm_ / pow(2.0, double(exponent)));
			ResultType A(R);
			ResultType B(R);

			for (size_t i = 0UL; i < N; ++i) {
				B(i, i) += BT(1);
			}

			BT factor(1);
			for (size_t k = 2UL; k < K; ++k) {
				factor *= BT(k);
				A *= R;
				addAssign(B, (A / factor));
			}

			for (int i = 0; i < exponent; ++i) {
				B *= B;
			}

			subAssign(*lhs, B);
		}
	}
	/*! \endcond */
	//**********************************************************************************************

	//**Subtraction assignment to sparse matrices***************************************************
	// No special implementation for the subtraction assignment to sparse matrices.
	//**********************************************************************************************

	//**Schur product assignment to dense matrices**************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Schur product assignment of a dense matrix exponential expression to a dense matrix.
	// \ingroup dense_matrix
	//
	// \param lhs The target left-hand side dense matrix.
	// \param rhs The right-hand side exponential expression for the Schur product.
	// \return void
	//
	// This function implements the performance optimized Schur product assignment of a dense
	// matrix exponential expression to a dense matrix.
	*/
	template <typename MT2 // Type of the target dense matrix
			  ,
			  bool SO2> // Storage order of the target dense matrix
	friend inline void schurAssign(DenseMatrix<MT2, SO2> &lhs, const DMatExpExpr &rhs)
	{
		METRIC_NUMERIC_FUNCTION_TRACE;

		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).rows() == rhs.rows(), "Invalid number of rows");
		METRIC_NUMERIC_INTERNAL_ASSERT((*lhs).columns() == rhs.columns(), "Invalid number of columns");

		const size_t N(rhs.rows());

		if (IsDiagonal_v<MT> || N < 2UL) {
			CompositeType_t<MT> tmp(rhs.dm_);
			for (size_t i = 0UL; i < N; ++i) {
				(*lhs)(i, i) *= exp(tmp(i, i));
			}
		} else {
			using BT = UnderlyingBuiltin_t<ET>;

			const BT norm(maxNorm(rhs.dm_));

			int exponent(0);
			frexp(norm, &exponent);
			exponent = max(0, exponent);

			ResultType R(rhs.dm_ / pow(2.0, double(exponent)));
			ResultType A(R);
			ResultType B(R);

			for (size_t i = 0UL; i < N; ++i) {
				B(i, i) += BT(1);
			}

			BT factor(1);
			for (size_t k = 2UL; k < K; ++k) {
				factor *= BT(k);
				A *= R;
				addAssign(B, (A / factor));
			}

			for (int i = 0; i < exponent; ++i) {
				B *= B;
			}

			schurAssign(*lhs, B);
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
/*!\brief Calculation of the exponential of the given dense matrix.
// \ingroup dense_matrix
//
// \param dm The dense matrix for the matrix exponential.
// \return The exponential of the given matrix.
//
// This function returns an expression representing the exponential of the given dense matrix:

				  \f[ e^X = \sum\limits_{k=0}^\infty \frac{1}{k!} X^k \f]

// Example:

   \code
   using mtrc::numeric::rowMajor;

   mtrc::numeric::DynamicMatrix<double,rowMajor> A, B;
   // ... Resizing and initialization
   B = matexp( A );
   \endcode

// \note The matrix exponential can only be used for dense matrices with \c float, \c double,
// \c complex<float> or \c complex<double> element type. The attempt to call the function with
// matrices of any other element type results in a compile time error!
//
// \note It is not possible to use any kind of view on the expression object returned by the
// \c matexp() function. Also, it is not possible to access individual elements via the function
// call operator on the expression object:

   \code
   row( matexp( A ), 2UL );  // Compilation error: Views cannot be used on an matexp() expression!
   matexp( A )(1,2);         // Compilation error: It is not possible to access individual elements!
   \endcode
*/
template <typename MT // Type of the dense matrix
		  ,
		  bool SO> // Storage order
inline decltype(auto) matexp(const DenseMatrix<MT, SO> &dm)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	METRIC_NUMERIC_CONSTRAINT_MUST_BE_BLAS_COMPATIBLE_TYPE(ElementType_t<MT>);

	if (!isSquare(*dm)) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid non-square matrix provided");
	}

	using ReturnType = const DMatExpExpr<MT, SO>;
	return ReturnType(*dm);
}
//*************************************************************************************************

//=================================================================================================
//
//  GLOBAL RESTRUCTURING FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Computation of the determinant of the given dense matrix exponential.
// \ingroup dense_matrix
//
// \param dm The given dense matrix exponential.
// \return The determinant of the given matrix exponential.
//
// This function computes the determinant of the given dense matrix exponential.
//
// \note The computation of the determinant is numerically unreliable since especially for large
// matrices the value can overflow during the computation. Please note that this function does
// not guarantee that it is possible to compute the determinant with the given matrix!
//
// \note This function can only be used if a fitting LAPACK library is available and linked to
// the executable. Otherwise a linker error will be created.
*/
template <typename MT // Type of the dense matrix
		  ,
		  bool SO> // Storage order
inline decltype(auto) det(const DMatExpExpr<MT, SO> &dm)
{
	METRIC_NUMERIC_FUNCTION_TRACE;

	return det(evaluate(*dm));
}
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
