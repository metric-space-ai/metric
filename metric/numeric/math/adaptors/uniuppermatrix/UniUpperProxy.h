// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_ADAPTORS_UNIUPPERMATRIX_UPPERPROXY_H
#define METRIC_NUMERIC_MATH_ADAPTORS_UNIUPPERMATRIX_UPPERPROXY_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/Aliases.h>
#include <metric/numeric/math/Exception.h>
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
/*!\brief Access proxy for upper unitriangular matrices.
// \ingroup uniupper_matrix
//
// The UniUpperProxy provides controlled access to the elements of a non-const upper unitriangular
// matrix. It guarantees that the uniupper matrix invariant is not violated, i.e. that elements
// in the lower part of the matrix remain 0 and the diagonal elements remain 1. The following
// example illustrates this by means of a \f$ 3 \times 3 \f$ dense upper unitriangular matrix:

   \code
   // Creating a 3x3 upper unitriangular dense matrix
   mtrc::numeric::UniUpperMatrix< mtrc::numeric::DynamicMatrix<int> > A( 3UL );

   A(0,1) = -2;  //        ( 1 -2  3 )
   A(0,2) =  3;  // => A = ( 0  1  5 )
   A(1,2) =  5;  //        ( 0  0  1 )

   A(1,1) =  4;  // Invalid assignment to diagonal matrix element; results in an exception!
   A(2,0) =  7;  // Invalid assignment to lower matrix element; results in an exception!
   \endcode
*/
template <typename MT> // Type of the adapted matrix
class UniUpperProxy : public Proxy<UniUpperProxy<MT>> {
  private:
	//**Type definitions****************************************************************************
	//! Reference type of the underlying matrix type.
	using ReferenceType = typename MT::Reference;
	//**********************************************************************************************

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
	//! Type of the represented matrix element.
	using RepresentedType = ElementType_t<MT>;

	//! Value type of the represented complex element.
	using ValueType =
		typename If_t<IsComplex_v<RepresentedType>, ComplexType<RepresentedType>, BuiltinType<RepresentedType>>::Type;

	using value_type = ValueType; //!< Value type of the represented complex element.
	//**********************************************************************************************

	//**Constructors********************************************************************************
	/*!\name Constructors */
	//@{
	inline UniUpperProxy(MT &matrix, size_t row, size_t column);

	UniUpperProxy(const UniUpperProxy &) = default;
	//@}
	//**********************************************************************************************

	//**Destructor**********************************************************************************
	/*!\name Destructor */
	//@{
	~UniUpperProxy() = default;
	//@}
	//**********************************************************************************************

	//**Assignment operators************************************************************************
	/*!\name Assignment operators */
	//@{
	inline const UniUpperProxy &operator=(const UniUpperProxy &uup) const;
	template <typename T> inline const UniUpperProxy &operator=(const T &value) const;
	template <typename T> inline const UniUpperProxy &operator+=(const T &value) const;
	template <typename T> inline const UniUpperProxy &operator-=(const T &value) const;
	template <typename T> inline const UniUpperProxy &operator*=(const T &value) const;
	template <typename T> inline const UniUpperProxy &operator/=(const T &value) const;
	template <typename T> inline const UniUpperProxy &operator%=(const T &value) const;
	//@}
	//**********************************************************************************************

	//**Access operators****************************************************************************
	/*!\name Access operators */
	//@{
	inline const UniUpperProxy *operator->() const noexcept;
	//@}
	//**********************************************************************************************

	//**Utility functions***************************************************************************
	/*!\name Utility functions */
	//@{
	inline void invert() const;

	inline RepresentedType get() const noexcept;
	inline bool isRestricted() const noexcept;
	//@}
	//**********************************************************************************************

	//**Conversion operator*************************************************************************
	/*!\name Conversion operator */
	//@{
	inline operator RepresentedType() const noexcept;
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
	ReferenceType value_; //!< Reference to the accessed matrix element.
	size_t row_;		  //!< Row index of the accessed matrix element.
	size_t column_;		  //!< Column index of the accessed matrix element.
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
	// \ingroup uniupper_matrix
	//
	// \param proxy The given access proxy.
	// \return void
	//
	// This function resets the element represented by the access proxy to its default initial
	// value.
	*/
	friend inline void reset(const UniUpperProxy &proxy)
	{
		using mtrc::numeric::reset;

		if (proxy.row_ < proxy.column_) {
			reset(proxy.value_);
		}
	}
	/*! \endcond */
	//**********************************************************************************************

	//**********************************************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Clearing the represented element.
	// \ingroup uniupper_matrix
	//
	// \param proxy The given access proxy.
	// \return void
	//
	// This function clears the element represented by the access proxy to its default initial
	// state.
	*/
	friend inline void clear(const UniUpperProxy &proxy)
	{
		using mtrc::numeric::clear;

		if (proxy.row_ < proxy.column_) {
			clear(proxy.value_);
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
/*!\brief Initialization constructor for a UniUpperProxy.
//
// \param matrix Reference to the adapted matrix.
// \param row The row-index of the accessed matrix element.
// \param column The column-index of the accessed matrix element.
*/
template <typename MT> // Type of the adapted matrix
inline UniUpperProxy<MT>::UniUpperProxy(MT &matrix, size_t row, size_t column)
	: value_(matrix(row, column)) // Reference to the accessed matrix element
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
/*!\brief Copy assignment operator for UniUpperProxy.
//
// \param uup Proxy to be copied.
// \return Reference to the assigned proxy.
// \exception std::invalid_argument Invalid assignment to diagonal or lower matrix element.
//
// In case the proxy represents an element on the diagonal or in the lower part of the matrix,
// a \a std::invalid_argument exception is thrown.
*/
template <typename MT> // Type of the adapted matrix
inline const UniUpperProxy<MT> &UniUpperProxy<MT>::operator=(const UniUpperProxy &uup) const
{
	if (isRestricted()) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid assignment to diagonal or lower matrix element");
	}

	value_ = uup.value_;

	return *this;
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Assignment to the accessed matrix element.
//
// \param value The new value of the matrix element.
// \return Reference to the assigned proxy.
// \exception std::invalid_argument Invalid assignment to diagonal or lower matrix element.
//
// In case the proxy represents an element on the diagonal or in the lower part of the matrix,
// a \a std::invalid_argument exception is thrown.
*/
template <typename MT> // Type of the adapted matrix
template <typename T>  // Type of the right-hand side value
inline const UniUpperProxy<MT> &UniUpperProxy<MT>::operator=(const T &value) const
{
	if (isRestricted()) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid assignment to diagonal or lower matrix element");
	}

	value_ = value;

	return *this;
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Addition assignment to the accessed matrix element.
//
// \param value The right-hand side value to be added to the matrix element.
// \return Reference to the assigned proxy.
// \exception std::invalid_argument Invalid assignment to diagonal or lower matrix element.
//
// In case the proxy represents an element on the diagonal or in the lower part of the matrix,
// a \a std::invalid_argument exception is thrown.
*/
template <typename MT> // Type of the adapted matrix
template <typename T>  // Type of the right-hand side value
inline const UniUpperProxy<MT> &UniUpperProxy<MT>::operator+=(const T &value) const
{
	if (isRestricted()) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid assignment to diagonal or lower matrix element");
	}

	value_ += value;

	return *this;
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Subtraction assignment to the accessed matrix element.
//
// \param value The right-hand side value to be subtracted from the matrix element.
// \return Reference to the assigned proxy.
// \exception std::invalid_argument Invalid assignment to diagonal or lower matrix element.
//
// In case the proxy represents an element on the diagonal or in the lower part of the matrix,
// a \a std::invalid_argument exception is thrown.
*/
template <typename MT> // Type of the adapted matrix
template <typename T>  // Type of the right-hand side value
inline const UniUpperProxy<MT> &UniUpperProxy<MT>::operator-=(const T &value) const
{
	if (isRestricted()) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid assignment to diagonal or lower matrix element");
	}

	value_ -= value;

	return *this;
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Multiplication assignment to the accessed matrix element.
//
// \param value The right-hand side value for the multiplication.
// \return Reference to the assigned proxy.
// \exception std::invalid_argument Invalid assignment to diagonal or lower matrix element.
//
// In case the proxy represents an element on the diagonal or in the lower part of the matrix,
// a \a std::invalid_argument exception is thrown.
*/
template <typename MT> // Type of the adapted matrix
template <typename T>  // Type of the right-hand side value
inline const UniUpperProxy<MT> &UniUpperProxy<MT>::operator*=(const T &value) const
{
	if (isRestricted()) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid assignment to diagonal or lower matrix element");
	}

	value_ *= value;

	return *this;
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Division assignment to the accessed matrix element.
//
// \param value The right-hand side value for the division.
// \return Reference to the assigned proxy.
// \exception std::invalid_argument Invalid assignment to diagonal or lower matrix element.
//
// In case the proxy represents an element on the diagonal or in the lower part of the matrix,
// a \a std::invalid_argument exception is thrown.
*/
template <typename MT> // Type of the adapted matrix
template <typename T>  // Type of the right-hand side value
inline const UniUpperProxy<MT> &UniUpperProxy<MT>::operator/=(const T &value) const
{
	if (isRestricted()) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid assignment to diagonal or lower matrix element");
	}

	value_ /= value;

	return *this;
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Modulo assignment to the accessed matrix element.
//
// \param value The right-hand side value for the modulo operation.
// \return Reference to the assigned proxy.
// \exception std::invalid_argument Invalid assignment to diagonal or lower matrix element.
//
// In case the proxy represents an element on the diagonal or in the lower part of the matrix,
// a \a std::invalid_argument exception is thrown.
*/
template <typename MT> // Type of the adapted matrix
template <typename T>  // Type of the right-hand side value
inline const UniUpperProxy<MT> &UniUpperProxy<MT>::operator%=(const T &value) const
{
	if (isRestricted()) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid assignment to diagonal or lower matrix element");
	}

	value_ %= value;

	return *this;
}
//*************************************************************************************************

//=================================================================================================
//
//  ACCESS OPERATORS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Direct access to the accessed matrix element.
//
// \return Pointer to the accessed matrix element.
*/
template <typename MT> // Type of the adapted matrix
inline const UniUpperProxy<MT> *UniUpperProxy<MT>::operator->() const noexcept
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
// \exception std::invalid_argument Invalid inversion of upper matrix element.
//
// In case the proxy represents a lower element, a \a std::invalid_argument exception is thrown.
*/
template <typename MT> // Type of the adapted matrix
inline void UniUpperProxy<MT>::invert() const
{
	using mtrc::numeric::invert;

	if (column_ < row_) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid inversion of lower matrix element");
	}

	if (row_ < column_)
		invert(value_);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Returning the value of the accessed matrix element.
//
// \return Direct/raw reference to the accessed matrix element.
*/
template <typename MT> // Type of the adapted matrix
inline typename UniUpperProxy<MT>::RepresentedType UniUpperProxy<MT>::get() const noexcept
{
	return value_;
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Returns whether the proxy represents a restricted matrix element..
//
// \return \a true in case access to the matrix element is restricted, \a false if not.
*/
template <typename MT> // Type of the adapted matrix
inline bool UniUpperProxy<MT>::isRestricted() const noexcept
{
	return column_ <= row_;
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
inline UniUpperProxy<MT>::operator RepresentedType() const noexcept
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
inline typename UniUpperProxy<MT>::ValueType UniUpperProxy<MT>::real() const
{
	return value_.real();
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Setting the real part of the represented complex number.
//
// \param value The new value for the real part.
// \return void
// \exception std::invalid_argument Invalid setting for diagonal or lower matrix element.
//
// In case the proxy represents a complex number, this function sets a new value to its real part.
// In case the represented value is a diagonal element or an element in the lower part of the
// matrix, a \a std::invalid_argument exception is thrown.
*/
template <typename MT> // Type of the adapted matrix
inline void UniUpperProxy<MT>::real(ValueType value) const
{
	if (isRestricted()) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid setting for diagonal or lower matrix element");
	}

	value_.real(value);
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
inline typename UniUpperProxy<MT>::ValueType UniUpperProxy<MT>::imag() const
{
	return value_.imag();
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Setting the imaginary part of the represented complex number.
//
// \param value The new value for the imaginary part.
// \return void
// \exception std::invalid_argument Invalid setting for diagonal or lower matrix element.
//
// In case the proxy represents a complex number, this function sets a new value to its imaginary
// part. In case the represented value is a diagonal element or an element in the lower part of
// the matrix, a \a std::invalid_argument exception is thrown.
*/
template <typename MT> // Type of the adapted matrix
inline void UniUpperProxy<MT>::imag(ValueType value) const
{
	if (isRestricted()) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid setting for diagonal or lower matrix element");
	}

	value_.imag(value);
}
//*************************************************************************************************

//=================================================================================================
//
//  GLOBAL FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\name UniUpperProxy global functions */
//@{
template <typename MT> void invert(const UniUpperProxy<MT> &proxy);
//@}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief In-place inversion of the represented element.
// \ingroup uniupper_matrix
//
// \param proxy The given access proxy.
// \return void
*/
template <typename MT> inline void invert(const UniUpperProxy<MT> &proxy) { proxy.invert(); }
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
