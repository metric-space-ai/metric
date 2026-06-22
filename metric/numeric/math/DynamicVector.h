// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_DYNAMICVECTOR_H
#define METRIC_NUMERIC_MATH_DYNAMICVECTOR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/DenseVector.h>
#include <metric/numeric/math/DynamicMatrix.h>
#include <metric/numeric/math/ZeroVector.h>
#include <metric/numeric/math/dense/DynamicVector.h>
#include <metric/numeric/math/dense/StaticVector.h>
#include <metric/numeric/util/Random.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  RAND SPECIALIZATION
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the Rand class template for DynamicVector.
// \ingroup random
//
// This specialization of the Rand class creates random instances of DynamicVector.
*/
template <typename Type // Data type of the vector
		  ,
		  bool TF // Transpose flag
		  ,
		  typename Tag> // Type tag
class Rand<DynamicVector<Type, TF, Tag>> {
  public:
	//**********************************************************************************************
	/*!\brief Generation of a random DynamicVector.
	//
	// \param n The size of the random vector.
	// \return The generated random vector.
	*/
	inline const DynamicVector<Type, TF, Tag> generate(size_t n) const
	{
		DynamicVector<Type, TF, Tag> vector(n);
		randomize(vector);
		return vector;
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Generation of a random DynamicVector.
	//
	// \param n The size of the random vector.
	// \param min The smallest possible value for a vector element.
	// \param max The largest possible value for a vector element.
	// \return The generated random vector.
	*/
	template <typename Arg> // Min/max argument type
	inline const DynamicVector<Type, TF, Tag> generate(size_t n, const Arg &min, const Arg &max) const
	{
		DynamicVector<Type, TF, Tag> vector(n);
		randomize(vector, min, max);
		return vector;
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Randomization of a DynamicVector.
	//
	// \param vector The vector to be randomized.
	// \return void
	*/
	inline void randomize(DynamicVector<Type, TF, Tag> &vector) const
	{
		using mtrc::numeric::randomize;

		const size_t size(vector.size());
		for (size_t i = 0UL; i < size; ++i) {
			randomize(vector[i]);
		}
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Randomization of a DynamicVector.
	//
	// \param vector The vector to be randomized.
	// \param min The smallest possible value for a vector element.
	// \param max The largest possible value for a vector element.
	// \return void
	*/
	template <typename Arg> // Min/max argument type
	inline void randomize(DynamicVector<Type, TF, Tag> &vector, const Arg &min, const Arg &max) const
	{
		using mtrc::numeric::randomize;

		const size_t size(vector.size());
		for (size_t i = 0UL; i < size; ++i) {
			randomize(vector[i], min, max);
		}
	}
	//**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
