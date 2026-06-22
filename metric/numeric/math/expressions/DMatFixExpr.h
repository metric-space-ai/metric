// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_DMATFIXEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_DMATFIXEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <array>
#include <metric/numeric/math/Exception.h>
#include <metric/numeric/math/InitializerList.h>
#include <metric/numeric/math/constraints/DenseMatrix.h>
#include <metric/numeric/math/constraints/Expression.h>
#include <metric/numeric/math/constraints/StorageOrder.h>
#include <metric/numeric/math/expressions/DenseMatrix.h>
#include <metric/numeric/system/MacroDisable.h>
#include <metric/numeric/util/Types.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DMATFIXEXPR
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Expression object for fixing the size of a dense matrix.
// \ingroup dense_matrix_expression
//
// The DMatFixExpr class represents the compile time expression for fixing the size of dense
// matrices.
*/
template <typename MT // Type of the dense matrix
		  ,
		  bool SO> // Storage order
class DMatFixExpr {
  public:
	//**Constructor*********************************************************************************
	/*!\brief Constructor for the DMatTransposer class.
	//
	// \param dm The dense matrix operand.
	*/
	explicit inline DMatFixExpr(MT &dm) noexcept : dm_(dm) // The dense matrix operand
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
	DMatFixExpr &operator=(initializer_list<initializer_list<Type>> list)
	{
		if (dm_.rows() != list.size() || dm_.columns() != determineColumns(list)) {
			METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid assignment to fixed-size matrix");
		}

		dm_ = list;

		return *this;
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Array assignment to all matrix elements.
	//
	// \param array Static array for the assignment.
	// \exception std::invalid_argument Invalid assignment to fixed-size matrix.
	// \return Reference to the assigned fixed-size matrix.
	//
	// This assignment operator offers the option to directly (copy) assign to all elements of the
	// matrix by means of a static array. In case the size of the given array doesn't match the
	// size of this matrix, a \a std::invalid_argument exception is thrown.
	*/
	template <typename Other // Data type of the static array
			  ,
			  size_t Rows // Number of rows of the static array
			  ,
			  size_t Cols> // Number of columns of the static array
	DMatFixExpr &operator=(const Other (&array)[Rows][Cols])
	{
		if (dm_.rows() != Rows || dm_.columns() != Cols) {
			METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid assignment to fixed-size matrix");
		}

		dm_ = array;

		return *this;
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Array assignment to all matrix elements.
	//
	// \param array The given std::array for the assignment.
	// \exception std::invalid_argument Invalid assignment to fixed-size matrix.
	// \return Reference to the assigned fixed-size matrix.
	//
	// This assignment operator offers the option to directly (copy) assign to all elements of the
	// matrix by means of a std::array. In case the size of the given array doesn't match the size
	// of this matrix, a \a std::invalid_argument exception is thrown.
	*/
	template <typename Other // Data type of the std::array
			  ,
			  size_t Rows // Number of rows of the std::array
			  ,
			  size_t Cols> // Number of columns of the std::array
	DMatFixExpr &operator=(const std::array<std::array<Other, Cols>, Rows> &array)
	{
		if (dm_.rows() != Rows || dm_.columns() != Cols) {
			METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid assignment to fixed-size matrix");
		}

		dm_ = array;

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
	DMatFixExpr &operator=(const Matrix<MT2, SO2> &rhs)
	{
		if (dm_.rows() != (*rhs).rows() || dm_.columns() != (*rhs).columns()) {
			METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid assignment to fixed-size matrix");
		}

		dm_ = *rhs;

		return *this;
	}
	//**********************************************************************************************

  private:
	//**Member variables****************************************************************************
	MT &dm_; //!< The dense matrix operand.
	//**********************************************************************************************

	//**Compile time checks*************************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_DENSE_MATRIX_TYPE(MT);
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
/*!\brief Fixing the size of the given dense matrix.
// \ingroup dense_matrix
//
// \param dm The dense matrix to be size-fixed.
// \return The size-fixed dense matrix.
//
// This function returns an expression representing the size-fixed given dense matrix:

   \code
   mtrc::numeric::DynamicMatrix<double> A;
   mtrc::numeric::DynamicMatrix<double> B;
   // ... Resizing and initialization
   fix( B ) = A;
   \endcode
*/
template <typename MT // Type of the dense matrix
		  ,
		  bool SO> // Storage order
decltype(auto) fix(DenseMatrix<MT, SO> &dm) noexcept
{
	return DMatFixExpr<MT, SO>(*dm);
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
