// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_UNIFORMVECTOR_H
#define METRIC_NUMERIC_MATH_UNIFORMVECTOR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/DenseVector.h>
#include <metric/numeric/math/dense/UniformVector.h>
#include <metric/numeric/util/Random.h>
#include <metric/numeric/util/Types.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  RAND SPECIALIZATION
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the Rand class template for UniformVector.
// \ingroup random
//
// This specialization of the Rand class creates random instances of UniformVector.
*/
template <typename Type // Data type of the vector
		  ,
		  bool TF // Transpose flag
		  ,
		  typename Tag> // Type tag
class Rand<UniformVector<Type, TF, Tag>> {
  public:
	//**********************************************************************************************
	/*!\brief Generation of a random UniformVector.
	//
	// \param n The size of the random vector.
	// \return The generated random vector.
	*/
	inline const UniformVector<Type, TF, Tag> generate(size_t n) const
	{
		UniformVector<Type, TF, Tag> vector(n);
		randomize(vector);
		return vector;
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Generation of a random UniformVector.
	//
	// \param n The size of the random vector.
	// \param min The smallest possible value for a vector element.
	// \param max The largest possible value for a vector element.
	// \return The generated random vector.
	*/
	template <typename Arg> // Min/max argument type
	inline const UniformVector<Type, TF, Tag> generate(size_t n, const Arg &min, const Arg &max) const
	{
		UniformVector<Type, TF, Tag> vector(n);
		randomize(vector, min, max);
		return vector;
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Randomization of a UniformVector.
	//
	// \param vector The vector to be randomized.
	// \return void
	*/
	inline void randomize(UniformVector<Type, TF, Tag> &vector) const { vector = rand<Type>(); }
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Randomization of a UniformVector.
	//
	// \param vector The vector to be randomized.
	// \param min The smallest possible value for a vector element.
	// \param max The largest possible value for a vector element.
	// \return void
	*/
	template <typename Arg> // Min/max argument type
	inline void randomize(UniformVector<Type, TF, Tag> &vector, const Arg &min, const Arg &max) const
	{
		vector = rand<Type>(min, max);
	}
	//**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
