// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_COMPRESSEDVECTOR_H
#define METRIC_NUMERIC_MATH_COMPRESSEDVECTOR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/CompressedMatrix.h>
#include <metric/numeric/math/Exception.h>
#include <metric/numeric/math/SparseVector.h>
#include <metric/numeric/math/ZeroVector.h>
#include <metric/numeric/math/dense/StaticVector.h>
#include <metric/numeric/math/sparse/CompressedVector.h>
#include <metric/numeric/util/Indices.h>
#include <metric/numeric/util/Random.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  RAND SPECIALIZATION
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the Rand class template for CompressedVector.
// \ingroup random
//
// This specialization of the Rand class creates random instances of CompressedVector.
*/
template <typename Type // Data type of the vector
		  ,
		  bool TF // Transpose flag
		  ,
		  typename Tag> // Type tag
class Rand<CompressedVector<Type, TF, Tag>> {
  public:
	//**********************************************************************************************
	/*!\brief Generation of a random CompressedVector.
	//
	// \param size The size of the random vector.
	// \return The generated random vector.
	*/
	inline const CompressedVector<Type, TF, Tag> generate(size_t size) const
	{
		CompressedVector<Type, TF, Tag> vector(size);
		randomize(vector);

		return vector;
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Generation of a random CompressedVector.
	//
	// \param size The size of the random vector.
	// \param nonzeros The number of non-zero elements of the random vector.
	// \return The generated random vector.
	// \exception std::invalid_argument Invalid number of non-zero elements.
	*/
	inline const CompressedVector<Type, TF, Tag> generate(size_t size, size_t nonzeros) const
	{
		if (nonzeros > size) {
			METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid number of non-zero elements");
		}

		CompressedVector<Type, TF, Tag> vector(size, nonzeros);
		randomize(vector, nonzeros);

		return vector;
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Generation of a random CompressedVector.
	//
	// \param size The size of the random vector.
	// \param min The smallest possible value for a vector element.
	// \param max The largest possible value for a vector element.
	// \return The generated random vector.
	*/
	template <typename Arg> // Min/max argument type
	inline const CompressedVector<Type, TF, Tag> generate(size_t size, const Arg &min, const Arg &max) const
	{
		CompressedVector<Type, TF, Tag> vector(size);
		randomize(vector, min, max);

		return vector;
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Generation of a random CompressedVector.
	//
	// \param size The size of the random vector.
	// \param nonzeros The number of non-zero elements of the random vector.
	// \param min The smallest possible value for a vector element.
	// \param max The largest possible value for a vector element.
	// \return The generated random vector.
	// \exception std::invalid_argument Invalid number of non-zero elements.
	*/
	template <typename Arg> // Min/max argument type
	inline const CompressedVector<Type, TF, Tag> generate(size_t size, size_t nonzeros, const Arg &min,
														  const Arg &max) const
	{
		if (nonzeros > size) {
			METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid number of non-zero elements");
		}

		CompressedVector<Type, TF, Tag> vector(size, nonzeros);
		randomize(vector, nonzeros, min, max);

		return vector;
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Randomization of a CompressedVector.
	//
	// \param vector The vector to be randomized.
	// \return void
	*/
	inline void randomize(CompressedVector<Type, TF, Tag> &vector) const
	{
		const size_t size(vector.size());

		if (size == 0UL)
			return;

		const size_t nonzeros(rand<size_t>(1UL, std::ceil(0.5 * size)));

		randomize(vector, nonzeros);
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Randomization of a CompressedVector.
	//
	// \param vector The vector to be randomized.
	// \param nonzeros The number of non-zero elements of the random vector.
	// \return void
	// \exception std::invalid_argument Invalid number of non-zero elements.
	*/
	inline void randomize(CompressedVector<Type, TF, Tag> &vector, size_t nonzeros) const
	{
		const size_t size(vector.size());

		if (nonzeros > size) {
			METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid number of non-zero elements");
		}

		if (size == 0UL)
			return;

		vector.reset();
		vector.reserve(nonzeros);

		const Indices<size_t> indices(0UL, vector.size() - 1UL, nonzeros);

		for (size_t index : indices) {
			vector.append(index, rand<Type>());
		}
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Randomization of a CompressedVector.
	//
	// \param vector The vector to be randomized.
	// \param min The smallest possible value for a vector element.
	// \param max The largest possible value for a vector element.
	// \return void
	*/
	template <typename Arg> // Min/max argument type
	inline void randomize(CompressedVector<Type, TF, Tag> &vector, const Arg &min, const Arg &max) const
	{
		const size_t size(vector.size());

		if (size == 0UL)
			return;

		const size_t nonzeros(rand<size_t>(1UL, std::ceil(0.5 * size)));

		randomize(vector, nonzeros, min, max);
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Randomization of a CompressedVector.
	//
	// \param vector The vector to be randomized.
	// \param nonzeros The number of non-zero elements of the random vector.
	// \param min The smallest possible value for a vector element.
	// \param max The largest possible value for a vector element.
	// \return void
	// \exception std::invalid_argument Invalid number of non-zero elements.
	*/
	template <typename Arg> // Min/max argument type
	inline void randomize(CompressedVector<Type, TF, Tag> &vector, size_t nonzeros, const Arg &min,
						  const Arg &max) const
	{
		const size_t size(vector.size());

		if (nonzeros > size) {
			METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid number of non-zero elements");
		}

		if (size == 0UL)
			return;

		vector.reset();
		vector.reserve(nonzeros);

		const Indices<size_t> indices(0UL, vector.size() - 1UL, nonzeros);

		for (size_t index : indices) {
			vector.append(index, rand<Type>(min, max));
		}
	}
	//**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
