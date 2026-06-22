// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_SPARSE_MATRIXACCESSPROXY_H
#define METRIC_NUMERIC_MATH_SPARSE_MATRIXACCESSPROXY_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/Aliases.h>
#include <metric/numeric/math/InitializerList.h>
#include <metric/numeric/math/RelaxationFlag.h>
#include <metric/numeric/math/constraints/SparseMatrix.h>
#include <metric/numeric/math/proxy/Proxy.h>
#include <metric/numeric/math/shims/Clear.h>
#include <metric/numeric/math/shims/IsDefault.h>
#include <metric/numeric/math/shims/IsOne.h>
#include <metric/numeric/math/shims/IsReal.h>
#include <metric/numeric/math/shims/IsZero.h>
#include <metric/numeric/math/typetraits/IsRowMajorMatrix.h>
#include <metric/numeric/util/Assert.h>
#include <metric/numeric/util/Types.h>
#include <utility>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Access proxy for sparse, \f$ M \times N \f$ matrices.
// \ingroup sparse_matrix
//
// The MatrixAccessProxy provides safe access to the elements of a non-const sparse matrices.\n
// The proxied access to the elements of a sparse matrix is necessary since it may be possible
// that several insertion operations happen in the same statement. The following code illustrates
// this with two examples by means of the CompressedMatrix class:

   \code
   mtrc::numeric::CompressedMatrix<double> A( 4, 4 );

   // Standard usage of the function call operator to initialize a matrix element.
   // Only a single sparse matrix element is accessed!
   A(0,1) = 1.0;

   // Initialization of a matrix element via another matrix element.
   // Two sparse matrix accesses in one statement!
   A(1,2) = A(0,1);

   // Multiple accesses to elements of the sparse matrix in one statement!
   const double result = A(0,2) + A(1,2) + A(2,2);
   \endcode

// The problem (especially with the last statement) is that several insertion operations might
// take place due to the access via the function call operator. If the function call operator
// would return a direct reference to one of the accessed elements, this reference might be
// invalidated during the evaluation of a subsequent function call operator, which results in
// undefined behavior. This class provides the necessary functionality to guarantee a safe access
// to the sparse matrix elements while preserving the intuitive use of the function call operator.
*/
template <typename MT> // Type of the sparse matrix
class MatrixAccessProxy : public Proxy<MatrixAccessProxy<MT>, ElementType_t<MT>> {
  private:
	//**Enumerations********************************************************************************
	//! Compile time flag indicating whether the given matrix type is a row-major matrix.
	static constexpr bool rmm = IsRowMajorMatrix_v<MT>;
	//**********************************************************************************************

  public:
	//**Type definitions****************************************************************************
	using RepresentedType = ElementType_t<MT>; //!< Type of the represented sparse matrix element.
	using RawReference = RepresentedType &;	   //!< Raw reference to the represented element.
	//**********************************************************************************************

	//**Constructors********************************************************************************
	/*!\name Constructors */
	//@{
	explicit inline MatrixAccessProxy(MT &sm, size_t i, size_t j);

	MatrixAccessProxy(const MatrixAccessProxy &) = default;
	//@}
	//**********************************************************************************************

	//**Destructor**********************************************************************************
	/*!\name Destructor */
	//@{
	inline ~MatrixAccessProxy();
	//@}
	//**********************************************************************************************

	//**Operators***********************************************************************************
	/*!\name Operators */
	//@{
	inline const MatrixAccessProxy &operator=(const MatrixAccessProxy &map) const;

	template <typename T> inline const MatrixAccessProxy &operator=(initializer_list<T> list) const;

	template <typename T> inline const MatrixAccessProxy &operator=(initializer_list<initializer_list<T>> list) const;

	template <typename T> inline const MatrixAccessProxy &operator=(const T &value) const;
	template <typename T> inline const MatrixAccessProxy &operator+=(const T &value) const;
	template <typename T> inline const MatrixAccessProxy &operator-=(const T &value) const;
	template <typename T> inline const MatrixAccessProxy &operator*=(const T &value) const;
	template <typename T> inline const MatrixAccessProxy &operator/=(const T &value) const;
	template <typename T> inline const MatrixAccessProxy &operator%=(const T &value) const;
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
	inline operator RawReference() const noexcept;
	//@}
	//**********************************************************************************************

  private:
	//**Member variables****************************************************************************
	/*!\name Member variables */
	//@{
	MT &sm_;   //!< Reference to the accessed sparse matrix.
	size_t i_; //!< Row-index of the accessed sparse matrix element.
	size_t j_; //!< Column-index of the accessed sparse matrix element.
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
	METRIC_NUMERIC_CONSTRAINT_MUST_BE_SPARSE_MATRIX_TYPE(MT);
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
/*!\brief Initialization constructor for a MatrixAccessProxy.
//
// \param sm Reference to the accessed sparse matrix.
// \param i The row-index of the accessed sparse matrix element.
// \param j The column-index of the accessed sparse matrix element.
*/
template <typename MT> // Type of the sparse matrix
inline MatrixAccessProxy<MT>::MatrixAccessProxy(MT &sm, size_t i, size_t j)
	: sm_(sm) // Reference to the accessed sparse matrix
	  ,
	  i_(i) // Row-index of the accessed sparse matrix element
	  ,
	  j_(j) // Column-index of the accessed sparse matrix element
{
	const Iterator_t<MT> element(sm_.find(i_, j_));
	const size_t index(rmm ? i_ : j_);
	if (element == sm_.end(index))
		sm_.insert(i_, j_, RepresentedType{});
}
//*************************************************************************************************

//=================================================================================================
//
//  DESTRUCTORS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief The destructor for MatrixAccessProxy.
 */
template <typename MT> // Type of the sparse matrix
inline MatrixAccessProxy<MT>::~MatrixAccessProxy()
{
	const Iterator_t<MT> element(sm_.find(i_, j_));
	const size_t index(rmm ? i_ : j_);
	if (element != sm_.end(index) && isDefault<strict>(element->value()))
		sm_.erase(index, element);
}
//*************************************************************************************************

//=================================================================================================
//
//  OPERATORS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Copy assignment operator for MatrixAccessProxy.
//
// \param map Sparse matrix access proxy to be copied.
// \return Reference to the assigned access proxy.
*/
template <typename MT> // Type of the sparse matrix
inline const MatrixAccessProxy<MT> &MatrixAccessProxy<MT>::operator=(const MatrixAccessProxy &map) const
{
	const Iterator_t<MT> element(map.sm_.find(map.i_, map.j_));
	const size_t index(rmm ? map.i_ : map.j_);
	const auto &source(element != map.sm_.end(index) ? element->value() : RepresentedType{});

	get() = source;
	return *this;
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Initializer list assignment to the accessed sparse matrix element.
//
// \param list The list to be assigned to the sparse matrix element.
// \return Reference to the assigned access proxy.
*/
template <typename VT> // Type of the sparse matrix
template <typename T>  // Type of the right-hand side elements
inline const MatrixAccessProxy<VT> &MatrixAccessProxy<VT>::operator=(initializer_list<T> list) const
{
	get() = list;
	return *this;
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Initializer list assignment to the accessed sparse matrix element.
//
// \param list The list to be assigned to the sparse matrix element.
// \return Reference to the assigned access proxy.
*/
template <typename VT> // Type of the sparse matrix
template <typename T>  // Type of the right-hand side elements
inline const MatrixAccessProxy<VT> &MatrixAccessProxy<VT>::operator=(initializer_list<initializer_list<T>> list) const
{
	get() = list;
	return *this;
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Assignment to the accessed sparse matrix element.
//
// \param value The new value of the sparse matrix element.
// \return Reference to the assigned access proxy.
*/
template <typename MT> // Type of the sparse matrix
template <typename T>  // Type of the right-hand side value
inline const MatrixAccessProxy<MT> &MatrixAccessProxy<MT>::operator=(const T &value) const
{
	get() = value;
	return *this;
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Addition assignment to the accessed sparse matrix element.
//
// \param value The right-hand side value to be added to the sparse matrix element.
// \return Reference to the assigned access proxy.
*/
template <typename MT> // Type of the sparse matrix
template <typename T>  // Type of the right-hand side value
inline const MatrixAccessProxy<MT> &MatrixAccessProxy<MT>::operator+=(const T &value) const
{
	get() += value;
	return *this;
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Subtraction assignment to the accessed sparse matrix element.
//
// \param value The right-hand side value to be subtracted from the sparse matrix element.
// \return Reference to the assigned access proxy.
*/
template <typename MT> // Type of the sparse matrix
template <typename T>  // Type of the right-hand side value
inline const MatrixAccessProxy<MT> &MatrixAccessProxy<MT>::operator-=(const T &value) const
{
	get() -= value;
	return *this;
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Multiplication assignment to the accessed sparse matrix element.
//
// \param value The right-hand side value for the multiplication.
// \return Reference to the assigned access proxy.
*/
template <typename MT> // Type of the sparse matrix
template <typename T>  // Type of the right-hand side value
inline const MatrixAccessProxy<MT> &MatrixAccessProxy<MT>::operator*=(const T &value) const
{
	get() *= value;
	return *this;
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Division assignment to the accessed sparse matrix element.
//
// \param value The right-hand side value for the division.
// \return Reference to the assigned access proxy.
*/
template <typename MT> // Type of the sparse matrix
template <typename T>  // Type of the right-hand side value
inline const MatrixAccessProxy<MT> &MatrixAccessProxy<MT>::operator/=(const T &value) const
{
	get() /= value;
	return *this;
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Modulo assignment to the accessed sparse matrix element.
//
// \param value The right-hand side value for the modulo operation.
// \return Reference to the assigned access proxy.
//
// If the access proxy represents an element of numeric type, this function performs a modulo
// assignment, if the proxy represents a dense or sparse vector, a cross product is computed,
// and if the proxy represents a dense or sparse matrix, a Schur product is computed.
*/
template <typename MT> // Type of the sparse matrix
template <typename T>  // Type of the right-hand side value
inline const MatrixAccessProxy<MT> &MatrixAccessProxy<MT>::operator%=(const T &value) const
{
	get() %= value;
	return *this;
}
//*************************************************************************************************

//=================================================================================================
//
//  UTILITY FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Returning the value of the accessed sparse matrix element.
//
// \return Direct/raw reference to the accessed sparse matrix element.
*/
template <typename MT> // Type of the sparse matrix
inline typename MatrixAccessProxy<MT>::RawReference MatrixAccessProxy<MT>::get() const noexcept
{
	Iterator_t<MT> element(sm_.find(i_, j_));
	const size_t index(rmm ? i_ : j_);
	if (element == sm_.end(index))
		element = sm_.insert(i_, j_, RepresentedType{});
	METRIC_NUMERIC_INTERNAL_ASSERT(element != sm_.end(index), "Missing matrix element detected");
	return element->value();
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Returns whether the proxy represents a restricted sparse matrix element..
//
// \return \a true in case access to the sparse matrix element is restricted, \a false if not.
*/
template <typename MT> // Type of the sparse matrix
inline bool MatrixAccessProxy<MT>::isRestricted() const noexcept
{
	return false;
}
//*************************************************************************************************

//=================================================================================================
//
//  CONVERSION OPERATOR
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Conversion to the accessed sparse matrix element.
//
// \return Direct/raw reference to the accessed sparse matrix element.
*/
template <typename MT> // Type of the sparse matrix
inline MatrixAccessProxy<MT>::operator RawReference() const noexcept
{
	return get();
}
//*************************************************************************************************

//=================================================================================================
//
//  GLOBAL FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\name MatrixAccessProxy global functions */
//@{
template <typename MT> void swap(const MatrixAccessProxy<MT> &a, const MatrixAccessProxy<MT> &b) noexcept;

template <typename MT, typename T> void swap(const MatrixAccessProxy<MT> &a, T &b) noexcept;

template <typename T, typename MT> void swap(T &a, const MatrixAccessProxy<MT> &v) noexcept;
//@}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Swapping the contents of two access proxies.
// \ingroup sparse_matrix
//
// \param a The first access proxy to be swapped.
// \param b The second access proxy to be swapped.
// \return void
*/
template <typename MT> inline void swap(const MatrixAccessProxy<MT> &a, const MatrixAccessProxy<MT> &b) noexcept
{
	using std::swap;

	swap(a.get(), b.get());
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Swapping the contents of an access proxy with another element.
// \ingroup sparse_matrix
//
// \param a The access proxy to be swapped.
// \param b The other element to be swapped.
// \return void
*/
template <typename MT, typename T> inline void swap(const MatrixAccessProxy<MT> &a, T &b) noexcept
{
	using std::swap;

	swap(a.get(), b);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Swapping the contents of an access proxy with another element.
// \ingroup sparse_matrix
//
// \param a The other element to be swapped.
// \param b The access proxy to be swapped.
// \return void
*/
template <typename T, typename MT> inline void swap(T &a, const MatrixAccessProxy<MT> &b) noexcept
{
	using std::swap;

	swap(a, b.get());
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
