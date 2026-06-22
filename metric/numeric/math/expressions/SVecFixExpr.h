// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_SVECFIXEXPR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_SVECFIXEXPR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/Exception.h>
#include <metric/numeric/math/InitializerList.h>
#include <metric/numeric/math/constraints/Expression.h>
#include <metric/numeric/math/constraints/SparseVector.h>
#include <metric/numeric/math/constraints/TransposeFlag.h>
#include <metric/numeric/math/expressions/SparseVector.h>
#include <metric/numeric/system/MacroDisable.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS SVECFIXEXPR
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Expression object for fixing the size of a sparse vector.
// \ingroup sparse_vector_expression
//
// The SVecFixExpr class represents the compile time expression for fixing the size of
// sparse vectors.
*/
template <typename VT // Type of the sparse vector
		  ,
		  bool TF> // Transpose flag
class SVecFixExpr {
  public:
	//**Constructor*********************************************************************************
	/*!\brief Constructor for the SVecTransposer class.
	//
	// \param sv The sparse vector operand.
	*/
	explicit inline SVecFixExpr(VT &sv) noexcept : sv_(sv) // The sparse vector operand
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
	SVecFixExpr &operator=(initializer_list<Type> list)
	{
		if (sv_.size() != list.size()) {
			METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid assignment to fixed-size vector");
		}

		sv_ = list;

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
	SVecFixExpr &operator=(const Vector<VT2, TF> &rhs)
	{
		if (sv_.size() != (*rhs).size()) {
			METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid assignment to fixed-size vector");
		}

		sv_ = *rhs;

		return *this;
	}
	//**********************************************************************************************

  private:
	//**Member variables****************************************************************************
	VT &sv_; //!< The sparse vector operand.
	//**********************************************************************************************

	//**Compile time checks*************************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_SPARSE_VECTOR_TYPE(VT);
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
/*!\brief Fixing the size of the given sparse vector.
// \ingroup sparse_vector
//
// \param sv The sparse vector to be size-fixed.
// \return The size-fixed sparse vector.
//
// This function returns an expression representing the size-fixed given sparse vector:

   \code
   mtrc::numeric::CompressedVector<double> a;
   mtrc::numeric::CompressedVector<double> b;
   // ... Resizing and initialization
   fix( b ) = a;
   \endcode
*/
template <typename VT // Type of the sparse vector
		  ,
		  bool TF> // Transpose flag
decltype(auto) fix(SparseVector<VT, TF> &sv) noexcept
{
	return SVecFixExpr<VT, TF>(*sv);
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
