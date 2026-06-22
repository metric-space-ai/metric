// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_UNSIGNEDVALUE_H
#define METRIC_NUMERIC_UTIL_UNSIGNEDVALUE_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <iosfwd>
#include <metric/numeric/util/constraints/Unsigned.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Implementation of a wrapper for built-in unsigned integral values.
// \ingroup util
//
// This class wraps a value of built-in unsigned integral type in order to be able to extract
// non-negative unsigned integral values from an input stream.
*/
template <typename T> // Type of the unsigned value
class UnsignedValue {
  public:
	//**Constructors********************************************************************************
	/*!\name Constructors */
	//@{
	explicit inline UnsignedValue(T value = 0);
	//@}
	//**********************************************************************************************

	//**Assignment operators************************************************************************
	/*!\name Assignment operators */
	//@{
	inline UnsignedValue &operator=(T value);
	//@}
	//**********************************************************************************************

	//**Conversion operators************************************************************************
	/*!\name Conversion operators */
	//@{
	inline operator T() const;
	//@}
	//**********************************************************************************************

	//**Access function*****************************************************************************
	/*!\name Access functions */
	//@{
	inline T get() const;
	//@}
	//**********************************************************************************************

  private:
	//**Member variables****************************************************************************
	/*!\name Member variables */
	//@{
	T value_; //!< The wrapped built-in unsigned integral value.
	//@}
	//**********************************************************************************************

	//**Compile time checks*************************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_UNSIGNED_TYPE(T);
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
/*!\brief The default constructor for UnsignedInt.
//
// \param value The initial value for the unsigned integer.
*/
template <typename T>											// Type of the unsigned value
inline UnsignedValue<T>::UnsignedValue(T value) : value_(value) // The wrapped built-in unsigned integral value
{
}
//*************************************************************************************************

//=================================================================================================
//
//  ASSIGNMENT OPERATOR
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Assignment of a built-in unsigned integral value.
//
// \param value The unsigned integral value.
// \return Reference to the assigned UnsignedValue object.
*/
template <typename T> // Type of the unsigned value
inline UnsignedValue<T> &UnsignedValue<T>::operator=(T value)
{
	value_ = value;
	return *this;
}
//*************************************************************************************************

//=================================================================================================
//
//  CONVERSION OPERATOR
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Conversion to the built-in unsigned integral type.
//
// \return The wrapped built-in unsigned integral value.
*/
template <typename T> // Type of the unsigned value
inline UnsignedValue<T>::operator T() const
{
	return value_;
}
//*************************************************************************************************

//=================================================================================================
//
//  ACCESS FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Access to the wrapped built-in unsigned integral value.
//
// \return The wrapped built-in unsigned integral value.
*/
template <typename T> // Type of the unsigned value
inline T UnsignedValue<T>::get() const
{
	return value_;
}
//*************************************************************************************************

//=================================================================================================
//
//  GLOBAL OPERATORS
//
//=================================================================================================

//*************************************************************************************************
/*!\name UnsignedValue operators */
//@{
template <typename T1, typename T2> inline bool operator==(const UnsignedValue<T1> &lhs, const UnsignedValue<T2> &rhs);

template <typename T1, typename T2> inline bool operator!=(const UnsignedValue<T1> &lhs, const UnsignedValue<T2> &rhs);

template <typename T1, typename T2> inline bool operator<(const UnsignedValue<T1> &lhs, const UnsignedValue<T2> &rhs);

template <typename T1, typename T2> inline bool operator>(const UnsignedValue<T1> &lhs, const UnsignedValue<T2> &rhs);

template <typename T1, typename T2> inline bool operator<=(const UnsignedValue<T1> &lhs, const UnsignedValue<T2> &rhs);

template <typename T1, typename T2> inline bool operator>=(const UnsignedValue<T1> &lhs, const UnsignedValue<T2> &rhs);

template <typename T> inline std::ostream &operator<<(std::ostream &os, const UnsignedValue<T> &uv);

template <typename T> std::istream &operator>>(std::istream &is, UnsignedValue<T> &uv);
//@}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Equality comparison between two UnsignedValue objects.
//
// \param lhs The left-hand side UnsignedValue wrapper.
// \param rhs The right-hand side UnsignedValue wrapper.
// \return \a true if the two values are equal, \a false if not.
*/
template <typename T1 // Type of the left-hand side unsigned value
		  ,
		  typename T2> // Type of the right-hand side unsigned value
inline bool operator==(const UnsignedValue<T1> &lhs, const UnsignedValue<T2> &rhs)
{
	return lhs.get() == rhs.get();
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Inequality comparison between two UnsignedValue objects.
//
// \param lhs The left-hand side UnsignedValue wrapper.
// \param rhs The right-hand side UnsignedValue wrapper.
// \return \a true if the two values are not equal, \a true if they are equal.
*/
template <typename T1 // Type of the left-hand side unsigned value
		  ,
		  typename T2> // Type of the right-hand side unsigned value
inline bool operator!=(const UnsignedValue<T1> &lhs, const UnsignedValue<T2> &rhs)
{
	return lhs.get() != rhs.get();
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Less-than comparison between two UnsignedValue objects.
//
// \param lhs The left-hand side UnsignedValue wrapper.
// \param rhs The right-hand side UnsignedValue wrapper.
// \return \a true if the left value is less than the right value, \a false if not.
*/
template <typename T1 // Type of the left-hand side unsigned value
		  ,
		  typename T2> // Type of the right-hand side unsigned value
inline bool operator<(const UnsignedValue<T1> &lhs, const UnsignedValue<T2> &rhs)
{
	return lhs.get() < rhs.get();
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Greater-than comparison between two UnsignedValue objects.
//
// \param lhs The left-hand side UnsignedValue wrapper.
// \param rhs The right-hand side UnsignedValue wrapper.
// \return \a true if the left value if greater than the right value, \a false if not.
*/
template <typename T1 // Type of the left-hand side unsigned value
		  ,
		  typename T2> // Type of the right-hand side unsigned value
inline bool operator>(const UnsignedValue<T1> &lhs, const UnsignedValue<T2> &rhs)
{
	return lhs.get() > rhs.get();
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Less-or-equal-than comparison between two UnsignedValue objects.
//
// \param lhs The left-hand side UnsignedValue wrapper.
// \param rhs The right-hand side UnsignedValue wrapper.
// \return \a true if the left value is less or equal than the right value, \a false if not.
*/
template <typename T1 // Type of the left-hand side unsigned value
		  ,
		  typename T2> // Type of the right-hand side unsigned value
inline bool operator<=(const UnsignedValue<T1> &lhs, const UnsignedValue<T2> &rhs)
{
	return lhs.get() <= rhs.get();
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Greater-or-equal-than comparison between two UnsignedValue objects.
//
// \param lhs The left-hand side UnsignedValue wrapper.
// \param rhs The right-hand side UnsignedValue wrapper.
// \return \a true if the left value is greater or equal than the right value, \a false if not.
*/
template <typename T1 // Type of the left-hand side unsigned value
		  ,
		  typename T2> // Type of the right-hand side unsigned value
inline bool operator>=(const UnsignedValue<T1> &lhs, const UnsignedValue<T2> &rhs)
{
	return lhs.get() >= rhs.get();
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Global output operator for the UnsignedValue wrapper.
//
// \param os Reference to the output stream.
// \param uv Reference to a UnsignedValue object.
// \return The output stream.
*/
template <typename T> // Type of the unsigned value
inline std::ostream &operator<<(std::ostream &os, const UnsignedValue<T> &uv)
{
	return os << uv.get();
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Global input operator for the UnsignedValue wrapper.
//
// \param is Reference to the input stream.
// \param uv Reference to a UnsignedValue object.
// \return The input stream.
//
// The input operator guarantees that this object is not changed in the case of an input error.
// Only values suitable for the according built-in unsigned integral data type \a T are allowed.
// Otherwise, the input stream's position is returned to its previous position and the
// \a std::istream::failbit is set.
*/
template <typename T> // Type of the unsigned value
std::istream &operator>>(std::istream &is, UnsignedValue<T> &uv)
{
	T tmp;
	const std::istream::pos_type pos(is.tellg());

	// Skipping any leading whitespaces
	is >> std::ws;

	// Extracting the value
	if (is.peek() == '-' || !(is >> tmp)) {
		is.clear();
		is.seekg(pos);
		is.setstate(std::istream::failbit);
		return is;
	}

	// Transfering the input to the unsigned integer value
	uv = tmp;

	return is;
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
