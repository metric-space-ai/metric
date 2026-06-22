// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_ADAPTORS_SYMMETRICMATRIX_SHAREDVALUE_H
#define METRIC_NUMERIC_MATH_ADAPTORS_SYMMETRICMATRIX_SHAREDVALUE_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <memory>
#include <metric/numeric/math/RelaxationFlag.h>
#include <metric/numeric/util/constraints/Const.h>
#include <metric/numeric/util/constraints/Pointer.h>
#include <metric/numeric/util/constraints/Reference.h>
#include <metric/numeric/util/constraints/Volatile.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Value shared among several positions within a symmetric matrix.
// \ingroup symmetric_matrix
//
// The SharedValue class template represents a single value of a symmetric matrix that is shared
// among several positions within the symmetric matrix. Changes to the value of one position
// are therefore applied to all positions sharing the same value.
*/
template <typename Type> // Type of the shared value
class SharedValue {
  public:
	//**Type definitions****************************************************************************
	using ValueType = Type;				 //!< Type of the shared value.
	using Reference = Type &;			 //!< Reference to the shared value.
	using ConstReference = const Type &; //!< Reference-to-const to the shared value.
	using Pointer = Type *;				 //!< Pointer to the shared value.
	using ConstPointer = const Type *;	 //!< Pointer-to-const to the shared value.
	//**********************************************************************************************

	//**Constructors********************************************************************************
	/*!\name Constructors */
	//@{
	inline SharedValue();
	explicit inline SharedValue(const Type &value);

	SharedValue(const SharedValue &) = default;
	SharedValue(SharedValue &&) = default;
	//@}
	//**********************************************************************************************

	//**Destructor**********************************************************************************
	/*!\name Destructor */
	//@{
	~SharedValue() = default;
	//@}
	//**********************************************************************************************

	//**Assignment operators************************************************************************
	/*!\name Assignment operators */
	//@{
	SharedValue &operator=(const SharedValue &) = default;
	SharedValue &operator=(SharedValue &&) = default;
	//@}
	//**********************************************************************************************

	//**Access operators****************************************************************************
	/*!\name Access operators */
	//@{
	inline Reference operator*();
	inline ConstReference operator*() const;
	//@}
	//**********************************************************************************************

	//**Utility functions***************************************************************************
	/*!\name Utility functions */
	//@{
	inline Pointer base() const noexcept;
	//@}
	//**********************************************************************************************

  private:
	//**Member variables****************************************************************************
	/*!\name Member variables */
	//@{
	mutable std::shared_ptr<Type> value_; //!< The shared value.
	//@}
	//**********************************************************************************************

	//**Forbidden operations************************************************************************
	/*!\name Forbidden operations */
	//@{
	void *operator&() const; //!< Address operator (private & undefined)
	//@}
	//**********************************************************************************************

	//**Compile time checks*************************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_POINTER_TYPE(Type);
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_REFERENCE_TYPE(Type);
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_CONST(Type);
	METRIC_NUMERIC_CONSTRAINT_MUST_NOT_BE_VOLATILE(Type);
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
/*!\brief Default constructor for a SharedValue.
 */
template <typename Type>						   // Type of the shared value
inline SharedValue<Type>::SharedValue() : value_() // The shared value
{
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Constructor for a SharedValue.
//
// \param value The value to be shared.
//
// This constructor creates a shared value as a copy of the given value.
*/
template <typename Type>														   // Type of the shared value
inline SharedValue<Type>::SharedValue(const Type &value) : value_(new Type(value)) // The shared value
{
}
//*************************************************************************************************

//=================================================================================================
//
//  ACCESS OPERATORS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Direct access to the shared value.
//
// \return Reference to the shared value.
*/
template <typename Type> // Type of the shared value
inline typename SharedValue<Type>::Reference SharedValue<Type>::operator*()
{
	if (!value_)
		value_.reset(new Type());
	return *value_;
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Direct access to the shared value.
//
// \return Reference to the shared value.
*/
template <typename Type> // Type of the shared value
inline typename SharedValue<Type>::ConstReference SharedValue<Type>::operator*() const
{
	if (!value_)
		value_.reset(new Type());
	return *value_;
}
//*************************************************************************************************

//=================================================================================================
//
//  UTILITY FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Low-level access to the underlying, shared value.
//
// \return Pointer to the shared value.
*/
template <typename Type> // Type of the shared value
inline typename SharedValue<Type>::Pointer SharedValue<Type>::base() const noexcept
{
	return value_.get();
}
//*************************************************************************************************

//=================================================================================================
//
//  GLOBAL OPERATORS
//
//=================================================================================================

//*************************************************************************************************
/*!\name SharedValue operators */
//@{
template <typename T1, typename T2> bool operator==(const SharedValue<T1> &lhs, const SharedValue<T2> &rhs);

template <typename T1, typename T2> bool operator!=(const SharedValue<T1> &lhs, const SharedValue<T2> &rhs);
//@}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Equality comparison between two SharedValue objects.
// \ingroup symmetric_matrix
//
// \param lhs The left-hand side SharedValue object.
// \param rhs The right-hand side SharedValue object.
// \return \a true if both shared values refer to the same value, \a false if they don't.
*/
template <typename T1, typename T2> inline bool operator==(const SharedValue<T1> &lhs, const SharedValue<T2> &rhs)
{
	return (lhs.base() == rhs.base());
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Inequality comparison between two SharedValue objects.
// \ingroup symmetric_matrix
//
// \param lhs The left-hand side SharedValue object.
// \param rhs The right-hand side SharedValue object.
// \return \a true if both shared values refer to different values, \a false if they don't.
*/
template <typename T1, typename T2> inline bool operator!=(const SharedValue<T1> &lhs, const SharedValue<T2> &rhs)
{
	return (lhs.base() != rhs.base());
}
//*************************************************************************************************

//=================================================================================================
//
//  GLOBAL FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\name SharedValue global functions */
//@{
template <RelaxationFlag RF, typename Type> bool isDefault(const SharedValue<Type> &value);

template <RelaxationFlag RF, typename T1, typename T2>
bool equal(const SharedValue<T1> &lhs, const SharedValue<T2> &rhs);
//@}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Returns whether the shared value is in default state.
// \ingroup symmetric_matrix
//
// \param value The given shared value.
// \return \a true in case the shared value is in default state, \a false otherwise.
//
// This function checks whether the given shared value is in default state. In case it is in
// default state, the function returns \a true, otherwise it returns \a false.
*/
template <RelaxationFlag RF, typename Type> inline bool isDefault(const SharedValue<Type> &value)
{
	using mtrc::numeric::isDefault;

	return isDefault<RF>(*value);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Equality check for a two shared values.
// \ingroup symmetric_matrix
//
// \param a The left-hand side shared value.
// \param b The right-hand side shared value.
// \return \a true if the two shared values are equal, \a false if not.
//
// This function checks whether the two given shared values are equal, taking the limited
// machine accuracy into account. In case the two values are equal, the function returns
// \a true, otherwise it returns \a false.
*/
template <RelaxationFlag RF, typename T1, typename T2>
inline bool equal(const SharedValue<T1> &a, const SharedValue<T2> &b)
{
	return equal<RF>(*a, *b);
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
