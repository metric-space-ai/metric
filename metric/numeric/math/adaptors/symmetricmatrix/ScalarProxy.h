// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_ADAPTORS_SYMMETRICMATRIX_SCALARPROXY_H
#define METRIC_NUMERIC_MATH_ADAPTORS_SYMMETRICMATRIX_SCALARPROXY_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/Aliases.h>
#include <metric/numeric/math/RelaxationFlag.h>
#include <metric/numeric/math/constraints/Computation.h>
#include <metric/numeric/math/constraints/Hermitian.h>
#include <metric/numeric/math/constraints/Lower.h>
#include <metric/numeric/math/constraints/Matrix.h>
#include <metric/numeric/math/constraints/Scalar.h>
#include <metric/numeric/math/constraints/Symmetric.h>
#include <metric/numeric/math/constraints/Transformation.h>
#include <metric/numeric/math/constraints/Upper.h>
#include <metric/numeric/math/constraints/View.h>
#include <metric/numeric/math/proxy/Proxy.h>
#include <metric/numeric/math/shims/Clear.h>
#include <metric/numeric/math/shims/Invert.h>
#include <metric/numeric/math/shims/IsDefault.h>
#include <metric/numeric/math/shims/IsOne.h>
#include <metric/numeric/math/shims/IsReal.h>
#include <metric/numeric/math/shims/IsZero.h>
#include <metric/numeric/math/shims/Reset.h>
#include <metric/numeric/util/InvalidType.h>
#include <metric/numeric/util/Types.h>
#include <metric/numeric/util/constraints/Const.h>
#include <metric/numeric/util/constraints/Pointer.h>
#include <metric/numeric/util/constraints/Reference.h>
#include <metric/numeric/util/constraints/Volatile.h>
#include <metric/numeric/util/mpl/If.h>
#include <metric/numeric/util/typetraits/IsComplex.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Access proxy for symmetric, square matrices with scalar element types.
// \ingroup symmetric_matrix
//
// The ScalarProxy provides controlled access to the elements of a non-const symmetric matrix
// with scalar element type (e.g. integral values, floating point values, and complex values).
// It guarantees that a modification of element \f$ a_{ij} \f$ of the accessed matrix is also
// applied to element \f$ a_{ji} \f$. The following example illustrates this by means of a
// \f$ 3 \times 3 \f$ dense symmetric matrix:

   \code
   // Creating a 3x3 symmetric dense matrix
   mtrc::numeric::SymmetricMatrix< mtrc::numeric::DynamicMatrix<int> > A( 3UL );

   A(0,2) = -2;  //        (  0 0 -2 )
   A(1,1) =  3;  // => A = (  0 3  5 )
   A(1,2) =  5;  //        ( -2 5  0 )
   \endcode
*/
template <typename MT> // Type of the adapted matrix
class ScalarProxy : public Proxy<ScalarProxy<MT>> {
  private:
	//**struct BuiltinType**************************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Auxiliary struct to determine the value type of the represented complex element.
	 */
	template <typename T> struct BuiltinType {
		using Type = INVALID_TYPE;
	};
	/*! \endcond */
	//**********************************************************************************************

	//**struct ComplexType**************************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Auxiliary struct to determine the value type of the represented complex element.
	 */
	template <typename T> struct ComplexType {
		using Type = typename T::value_type;
	};
	/*! \endcond */
	//**********************************************************************************************

  public:
	//**Type definitions****************************************************************************
	using RepresentedType = ElementType_t<MT>;	 //!< Type of the represented matrix element.
	using Reference = Reference_t<MT>;			 //!< Reference to the represented element.
	using ConstReference = ConstReference_t<MT>; //!< Reference-to-const to the represented element.
	using Pointer = ScalarProxy *;				 //!< Pointer to the represented element.
	using ConstPointer = const ScalarProxy *;	 //!< Pointer-to-const to the represented element.

	//! Value type of the represented complex element.
	using ValueType =
		typename If_t<IsComplex_v<RepresentedType>, ComplexType<RepresentedType>, BuiltinType<RepresentedType>>::Type;
	//**********************************************************************************************

	//**Constructors********************************************************************************
	/*!\name Constructors */
	//@{
	inline ScalarProxy(MT &matrix, size_t row, size_t column);

	ScalarProxy(const ScalarProxy &) = default;
	//@}
	//**********************************************************************************************

	//**Destructor**********************************************************************************
	/*!\name Destructor */
	//@{
	~ScalarProxy() = default;
	//@}
	//**********************************************************************************************

	//**Assignment operators************************************************************************
	/*!\name Assignment operators */
	//@{
	inline ScalarProxy &operator=(const ScalarProxy &sp);
	template <typename T> inline ScalarProxy &operator=(const T &value);
	template <typename T> inline ScalarProxy &operator+=(const T &value);
	template <typename T> inline ScalarProxy &operator-=(const T &value);
	template <typename T> inline ScalarProxy &operator*=(const T &value);
	template <typename T> inline ScalarProxy &operator/=(const T &value);
	template <typename T> inline ScalarProxy &operator%=(const T &value);
	//@}
	//**********************************************************************************************

	//**Access operators****************************************************************************
	/*!\name Access operators */
	//@{
	inline Pointer operator->();
	inline ConstPointer operator->() const;
	//@}
	//**********************************************************************************************

	//**Utility functions***************************************************************************
	/*!\name Utility functions */
	//@{
	inline void invert() const;

	inline ConstReference get() const noexcept;
	//@}
	//**********************************************************************************************

	//**Conversion operator*************************************************************************
	/*!\name Conversion operator */
	//@{
	inline operator ConstReference() const noexcept;
	//@}
	//**********************************************************************************************

	//**Complex data access functions***************************************************************
	/*!\name Complex data access functions */
	//@{
	inline ValueType real() const;
	inline void real(ValueType value) const;
	inline ValueType imag() const;
	inline void imag(ValueType value) const;
	//@}
	//**********************************************************************************************

  private:
	//**Member variables****************************************************************************
	/*!\name Member variables */
	//@{
	MT &matrix_;	//!< Reference to the adapted matrix.
	size_t row_;	//!< Row index of the accessed matrix element.
	size_t column_; //!< Column index of the accessed matrix element.
	//@}
	//**********************************************************************************************

	//**Compile time checks*************************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_MATRIX_TYPE(MT);
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_REFERENCE_TYPE(MT);
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_POINTER_TYPE(MT);
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_CONST(MT);
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_VOLATILE(MT);
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_VIEW_TYPE(MT);
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_COMPUTATION_TYPE(MT);
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_TRANSFORMATION_TYPE(MT);
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_SYMMETRIC_MATRIX_TYPE(MT);
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_HERMITIAN_MATRIX_TYPE(MT);
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_LOWER_MATRIX_TYPE(MT);
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_UPPER_MATRIX_TYPE(MT);
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_SCALAR_TYPE(RepresentedType);
	/*! \endcond */
	//**********************************************************************************************

	//**********************************************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Resetting the represented element to the default initial values.
	// \ingroup symmetric_matrix
	//
	// \param proxy The given access proxy.
	// \return void
	//
	// This function resets the element represented by the scalar proxy to its default initial
	// value.
	*/
	friend inline void reset(const ScalarProxy &proxy)
	{
		using mtrc::numeric::reset;

		reset(proxy.matrix_(proxy.row_, proxy.column_));
		if (proxy.row_ != proxy.column_) {
			reset(proxy.matrix_(proxy.column_, proxy.row_));
		}
	}
	/*! \endcond */
	//**********************************************************************************************

	//**********************************************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Clearing the represented element.
	// \ingroup symmetric_matrix
	//
	// \param proxy The given access proxy.
	// \return void
	//
	// This function clears the element represented by the scalar proxy to its default initial
	// state.
	*/
	friend inline void clear(const ScalarProxy &proxy)
	{
		using mtrc::numeric::clear;

		clear(proxy.matrix_(proxy.row_, proxy.column_));
		if (proxy.row_ != proxy.column_) {
			clear(proxy.matrix_(proxy.column_, proxy.row_));
		}
	}
	/*! \endcond */
	//**********************************************************************************************
};
//*************************************************************************************************

//=================================================================================================
//
//  CONSTRUCTORS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Initialization constructor for a ScalarProxy.
//
// \param matrix Reference to the adapted matrix.
// \param row The row-index of the accessed matrix element.
// \param column The column-index of the accessed matrix element.
*/
template <typename MT> // Type of the adapted matrix
inline ScalarProxy<MT>::ScalarProxy(MT &matrix, size_t row, size_t column)
	: matrix_(matrix) // Reference to the adapted matrix
	  ,
	  row_(row) // Row index of the accessed matrix element
	  ,
	  column_(column) // Column index of the accessed matrix element
{
}
//*************************************************************************************************

//=================================================================================================
//
//  OPERATORS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Copy assignment operator for ScalarProxy.
//
// \param sp Scalar proxy to be copied.
// \return Reference to the assigned proxy.
*/
template <typename MT> // Type of the adapted matrix
inline ScalarProxy<MT> &ScalarProxy<MT>::operator=(const ScalarProxy &sp)
{
	matrix_(row_, column_) = sp.matrix_(sp.row_, sp.column_);
	matrix_(column_, row_) = sp.matrix_(sp.row_, sp.column_);

	return *this;
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Assignment to the accessed matrix element.
//
// \param value The new value of the matrix element.
// \return Reference to the assigned proxy.
*/
template <typename MT> // Type of the adapted matrix
template <typename T>  // Type of the right-hand side value
inline ScalarProxy<MT> &ScalarProxy<MT>::operator=(const T &value)
{
	matrix_(row_, column_) = value;
	if (row_ != column_)
		matrix_(column_, row_) = value;

	return *this;
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Addition assignment to the accessed matrix element.
//
// \param value The right-hand side value to be added to the matrix element.
// \return Reference to the assigned proxy.
*/
template <typename MT> // Type of the adapted matrix
template <typename T>  // Type of the right-hand side value
inline ScalarProxy<MT> &ScalarProxy<MT>::operator+=(const T &value)
{
	matrix_(row_, column_) += value;
	if (row_ != column_)
		matrix_(column_, row_) += value;

	return *this;
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Subtraction assignment to the accessed matrix element.
//
// \param value The right-hand side value to be subtracted from the matrix element.
// \return Reference to the assigned proxy.
*/
template <typename MT> // Type of the adapted matrix
template <typename T>  // Type of the right-hand side value
inline ScalarProxy<MT> &ScalarProxy<MT>::operator-=(const T &value)
{
	matrix_(row_, column_) -= value;
	if (row_ != column_)
		matrix_(column_, row_) -= value;

	return *this;
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Multiplication assignment to the accessed matrix element.
//
// \param value The right-hand side value for the multiplication.
// \return Reference to the assigned proxy.
*/
template <typename MT> // Type of the adapted matrix
template <typename T>  // Type of the right-hand side value
inline ScalarProxy<MT> &ScalarProxy<MT>::operator*=(const T &value)
{
	matrix_(row_, column_) *= value;
	if (row_ != column_)
		matrix_(column_, row_) *= value;

	return *this;
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Division assignment to the accessed matrix element.
//
// \param value The right-hand side value for the division.
// \return Reference to the assigned proxy.
*/
template <typename MT> // Type of the adapted matrix
template <typename T>  // Type of the right-hand side value
inline ScalarProxy<MT> &ScalarProxy<MT>::operator/=(const T &value)
{
	matrix_(row_, column_) /= value;
	if (row_ != column_)
		matrix_(column_, row_) /= value;

	return *this;
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Modulo assignment to the accessed matrix element.
//
// \param value The right-hand side value for the modulo operation.
// \return Reference to the assigned proxy.
*/
template <typename MT> // Type of the adapted matrix
template <typename T>  // Type of the right-hand side value
inline ScalarProxy<MT> &ScalarProxy<MT>::operator%=(const T &value)
{
	matrix_(row_, column_) %= value;
	if (row_ != column_)
		matrix_(column_, row_) %= value;

	return *this;
}
//*************************************************************************************************

//=================================================================================================
//
//  ACCESS OPERATORS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Direct access to the represented matrix element.
//
// \return Pointer to the represented matrix element.
*/
template <typename MT> // Type of the adapted matrix
inline typename ScalarProxy<MT>::Pointer ScalarProxy<MT>::operator->()
{
	return this;
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Direct access to the represented matrix element.
//
// \return Pointer to the represented matrix element.
*/
template <typename MT> // Type of the adapted matrix
inline typename ScalarProxy<MT>::ConstPointer ScalarProxy<MT>::operator->() const
{
	return this;
}
//*************************************************************************************************

//=================================================================================================
//
//  UTILITY FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief In-place inversion of the represented element
//
// \return void
*/
template <typename MT> // Type of the adapted matrix
inline void ScalarProxy<MT>::invert() const
{
	using mtrc::numeric::invert;

	invert(matrix_(row_, column_));
	if (row_ != column_)
		matrix_(column_, row_) = matrix_(row_, column_);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Returning the value of the accessed matrix element.
//
// \return Direct/raw reference to the accessed matrix element.
*/
template <typename MT> // Type of the adapted matrix
inline typename ScalarProxy<MT>::ConstReference ScalarProxy<MT>::get() const noexcept
{
	return const_cast<const MT &>(matrix_)(row_, column_);
}
//*************************************************************************************************

//=================================================================================================
//
//  CONVERSION OPERATOR
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Conversion to the accessed matrix element.
//
// \return Direct/raw reference to the accessed matrix element.
*/
template <typename MT> // Type of the adapted matrix
inline ScalarProxy<MT>::operator ConstReference() const noexcept
{
	return get();
}
//*************************************************************************************************

//=================================================================================================
//
//  COMPLEX DATA ACCESS FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Returns the real part of the represented complex number.
//
// \return The current real part of the represented complex number.
//
// In case the proxy represents a complex number, this function returns the current value of its
// real part.
*/
template <typename MT> // Type of the adapted matrix
inline typename ScalarProxy<MT>::ValueType ScalarProxy<MT>::real() const
{
	return matrix_(row_, column_).real();
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Setting the real part of the represented complex number.
//
// \param value The new value for the real part.
// \return void
//
// In case the proxy represents a complex number, this function sets a new value to its real part.
*/
template <typename MT> // Type of the adapted matrix
inline void ScalarProxy<MT>::real(ValueType value) const
{
	matrix_(row_, column_).real(value);
	if (row_ != column_)
		matrix_(column_, row_).real(value);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Returns the imaginary part of the represented complex number.
//
// \return The current imaginary part of the represented complex number.
//
// In case the proxy represents a complex number, this function returns the current value of its
// imaginary part.
*/
template <typename MT> // Type of the adapted matrix
inline typename ScalarProxy<MT>::ValueType ScalarProxy<MT>::imag() const
{
	return matrix_(row_, column_).imag();
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Setting the imaginary part of the represented complex number.
//
// \param value The new value for the imaginary part.
// \return void
//
// In case the proxy represents a complex number, this function sets a new value to its imaginary
// part.
*/
template <typename MT> // Type of the adapted matrix
inline void ScalarProxy<MT>::imag(ValueType value) const
{
	matrix_(row_, column_).imag(value);
	if (row_ != column_)
		matrix_(column_, row_).imag(value);
}
//*************************************************************************************************

//=================================================================================================
//
//  GLOBAL FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\name ScalarProxy global functions */
//@{
template <typename MT> void invert(const ScalarProxy<MT> &proxy);
//@}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief In-place inversion of the represented element.
// \ingroup symmetric_matrix
//
// \param proxy The given proxy instance.
// \return void
*/
template <typename MT> inline void invert(const ScalarProxy<MT> &proxy) { proxy.invert(); }
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
