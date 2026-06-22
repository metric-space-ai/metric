// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_EXPRESSIONS_DENSEVECTOR_H
#define METRIC_NUMERIC_MATH_EXPRESSIONS_DENSEVECTOR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/expressions/Vector.h>
#include <metric/numeric/math/typetraits/HasConstDataAccess.h>
#include <metric/numeric/math/typetraits/HasMutableDataAccess.h>
#include <metric/numeric/system/Inline.h>
#include <metric/numeric/util/EnableIf.h>
#include <metric/numeric/util/MaybeUnused.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\defgroup dense_vector Dense Vectors
// \ingroup vector
*/
/*!\defgroup dense_vector_expression Expressions
// \ingroup dense_vector
*/
/*!\brief Base class for N-dimensional dense vectors.
// \ingroup dense_vector
//
// The DenseVector class is a base class for all arbitrarily sized (N-dimensional) dense
// vectors. It provides an abstraction from the actual type of the dense vector, but enables
// a conversion back to this type via the Vector base class.
*/
template <typename VT // Type of the dense vector
		  ,
		  bool TF> // Transpose flag
class DenseVector : public Vector<VT, TF> {
  protected:
	//**Special member functions********************************************************************
	/*!\name Special member functions */
	//@{
	DenseVector() = default;
	DenseVector(const DenseVector &) = default;
	DenseVector(DenseVector &&) = default;
	~DenseVector() = default;
	DenseVector &operator=(const DenseVector &) = default;
	DenseVector &operator=(DenseVector &&) = default;
	//@}
	//**********************************************************************************************
};
//*************************************************************************************************

//=================================================================================================
//
//  GLOBAL FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\name DenseVector global functions */
//@{
template <typename VT, bool TF> typename VT::ElementType *data(DenseVector<VT, TF> &dv) noexcept;

template <typename VT, bool TF> const typename VT::ElementType *data(const DenseVector<VT, TF> &dv) noexcept;
//@}
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Backend implementation of the \c data() function for vectors without mutable data access.
// \ingroup dense_vector
//
// \param dv The given dense vector.
// \return Pointer to the internal element storage.
//
// This function returns the internal storage of a dense vector without mutable data access,
// which is represented by \c nullptr.
*/
template <typename VT // Type of the vector
		  ,
		  bool TF> // Transpose flag of the vector
METRIC_NUMERIC_ALWAYS_INLINE auto data_backend(DenseVector<VT, TF> &dv) noexcept
	-> DisableIf_t<HasMutableDataAccess_v<VT>, typename VT::ElementType *>
{
	MAYBE_UNUSED(dv);

	return nullptr;
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Backend implementation of the \c data() function for vectors with mutable data access.
// \ingroup dense_vector
//
// \param dv The given dense vector.
// \return Pointer to the internal element storage.
//
// This function returns the internal storage of a dense vector with mutable data access.
*/
template <typename VT // Type of the vector
		  ,
		  bool TF> // Transpose flag of the vector
METRIC_NUMERIC_ALWAYS_INLINE auto data_backend(DenseVector<VT, TF> &dv) noexcept
	-> EnableIf_t<HasMutableDataAccess_v<VT>, typename VT::ElementType *>
{
	return (*dv).data();
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Low-level data access to the dense vector elements.
// \ingroup dense_vector
//
// \param dv The given dense vector.
// \return Pointer to the internal element storage.
//
// This function provides a unified interface to access the given dense vector's internal
// element storage. In contrast to the \c data() member function, which is only available
// in case the vector has some internal storage, this function can be used on all kinds of
// dense vectors. In case the given dense vector does not provide low-level data access,
// the function returns \c nullptr.
*/
template <typename VT // Type of the vector
		  ,
		  bool TF> // Transpose flag of the vector
METRIC_NUMERIC_ALWAYS_INLINE typename VT::ElementType *data(DenseVector<VT, TF> &dv) noexcept
{
	return data_backend(*dv);
}
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Backend implementation of the \c data() function for vectors without constant data access.
// \ingroup dense_vector
//
// \param dv The given dense vector.
// \return Pointer to the internal element storage.
//
// This function returns the internal storage of a dense vector without constant data access,
// which is represented by \c nullptr.
*/
template <typename VT // Type of the vector
		  ,
		  bool TF> // Transpose flag of the vector
METRIC_NUMERIC_ALWAYS_INLINE auto data_backend(const DenseVector<VT, TF> &dv) noexcept
	-> DisableIf_t<HasConstDataAccess_v<VT>, const typename VT::ElementType *>
{
	MAYBE_UNUSED(dv);

	return nullptr;
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Backend implementation of the \c data() function for vectors with constant data access.
// \ingroup dense_vector
//
// \param dv The given dense vector.
// \return Pointer to the internal element storage.
//
// This function returns the internal storage of a dense vector with constant data access.
*/
template <typename VT // Type of the vector
		  ,
		  bool TF> // Transpose flag of the vector
METRIC_NUMERIC_ALWAYS_INLINE auto data_backend(const DenseVector<VT, TF> &dv) noexcept
	-> EnableIf_t<HasConstDataAccess_v<VT>, const typename VT::ElementType *>
{
	return (*dv).data();
}
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Low-level data access to the dense vector elements.
// \ingroup dense_vector
//
// \param dv The given dense vector.
// \return Pointer to the internal element storage.
//
// This function provides a unified interface to access the given dense vector's internal
// element storage. In contrast to the \c data() member function, which is only available
// in case the vector has some internal storage, this function can be used on all kinds of
// dense vectors. In case the given dense vector does not provide low-level data access,
// the function returns \c nullptr.
*/
template <typename VT // Type of the vector
		  ,
		  bool TF> // Transpose flag of the vector
METRIC_NUMERIC_ALWAYS_INLINE const typename VT::ElementType *data(const DenseVector<VT, TF> &dv) noexcept
{
	return data_backend(*dv);
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
