// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_INDICES_H
#define METRIC_NUMERIC_UTIL_INDICES_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <algorithm>
#include <metric/numeric/util/Exception.h>
#include <metric/numeric/util/Random.h>
#include <metric/numeric/util/Types.h>
#include <numeric>
#include <stdexcept>
#include <vector>

namespace mtrc::numeric {

//=================================================================================================
//
//  CLASS DEFINITION
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Auxiliary class for the generation of random indices.
//
// This auxiliary class can be used to generate a set of random indices.
*/
template <typename T> // Type of the indices
class Indices {
  public:
	//**Type definitions****************************************************************************
	//! Iterator over the generated indices.
	using ConstIterator = typename std::vector<T>::const_iterator;
	//**********************************************************************************************

	//**Constructors********************************************************************************
	/*!\name Constructors */
	//@{
	inline Indices(T min, T max, T number);
	//@}
	//**********************************************************************************************

	//**Utility functions***************************************************************************
	/*!\name Utility functions */
	//@{
	inline size_t size() const noexcept;
	inline ConstIterator begin() const noexcept;
	inline ConstIterator end() const noexcept;
	//@}
	//**********************************************************************************************

  private:
	//**Member variables****************************************************************************
	/*!\name Member variables */
	//@{
	std::vector<T> indices_; //!< The generated indices.
							 //@}
	//**********************************************************************************************
};
//*************************************************************************************************

//=================================================================================================
//
//  CONSTRUCTORS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief The constructor for the Indices class.
//
// \param min The lower limit of the random indices.
// \param max The upper limit of the random indices.
// \param number The number of random indices to generate.
// \exception std::invalid_argument Invalid index range.
// \exception std::invalid_argument Invalid number of indices.
//
// This constructor initializes an Indices object by generating \a number random, unique indices
// in the range \a min to \a max. In case \a number is larger than the possible number of incides
// in the specified range, a \a std::invalid_argument exception is thrown.
*/
template <typename T>											// Type of the indices
inline Indices<T>::Indices(T min, T max, T number) : indices_() // The generated indices
{
	if (max < min) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid index range");
	}

	const T maxNumber(max + T(1) - min);

	if (number > maxNumber) {
		METRIC_NUMERIC_THROW_INVALID_ARGUMENT("Invalid number of indices");
	}

	if (number == 0UL) {
		return;
	}

	if (number <= T(maxNumber * 0.5)) {
		indices_.reserve(number);

		while (indices_.size() < number) {
			const T value = rand<T>(min, max);
			METRIC_NUMERIC_INTERNAL_ASSERT(min <= value && value <= max, "Invalid index detected");
			const auto pos = std::lower_bound(indices_.begin(), indices_.end(), value);

			if (pos == indices_.end() || *pos != value) {
				indices_.insert(pos, value);
			}
		}
	} else {
		indices_.resize(maxNumber);
		std::iota(indices_.begin(), indices_.end(), min);

		while (indices_.size() > number) {
			const T value = rand<T>(min, max);
			METRIC_NUMERIC_INTERNAL_ASSERT(min <= value && value <= max, "Invalid index detected");
			const auto pos = std::lower_bound(indices_.begin(), indices_.end(), value);

			if (pos != indices_.end() && *pos == value) {
				indices_.erase(pos);
			}
		}
	}
}
//*************************************************************************************************

//=================================================================================================
//
//  UTILITY FUNCTIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Returns the total number of random indices.
//
// \return The total number of random indices.
*/
template <typename T> // Type of the indices
inline size_t Indices<T>::size() const noexcept
{
	return indices_.size();
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Returns an iterator to the beginning of the vector.
//
// \return Iterator to the beginning of the vector.
*/
template <typename T> // Type of the indices
inline typename Indices<T>::ConstIterator Indices<T>::begin() const noexcept
{
	return indices_.begin();
}
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Returns an iterator just past the last element of the vector.
//
// \return Iterator just past the last element of the vector.
*/
template <typename T> // Type of the indices
inline typename Indices<T>::ConstIterator Indices<T>::end() const noexcept
{
	return indices_.end();
}
//*************************************************************************************************

} // namespace mtrc::numeric

#endif
