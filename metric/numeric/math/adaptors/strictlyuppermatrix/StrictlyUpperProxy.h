// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_ADAPTORS_STRICTLYUPPERMATRIX_UPPERPROXY_H
#define METRIC_NUMERIC_MATH_ADAPTORS_STRICTLYUPPERMATRIX_UPPERPROXY_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/Aliases.h>
#include <metric/numeric/math/Exception.h>
#include <metric/numeric/math/InitializerList.h>
#include <metric/numeric/math/RelaxationFlag.h>
#include <metric/numeric/math/constraints/Computation.h>
#include <metric/numeric/math/constraints/Hermitian.h>
#include <metric/numeric/math/constraints/Lower.h>
#include <metric/numeric/math/constraints/Matrix.h>
#include <metric/numeric/math/constraints/Symmetric.h>
#include <metric/numeric/math/constraints/Transformation.h>
#include <metric/numeric/math/constraints/Upper.h>
#include <metric/numeric/math/constraints/View.h>
#include <metric/numeric/math/proxy/Proxy.h>
#include <metric/numeric/math/shims/IsDefault.h>
#include <metric/numeric/math/shims/IsOne.h>
#include <metric/numeric/math/shims/IsReal.h>
#include <metric/numeric/math/shims/IsZero.h>
#include <metric/numeric/util/Types.h>
#include <metric/numeric/util/constraints/Const.h>
#include <metric/numeric/util/constraints/Pointer.h>
#include <metric/numeric/util/constraints/Reference.h>
#include <metric/numeric/util/constraints/Volatile.h>
#include <metric/numeric/util/typetraits/AddConst.h>
#include <metric/numeric/util/typetraits/AddLValueReference.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Access proxy for strictly upper triangular matrices.
// \ingroup strictly_upper_matrix
//
// The StrictlyUpperProxy provides controlled access to the elements of a non-const strictly upper
// triangular matrix. It guarantees that the strictly upper matrix invariant is not violated, i.e.
// that elements on the diagonal and in the lower part of the matrix remain 0. The following
// example illustrates this by means of a \f$ 3 \times 3 \f$ dense strictly upper triangular
// matrix:

   \code
   // Creating a 3x3 strictly upper triangular dense matrix
   mtrc::numeric::StrictlyUpperMatrix< mtrc::numeric::DynamicMatrix<int> > A( 3UL );

   A(0,1) = -2;  //        ( 0 -2  3 )
   A(0,2) =  3;  // => A = ( 0  0  5 )
   A(1,2) =  5;  //        ( 0  0  0 )

   A(1,1) =  4;  // Invalid assignment to diagonal matrix element; results in an exception!
   A(2,0) =  7;  // Invalid assignment to lower matrix element; results in an exception!
   \endcode
*/
template <typename MT> // Type of the adapted matrix
class StrictlyUpperProxy : public Proxy<StrictlyUpperProxy<MT>, ElementType_t<MT>> {
  private:
	//**Type definitions****************************************************************************
	//! Reference type of the underlying matrix type.
	using ReferenceType = AddConst_t<Reference_t<MT>>;
	//**********************************************************************************************

  public:
	//**Type definitions****************************************************************************
	using RepresentedType = ElementType_t<MT>;				  //!< Type of the represented matrix element.
	using RawReference = AddLValueReference_t<ReferenceType>; //!< Reference-to-non-const to the represented element.
	using ConstReference = const RepresentedType &;			  //!< Reference-to-const to the represented element.
	//**********************************************************************************************

	//**Constructors********************************************************************************
	/*!\name Constructors */
	//@{
	inline StrictlyUpperProxy(MT &matrix, size_t row, size_t column);

	StrictlyUpperProxy(const StrictlyUpperProxy &) = default;
	//@}
	//**********************************************************************************************

	//**Destructor**********************************************************************************
	/*!\name Destructor */
	//@{
	~StrictlyUpperProxy() = default;
	//@}
	//**********************************************************************************************

	//**Assignment operators************************************************************************
	/*!\name Assignment operators */
	//@{
	inline const StrictlyUpperProxy &operator=(const StrictlyUpperProxy &uup) const;

	template <typename T> inline const StrictlyUpperProxy &operator=(initializer_list<T> list) const;

	template <typename T> inline const StrictlyUpperProxy &operator=(initializer_list<initializer_list<T>> list) const;

	template <typename T> inline const StrictlyUpperProxy &operator=(const T &value) const;
	template <typename T> inline const StrictlyUpperProxy &operator+=(const T &value) const;
	template <typename T> inline const StrictlyUpperProxy &operator-=(const T &value) const;
	template <typename T> inline const StrictlyUpperProxy &operator*=(const T &value) const;
	template <typename T> inline const StrictlyUpperProxy &operator/=(const T &value) const;
	template <typename T> inline const StrictlyUpperProxy &operator%=(const T &value) const;
	//@}
	//**********************************************************************************************

	//**Access operators****************************************************************************
	/*!\name Access operators */
	//@{
	inline const StrictlyUpperProxy *operator->() const noexcept;
	//@}
	//**********************************************************************************************

	//**Utility functions***************************************************************************
	/*!\name Utility functions */
	//@{
	inline RawReference get() const noexcept;
	inline bool isRestricted() const noexcept;
	//@}
	//**********************************************************************************************

	//**Conversion operator*************************************************************************
	/*!\name Conversion operator */
	//@{
	inline operator ConstReference() const noexcept;
	//@}
	//**********************************************************************************************

  private:
	//**Member variables****************************************************************************
	/*!\name Member variables */
	//@{
	ReferenceType value_;	//!< Reference to the accessed matrix element.
	const bool restricted_; //!< Access flag for the accessed matrix element.
							/*!< The flag indicates if access to the matrix element is
								 restricted. It is \a true in case the proxy represents an
								 element on the diagonal or in the lower part of the matrix. */
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
/*!\brief Initialization constructor for a StrictlyUpperProxy.
//
// \param matrix Reference to the adapted matrix.
// \param row The row-index of the accessed matrix element.
// \param column The column-index of the accessed matrix element.
*/
template <typename MT> // Type of the adapted matrix
inline StrictlyUpperProxy<MT>::StrictlyUpperProxy(MT &matrix, size_t row, size_t column)
	: value_(matrix(row, column)) // Reference to the accessed matrix element
	  ,
	  restricted_(column <= row) // Access flag for the accessed matrix element
{
}
//*************************************************************************************************

//=================================================================================================
//
//  OPERATORS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Copy assignment operator for StrictlyUpperProxy.
//
// \param uup Proxy to be copied.
// \return Reference to the assigned proxy.
// \exception std::invalid_argument Invalid assignment to diagonal or lower matrix element.
//
// In case the proxy represents an element on the diagonal or in the lower part of the matrix,
// a \a std::invalid_argument exception is thrown.
*/
template <typename MT> // Type of the adapted matrix
inline const StrictlyUpperProxy<MT> &StrictlyUpperProxy<MT>::operator=(const StrictlyUpperProxy &uup) const
{
	if (restricted_) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid assignment to diagonal or lower matrix element");
	}

	value_ = uup.value_;

	return *this;
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Initializer list assignment to the accessed matrix element.
//
// \param list The list to be assigned to the matrix element.
// \return Reference to the assigned proxy.
// \exception std::invalid_argument Invalid assignment to diagonal or lower matrix element.
//
// In case the proxy represents an element on the diagonal or in the lower part of the matrix,
// a \a std::invalid_argument exception is thrown.
*/
template <typename MT> // Type of the adapted matrix
template <typename T>  // Type of the right-hand side value
inline const StrictlyUpperProxy<MT> &StrictlyUpperProxy<MT>::operator=(initializer_list<T> list) const
{
	if (restricted_) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid assignment to diagonal or lower matrix element");
	}

	value_ = list;

	return *this;
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Initializer list assignment to the accessed matrix element.
//
// \param list The list to be assigned to the matrix element.
// \return Reference to the assigned proxy.
// \exception std::invalid_argument Invalid assignment to diagonal or lower matrix element.
//
// In case the proxy represents an element on the diagonal or in the lower part of the matrix,
// a \a std::invalid_argument exception is thrown.
*/
template <typename MT> // Type of the adapted matrix
template <typename T>  // Type of the right-hand side value
inline const StrictlyUpperProxy<MT> &StrictlyUpperProxy<MT>::operator=(initializer_list<initializer_list<T>> list) const
{
	if (restricted_) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid assignment to diagonal or lower matrix element");
	}

	value_ = list;

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
inline const StrictlyUpperProxy<MT> &StrictlyUpperProxy<MT>::operator=(const T &value) const
{
	if (restricted_) {
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
inline const StrictlyUpperProxy<MT> &StrictlyUpperProxy<MT>::operator+=(const T &value) const
{
	if (restricted_) {
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
inline const StrictlyUpperProxy<MT> &StrictlyUpperProxy<MT>::operator-=(const T &value) const
{
	if (restricted_) {
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
inline const StrictlyUpperProxy<MT> &StrictlyUpperProxy<MT>::operator*=(const T &value) const
{
	if (restricted_) {
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
inline const StrictlyUpperProxy<MT> &StrictlyUpperProxy<MT>::operator/=(const T &value) const
{
	if (restricted_) {
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
inline const StrictlyUpperProxy<MT> &StrictlyUpperProxy<MT>::operator%=(const T &value) const
{
	if (restricted_) {
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
inline const StrictlyUpperProxy<MT> *StrictlyUpperProxy<MT>::operator->() const noexcept
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
/*!\brief Returning the value of the accessed matrix element.
//
// \return Direct/raw reference to the accessed matrix element.
*/
template <typename MT> // Type of the adapted matrix
inline typename StrictlyUpperProxy<MT>::RawReference StrictlyUpperProxy<MT>::get() const noexcept
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
inline bool StrictlyUpperProxy<MT>::isRestricted() const noexcept
{
	return restricted_;
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
// \return Reference-to-const to the accessed matrix element.
*/
template <typename MT> // Type of the adapted matrix
inline StrictlyUpperProxy<MT>::operator ConstReference() const noexcept
{
	return static_cast<ConstReference>(value_);
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
