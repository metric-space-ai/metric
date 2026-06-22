// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_ELEMENTS_H
#define METRIC_NUMERIC_MATH_ELEMENTS_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/Aliases.h>
#include <metric/numeric/math/Exception.h>
#include <metric/numeric/math/constraints/DenseVector.h>
#include <metric/numeric/math/constraints/Elements.h>
#include <metric/numeric/math/constraints/SparseVector.h>
#include <metric/numeric/math/smp/DenseVector.h>
#include <metric/numeric/math/smp/SparseVector.h>
#include <metric/numeric/math/views/Elements.h>
#include <metric/numeric/util/Random.h>
#include <metric/numeric/util/typetraits/RemoveReference.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  RAND SPECIALIZATION FOR DENSE ELEMENT SELECTIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the Rand class template for dense element selections.
// \ingroup random
//
// This specialization of the Rand class randomizes dense element selections.
*/
template <typename VT // Type of the vector
		  ,
		  bool TF // Transpose flag
		  ,
		  typename... CEAs> // Compile time element arguments
class Rand<Elements<VT, TF, true, CEAs...>> {
  public:
	//**********************************************************************************************
	/*!\brief Randomization of a dense element selection.
	//
	// \param elements The element selection to be randomized.
	// \return void
	*/
	template <typename ET> // Type of the element selection
	inline void randomize(ET &&elements) const
	{
		using mtrc::numeric::randomize;

		using ElementsType = RemoveReference_t<ET>;

		METRIC_NUMERIC_CONSTRAINT_MUST_BE_ELEMENTS_TYPE(ElementsType);
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_DENSE_VECTOR_TYPE(ElementsType);

		for (size_t i = 0UL; i < elements.size(); ++i) {
			randomize(elements[i]);
		}
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Randomization of a dense element selection.
	//
	// \param elements The element selection to be randomized.
	// \param min The smallest possible value for an element.
	// \param max The largest possible value for an element.
	// \return void
	*/
	template <typename ET // Type of the element selection
			  ,
			  typename Arg> // Min/max argument type
	inline void randomize(ET &&elements, const Arg &min, const Arg &max) const
	{
		using mtrc::numeric::randomize;

		using ElementsType = RemoveReference_t<ET>;

		METRIC_NUMERIC_CONSTRAINT_MUST_BE_ELEMENTS_TYPE(ElementsType);
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_DENSE_VECTOR_TYPE(ElementsType);

		for (size_t i = 0UL; i < elements.size(); ++i) {
			randomize(elements[i], min, max);
		}
	}
	//**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  RAND SPECIALIZATION FOR SPARSE ELEMENT SELECTIONS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the Rand class template for sparse element selections.
// \ingroup random
//
// This specialization of the Rand class randomizes sparse element selections.
*/
template <typename VT // Type of the vector
		  ,
		  bool TF // Transpose flag
		  ,
		  typename... CEAs> // Compile time element arguments
class Rand<Elements<VT, TF, false, CEAs...>> {
  public:
	//**********************************************************************************************
	/*!\brief Randomization of a sparse element selection.
	//
	// \param elements The element selection to be randomized.
	// \return void
	*/
	template <typename ET> // Type of the element selection
	inline void randomize(ET &&elements) const
	{
		using ElementsType = RemoveReference_t<ET>;
		using ElementType = ElementType_t<ElementsType>;

		METRIC_NUMERIC_CONSTRAINT_MUST_BE_ELEMENTS_TYPE(ElementsType);
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_SPARSE_VECTOR_TYPE(ElementsType);

		const size_t size(elements.size());

		if (size == 0UL)
			return;

		const size_t nonzeros(rand<size_t>(1UL, std::ceil(0.5 * size)));

		elements.reset();
		elements.reserve(nonzeros);

		while (elements.nonZeros() < nonzeros) {
			elements[rand<size_t>(0UL, size - 1UL)] = rand<ElementType>();
		}
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Randomization of a sparse element selection.
	//
	// \param elements The element selection to be randomized.
	// \param nonzeros The number of non-zero elements of the random element selection.
	// \return void
	// \exception std::invalid_argument Invalid number of non-zero elements.
	*/
	template <typename ET> // Type of the element selection
	inline void randomize(ET &&elements, size_t nonzeros) const
	{
		using ElementsType = RemoveReference_t<ET>;
		using ElementType = ElementType_t<ElementsType>;

		METRIC_NUMERIC_CONSTRAINT_MUST_BE_ELEMENTS_TYPE(ElementsType);
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_SPARSE_VECTOR_TYPE(ElementsType);

		const size_t size(elements.size());

		if (nonzeros > size) {
			METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid number of non-zero elements");
		}

		if (size == 0UL)
			return;

		elements.reset();
		elements.reserve(nonzeros);

		while (elements.nonZeros() < nonzeros) {
			elements[rand<size_t>(0UL, size - 1UL)] = rand<ElementType>();
		}
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Randomization of a sparse element selection.
	//
	// \param elements The element selection to be randomized.
	// \param min The smallest possible value for an element.
	// \param max The largest possible value for an element.
	// \return void
	*/
	template <typename ET // Type of the element selection
			  ,
			  typename Arg> // Min/max argument type
	inline void randomize(ET &&elements, const Arg &min, const Arg &max) const
	{
		using ElementsType = RemoveReference_t<ET>;
		using ElementType = ElementType_t<ElementsType>;

		METRIC_NUMERIC_CONSTRAINT_MUST_BE_ELEMENTS_TYPE(ElementsType);
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_SPARSE_VECTOR_TYPE(ElementsType);

		const size_t size(elements.size());

		if (size == 0UL)
			return;

		const size_t nonzeros(rand<size_t>(1UL, std::ceil(0.5 * size)));

		elements.reset();
		elements.reserve(nonzeros);

		while (elements.nonZeros() < nonzeros) {
			elements[rand<size_t>(0UL, size - 1UL)] = rand<ElementType>(min, max);
		}
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Randomization of a sparse element selection.
	//
	// \param elements The element selection to be randomized.
	// \param nonzeros The number of non-zero elements of the random element selection.
	// \param min The smallest possible value for an element.
	// \param max The largest possible value for an element.
	// \return void
	// \exception std::invalid_argument Invalid number of non-zero elements.
	*/
	template <typename ET // Type of the element selection
			  ,
			  typename Arg> // Min/max argument type
	inline void randomize(ET &&elements, size_t nonzeros, const Arg &min, const Arg &max) const
	{
		using ElementsType = RemoveReference_t<ET>;
		using ElementType = ElementType_t<ElementsType>;

		METRIC_NUMERIC_CONSTRAINT_MUST_BE_ELEMENTS_TYPE(ElementsType);
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_SPARSE_VECTOR_TYPE(ElementsType);

		const size_t size(elements.size());

		if (nonzeros > size) {
			METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid number of non-zero elements");
		}

		if (size == 0UL)
			return;

		elements.reset();
		elements.reserve(nonzeros);

		while (elements.nonZeros() < nonzeros) {
			elements[rand<size_t>(0UL, size - 1UL)] = rand<ElementType>(min, max);
		}
	}
	//**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
