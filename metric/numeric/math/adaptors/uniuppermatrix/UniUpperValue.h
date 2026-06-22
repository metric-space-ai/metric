// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_ADAPTORS_UNIUPPERMATRIX_UNIUPPERVALUE_H
#define METRIC_NUMERIC_MATH_ADAPTORS_UNIUPPERMATRIX_UNIUPPERVALUE_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/Aliases.h>
#include <metric/numeric/math/Exception.h>
#include <metric/numeric/math/RelaxationFlag.h>
#include <metric/numeric/math/constraints/Computation.h>
#include <metric/numeric/math/constraints/Hermitian.h>
#include <metric/numeric/math/constraints/Lower.h>
#include <metric/numeric/math/constraints/Scalar.h>
#include <metric/numeric/math/constraints/SparseMatrix.h>
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
/*!\brief Representation of a value within a sparse upper unitriangular matrix.
// \ingroup uniupper_matrix
//
// The UniUpperValue class represents a single value within a sparse upper unitriangular matrix.
// It guarantees that the uniupper matrix invariant is not violated, i.e. that elements in the
// upper part of the matrix remain 0 and the diagonal elements remain 1. The following example
// illustrates this by means of a \f$ 3 \times 3 \f$ sparse upper unitriangular matrix:

   \code
   using UniUpper = mtrc::numeric::UniUpperMatrix< mtrc::numeric::CompressedMatrix<int> >;

   // Creating a 3x3 upper unitriangular sparse matrix
   UniUpper A( 3UL );

   A(0,1) = -2;  //        ( 1 -2  3 )
   A(0,2) =  3;  // => A = ( 0  1  5 )
   A(1,2) =  5;  //        ( 0  0  1 )

   UniUpper::Iterator it = A.begin( 1UL );
   it->value() = 9;  // Invalid assignment to diagonal matrix element; results in an exception!
   ++it;
   it->value() = 4;  // Modification of matrix element (1,2)
   \endcode
*/
template <typename MT> // Type of the adapted matrix
class UniUpperValue : public Proxy<UniUpperValue<MT>> {
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
	using RepresentedType = ElementType_t<MT>; //!< Type of the represented matrix element.

	//! Value type of the represented complex element.
	using ValueType =
		typename If_t<IsComplex_v<RepresentedType>, ComplexType<RepresentedType>, BuiltinType<RepresentedType>>::Type;

	using value_type = ValueType; //!< Value type of the represented complex element.
	//**********************************************************************************************

	//**Constructors********************************************************************************
	/*!\name Constructors */
	//@{
	inline UniUpperValue(RepresentedType &value, bool diagonal);

	UniUpperValue(const UniUpperValue &) = default;
	//@}
	//**********************************************************************************************

	//**Destructor**********************************************************************************
	/*!\name Destructor */
	//@{
	~UniUpperValue() = default;
	//@}
	//**********************************************************************************************

	//**Assignment operators************************************************************************
	/*!\name Assignment operators */
	//@{
	inline UniUpperValue &operator=(const UniUpperValue &uuv);
	template <typename T> inline UniUpperValue &operator=(const T &value);
	template <typename T> inline UniUpperValue &operator+=(const T &value);
	template <typename T> inline UniUpperValue &operator-=(const T &value);
	template <typename T> inline UniUpperValue &operator*=(const T &value);
	template <typename T> inline UniUpperValue &operator/=(const T &value);
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
	RepresentedType *value_; //!< The represented value.
	bool diagonal_;			 //!< \a true in case the element is on the diagonal, \a false if not.
	//**********************************************************************************************

	//**Compile time checks*************************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_SPARSE_MATRIX_TYPE(MT);
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
	/*!\brief Resetting the uniupper value to the default initial values.
	// \ingroup uniupper_matrix
	//
	// \param value The given uniupper value.
	// \return void
	//
	// This function resets the uniupper value to its default initial value.
	*/
	friend inline void reset(const UniUpperValue &value)
	{
		using mtrc::numeric::reset;

		if (!value.diagonal_) {
			reset(*value.value_);
		}
	}
	/*! \endcond */
	//**********************************************************************************************

	//**********************************************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Clearing the uniupper value.
	// \ingroup uniupper_matrix
	//
	// \param value The given uniupper value.
	// \return void
	//
	// This function clears the uniupper value to its default initial state.
	*/
	friend inline void clear(const UniUpperValue &value)
	{
		using mtrc::numeric::clear;

		if (!value.diagonal_) {
			clear(*value.value_);
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
/*!\brief Constructor for the UniUpperValue class.
//
// \param value Reference to the represented value.
// \param diagonal \a true in case the element is on the diagonal, \a false if not.
*/
template <typename MT> // Type of the adapted matrix
inline UniUpperValue<MT>::UniUpperValue(RepresentedType &value, bool diagonal)
	: value_(&value) // The represented value.
	  ,
	  diagonal_(diagonal) // true in case the element is on the diagonal, false if not
{
}
//*************************************************************************************************

//=================================================================================================
//
//  OPERATORS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Copy assignment operator for UniUpperValue.
//
// \param uuv The uniupper value to be copied.
// \return Reference to the assigned uniupper value.
// \exception std::invalid_argument Invalid assignment to diagonal matrix element.
*/
template <typename MT> // Type of the adapted matrix
inline UniUpperValue<MT> &UniUpperValue<MT>::operator=(const UniUpperValue &uuv)
{
	if (diagonal_) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid assignment to diagonal matrix element");
	}

	*value_ = *uuv.value;

	return *this;
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Assignment to the uniupper value.
//
// \param value The new value of the uniupper value.
// \return Reference to the assigned uniupper value.
// \exception std::invalid_argument Invalid assignment to diagonal matrix element.
*/
template <typename MT> // Type of the adapted matrix
template <typename T>  // Type of the right-hand side value
inline UniUpperValue<MT> &UniUpperValue<MT>::operator=(const T &value)
{
	if (diagonal_) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid assignment to diagonal matrix element");
	}

	*value_ = value;

	return *this;
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Addition assignment to the uniupper value.
//
// \param value The right-hand side value to be added to the uniupper value.
// \return Reference to the assigned uniupper value.
// \exception std::invalid_argument Invalid assignment to diagonal matrix element.
*/
template <typename MT> // Type of the adapted matrix
template <typename T>  // Type of the right-hand side value
inline UniUpperValue<MT> &UniUpperValue<MT>::operator+=(const T &value)
{
	if (diagonal_) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid assignment to diagonal matrix element");
	}

	*value_ += value;

	return *this;
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Subtraction assignment to the uniupper value.
//
// \param value The right-hand side value to be subtracted from the uniupper value.
// \return Reference to the assigned uniupper value.
// \exception std::invalid_argument Invalid assignment to diagonal matrix element.
*/
template <typename MT> // Type of the adapted matrix
template <typename T>  // Type of the right-hand side value
inline UniUpperValue<MT> &UniUpperValue<MT>::operator-=(const T &value)
{
	if (diagonal_) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid assignment to diagonal matrix element");
	}

	*value_ -= value;

	return *this;
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Multiplication assignment to the uniupper value.
//
// \param value The right-hand side value for the multiplication.
// \return Reference to the assigned uniupper value.
// \exception std::invalid_argument Invalid assignment to diagonal matrix element.
*/
template <typename MT> // Type of the adapted matrix
template <typename T>  // Type of the right-hand side value
inline UniUpperValue<MT> &UniUpperValue<MT>::operator*=(const T &value)
{
	if (diagonal_) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid assignment to diagonal matrix element");
	}

	*value_ *= value;

	return *this;
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Division assignment to the uniupper value.
//
// \param value The right-hand side value for the division.
// \return Reference to the assigned uniupper value.
// \exception std::invalid_argument Invalid assignment to diagonal matrix element.
*/
template <typename MT> // Type of the adapted matrix
template <typename T>  // Type of the right-hand side value
inline UniUpperValue<MT> &UniUpperValue<MT>::operator/=(const T &value)
{
	if (diagonal_) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid assignment to diagonal matrix element");
	}

	*value_ /= value;

	return *this;
}
//*************************************************************************************************

//=================================================================================================
//
//  UTILITY FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief In-place inversion of the uniupper value
//
// \return void
*/
template <typename MT> // Type of the adapted matrix
inline void UniUpperValue<MT>::invert() const
{
	using mtrc::numeric::invert;

	if (!diagonal_)
		invert(*value_);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Access to the represented value.
//
// \return Copy of the represented value.
*/
template <typename MT> // Type of the adapted matrix
inline typename UniUpperValue<MT>::RepresentedType UniUpperValue<MT>::get() const noexcept
{
	return *value_;
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Returns whether the value represents a restricted matrix element..
//
// \return \a true in case access to the matrix element is restricted, \a false if not.
*/
template <typename MT> // Type of the adapted matrix
inline bool UniUpperValue<MT>::isRestricted() const noexcept
{
	return diagonal_;
}
//*************************************************************************************************

//=================================================================================================
//
//  CONVERSION OPERATOR
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Conversion to the represented value.
//
// \return Copy of the represented value.
*/
template <typename MT> // Type of the adapted matrix
inline UniUpperValue<MT>::operator RepresentedType() const noexcept
{
	return *value_;
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
inline typename UniUpperValue<MT>::ValueType UniUpperValue<MT>::real() const
{
	return value_->real();
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Setting the real part of the represented complex number.
//
// \param value The new value for the real part.
// \return void
// \exception std::invalid_argument Invalid setting for diagonal matrix element.
//
// In case the proxy represents a complex number, this function sets a new value to its real part.
*/
template <typename MT> // Type of the adapted matrix
inline void UniUpperValue<MT>::real(ValueType value) const
{
	if (isRestricted()) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid setting for diagonal matrix element");
	}

	value_->real(value);
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
inline typename UniUpperValue<MT>::ValueType UniUpperValue<MT>::imag() const
{
	return value_->imag();
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
inline void UniUpperValue<MT>::imag(ValueType value) const
{
	if (isRestricted()) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid setting for diagonal matrix element");
	}

	value_->imag(value);
}
//*************************************************************************************************

//=================================================================================================
//
//  GLOBAL FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\name UniUpperValue global functions */
//@{
template <typename MT> void invert(const UniUpperValue<MT> &value);
//@}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief In-place inversion of the uniupper value.
// \ingroup uniupper_matrix
//
// \param value The given uniupper value.
// \return void
*/
template <typename MT> inline void invert(const UniUpperValue<MT> &value) { value.invert(); }
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
