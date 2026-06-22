// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_ROW_H
#define METRIC_NUMERIC_MATH_ROW_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/Aliases.h>
#include <metric/numeric/math/Exception.h>
#include <metric/numeric/math/constraints/DenseVector.h>
#include <metric/numeric/math/constraints/Row.h>
#include <metric/numeric/math/constraints/SparseVector.h>
#include <metric/numeric/math/smp/DenseVector.h>
#include <metric/numeric/math/smp/SparseVector.h>
#include <metric/numeric/math/views/Column.h>
#include <metric/numeric/math/views/Row.h>
#include <metric/numeric/util/Random.h>
#include <metric/numeric/util/typetraits/RemoveReference.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  RAND SPECIALIZATION FOR DENSE ROWS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the Rand class template for dense rows.
// \ingroup random
//
// This specialization of the Rand class randomizes dense rows.
*/
template <typename MT // Type of the matrix
		  ,
		  bool SO // Storage order
		  ,
		  bool SF // Symmetry flag
		  ,
		  size_t... CRAs> // Compile time row arguments
class Rand<Row<MT, SO, true, SF, CRAs...>> {
  public:
	//**********************************************************************************************
	/*!\brief Randomization of a dense row.
	//
	// \param row The row to be randomized.
	// \return void
	*/
	template <typename RT> // Type of the row
	inline void randomize(RT &&row) const
	{
		using mtrc::numeric::randomize;

		using RowType = RemoveReference_t<RT>;

		METRIC_NUMERIC_CONSTRAINT_MUST_BE_ROW_TYPE(RowType);
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_DENSE_VECTOR_TYPE(RowType);

		for (size_t i = 0UL; i < row.size(); ++i) {
			randomize(row[i]);
		}
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Randomization of a dense row.
	//
	// \param row The row to be randomized.
	// \param min The smallest possible value for a row element.
	// \param max The largest possible value for a row element.
	// \return void
	*/
	template <typename RT // Type of the row
			  ,
			  typename Arg> // Min/max argument type
	inline void randomize(RT &&row, const Arg &min, const Arg &max) const
	{
		using mtrc::numeric::randomize;

		using RowType = RemoveReference_t<RT>;

		METRIC_NUMERIC_CONSTRAINT_MUST_BE_ROW_TYPE(RowType);
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_DENSE_VECTOR_TYPE(RowType);

		for (size_t i = 0UL; i < row.size(); ++i) {
			randomize(row[i], min, max);
		}
	}
	//**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  RAND SPECIALIZATION FOR SPARSE ROWS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the Rand class template for sparse rows.
// \ingroup random
//
// This specialization of the Rand class randomizes sparse rows.
*/
template <typename MT // Type of the matrix
		  ,
		  bool SO // Storage order
		  ,
		  bool SF // Symmetry flag
		  ,
		  size_t... CRAs> // Compile time row arguments
class Rand<Row<MT, SO, false, SF, CRAs...>> {
  public:
	//**********************************************************************************************
	/*!\brief Randomization of a sparse row.
	//
	// \param row The row to be randomized.
	// \return void
	*/
	template <typename RT> // Type of the row
	inline void randomize(RT &&row) const
	{
		using RowType = RemoveReference_t<RT>;
		using ElementType = ElementType_t<RowType>;

		METRIC_NUMERIC_CONSTRAINT_MUST_BE_ROW_TYPE(RowType);
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_SPARSE_VECTOR_TYPE(RowType);

		const size_t size(row.size());

		if (size == 0UL)
			return;

		const size_t nonzeros(rand<size_t>(1UL, std::ceil(0.5 * size)));

		row.reset();
		row.reserve(nonzeros);

		while (row.nonZeros() < nonzeros) {
			row[rand<size_t>(0UL, size - 1UL)] = rand<ElementType>();
		}
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Randomization of a sparse row.
	//
	// \param row The row to be randomized.
	// \param nonzeros The number of non-zero elements of the random row.
	// \return void
	// \exception std::invalid_argument Invalid number of non-zero elements.
	*/
	template <typename RT> // Type of the row
	inline void randomize(RT &&row, size_t nonzeros) const
	{
		using RowType = RemoveReference_t<RT>;
		using ElementType = ElementType_t<RowType>;

		METRIC_NUMERIC_CONSTRAINT_MUST_BE_ROW_TYPE(RowType);
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_SPARSE_VECTOR_TYPE(RowType);

		const size_t size(row.size());

		if (nonzeros > size) {
			METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid number of non-zero elements");
		}

		if (size == 0UL)
			return;

		row.reset();
		row.reserve(nonzeros);

		while (row.nonZeros() < nonzeros) {
			row[rand<size_t>(0UL, size - 1UL)] = rand<ElementType>();
		}
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Randomization of a sparse row.
	//
	// \param row The row to be randomized.
	// \param min The smallest possible value for a row element.
	// \param max The largest possible value for a row element.
	// \return void
	*/
	template <typename RT // Type of the row
			  ,
			  typename Arg> // Min/max argument type
	inline void randomize(RT &&row, const Arg &min, const Arg &max) const
	{
		using RowType = RemoveReference_t<RT>;
		using ElementType = ElementType_t<RowType>;

		METRIC_NUMERIC_CONSTRAINT_MUST_BE_ROW_TYPE(RowType);
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_SPARSE_VECTOR_TYPE(RowType);

		const size_t size(row.size());

		if (size == 0UL)
			return;

		const size_t nonzeros(rand<size_t>(1UL, std::ceil(0.5 * size)));

		row.reset();
		row.reserve(nonzeros);

		while (row.nonZeros() < nonzeros) {
			row[rand<size_t>(0UL, size - 1UL)] = rand<ElementType>(min, max);
		}
	}
	//**********************************************************************************************

	//**********************************************************************************************
	/*!\brief Randomization of a sparse row.
	//
	// \param row The row to be randomized.
	// \param nonzeros The number of non-zero elements of the random row.
	// \param min The smallest possible value for a row element.
	// \param max The largest possible value for a row element.
	// \return void
	// \exception std::invalid_argument Invalid number of non-zero elements.
	*/
	template <typename RT // Type of the row
			  ,
			  typename Arg> // Min/max argument type
	inline void randomize(RT &&row, size_t nonzeros, const Arg &min, const Arg &max) const
	{
		using RowType = RemoveReference_t<RT>;
		using ElementType = ElementType_t<RowType>;

		METRIC_NUMERIC_CONSTRAINT_MUST_BE_ROW_TYPE(RowType);
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_SPARSE_VECTOR_TYPE(RowType);

		const size_t size(row.size());

		if (nonzeros > size) {
			METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid number of non-zero elements");
		}

		if (size == 0UL)
			return;

		row.reset();
		row.reserve(nonzeros);

		while (row.nonZeros() < nonzeros) {
			row[rand<size_t>(0UL, size - 1UL)] = rand<ElementType>(min, max);
		}
	}
	//**********************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
