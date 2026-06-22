// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_HYBRIDVECTOR_H
#define METRIC_NUMERIC_MATH_HYBRIDVECTOR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/DenseVector.h>
#include <metric/numeric/math/DynamicMatrix.h>
#include <metric/numeric/math/ZeroVector.h>
#include <metric/numeric/math/dense/HybridVector.h>
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
/*!\brief Specialization of the Rand class template for HybridVector.
// \ingroup random
//
// This specialization of the Rand class creates random instances of HybridVector.
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
class Rand<HybridVector<Type, N, TF, AF, PF, Tag>> {
  public:
	//**********************************************************************************************
	/*!\brief Generation of a random HybridVector.
	//
	// \param n The size of the random vector.
	// \return The generated random vector.
	*/
	inline const HybridVector<Type, N, TF, AF, PF, Tag> generate(size_t n) const
	{
		HybridVector<Type, N, TF, AF, PF, Tag> vector(n);
		randomize(vector);
		return vector;
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Generation of a random HybridVector.
	//
	// \param n The size of the random vector.
	// \param min The smallest possible value for a vector element.
	// \param max The largest possible value for a vector element.
	// \return The generated random vector.
	*/
	template <typename Arg> // Min/max argument type
	inline const HybridVector<Type, N, TF, AF, PF, Tag> generate(size_t n, const Arg &min, const Arg &max) const
	{
		HybridVector<Type, N, TF, AF, PF, Tag> vector(n);
		randomize(vector, min, max);
		return vector;
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Randomization of a HybridVector.
	//
	// \param vector The vector to be randomized.
	// \return void
	*/
	inline void randomize(HybridVector<Type, N, TF, AF, PF, Tag> &vector) const
	{
		using mtrc::numeric::randomize;

		for (size_t i = 0UL; i < vector.size(); ++i) {
			randomize(vector[i]);
		}
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Randomization of a HybridVector.
	//
	// \param vector The vector to be randomized.
	// \param min The smallest possible value for a vector element.
	// \param max The largest possible value for a vector element.
	// \return void
	*/
	template <typename Arg> // Min/max argument type
	inline void randomize(HybridVector<Type, N, TF, AF, PF, Tag> &vector, const Arg &min, const Arg &max) const
	{
		using mtrc::numeric::randomize;

		for (size_t i = 0UL; i < vector.size(); ++i) {
			randomize(vector[i], min, max);
		}
	}
	//**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
