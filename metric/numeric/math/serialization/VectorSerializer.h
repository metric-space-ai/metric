// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_SERIALIZATION_VECTORSERIALIZER_H
#define METRIC_NUMERIC_MATH_SERIALIZATION_VECTORSERIALIZER_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/Aliases.h>
#include <metric/numeric/math/Exception.h>
#include <metric/numeric/math/constraints/Vector.h>
#include <metric/numeric/math/expressions/DenseVector.h>
#include <metric/numeric/math/expressions/SparseVector.h>
#include <metric/numeric/math/expressions/Vector.h>
#include <metric/numeric/math/serialization/TypeValueMapping.h>
#include <metric/numeric/math/typetraits/IsDenseVector.h>
#include <metric/numeric/math/typetraits/IsResizable.h>
#include <metric/numeric/util/Assert.h>
#include <metric/numeric/util/EnableIf.h>
#include <metric/numeric/util/Types.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Serializer for dense and sparse vectors.
// \ingroup math_serialization
//
// The VectorSerializer implements the necessary logic to serialize dense and sparse vectors, i.e.
// to convert them into a portable, binary representation. The following example demonstrates the
// (de-)serialization process of vectors:

   \code
   using mtrc::numeric::columnVector;
   using mtrc::numeric::rowVector;

   // Serialization of both vectors
   {
	  mtrc::numeric::StaticVector<double,5UL,rowVector> d;
	  mtrc::numeric::CompressedVector<int,columnVector> s;

	  // ... Resizing and initialization

	  // Creating an archive that writes into a the file "vectors.metric/numeric"
	  mtrc::numeric::Archive<std::ofstream> archive( "vectors.metric/numeric" );

	  // Serialization of both vectors into the same archive. Note that d lies before s!
	  archive << d << s;
   }

   // Reconstitution of both vectors
   {
	  mtrc::numeric::DynamicVector<double,rowVector> d1;
	  mtrc::numeric::DynamicVector<int,rowVector> d2;

	  // Creating an archive that reads from the file "vectors.metric/numeric"
	  mtrc::numeric::Archive<std::ofstream> archive( "vectors.metric/numeric" );

	  // Reconstituting the former d vector into d1. Note that it is possible to reconstitute
	  // the vector into a differrent kind of vector (StaticVector -> DynamicVector), but that
	  // the type of elements has to be the same.
	  archive >> d1;

	  // Reconstituting the former s vector into d2. Note that is is even possible to reconstitute
	  // a sparse vector as a dense vector (also the reverse is possible) and that a column vector
	  // can be reconstituted as row vector (and vice versa). Note however that also in this case
	  // the type of elements is the same!
	  archive >> d2
   }
   \endcode

// Note that it is even possible to (de-)serialize vectors with vector or matrix elements:

   \code
   // Serialization
   {
	  mtrc::numeric::CompressedVector< mtrc::numeric::DynamicVector< mtrc::numeric::complex<double> > > vec;

	  // ... Resizing and initialization

	  // Creating an archive that writes into a the file "vector.metric/numeric"
	  mtrc::numeric::Archive<std::ofstream> archive( "vector.metric/numeric" );

	  // Serialization of the vector into the archive
	  archive << vec;
   }

   // Deserialization
   {
	  mtrc::numeric::CompressedVector< mtrc::numeric::DynamicVector< mtrc::numeric::complex<double> > > vec;

	  // Creating an archive that reads from the file "vector.metric/numeric"
	  mtrc::numeric::Archive<std::ofstream> archive( "vector.metric/numeric" );

	  // Reconstitution of the vector from the archive
	  archive >> vec;
   }
   \endcode

// As the examples demonstrates, the vector serialization offers an enormous flexibility. However,
// several actions result in errors:
//
//  - vectors cannot be reconstituted as matrices (and vice versa)
//  - the element type of the serialized and reconstituted vector must match, which means
//    that on the source and destination platform the general type (signed/unsigned integral
//    or floating point) and the size of the type must be exactly the same
//  - when reconstituting a StaticVector, its size must match the size of the serialized vector
//
// In case an error is encountered during (de-)serialization, a \a std::runtime_exception is
// thrown.
*/
class VectorSerializer {
  private:
	//**Private class VectorValueMappingHelper******************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Auxiliary helper class for the VectorValueMapping class template.
	//
	// The VectorValueMapping class template is an auxiliary class for the VectorSerializer. It
	// maps a vector type into an integral representation. For the mapping, the following bit
	// mapping is used:

	   \code
	   0x01 - Vector/Matrix flag
	   0x02 - Dense/Sparse flag
	   0x04 - Row-/Column-major flag
	   \endcode
	*/
	template <bool IsDenseVector> struct VectorValueMappingHelper;
	/*! \endcond */
	//**********************************************************************************************

	//**Private class VectorValueMapping************************************************************
	/*! \cond METRIC_NUMERIC_INTERNAL */
	/*!\brief Serialization of the type of a vector.
	//
	// This class template converts the given vector type into an integral representation suited
	// for serialization. Depending on the given vector type, the \a value member enumeration is
	// set to the according integral representation.
	*/
	template <typename T> struct VectorValueMapping {
		enum { value = VectorValueMappingHelper<IsDenseVector_v<T>>::value };
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_VECTOR_TYPE(T);
	};
	/*! \endcond */
	//**********************************************************************************************

  public:
	//**Constructor*********************************************************************************
	/*!\name Constructor */
	//@{
	inline VectorSerializer();
	//@}
	//**********************************************************************************************

	//**Serialization functions*********************************************************************
	/*!\name Serialization functions */
	//@{
	template <typename Archive, typename VT, bool TF> void serialize(Archive &archive, const Vector<VT, TF> &vec);
	//@}
	//**********************************************************************************************

	//**Deserialization functions*********************************************************************
	/*!\name Deserialization functions */
	//@{
	template <typename Archive, typename VT, bool TF> void deserialize(Archive &archive, Vector<VT, TF> &vec);
	//@}
	//**********************************************************************************************

  private:
	//**Serialization functions*********************************************************************
	/*!\name Serialization functions */
	//@{
	template <typename Archive, typename VT> void serializeHeader(Archive &archive, const VT &vec);

	template <typename Archive, typename VT, bool TF>
	void serializeVector(Archive &archive, const DenseVector<VT, TF> &vec);

	template <typename Archive, typename VT, bool TF>
	void serializeVector(Archive &archive, const SparseVector<VT, TF> &vec);
	//@}
	//**********************************************************************************************

	//**Deserialization functions*******************************************************************
	/*!\name Deserialization functions */
	//@{
	template <typename Archive, typename VT> void deserializeHeader(Archive &archive, const VT &vec);

	template <typename VT, bool TF> DisableIf_t<IsResizable_v<VT>> prepareVector(DenseVector<VT, TF> &vec);

	template <typename VT, bool TF> DisableIf_t<IsResizable_v<VT>> prepareVector(SparseVector<VT, TF> &vec);

	template <typename VT> EnableIf_t<IsResizable_v<VT>> prepareVector(VT &vec);

	template <typename Archive, typename VT> void deserializeVector(Archive &archive, VT &vec);

	template <typename Archive, typename VT, bool TF>
	DisableIf_t<VT::simdEnabled> deserializeDenseVector(Archive &archive, DenseVector<VT, TF> &vec);

	template <typename Archive, typename VT, bool TF>
	EnableIf_t<VT::simdEnabled> deserializeDenseVector(Archive &archive, DenseVector<VT, TF> &vec);

	template <typename Archive, typename VT, bool TF>
	void deserializeDenseVector(Archive &archive, SparseVector<VT, TF> &vec);

	template <typename Archive, typename VT, bool TF>
	void deserializeSparseVector(Archive &archive, DenseVector<VT, TF> &vec);

	template <typename Archive, typename VT, bool TF>
	void deserializeSparseVector(Archive &archive, SparseVector<VT, TF> &vec);
	//@}
	//**********************************************************************************************

	//**Member variables****************************************************************************
	/*!\name Member variables */
	//@{
	uint8_t version_;	  //!< The version of the archive.
	uint8_t type_;		  //!< The type of the vector.
	uint8_t elementType_; //!< The type of an element.
	uint8_t elementSize_; //!< The size in bytes of a single element of the vector.
	uint64_t size_;		  //!< The size of the vector.
	uint64_t number_;	  //!< The total number of elements contained in the vector.
						  //@}
	//**********************************************************************************************
};
//*************************************************************************************************

//=================================================================================================
//
//  CONSTRUCTOR
//
//=================================================================================================

//*************************************************************************************************
/*!\brief The default constructor of the VectorSerializer class.
 */
VectorSerializer::VectorSerializer()
	: version_(0U) // The version of the archive
	  ,
	  type_(0U) // The type of the vector
	  ,
	  elementType_(0U) // The type of an element
	  ,
	  elementSize_(0U) // The size in bytes of a single element of the vector
	  ,
	  size_(0UL) // The size of the vector
	  ,
	  number_(0UL) // The total number of elements contained in the vector
{
}
//*************************************************************************************************

//=================================================================================================
//
//  SERIALIZATION FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Serializes the given vector and writes it to the archive.
//
// \param archive The archive to be written.
// \param vec The vector to be serialized.
// \return void
// \exception std::runtime_error Error during serialization.
//
// This function serializes the given vector and writes it to the given archive. In case any
// error is detected during the serialization, a \a std::runtime_error is thrown.
*/
template <typename Archive // Type of the archive
		  ,
		  typename VT // Type of the vector
		  ,
		  bool TF> // Transpose flag
void VectorSerializer::serialize(Archive &archive, const Vector<VT, TF> &vec)
{
	if (!archive) {
		METRIC_NUMERIC_THROW_RUNTIME_ERROR("Faulty archive detected");
	}

	serializeHeader(archive, *vec);
	serializeVector(archive, *vec);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Serializes all meta information about the given vector.
//
// \param archive The archive to be written.
// \param vec The vector to be serialized.
// \return void
// \exception std::runtime_error File header could not be serialized.
*/
template <typename Archive // Type of the archive
		  ,
		  typename VT> // Type of the vector
void VectorSerializer::serializeHeader(Archive &archive, const VT &vec)
{
	using ET = ElementType_t<VT>;

	archive << uint8_t(1U);
	archive << uint8_t(VectorValueMapping<VT>::value);
	archive << uint8_t(TypeValueMapping<ET>::value);
	archive << uint8_t(sizeof(ET));
	archive << uint64_t(vec.size());
	archive << uint64_t(IsDenseVector_v<VT> ? vec.size() : vec.nonZeros());

	if (!archive) {
		METRIC_NUMERIC_THROW_RUNTIME_ERROR("File header could not be serialized");
	}
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Serializes the elements of a dense vector.
//
// \param archive The archive to be written.
// \param vec The vector to be serialized.
// \return void
// \exception std::runtime_error Dense vector could not be serialized.
*/
template <typename Archive // Type of the archive
		  ,
		  typename VT // Type of the vector
		  ,
		  bool TF> // Transpose flag
void VectorSerializer::serializeVector(Archive &archive, const DenseVector<VT, TF> &vec)
{
	size_t i(0UL);
	while ((i < (*vec).size()) && (archive << (*vec)[i])) {
		++i;
	}

	if (!archive) {
		METRIC_NUMERIC_THROW_RUNTIME_ERROR("Dense vector could not be serialized");
	}
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Serializes the elements of a sparse vector.
//
// \param archive The archive to be written.
// \param vec The vector to be serialized.
// \return void
// \exception std::runtime_error Sparse vector could not be serialized.
*/
template <typename Archive // Type of the archive
		  ,
		  typename VT // Type of the vector
		  ,
		  bool TF> // Transpose flag
void VectorSerializer::serializeVector(Archive &archive, const SparseVector<VT, TF> &vec)
{
	using ConstIterator = ConstIterator_t<VT>;

	ConstIterator element((*vec).begin());
	while ((element != (*vec).end()) && (archive << element->index() << element->value())) {
		++element;
	}

	if (!archive) {
		METRIC_NUMERIC_THROW_RUNTIME_ERROR("Sparse vector could not be serialized");
	}
}
//*************************************************************************************************

//=================================================================================================
//
//  DESERIALIZATION FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Deserializes a vector from the given archive.
//
// \param archive The archive to be read from.
// \param vec The vector to be deserialized.
// \return void
// \exception std::runtime_error Error during deserialization.
*/
template <typename Archive // Type of the archive
		  ,
		  typename VT // Type of the vector
		  ,
		  bool TF> // Transpose flag
void VectorSerializer::deserialize(Archive &archive, Vector<VT, TF> &vec)
{
	if (!archive) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Faulty archive detected");
	}

	deserializeHeader(archive, *vec);
	prepareVector(*vec);
	deserializeVector(archive, *vec);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Deserializes all meta information about the given vector.
//
// \param archive The archive to be read from.
// \param vec The vector to be deserialized.
// \return void
// \exception std::runtime_error Error during deserialization.
//
// This function deserializes all meta information about the given vector contained in the
// header of the given archive. In case any error is detected during the deserialization
// process (for instance an invalid type of vector, element type, element size, or vector
// size) a \a std::runtime_error is thrown.
*/
template <typename Archive // Type of the archive
		  ,
		  typename VT> // Type of the vector
void VectorSerializer::deserializeHeader(Archive &archive, const VT &vec)
{
	using ET = ElementType_t<VT>;

	if (!(archive >> version_ >> type_ >> elementType_ >> elementSize_ >> size_ >> number_)) {
		METRIC_NUMERIC_THROW_RUNTIME_ERROR("Corrupt archive detected");
	} else if (version_ != 1UL) {
		METRIC_NUMERIC_THROW_RUNTIME_ERROR("Invalid version detected");
	} else if ((type_ & 1U) != 0U || (type_ & (~3U)) != 0U) {
		METRIC_NUMERIC_THROW_RUNTIME_ERROR("Invalid vector type detected");
	} else if (elementType_ != TypeValueMapping<ET>::value) {
		METRIC_NUMERIC_THROW_RUNTIME_ERROR("Invalid element type detected");
	} else if (elementSize_ != sizeof(ET)) {
		METRIC_NUMERIC_THROW_RUNTIME_ERROR("Invalid element size detected");
	} else if (!IsResizable_v<VT> && size_ != vec.size()) {
		METRIC_NUMERIC_THROW_RUNTIME_ERROR("Invalid vector size detected");
	} else if (number_ > size_) {
		METRIC_NUMERIC_THROW_RUNTIME_ERROR("Invalid number of elements detected");
	}
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Prepares the given non-resizable dense vector for the deserialization process.
//
// \param vec The dense vector to be prepared.
// \return void
*/
template <typename VT // Type of the dense vector
		  ,
		  bool TF> // Transpose flag
DisableIf_t<IsResizable_v<VT>> VectorSerializer::prepareVector(DenseVector<VT, TF> &vec)
{
	reset(*vec);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Prepares the given non-resizable sparse vector for the deserialization process.
//
// \param vec The sparse vector to be prepared.
// \return void
*/
template <typename VT // Type of the sparse vector
		  ,
		  bool TF> // Transpose flag
DisableIf_t<IsResizable_v<VT>> VectorSerializer::prepareVector(SparseVector<VT, TF> &vec)
{
	(*vec).reserve(number_);
	reset(*vec);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Prepares the given resizable vector for the deserialization process.
//
// \param vec The vector to be prepared.
// \return void
*/
template <typename VT> // Type of the vector
EnableIf_t<IsResizable_v<VT>> VectorSerializer::prepareVector(VT &vec)
{
	vec.resize(size_, false);
	vec.reserve(number_);
	reset(vec);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Deserializes a vector from the archive.
//
// \param archive The archive to be read from.
// \param vec The vector to be reconstituted.
// \return void
// \exception std::runtime_error Error during deserialization.
//
// This function deserializes the contents of the vector from the archive and reconstitutes the
// given vector.
*/
template <typename Archive // Type of the archive
		  ,
		  typename VT> // Type of the vector
void VectorSerializer::deserializeVector(Archive &archive, VT &vec)
{
	if (type_ == 0U) {
		deserializeDenseVector(archive, vec);
	} else if (type_ == 2U) {
		deserializeSparseVector(archive, vec);
	} else {
		METRIC_NUMERIC_INTERNAL_ASSERT(false, "Undefined type flag");
	}
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Deserializes a dense vector from the archive.
//
// \param archive The archive to be read from.
// \param vec The dense vector to be reconstituted.
// \return void
// \exception std::runtime_error Dense vector could not be deserialized.
//
// This function deserializes a dense vector from the archive and reconstitutes the given
// dense vector. In case any error is detected during the deserialization process, a
// \a std::runtime_error is thrown.
*/
template <typename Archive // Type of the archive
		  ,
		  typename VT // Type of the vector
		  ,
		  bool TF> // Transpose flag
DisableIf_t<VT::simdEnabled> VectorSerializer::deserializeDenseVector(Archive &archive, DenseVector<VT, TF> &vec)
{
	using ET = ElementType_t<VT>;

	size_t i(0UL);
	ET value{};

	while ((i != size_) && (archive >> value)) {
		(*vec)[i] = value;
		++i;
	}

	if (!archive) {
		METRIC_NUMERIC_THROW_RUNTIME_ERROR("Dense vector could not be deserialized");
	}
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Deserializes a dense vector from the archive.
//
// \param archive The archive to be read from.
// \param vec The dense vector to be reconstituted.
// \return void
// \exception std::runtime_error Dense vector could not be deserialized.
//
// This function deserializes a dense vector from the archive and reconstitutes the given
// dense vector. In case any error is detected during the deserialization process, a
// \a std::runtime_error is thrown.
*/
template <typename Archive // Type of the archive
		  ,
		  typename VT // Type of the vector
		  ,
		  bool TF> // Transpose flag
EnableIf_t<VT::simdEnabled> VectorSerializer::deserializeDenseVector(Archive &archive, DenseVector<VT, TF> &vec)
{
	if (size_ == 0UL)
		return;
	archive.read(&(*vec)[0], size_);

	if (!archive) {
		METRIC_NUMERIC_THROW_RUNTIME_ERROR("Dense vector could not be deserialized");
	}
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Deserializes a dense vector from the archive.
//
// \param archive The archive to be read from.
// \param vec The sparse vector to be reconstituted.
// \return void
// \exception std::runtime_error Sparse vector could not be deserialized.
//
// This function deserializes a dense vector from the archive and reconstitutes the given
// sparse vector. In case any error is detected during the deserialization process, a
// \a std::runtime_error is thrown.
*/
template <typename Archive // Type of the archive
		  ,
		  typename VT // Type of the vector
		  ,
		  bool TF> // Transpose flag
void VectorSerializer::deserializeDenseVector(Archive &archive, SparseVector<VT, TF> &vec)
{
	using ET = ElementType_t<VT>;

	size_t i(0UL);
	ET value{};

	while ((i != size_) && (archive >> value)) {
		(*vec)[i] = value;
		++i;
	}

	if (!archive) {
		METRIC_NUMERIC_THROW_RUNTIME_ERROR("Sparse vector could not be deserialized");
	}
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Deserializes a sparse vector from the archive.
//
// \param archive The archive to be read from.
// \param vec The dense vector to be reconstituted.
// \return void
// \exception std::runtime_error Dense vector could not be deserialized.
//
// This function deserializes a sparse vector from the archive and reconstitutes the given
// dense vector. In case any error is detected during the deserialization process, a
// \a std::runtime_error is thrown.
*/
template <typename Archive // Type of the archive
		  ,
		  typename VT // Type of the vector
		  ,
		  bool TF> // Transpose flag
void VectorSerializer::deserializeSparseVector(Archive &archive, DenseVector<VT, TF> &vec)
{
	using ET = ElementType_t<VT>;

	size_t i(0UL);
	size_t index(0UL);
	ET value{};

	while ((i != number_) && (archive >> index >> value)) {
		(*vec)[index] = value;
		++i;
	}

	if (!archive) {
		METRIC_NUMERIC_THROW_RUNTIME_ERROR("Dense vector could not be deserialized");
	}
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Deserializes a sparse vector from the archive.
//
// \param archive The archive to be read from.
// \param vec The sparse vector to be reconstituted.
// \return void
// \exception std::runtime_error Sparse vector could not be deserialized.
//
// This function deserializes a sparse vector from the archive and reconstitutes the given
// sparse vector. In case any error is detected during the deserialization process, a
// \a std::runtime_error is thrown.
*/
template <typename Archive // Type of the archive
		  ,
		  typename VT // Type of the vector
		  ,
		  bool TF> // Transpose flag
void VectorSerializer::deserializeSparseVector(Archive &archive, SparseVector<VT, TF> &vec)
{
	using ET = ElementType_t<VT>;

	size_t i(0UL);
	size_t index(0UL);
	ET value{};

	while ((i != number_) && (archive >> index >> value)) {
		(*vec).append(index, value, false);
		++i;
	}

	if (!archive) {
		METRIC_NUMERIC_THROW_RUNTIME_ERROR("Sparse vector could not be deserialized");
	}
}
//*************************************************************************************************

//=================================================================================================
//
//  VECTORVALUEMAPPINGHELPER SPECIALIZATIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the VectorValueMappingHelper class template for dense vectors.
 */
template <> struct VectorSerializer::VectorValueMappingHelper<true> {
	enum { value = 0 };
};
/*! \endcond */
//*************************************************************************************************

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the VectorValueMappingHelper class template for sparse vectors.
 */
template <> struct VectorSerializer::VectorValueMappingHelper<false> {
	enum { value = 2 };
};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  GLOBAL FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Serializes the given vector and writes it to the archive.
//
// \param archive The archive to be written.
// \param vec The vector to be serialized.
// \return void
// \exception std::runtime_error Error during serialization.
//
// The serialize() function converts the given vector into a portable, binary representation.
// The following example demonstrates the (de-)serialization process of vectors:

   \code
   using mtrc::numeric::columnVector;
   using mtrc::numeric::rowVector;

   // Serialization of both vectors
   {
	  mtrc::numeric::StaticVector<double,5UL,rowVector> d;
	  mtrc::numeric::CompressedVector<int,columnVector> s;

	  // ... Resizing and initialization

	  // Creating an archive that writes into a the file "vectors.metric/numeric"
	  mtrc::numeric::Archive<std::ofstream> archive( "vectors.metric/numeric" );

	  // Serialization of both vectors into the same archive. Note that d lies before s!
	  archive << d << s;
   }

   // Reconstitution of both vectors
   {
	  mtrc::numeric::DynamicVector<double,rowVector> d1;
	  mtrc::numeric::DynamicVector<int,rowVector> d2;

	  // ... Resizing and initialization

	  // Creating an archive that reads from the file "vectors.metric/numeric"
	  mtrc::numeric::Archive<std::ofstream> archive( "vectors.metric/numeric" );

	  // Reconstituting the former d vector into d1. Note that it is possible to reconstitute
	  // the vector into a differrent kind of vector (StaticVector -> DynamicVector), but that
	  // the type of elements has to be the same.
	  archive >> d1;

	  // Reconstituting the former s vector into d2. Note that is is even possible to reconstitute
	  // a sparse vector as a dense vector (also the reverse is possible) and that a column vector
	  // can be reconstituted as row vector (and vice versa). Note however that also in this case
	  // the type of elements is the same!
	  archive >> d2
   }
   \endcode

// As the example demonstrates, the vector serialization offers an enormous flexibility. However,
// several actions result in errors:
//
//  - vectors cannot be reconstituted as matrices (and vice versa)
//  - the element type of the serialized and reconstituted vector must match, which means
//    that on the source and destination platform the general type (signed/unsigned integral
//    or floating point) and the size of the type must be exactly the same
//  - when reconstituting a StaticVector, its size must match the size of the serialized vector
//
// In case an error is encountered during (de-)serialization, a \a std::runtime_exception is
// thrown.
*/
template <typename Archive // Type of the archive
		  ,
		  typename VT // Type of the vector
		  ,
		  bool TF> // Transpose flag
void serialize(Archive &archive, const Vector<VT, TF> &vec)
{
	VectorSerializer().serialize(archive, *vec);
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Deserializes a vector from the given archive.
//
// \param archive The archive to be read from.
// \param vec The vector to be deserialized.
// \return void
// \exception std::runtime_error Vector could not be deserialized.
//
// The deserialize() function converts the portable, binary representation contained in
// the given archive into the given vector type. For a detailed example that demonstrates
// the (de-)serialization process of vectors, see the serialize() function.
*/
template <typename Archive // Type of the archive
		  ,
		  typename VT // Type of the vector
		  ,
		  bool TF> // Transpose flag
void deserialize(Archive &archive, Vector<VT, TF> &vec)
{
	VectorSerializer().deserialize(archive, *vec);
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
