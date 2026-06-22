// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_DVECFIXEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_DVECFIXEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <array>
#include <metric/numeric/math/Exception.h>
#include <metric/numeric/math/InitializerList.h>
#include <metric/numeric/math/constraints/DenseVector.h>
#include <metric/numeric/math/constraints/Expression.h>
#include <metric/numeric/math/constraints/TransposeFlag.h>
#include <metric/numeric/math/expressions/DenseVector.h>
#include <metric/numeric/system/MacroDisable.h>
#include <metric/numeric/util/Types.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DVECFIXEXPR
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Expression object for fixing the size of a dense vector.
// \ingroup dense_vector_expression
//
// The DVecFixExpr class represents the compile time expression for fixing the size of dense
// vectors.
*/
template <typename VT // Type of the dense vector
		  ,
		  bool TF> // Transpose flag
class DVecFixExpr {
  public:
	//**Constructor*********************************************************************************
	/*!\brief Constructor for the DVecTransposer class.
	//
	// \param dv The dense vector operand.
	*/
	explicit inline DVecFixExpr(VT &dv) noexcept : dv_(dv) // The dense vector operand
	{
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief List assignment to all vector elements.
	//
	// \param list The initializer list.
	// \exception std::invalid_argument Invalid assignment to fixed-size vector.
	// \return Reference to the assigned fixed-size vector.
	//
	// This assignment operator offers the option to directly (copy) assign to all elements of the
	// vector by means of an initializer list. In case the size of the given initializer doesn't
	// match the size of this vector, a \a std::invalid_argument exception is thrown.
	*/
	template <typename Type> // Type of the initializer list elements
	DVecFixExpr &operator=(initializer_list<Type> list)
	{
		if (dv_.size() != list.size()) {
			METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid assignment to fixed-size vector");
		}

		dv_ = list;

		return *this;
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Array assignment to all vector elements.
	//
	// \param array Static array for the assignment.
	// \exception std::invalid_argument Invalid assignment to fixed-size vector.
	// \return Reference to the assigned fixed-size vector.
	//
	// This assignment operator offers the option to directly (copy) assign to all elements of the
	// vector by means of a static array. In case the size of the given array doesn't match the
	// size of this vector, a \a std::invalid_argument exception is thrown.
	*/
	template <typename Other // Data type of the static array
			  ,
			  size_t Dim> // Dimension of the static array
	DVecFixExpr &operator=(const Other (&array)[Dim])
	{
		if (dv_.size() != Dim) {
			METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid assignment to fixed-size vector");
		}

		dv_ = array;

		return *this;
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Array assignment to all vector elements.
	//
	// \param array The given std::array for the assignment.
	// \exception std::invalid_argument Invalid assignment to fixed-size vector.
	// \return Reference to the assigned fixed-size vector.
	//
	// This assignment operator offers the option to directly (copy) assign to all elements of the
	// vector by means of a std::array. In case the size of the given array doesn't match the size
	// of this vector, a \a std::invalid_argument exception is thrown.
	*/
	template <typename Other // Data type of the std::array
			  ,
			  size_t Dim> // Dimension of the std::array
	DVecFixExpr &operator=(const std::array<Other, Dim> &array)
	{
		if (dv_.size() != Dim) {
			METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid assignment to fixed-size vector");
		}

		dv_ = array;

		return *this;
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Assignment operator for different vectors.
	//
	// \param rhs Vector to be copied.
	// \exception std::invalid_argument Invalid assignment to fixed-size vector.
	// \return Reference to the assigned fixed-size vector.
	//
	// This assignment operator offers the option to directly (copy) assign to all elements of the
	// vector by means of a vector. In case the size of the given vector doesn't match the size
	// of this vector, a \a std::invalid_argument exception is thrown.
	*/
	template <typename VT2> // Type of the right-hand side vector
	DVecFixExpr &operator=(const Vector<VT2, TF> &rhs)
	{
		if (dv_.size() != (*rhs).size()) {
			METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid assignment to fixed-size vector");
		}

		dv_ = *rhs;

		return *this;
	}
	//**********************************************************************************************

  private:
	//**Member variables****************************************************************************
	VT &dv_; //!< The dense vector operand.
	//**********************************************************************************************

	//**Compile time checks*************************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_DENSE_VECTOR_TYPE(VT);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_VECTOR_WITH_TRANSPOSE_FLAG(VT, TF);
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_EXPRESSION_TYPE(VT);
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
/*!\brief Fixing the size of the given dense vector.
// \ingroup dense_vector
//
// \param dv The dense vector to be size-fixed.
// \return The size-fixed dense vector.
//
// This function returns an expression representing the size-fixed given dense vector:

   \code
   mtrc::numeric::DynamicVector<double> a;
   mtrc::numeric::DynamicVector<double> b;
   // ... Resizing and initialization
   fix( b ) = a;
   \endcode
*/
template <typename VT // Type of the dense vector
		  ,
		  bool TF> // Transpose flag
decltype(auto) fix(DenseVector<VT, TF> &dv) noexcept
{
	return DVecFixExpr<VT, TF>(*dv);
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
