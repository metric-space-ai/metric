// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_COLUMN_H
#define METRIC_NUMERIC_MATH_COLUMN_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/math/Aliases.h>
#include <metric/numeric/math/Exception.h>
#include <metric/numeric/math/constraints/Column.h>
#include <metric/numeric/math/constraints/DenseVector.h>
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
//  RAND SPECIALIZATION FOR DENSE COLUMNS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the Rand class template for dense columns.
// \ingroup random
//
// This specialization of the Rand class randomizes dense columns.
*/
template <typename MT // Type of the matrix
		  ,
		  bool SO // Storage order
		  ,
		  bool SF // Symmetry flag
		  ,
		  size_t... CCAs> // Compile time column arguments
class Rand<Column<MT, SO, true, SF, CCAs...>> {
  public:
	//*************************************************************************************************
	/*!\brief Randomization of a dense column.
	//
	// \param column The column to be randomized.
	// \return void
	*/
	template <typename CT> // Type of the column
	inline void randomize(CT &&column) const
	{
		using mtrc::numeric::randomize;

		using ColumnType = RemoveReference_t<CT>;

		METRIC_NUMERIC_CONSTRAINT_MUST_BE_COLUMN_TYPE(ColumnType);
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_DENSE_VECTOR_TYPE(ColumnType);

		for (size_t i = 0UL; i < column.size(); ++i) {
			randomize(column[i]);
		}
	}
	//*************************************************************************************************

	//*************************************************************************************************
	/*!\brief Randomization of a dense column.
	//
	// \param column The column to be randomized.
	// \param min The smallest possible value for a column element.
	// \param max The largest possible value for a column element.
	// \return void
	*/
	template <typename CT // Type of the column
			  ,
			  typename Arg> // Min/max argument type
	inline void randomize(CT &&column, const Arg &min, const Arg &max) const
	{
		using mtrc::numeric::randomize;

		using ColumnType = RemoveReference_t<CT>;

		METRIC_NUMERIC_CONSTRAINT_MUST_BE_COLUMN_TYPE(ColumnType);
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_DENSE_VECTOR_TYPE(ColumnType);

		for (size_t i = 0UL; i < column.size(); ++i) {
			randomize(column[i], min, max);
		}
	}
	//*************************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

//=================================================================================================
//
//  RAND SPECIALIZATION FOR SPARSE COLUMNS
//
//=================================================================================================

//*************************************************************************************************
/*! \cond METRIC_NUMERIC_INTERNAL */
/*!\brief Specialization of the Rand class template for sparse columns.
// \ingroup random
//
// This specialization of the Rand class randomizes sparse columns.
*/
template <typename MT // Type of the matrix
		  ,
		  bool SO // Storage order
		  ,
		  bool SF // Symmetry flag
		  ,
		  size_t... CCAs> // Compile time column arguments
class Rand<Column<MT, SO, false, SF, CCAs...>> {
  public:
	//*************************************************************************************************
	/*!\brief Randomization of a sparse column.
	//
	// \param column The column to be randomized.
	// \return void
	*/
	template <typename CT> // Type of the column
	inline void randomize(CT &&column) const
	{
		using ColumnType = RemoveReference_t<CT>;
		using ElementType = ElementType_t<ColumnType>;

		METRIC_NUMERIC_CONSTRAINT_MUST_BE_COLUMN_TYPE(ColumnType);
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_SPARSE_VECTOR_TYPE(ColumnType);

		const size_t size(column.size());

		if (size == 0UL)
			return;

		const size_t nonzeros(rand<size_t>(1UL, std::ceil(0.5 * size)));

		column.reset();
		column.reserve(nonzeros);

		while (column.nonZeros() < nonzeros) {
			column[rand<size_t>(0UL, size - 1UL)] = rand<ElementType>();
		}
	}
	//*************************************************************************************************

	//*************************************************************************************************
	/*!\brief Randomization of a sparse column.
	//
	// \param column The column to be randomized.
	// \param nonzeros The number of non-zero elements of the random column.
	// \return void
	// \exception std::invalid_argument Invalid number of non-zero elements.
	*/
	template <typename CT> // Type of the column
	inline void randomize(CT &&column, size_t nonzeros) const
	{
		using ColumnType = RemoveReference_t<CT>;
		using ElementType = ElementType_t<ColumnType>;

		METRIC_NUMERIC_CONSTRAINT_MUST_BE_COLUMN_TYPE(ColumnType);
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_SPARSE_VECTOR_TYPE(ColumnType);

		const size_t size(column.size());

		if (nonzeros > size) {
			METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid number of non-zero elements");
		}

		if (size == 0UL)
			return;

		column.reset();
		column.reserve(nonzeros);

		while (column.nonZeros() < nonzeros) {
			column[rand<size_t>(0UL, size - 1UL)] = rand<ElementType>();
		}
	}
	//*************************************************************************************************

	//*************************************************************************************************
	/*!\brief Randomization of a sparse column.
	//
	// \param column The column to be randomized.
	// \param min The smallest possible value for a column element.
	// \param max The largest possible value for a column element.
	// \return void
	*/
	template <typename CT // Type of the column
			  ,
			  typename Arg> // Min/max argument type
	inline void randomize(CT &&column, const Arg &min, const Arg &max) const
	{
		using ColumnType = RemoveReference_t<CT>;
		using ElementType = ElementType_t<ColumnType>;

		METRIC_NUMERIC_CONSTRAINT_MUST_BE_COLUMN_TYPE(ColumnType);
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_SPARSE_VECTOR_TYPE(ColumnType);

		const size_t size(column.size());

		if (size == 0UL)
			return;

		const size_t nonzeros(rand<size_t>(1UL, std::ceil(0.5 * size)));

		column.reset();
		column.reserve(nonzeros);

		while (column.nonZeros() < nonzeros) {
			column[rand<size_t>(0UL, size - 1UL)] = rand<ElementType>(min, max);
		}
	}
	//*************************************************************************************************

	//*************************************************************************************************
	/*!\brief Randomization of a sparse column.
	//
	// \param column The column to be randomized.
	// \param nonzeros The number of non-zero elements of the random column.
	// \param min The smallest possible value for a column element.
	// \param max The largest possible value for a column element.
	// \return void
	// \exception std::invalid_argument Invalid number of non-zero elements.
	*/
	template <typename CT // Type of the column
			  ,
			  typename Arg> // Min/max argument type
	inline void randomize(CT &&column, size_t nonzeros, const Arg &min, const Arg &max) const
	{
		using ColumnType = RemoveReference_t<CT>;
		using ElementType = ElementType_t<ColumnType>;

		METRIC_NUMERIC_CONSTRAINT_MUST_BE_COLUMN_TYPE(ColumnType);
		METRIC_NUMERIC_CONSTRAINT_MUST_BE_SPARSE_VECTOR_TYPE(ColumnType);

		const size_t size(column.size());

		if (nonzeros > size) {
			METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid number of non-zero elements");
		}

		if (size == 0UL)
			return;

		column.reset();
		column.reserve(nonzeros);

		while (column.nonZeros() < nonzeros) {
			column[rand<size_t>(0UL, size - 1UL)] = rand<ElementType>(min, max);
		}
	}
	//*************************************************************************************************
};
/*! \endcond */
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
