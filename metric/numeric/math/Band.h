// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_BAND_H
#define METRIC_NUMERIC_MATH_BAND_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/Aliases.h>
#include <metric/numeric/math/Exception.h>
#include <metric/numeric/math/constraints/Band.h>
#include <metric/numeric/math/constraints/DenseVector.h>
#include <metric/numeric/math/constraints/SparseVector.h>
#include <metric/numeric/math/smp/DenseVector.h>
#include <metric/numeric/math/smp/SparseVector.h>
#include <metric/numeric/math/views/Band.h>
#include <metric/numeric/util/Random.h>
#include <metric/numeric/util/typetraits/RemoveReference.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  RAND SPECIALIZATION FOR DENSE BANDS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the Rand class template for dense bands.
// \ingroup random
//
// This specialization of the Rand class randomizes dense bands.
*/
template <typename MT // Type of the matrix
		  ,
		  bool TF // Transpose flag
		  ,
		  bool MF // Multiplication flag
		  ,
		  ptrdiff_t... CBAs> // Compile time band arguments
class Rand<Band<MT, TF, true, MF, CBAs...>> {
  public:
	//**********************************************************************************************
	/*!\brief Randomization of a dense band.
	//
	// \param band The band to be randomized.
	// \return void
	*/
	template <typename BT> // Type of the band
	inline void randomize(BT &&band) const
	{
		using mtrc::numeric::randomize;

		using BandType = RemoveReference_t<BT>;

		METRIC_NUMERIC_CONSTRAINT_MUST_BE_BAND_TYPE(BandType);
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_DENSE_VECTOR_TYPE(BandType);

		for (size_t i = 0UL; i < band.size(); ++i) {
			randomize(band[i]);
		}
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Randomization of a dense band.
	//
	// \param band The band to be randomized.
	// \param min The smallest possible value for a band element.
	// \param max The largest possible value for a band element.
	// \return void
	*/
	template <typename BT // Type of the band
			  ,
			  typename Arg> // Min/max argument type
	inline void randomize(BT &&band, const Arg &min, const Arg &max) const
	{
		using mtrc::numeric::randomize;

		using BandType = RemoveReference_t<BT>;

		METRIC_NUMERIC_CONSTRAINT_MUST_BE_BAND_TYPE(BandType);
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_DENSE_VECTOR_TYPE(BandType);

		for (size_t i = 0UL; i < band.size(); ++i) {
			randomize(band[i], min, max);
		}
	}
	//**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  RAND SPECIALIZATION FOR SPARSE BANDS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the Rand class template for sparse bands.
// \ingroup random
//
// This specialization of the Rand class randomizes sparse bands.
*/
template <typename MT // Type of the matrix
		  ,
		  bool TF // Transpose flag
		  ,
		  bool MF // Multiplication flag
		  ,
		  ptrdiff_t... CBAs> // Compile time band arguments
class Rand<Band<MT, TF, false, MF, CBAs...>> {
  public:
	//**********************************************************************************************
	/*!\brief Randomization of a sparse band.
	//
	// \param band The band to be randomized.
	// \return void
	*/
	template <typename BT> // Type of the band
	inline void randomize(BT &&band) const
	{
		using BandType = RemoveReference_t<BT>;
		using ElementType = ElementType_t<BandType>;

		METRIC_NUMERIC_CONSTRAINT_MUST_BE_BAND_TYPE(BandType);
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_SPARSE_VECTOR_TYPE(BandType);

		const size_t size(band.size());

		if (size == 0UL)
			return;

		const size_t nonzeros(rand<size_t>(1UL, std::ceil(0.5 * size)));

		band.reset();
		band.reserve(nonzeros);

		while (band.nonZeros() < nonzeros) {
			band[rand<size_t>(0UL, size - 1UL)] = rand<ElementType>();
		}
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Randomization of a sparse band.
	//
	// \param band The band to be randomized.
	// \param nonzeros The number of non-zero elements of the random band.
	// \return void
	// \exception std::invalid_argument Invalid number of non-zero elements.
	*/
	template <typename BT> // Type of the band
	inline void randomize(BT &&band, size_t nonzeros) const
	{
		using BandType = RemoveReference_t<BT>;
		using ElementType = ElementType_t<BandType>;

		METRIC_NUMERIC_CONSTRAINT_MUST_BE_BAND_TYPE(BandType);
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_SPARSE_VECTOR_TYPE(BandType);

		const size_t size(band.size());

		if (nonzeros > size) {
			METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid number of non-zero elements");
		}

		if (size == 0UL)
			return;

		band.reset();
		band.reserve(nonzeros);

		while (band.nonZeros() < nonzeros) {
			band[rand<size_t>(0UL, size - 1UL)] = rand<ElementType>();
		}
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Randomization of a sparse band.
	//
	// \param band The band to be randomized.
	// \param min The smallest possible value for a band element.
	// \param max The largest possible value for a band element.
	// \return void
	*/
	template <typename BT // Type of the band
			  ,
			  typename Arg> // Min/max argument type
	inline void randomize(BT &&band, const Arg &min, const Arg &max) const
	{
		using BandType = RemoveReference_t<BT>;
		using ElementType = ElementType_t<BandType>;

		METRIC_NUMERIC_CONSTRAINT_MUST_BE_BAND_TYPE(BandType);
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_SPARSE_VECTOR_TYPE(BandType);

		const size_t size(band.size());

		if (size == 0UL)
			return;

		const size_t nonzeros(rand<size_t>(1UL, std::ceil(0.5 * size)));

		band.reset();
		band.reserve(nonzeros);

		while (band.nonZeros() < nonzeros) {
			band[rand<size_t>(0UL, size - 1UL)] = rand<ElementType>(min, max);
		}
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Randomization of a sparse band.
	//
	// \param band The band to be randomized.
	// \param nonzeros The number of non-zero elements of the random band.
	// \param min The smallest possible value for a band element.
	// \param max The largest possible value for a band element.
	// \return void
	// \exception std::invalid_argument Invalid number of non-zero elements.
	*/
	template <typename BT // Type of the band
			  ,
			  typename Arg> // Min/max argument type
	inline void randomize(BT &&band, size_t nonzeros, const Arg &min, const Arg &max) const
	{
		using BandType = RemoveReference_t<BT>;
		using ElementType = ElementType_t<BandType>;

		METRIC_NUMERIC_CONSTRAINT_MUST_BE_BAND_TYPE(BandType);
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_SPARSE_VECTOR_TYPE(BandType);

		const size_t size(band.size());

		if (nonzeros > size) {
			METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid number of non-zero elements");
		}

		if (size == 0UL)
			return;

		band.reset();
		band.reserve(nonzeros);

		while (band.nonZeros() < nonzeros) {
			band[rand<size_t>(0UL, size - 1UL)] = rand<ElementType>(min, max);
		}
	}
	//**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
