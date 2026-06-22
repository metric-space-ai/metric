// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_SMATFIXEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_SMATFIXEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/Exception.h>
#include <metric/numeric/math/InitializerList.h>
#include <metric/numeric/math/constraints/Expression.h>
#include <metric/numeric/math/constraints/SparseMatrix.h>
#include <metric/numeric/math/constraints/StorageOrder.h>
#include <metric/numeric/math/expressions/SparseMatrix.h>
#include <metric/numeric/system/MacroDisable.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS SMATFIXEXPR
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Expression object for fixing the size of a sparse matrix.
// \ingroup sparse_matrix_expression
//
// The SMatFixExpr class represents the compile time expression for fixing the size of
// sparse matrices.
*/
template <typename MT // Type of the sparse matrix
		  ,
		  bool SO> // Storage order
class SMatFixExpr {
  public:
	//**Constructor*********************************************************************************
	/*!\brief Constructor for the SMatTransposer class.
	//
	// \param sm The sparse matrix operand.
	*/
	explicit inline SMatFixExpr(MT &sm) noexcept : sm_(sm) // The sparse matrix operand
	{
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief List assignment to all matrix elements.
	//
	// \param list The initializer list.
	// \exception std::invalid_argument Invalid assignment to fixed-size matrix.
	// \return Reference to the assigned fixed-size matrix.
	//
	// This assignment operator offers the option to directly (copy) assign to all elements of the
	// matrix by means of an initializer list. In case the size of the given initializer doesn't
	// match the size of this matrix, a \a std::invalid_argument exception is thrown.
	*/
	template <typename Type> // Type of the initializer list elements
	SMatFixExpr &operator=(initializer_list<initializer_list<Type>> list)
	{
		if (sm_.rows() != list.size() || sm_.columns() != determineColumns(list)) {
			METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid assignment to fixed-size matrix");
		}

		sm_ = list;

		return *this;
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Assignment operator for different matrices.
	//
	// \param rhs Matrix to be copied.
	// \exception std::invalid_argument Invalid assignment to fixed-size matrix.
	// \return Reference to the assigned fixed-size matrix.
	//
	// This assignment operator offers the option to directly (copy) assign to all elements of the
	// matrix by means of a matrix. In case the size of the given matrix doesn't match the size
	// of this matrix, a \a std::invalid_argument exception is thrown.
	*/
	template <typename MT2 // Type of the right-hand side matrix
			  ,
			  bool SO2> // Storage order of the right-hand side matrix
	SMatFixExpr &operator=(const Matrix<MT2, SO2> &rhs)
	{
		if (sm_.rows() != (*rhs).rows() || sm_.columns() != (*rhs).columns()) {
			METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid assignment to fixed-size matrix");
		}

		sm_ = *rhs;

		return *this;
	}
	//**********************************************************************************************

  private:
	//**Member variables****************************************************************************
	MT &sm_; //!< The sparse matrix operand.
	//**********************************************************************************************

	//**Compile time checks*************************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_SPARSE_MATRIX_TYPE(MT);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_MATRIX_WITH_STORAGE_ORDER(MT, SO);
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_EXPRESSION_TYPE(MT);
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
/*!\brief Fixing the size of the given sparse matrix.
// \ingroup sparse_matrix
//
// \param sm The sparse matrix to be size-fixed.
// \return The size-fixed sparse matrix.
//
// This function returns an expression representing the size-fixed given sparse matrix:

   \code
   mtrc::numeric::CompressedMatrix<double> A;
   mtrc::numeric::CompressedMatrix<double> B;
   // ... Resizing and initialization
   fix( B ) = A;
   \endcode
*/
template <typename MT // Type of the sparse matrix
		  ,
		  bool SO> // Storage order
decltype(auto) fix(SparseMatrix<MT, SO> &sm) noexcept
{
	return SMatFixExpr<MT, SO>(*sm);
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
