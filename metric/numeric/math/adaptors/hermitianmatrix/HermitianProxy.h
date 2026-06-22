// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_ADAPTORS_HERMITIANMATRIX_HERMITIANPROXY_H
#define METRIC_NUMERIC_MATH_ADAPTORS_HERMITIANMATRIX_HERMITIANPROXY_H
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
#include <metric/numeric/math/shims/Conjugate.h>
#include <metric/numeric/math/shims/Invert.h>
#include <metric/numeric/math/shims/IsDefault.h>
#include <metric/numeric/math/shims/IsOne.h>
#include <metric/numeric/math/shims/IsReal.h>
#include <metric/numeric/math/shims/IsZero.h>
#include <metric/numeric/math/shims/Reset.h>
#include <metric/numeric/math/typetraits/UnderlyingBuiltin.h>
#include <metric/numeric/util/Types.h>
#include <metric/numeric/util/constraints/Const.h>
#include <metric/numeric/util/constraints/Pointer.h>
#include <metric/numeric/util/constraints/Reference.h>
#include <metric/numeric/util/constraints/Volatile.h>
#include <metric/numeric/util/typetraits/IsComplex.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Access proxy for Hermitian matrices.
// \ingroup hermitian_matrix
//
// The HermitianProxy provides controlled access to the elements of a non-const Hermitian matrix.
// It guarantees that a modification of element \f$ a_{ij} \f$ of the accessed matrix is also
// applied to element \f$ a_{ji} \f$. The following example illustrates this by means of a
// \f$ 3 \times 3 \f$ dense Hermitian matrix:

   \code
   // Creating a 3x3 Hermitian dense matrix
   mtrc::numeric::HermitianMatrix< mtrc::numeric::DynamicMatrix<int> > A( 3UL );

   A(0,2) = -2;  //        (  0 0 -2 )
   A(1,1) =  3;  // => A = (  0 3  5 )
   A(1,2) =  5;  //        ( -2 5  0 )
   \endcode
*/
template <typename MT> // Type of the adapted matrix
class HermitianProxy : public Proxy<HermitianProxy<MT>> {
  public:
	//**Type definitions****************************************************************************
	using RepresentedType = ElementType_t<MT>;	 //!< Type of the represented matrix element.
	using Reference = Reference_t<MT>;			 //!< Reference to the represented element.
	using ConstReference = ConstReference_t<MT>; //!< Reference-to-const to the represented element.
	using Pointer = HermitianProxy *;			 //!< Pointer to the represented element.
	using ConstPointer = const HermitianProxy *; //!< Pointer-to-const to the represented element.

	//! Value type of the represented complex element.
	using ValueType = UnderlyingBuiltin_t<RepresentedType>;

	//! Value type of the represented complex element.
	using value_type = ValueType;
	//**********************************************************************************************

	//**Constructors********************************************************************************
	/*!\name Constructors */
	//@{
	inline HermitianProxy(MT &matrix, size_t row, size_t column);

	HermitianProxy(const HermitianProxy &) = default;
	//@}
	//**********************************************************************************************

	//**Destructor**********************************************************************************
	/*!\name Destructor */
	//@{
	~HermitianProxy() = default;
	//@}
	//**********************************************************************************************

	//**Assignment operators************************************************************************
	/*!\name Assignment operators */
	//@{
	inline HermitianProxy &operator=(const HermitianProxy &hp);
	template <typename T> inline HermitianProxy &operator=(const T &value);
	template <typename T> inline HermitianProxy &operator+=(const T &value);
	template <typename T> inline HermitianProxy &operator-=(const T &value);
	template <typename T> inline HermitianProxy &operator*=(const T &value);
	template <typename T> inline HermitianProxy &operator/=(const T &value);
	template <typename T> inline HermitianProxy &operator%=(const T &value);
	//@}
	//**********************************************************************************************

	//**Access operators****************************************************************************
	/*!\name Access operators */
	//@{
	inline Pointer operator->() noexcept;
	inline ConstPointer operator->() const noexcept;
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
	Reference value1_;	  //!< Reference to the first accessed matrix element.
	Reference value2_;	  //!< Reference to the second accessed matrix element.
	const bool diagonal_; //!< Flag for the accessed matrix element.
						  /*!< The flag indicates if the accessed element is a diagonal element.
							   It is \a true in case the proxy represents an element on the
							   diagonal. */
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
	// \ingroup hermitian_matrix
	//
	// \param proxy The given access proxy.
	// \return void
	//
	// This function resets the element represented by the access proxy to its default initial
	// value.
	*/
	friend inline void reset(const HermitianProxy &proxy)
	{
		using mtrc::numeric::reset;

		reset(proxy.value1_);
		if (!proxy.diagonal_) {
			reset(proxy.value2_);
		}
	}
	/*! \endcond */
	//**********************************************************************************************

	//**********************************************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Clearing the represented element.
	//
	// \return void
	//
	// This function clears the element represented by the proxy to its default initial state.
	*/
	friend inline void clear(const HermitianProxy &proxy)
	{
		using mtrc::numeric::clear;

		clear(proxy.value1_);
		if (!proxy.diagonal_) {
			clear(proxy.value2_);
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
/*!\brief Initialization constructor for a HermitianProxy.
//
// \param matrix Reference to the adapted matrix.
// \param row The row-index of the accessed matrix element.
// \param column The column-index of the accessed matrix element.
*/
template <typename MT> // Type of the adapted matrix
inline HermitianProxy<MT>::HermitianProxy(MT &matrix, size_t row, size_t column)
	: value1_(matrix(row, column)) // Reference to the first accessed matrix element
	  ,
	  value2_(matrix(column, row)) // Reference to the second accessed matrix element
	  ,
	  diagonal_(row == column) // Flag for the accessed matrix element
{
}
//*************************************************************************************************

//=================================================================================================
//
//  OPERATORS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Copy assignment operator for HermitianProxy.
//
// \param hp Hermitian proxy to be copied.
// \return Reference to the assigned proxy.
// \exception std::invalid_argument Invalid assignment to diagonal matrix element.
//
// In case the proxy represents a diagonal element and the assigned value does not represent
// a real number, a \a std::invalid_argument exception is thrown.
*/
template <typename MT> // Type of the adapted matrix
inline HermitianProxy<MT> &HermitianProxy<MT>::operator=(const HermitianProxy &hp)
{
	using ET = ElementType_t<MT>;

	if (IsComplex_v<ET> && diagonal_ && !isReal(hp.value1_)) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid assignment to diagonal matrix element");
	}

	value1_ = hp.value1_;
	if (!diagonal_)
		value2_ = conj(value1_);

	return *this;
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Assignment to the accessed matrix element.
//
// \param value The new value of the matrix element.
// \return Reference to the assigned proxy.
// \exception std::invalid_argument Invalid assignment to diagonal matrix element.
//
// In case the proxy represents a diagonal element and the assigned value does not represent
// a real number, a \a std::invalid_argument exception is thrown.
*/
template <typename MT> // Type of the adapted matrix
template <typename T>  // Type of the right-hand side value
inline HermitianProxy<MT> &HermitianProxy<MT>::operator=(const T &value)
{
	using ET = ElementType_t<MT>;

	if (IsComplex_v<ET> && diagonal_ && !isReal(value)) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid assignment to diagonal matrix element");
	}

	value1_ = value;
	if (!diagonal_)
		value2_ = conj(value1_);

	return *this;
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Addition assignment to the accessed matrix element.
//
// \param value The right-hand side value to be added to the matrix element.
// \return Reference to the assigned proxy.
// \exception std::invalid_argument Invalid assignment to diagonal matrix element.
//
// In case the proxy represents a diagonal element and the assigned value does not represent
// a real number, a \a std::invalid_argument exception is thrown.
*/
template <typename MT> // Type of the adapted matrix
template <typename T>  // Type of the right-hand side value
inline HermitianProxy<MT> &HermitianProxy<MT>::operator+=(const T &value)
{
	using ET = ElementType_t<MT>;

	if (IsComplex_v<ET> && diagonal_ && !isReal(value)) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid assignment to diagonal matrix element");
	}

	value1_ += value;
	if (!diagonal_)
		value2_ = conj(value1_);

	return *this;
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Subtraction assignment to the accessed matrix element.
//
// \param value The right-hand side value to be subtracted from the matrix element.
// \return Reference to the assigned proxy.
// \exception std::invalid_argument Invalid assignment to diagonal matrix element.
//
// In case the proxy represents a diagonal element and the assigned value does not represent
// a real number, a \a std::invalid_argument exception is thrown.
*/
template <typename MT> // Type of the adapted matrix
template <typename T>  // Type of the right-hand side value
inline HermitianProxy<MT> &HermitianProxy<MT>::operator-=(const T &value)
{
	using ET = ElementType_t<MT>;

	if (IsComplex_v<ET> && diagonal_ && !isReal(value)) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid assignment to diagonal matrix element");
	}

	value1_ -= value;
	if (!diagonal_)
		value2_ = conj(value1_);

	return *this;
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Multiplication assignment to the accessed matrix element.
//
// \param value The right-hand side value for the multiplication.
// \return Reference to the assigned proxy.
// \exception std::invalid_argument Invalid assignment to diagonal matrix element.
//
// In case the proxy represents a diagonal element and the assigned value does not represent
// a real number, a \a std::invalid_argument exception is thrown.
*/
template <typename MT> // Type of the adapted matrix
template <typename T>  // Type of the right-hand side value
inline HermitianProxy<MT> &HermitianProxy<MT>::operator*=(const T &value)
{
	using ET = ElementType_t<MT>;

	if (IsComplex_v<ET> && diagonal_ && !isReal(value)) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid assignment to diagonal matrix element");
	}

	value1_ *= value;
	if (!diagonal_)
		value2_ = conj(value1_);

	return *this;
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Division assignment to the accessed matrix element.
//
// \param value The right-hand side value for the division.
// \return Reference to the assigned proxy.
// \exception std::invalid_argument Invalid assignment to diagonal matrix element.
//
// In case the proxy represents a diagonal element and the assigned value does not represent
// a real number, a \a std::invalid_argument exception is thrown.
*/
template <typename MT> // Type of the adapted matrix
template <typename T>  // Type of the right-hand side value
inline HermitianProxy<MT> &HermitianProxy<MT>::operator/=(const T &value)
{
	using ET = ElementType_t<MT>;

	if (IsComplex_v<ET> && diagonal_ && !isReal(value)) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid assignment to diagonal matrix element");
	}

	value1_ /= value;
	if (!diagonal_)
		value2_ = conj(value1_);

	return *this;
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Modulo assignment to the accessed matrix element.
//
// \param value The right-hand side value for the modulo operation.
// \return Reference to the assigned proxy.
// \exception std::invalid_argument Invalid assignment to diagonal matrix element.
//
// In case the proxy represents a diagonal element and the assigned value does not represent
// a real number, a \a std::invalid_argument exception is thrown.
*/
template <typename MT> // Type of the adapted matrix
template <typename T>  // Type of the right-hand side value
inline HermitianProxy<MT> &HermitianProxy<MT>::operator%=(const T &value)
{
	using ET = ElementType_t<MT>;

	if (IsComplex_v<ET> && diagonal_ && !isReal(value)) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid assignment to diagonal matrix element");
	}

	value1_ %= value;
	if (!diagonal_)
		value2_ = conj(value1_);

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
inline typename HermitianProxy<MT>::Pointer HermitianProxy<MT>::operator->() noexcept
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
inline typename HermitianProxy<MT>::ConstPointer HermitianProxy<MT>::operator->() const noexcept
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
inline void HermitianProxy<MT>::invert() const
{
	using mtrc::numeric::invert;

	invert(value1_);
	if (!diagonal_)
		value2_ = conj(value1_);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Returning the value of the accessed matrix element.
//
// \return Direct/raw reference to the accessed matrix element.
*/
template <typename MT> // Type of the adapted matrix
inline typename HermitianProxy<MT>::ConstReference HermitianProxy<MT>::get() const noexcept
{
	return value1_;
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
inline HermitianProxy<MT>::operator ConstReference() const noexcept
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
inline typename HermitianProxy<MT>::ValueType HermitianProxy<MT>::real() const
{
	return value1_.real();
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
inline void HermitianProxy<MT>::real(ValueType value) const
{
	value1_.real(value);
	if (!diagonal_)
		value2_.real(value);
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
inline typename HermitianProxy<MT>::ValueType HermitianProxy<MT>::imag() const
{
	return value1_.imag();
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Setting the imaginary part of the represented complex number.
//
// \param value The new value for the imaginary part.
// \return void
// \exception std::invalid_argument Invalid setting for diagonal matrix element.
//
// In case the proxy represents a complex number, this function sets a new value to its imaginary
// part. In case the proxy represents a diagonal element and the given value is not zero, a
// \a std::invalid_argument exception is thrown.
*/
template <typename MT> // Type of the adapted matrix
inline void HermitianProxy<MT>::imag(ValueType value) const
{
	if (diagonal_ && !isZero(value)) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid setting for diagonal matrix element");
	}

	value1_.imag(value);
	if (!diagonal_)
		value2_.imag(-value);
}
//*************************************************************************************************

//=================================================================================================
//
//  GLOBAL FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\name HermitianProxy global functions */
//@{
template <typename MT> void invert(const HermitianProxy<MT> &proxy);
//@}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief In-place inversion of the represented element.
// \ingroup hermitian_matrix
//
// \param proxy The given proxy instance.
// \return void
*/
template <typename MT> inline void invert(const HermitianProxy<MT> &proxy) { proxy.invert(); }
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
