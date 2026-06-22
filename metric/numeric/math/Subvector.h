// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_SUBVECTOR_H
#define METRIC_NUMERIC_MATH_SUBVECTOR_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/Aliases.h>
#include <metric/numeric/math/Exception.h>
#include <metric/numeric/math/constraints/DenseVector.h>
#include <metric/numeric/math/constraints/SparseVector.h>
#include <metric/numeric/math/constraints/Subvector.h>
#include <metric/numeric/math/smp/DenseVector.h>
#include <metric/numeric/math/smp/SparseVector.h>
#include <metric/numeric/math/views/Submatrix.h>
#include <metric/numeric/math/views/Subvector.h>
#include <metric/numeric/util/Random.h>
#include <metric/numeric/util/typetraits/RemoveReference.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  RAND SPECIALIZATION FOR DENSE SUBVECTORS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the Rand class template for dense subvectors.
// \ingroup random
//
// This specialization of the Rand class randomizes dense subvectors.
*/
template <typename VT // Type of the vector
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  bool TF // Transpose flag
		  ,
		  size_t... CSAs> // Compile time subvector arguments
class Rand<Subvector<VT, AF, TF, true, CSAs...>> {
  public:
	//**********************************************************************************************
	/*!\brief Randomization of a dense subvector.
	//
	// \param subvector The subvector to be randomized.
	// \return void
	*/
	template <typename SVT> // Type of the subvector
	inline void randomize(SVT &&subvector) const
	{
		using mtrc::numeric::randomize;

		using SubvectorType = RemoveReference_t<SVT>;

		METRIC_NUMERIC_CONSTRAINT_MUST_BE_SUBVECTOR_TYPE(SubvectorType);
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_DENSE_VECTOR_TYPE(SubvectorType);

		for (size_t i = 0UL; i < subvector.size(); ++i) {
			randomize(subvector[i]);
		}
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Randomization of a dense subvector.
	//
	// \param subvector The subvector to be randomized.
	// \param min The smallest possible value for a subvector element.
	// \param max The largest possible value for a subvector element.
	// \return void
	*/
	template <typename SVT // Type of the subvector
			  ,
			  typename Arg> // Min/max argument type
	inline void randomize(SVT &&subvector, const Arg &min, const Arg &max) const
	{
		using mtrc::numeric::randomize;

		using SubvectorType = RemoveReference_t<SVT>;

		METRIC_NUMERIC_CONSTRAINT_MUST_BE_SUBVECTOR_TYPE(SubvectorType);
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_DENSE_VECTOR_TYPE(SubvectorType);

		for (size_t i = 0UL; i < subvector.size(); ++i) {
			randomize(subvector[i], min, max);
		}
	}
	//**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  RAND SPECIALIZATION FOR SPARSE SUBVECTORS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the Rand class template for sparse subvectors.
// \ingroup random
//
// This specialization of the Rand class randomizes sparse subvectors.
*/
template <typename VT // Type of the vector
		  ,
		  AlignmentFlag AF // Alignment flag
		  ,
		  bool TF // Transpose flag
		  ,
		  size_t... CSAs> // Compile time subvector arguments
class Rand<Subvector<VT, AF, TF, false, CSAs...>> {
  public:
	//**********************************************************************************************
	/*!\brief Randomization of a sparse subvector.
	//
	// \param subvector The subvector to be randomized.
	// \return void
	*/
	template <typename SVT> // Type of the subvector
	inline void randomize(SVT &&subvector) const
	{
		using SubvectorType = RemoveReference_t<SVT>;
		using ElementType = ElementType_t<SubvectorType>;

		METRIC_NUMERIC_CONSTRAINT_MUST_BE_SUBVECTOR_TYPE(SubvectorType);
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_SPARSE_VECTOR_TYPE(SubvectorType);

		const size_t size(subvector.size());

		if (size == 0UL)
			return;

		const size_t nonzeros(rand<size_t>(1UL, std::ceil(0.5 * size)));

		subvector.reset();
		subvector.reserve(nonzeros);

		while (subvector.nonZeros() < nonzeros) {
			subvector[rand<size_t>(0UL, size - 1UL)] = rand<ElementType>();
		}
	}
	//*************************************************************************************************

	//*************************************************************************************************
	/*!\brief Randomization of a sparse subvector.
	//
	// \param subvector The subvector to be randomized.
	// \param nonzeros The number of non-zero elements of the random subvector.
	// \return void
	// \exception std::invalid_argument Invalid number of non-zero elements.
	*/
	template <typename SVT> // Type of the subvector
	inline void randomize(SVT &&subvector, size_t nonzeros) const
	{
		using SubvectorType = RemoveReference_t<SVT>;
		using ElementType = ElementType_t<SubvectorType>;

		METRIC_NUMERIC_CONSTRAINT_MUST_BE_SUBVECTOR_TYPE(SubvectorType);
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_SPARSE_VECTOR_TYPE(SubvectorType);

		const size_t size(subvector.size());

		if (nonzeros > size) {
			METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid number of non-zero elements");
		}

		if (size == 0UL)
			return;

		subvector.reset();
		subvector.reserve(nonzeros);

		while (subvector.nonZeros() < nonzeros) {
			subvector[rand<size_t>(0UL, size - 1UL)] = rand<ElementType>();
		}
	}
	//*************************************************************************************************

	//*************************************************************************************************
	/*!\brief Randomization of a sparse subvector.
	//
	// \param subvector The subvector to be randomized.
	// \param min The smallest possible value for a subvector element.
	// \param max The largest possible value for a subvector element.
	// \return void
	*/
	template <typename SVT // Type of the subvector
			  ,
			  typename Arg> // Min/max argument type
	inline void randomize(SVT &&subvector, const Arg &min, const Arg &max) const
	{
		using SubvectorType = RemoveReference_t<SVT>;
		using ElementType = ElementType_t<SubvectorType>;

		METRIC_NUMERIC_CONSTRAINT_MUST_BE_SUBVECTOR_TYPE(SubvectorType);
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_SPARSE_VECTOR_TYPE(SubvectorType);

		const size_t size(subvector.size());

		if (size == 0UL)
			return;

		const size_t nonzeros(rand<size_t>(1UL, std::ceil(0.5 * size)));

		subvector.reset();
		subvector.reserve(nonzeros);

		while (subvector.nonZeros() < nonzeros) {
			subvector[rand<size_t>(0UL, size - 1UL)] = rand<ElementType>(min, max);
		}
	}
	//*************************************************************************************************

	//*************************************************************************************************
	/*!\brief Randomization of a sparse subvector.
	//
	// \param subvector The subvector to be randomized.
	// \param nonzeros The number of non-zero elements of the random subvector.
	// \param min The smallest possible value for a subvector element.
	// \param max The largest possible value for a subvector element.
	// \return void
	// \exception std::invalid_argument Invalid number of non-zero elements.
	*/
	template <typename SVT // Type of the subvector
			  ,
			  typename Arg> // Min/max argument type
	inline void randomize(SVT &&subvector, size_t nonzeros, const Arg &min, const Arg &max) const
	{
		using SubvectorType = RemoveReference_t<SVT>;
		using ElementType = ElementType_t<SubvectorType>;

		METRIC_NUMERIC_CONSTRAINT_MUST_BE_SUBVECTOR_TYPE(SubvectorType);
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_SPARSE_VECTOR_TYPE(SubvectorType);

		const size_t size(subvector.size());

		if (nonzeros > size) {
			METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid number of non-zero elements");
		}

		if (size == 0UL)
			return;

		subvector.reset();
		subvector.reserve(nonzeros);

		while (subvector.nonZeros() < nonzeros) {
			subvector[rand<size_t>(0UL, size - 1UL)] = rand<ElementType>(min, max);
		}
	}
	//*************************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
