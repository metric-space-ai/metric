// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_STATICVECTOR_H
#define METRIC_NUMERIC_MATH_STATICVECTOR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/DenseVector.h>
#include <metric/numeric/math/DynamicVector.h>
#include <metric/numeric/math/StaticMatrix.h>
#include <metric/numeric/math/ZeroVector.h>
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
/*!\brief Specialization of the Rand class template for StaticVector.
// \ingroup random
//
// This specialization of the Rand class creates random instances of StaticVector.
*/
template <typename Type // Data type of the vector
		  ,
		  size_t N // Number of elements
		  ,
		  bool TF // Transpose flag
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  PaddingFlag PF // Padding flag
		  ,
		  typename Tag> // Type tag
class Rand<StaticVector<Type, N, TF, AF, PF, Tag>> {
  public:
	//**********************************************************************************************
	/*!\brief Generation of a random StaticVector.
	//
	// \return The generated random vector.
	*/
	inline const StaticVector<Type, N, TF, AF, PF, Tag> generate() const
	{
		StaticVector<Type, N, TF, AF, PF, Tag> vector;
		randomize(vector);
		return vector;
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Generation of a random StaticVector.
	//
	// \param min The smallest possible value for a vector element.
	// \param max The largest possible value for a vector element.
	// \return The generated random vector.
	*/
	template <typename Arg> // Min/max argument type
	inline const StaticVector<Type, N, TF, AF, PF, Tag> generate(const Arg &min, const Arg &max) const
	{
		StaticVector<Type, N, TF, AF, PF, Tag> vector;
		randomize(vector, min, max);
		return vector;
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Randomization of a StaticVector.
	//
	// \param vector The vector to be randomized.
	// \return void
	*/
	inline void randomize(StaticVector<Type, N, TF, AF, PF, Tag> &vector) const
	{
		using mtrc::numeric::randomize;

		for (size_t i = 0UL; i < N; ++i) {
			randomize(vector[i]);
		}
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Randomization of a StaticVector.
	//
	// \param vector The vector to be randomized.
	// \param min The smallest possible value for a vector element.
	// \param max The largest possible value for a vector element.
	// \return void
	*/
	template <typename Arg> // Min/max argument type
	inline void randomize(StaticVector<Type, N, TF, AF, PF, Tag> &vector, const Arg &min, const Arg &max) const
	{
		using mtrc::numeric::randomize;

		for (size_t i = 0UL; i < N; ++i) {
			randomize(vector[i], min, max);
		}
	}
	//**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
