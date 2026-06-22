// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_ADAPTORS_HERMITIANMATRIX_HERMITIANVALUE_H
#define METRIC_NUMERIC_MATH_ADAPTORS_HERMITIANMATRIX_HERMITIANVALUE_H
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
#include <metric/numeric/math/shims/Conjugate.h>
#include <metric/numeric/math/shims/Invert.h>
#include <metric/numeric/math/shims/IsDefault.h>
#include <metric/numeric/math/shims/IsOne.h>
#include <metric/numeric/math/shims/IsReal.h>
#include <metric/numeric/math/shims/IsZero.h>
#include <metric/numeric/math/shims/Reset.h>
#include <metric/numeric/math/typetraits/IsRowMajorMatrix.h>
#include <metric/numeric/math/typetraits/UnderlyingBuiltin.h>
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
/*!\brief Representation of two synchronized values within a sparse Hermitian matrix.
// \ingroup hermitian_matrix
//
// The HermitianValue class represents two synchronized values within a sparse Hermitian matrix.
// It guarantees that a modification of value \f$ a_{ij} \f$ via iterator is also applied to the
// value \f$ a_{ji} \f$. The following example illustrates this by means of a \f$ 3 \times 3 \f$
// sparse Hermitian matrix:

   \code
   using cplx = std::complex<double>;
   using Hermitian = mtrc::numeric::HermitianMatrix< mtrc::numeric::CompressedMatrix<cplx> >;

   // Creating a 3x3 Hermitian dense matrix
   //
   // ( ( 0, 0) (0, 0) (-2,1) )
   // ( ( 0, 0) (3, 0) ( 5,2) )
   // ( (-2,-1) (5,-2) ( 0,0) )
   //
   Hermitian A( 3UL );
   A(0,2) = cplx(-2,1);
   A(1,1) = cplx( 3,0);
   A(1,2) = cplx( 5,2);

   // Modification of the values at position (2,0) and (0,2)
   //
   // ( (0,0) (0, 0) (4,-3) )
   // ( (0,0) (3, 0) (5, 2) )
   // ( (4,3) (5,-2) (0, 0) )
   //
   Hermitian::Iterator it = A.begin( 2UL );
   it->value() = cplx(4,3);
   \endcode
*/
template <typename MT> // Type of the adapted matrix
class HermitianValue : public Proxy<HermitianValue<MT>> {
  private:
	//**Type definitions****************************************************************************
	using IteratorType = typename MT::Iterator; //!< Type of the underlying sparse matrix iterators.
	//**********************************************************************************************

  public:
	//**Type definitions****************************************************************************
	using RepresentedType = ElementType_t<MT>; //!< Type of the represented matrix element.

	//! Value type of the represented complex element.
	using ValueType = UnderlyingBuiltin_t<RepresentedType>;

	using value_type = ValueType; //!< Value type of the represented complex element.
	//**********************************************************************************************

	//**Constructors********************************************************************************
	/*!\name Constructors */
	//@{
	inline HermitianValue(IteratorType pos, MT *matrix, size_t index);

	HermitianValue(const HermitianValue &) = default;
	//@}
	//**********************************************************************************************

	//**Destructor**********************************************************************************
	/*!\name Destructor */
	//@{
	~HermitianValue() = default;
	//@}
	//**********************************************************************************************

	//**Assignment operators************************************************************************
	/*!\name Assignment operators */
	//@{
	inline HermitianValue &operator=(const HermitianValue &hv);
	template <typename T> inline HermitianValue &operator=(const T &value);
	template <typename T> inline HermitianValue &operator+=(const T &value);
	template <typename T> inline HermitianValue &operator-=(const T &value);
	template <typename T> inline HermitianValue &operator*=(const T &value);
	template <typename T> inline HermitianValue &operator/=(const T &value);
	//@}
	//**********************************************************************************************

	//**Utility functions***************************************************************************
	/*!\name Utility functions */
	//@{
	inline void invert() const;

	inline RepresentedType get() const noexcept;
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
	//**Utility functions***************************************************************************
	/*!\name Utility functions */
	//@{
	inline void sync() const;
	//@}
	//**********************************************************************************************

	//**Member variables****************************************************************************
	IteratorType pos_; //!< Iterator to the current sparse Hermitian matrix element.
	MT *matrix_;	   //!< The sparse matrix containing the iterator.
	size_t index_;	   //!< The row/column index of the iterator.
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
	/*!\brief Resetting the Hermitian value to the default initial values.
	// \ingroup hermitian_matrix
	//
	// \param value The given Hermitian value.
	// \return void
	//
	// This function resets the Hermitian value to its default initial value.
	*/
	friend inline void reset(const HermitianValue &value)
	{
		using mtrc::numeric::reset;

		reset(value.pos_->value());

		if (value.pos_->index() != value.index_) {
			const size_t row((IsRowMajorMatrix_v<MT>) ? (value.pos_->index()) : (value.index_));
			const size_t column((IsRowMajorMatrix_v<MT>) ? (value.index_) : (value.pos_->index()));
			const IteratorType pos2(value.matrix_->find(row, column));

			reset(pos2->value());
		}
	}
	/*! \endcond */
	//**********************************************************************************************

	//**********************************************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Clearing the Hermitian value.
	// \ingroup hermitian_matrix
	//
	// \param value The given Hermitian value.
	// \return void
	//
	// This function clears the Hermitian value to its default initial state.
	*/
	friend inline void clear(const HermitianValue &value)
	{
		using mtrc::numeric::clear;

		clear(value.pos_->value());

		if (value.pos_->index() != value.index_) {
			const size_t row((IsRowMajorMatrix_v<MT>) ? (value.pos_->index()) : (value.index_));
			const size_t column((IsRowMajorMatrix_v<MT>) ? (value.index_) : (value.pos_->index()));
			const IteratorType pos2(value.matrix_->find(row, column));

			clear(pos2->value());
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
/*!\brief Constructor for the HermitianValue class.
//
// \param pos The initial position of the iterator.
// \param matrix The sparse matrix containing the iterator.
// \param index The row/column index of the iterator.
*/
template <typename MT> // Type of the adapted matrix
inline HermitianValue<MT>::HermitianValue(IteratorType pos, MT *matrix, size_t index)
	: pos_(pos) // Iterator to the current sparse Hermitian matrix element
	  ,
	  matrix_(matrix) // The sparse matrix containing the iterator
	  ,
	  index_(index) // The row/column index of the iterator
{
}
//*************************************************************************************************

//=================================================================================================
//
//  OPERATORS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Copy assignment operator for HermitianValue.
//
// \param hv The Hermitian value to be copied.
// \return Reference to the assigned Hermitian value.
// \exception std::invalid_argument Invalid assignment to diagonal matrix element.
*/
template <typename MT> // Type of the adapted matrix
inline HermitianValue<MT> &HermitianValue<MT>::operator=(const HermitianValue &hv)
{
	const bool isDiagonal(pos_->index() == index_);

	if (IsComplex_v<RepresentedType> && isDiagonal && !isReal(hv.pos_->value())) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid assignment to diagonal matrix element");
	}

	pos_->value() = hv.pos_->value();
	sync();
	return *this;
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Assignment to the Hermitian value.
//
// \param value The new value of the Hermitian value.
// \return Reference to the assigned Hermitian value.
// \exception std::invalid_argument Invalid assignment to diagonal matrix element.
*/
template <typename MT> // Type of the adapted matrix
template <typename T>  // Type of the right-hand side value
inline HermitianValue<MT> &HermitianValue<MT>::operator=(const T &value)
{
	const bool isDiagonal(pos_->index() == index_);

	if (IsComplex_v<RepresentedType> && isDiagonal && !isReal(value)) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid assignment to diagonal matrix element");
	}

	pos_->value() = value;
	sync();
	return *this;
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Addition assignment to the Hermitian value.
//
// \param value The right-hand side value to be added to the Hermitian value.
// \return Reference to the assigned Hermitian value.
// \exception std::invalid_argument Invalid assignment to diagonal matrix element.
*/
template <typename MT> // Type of the adapted matrix
template <typename T>  // Type of the right-hand side value
inline HermitianValue<MT> &HermitianValue<MT>::operator+=(const T &value)
{
	const bool isDiagonal(pos_->index() == index_);

	if (IsComplex_v<RepresentedType> && isDiagonal && !isReal(value)) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid assignment to diagonal matrix element");
	}

	pos_->value() += value;
	sync();
	return *this;
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Subtraction assignment to the Hermitian value.
//
// \param value The right-hand side value to be subtracted from the Hermitian value.
// \return Reference to the assigned Hermitian value.
// \exception std::invalid_argument Invalid assignment to diagonal matrix element.
*/
template <typename MT> // Type of the adapted matrix
template <typename T>  // Type of the right-hand side value
inline HermitianValue<MT> &HermitianValue<MT>::operator-=(const T &value)
{
	const bool isDiagonal(pos_->index() == index_);

	if (IsComplex_v<RepresentedType> && isDiagonal && !isReal(value)) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid assignment to diagonal matrix element");
	}

	pos_->value() -= value;
	sync();
	return *this;
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Multiplication assignment to the Hermitian value.
//
// \param value The right-hand side value for the multiplication.
// \return Reference to the assigned Hermitian value.
// \exception std::invalid_argument Invalid assignment to diagonal matrix element.
*/
template <typename MT> // Type of the adapted matrix
template <typename T>  // Type of the right-hand side value
inline HermitianValue<MT> &HermitianValue<MT>::operator*=(const T &value)
{
	const bool isDiagonal(pos_->index() == index_);

	if (IsComplex_v<RepresentedType> && isDiagonal && !isReal(value)) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid assignment to diagonal matrix element");
	}

	pos_->value() *= value;
	sync();
	return *this;
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Division assignment to the Hermitian value.
//
// \param value The right-hand side value for the division.
// \return Reference to the assigned Hermitian value.
// \exception std::invalid_argument Invalid assignment to diagonal matrix element.
*/
template <typename MT> // Type of the adapted matrix
template <typename T>  // Type of the right-hand side value
inline HermitianValue<MT> &HermitianValue<MT>::operator/=(const T &value)
{
	const bool isDiagonal(pos_->index() == index_);

	if (IsComplex_v<RepresentedType> && isDiagonal && !isReal(value)) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid assignment to diagonal matrix element");
	}

	pos_->value() /= value;
	sync();
	return *this;
}
//*************************************************************************************************

//=================================================================================================
//
//  UTILITY FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief In-place inversion of the Hermitian value
//
// \return void
*/
template <typename MT> // Type of the adapted matrix
inline void HermitianValue<MT>::invert() const
{
	using mtrc::numeric::invert;

	invert(pos_->value());

	if (pos_->index() != index_) {
		const size_t row((IsRowMajorMatrix_v<MT>) ? (pos_->index()) : (index_));
		const size_t column((IsRowMajorMatrix_v<MT>) ? (index_) : (pos_->index()));
		const IteratorType pos2(matrix_->find(row, column));

		pos2->value() = conj(pos_->value());
	}
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Access to the represented value.
//
// \return Copy of the represented value.
*/
template <typename MT> // Type of the adapted matrix
inline typename HermitianValue<MT>::RepresentedType HermitianValue<MT>::get() const noexcept
{
	return pos_->value();
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Synchronization of the current sparse element to the according paired element.
//
// \return void
*/
template <typename MT> // Type of the adapted matrix
inline void HermitianValue<MT>::sync() const
{
	if (pos_->index() == index_ || isDefault(pos_->value()))
		return;

	const size_t row((IsRowMajorMatrix_v<MT>) ? (pos_->index()) : (index_));
	const size_t column((IsRowMajorMatrix_v<MT>) ? (index_) : (pos_->index()));

	matrix_->set(row, column, conj(pos_->value()));
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
inline HermitianValue<MT>::operator RepresentedType() const noexcept
{
	return pos_->value();
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
// In case the value represents a complex number, this function returns the current value
// of its real part.
*/
template <typename MT> // Type of the adapted matrix
inline typename HermitianValue<MT>::ValueType HermitianValue<MT>::real() const
{
	return pos_->value().real();
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Setting the real part of the represented complex number.
//
// \param value The new value for the real part.
// \return void
//
// In case the value represents a complex number, this function sets a new value to its
// real part.
*/
template <typename MT> // Type of the adapted matrix
inline void HermitianValue<MT>::real(ValueType value) const
{
	pos_->value().real() = value;
	sync();
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Returns the imaginary part of the represented complex number.
//
// \return The current imaginary part of the represented complex number.
//
// In case the value represents a complex number, this function returns the current value of its
// imaginary part.
*/
template <typename MT> // Type of the adapted matrix
inline typename HermitianValue<MT>::ValueType HermitianValue<MT>::imag() const
{
	return pos_->value.imag();
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Setting the imaginary part of the represented complex number.
//
// \param value The new value for the imaginary part.
// \return void
//
// In case the proxy represents a complex number, this function sets a new value to its
// imaginary part.
*/
template <typename MT> // Type of the adapted matrix
inline void HermitianValue<MT>::imag(ValueType value) const
{
	pos_->value().imag(value);
	sync();
}
//*************************************************************************************************

//=================================================================================================
//
//  GLOBAL FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\name HermitianValue global functions */
//@{
template <typename MT> void invert(const HermitianValue<MT> &value);
//@}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief In-place inversion of the Hermitian value.
// \ingroup hermitian_matrix
//
// \param value The given Hermitian value.
// \return void
*/
template <typename MT> inline void invert(const HermitianValue<MT> &value) { value.invert(); }
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
