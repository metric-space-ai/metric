#ifndef METRIC_TRANSFORM_HOG_HPP
#define METRIC_TRANSFORM_HOG_HPP

#include <metric/numeric/Math.h>

namespace mtrc {

template <typename T> class HOG {
  public:
	using Matrix = mtrc::numeric::DynamicMatrix<T>;
	using DistanceMatrix = mtrc::numeric::SymmetricMatrix<Matrix>;
	using Vector = mtrc::numeric::DynamicVector<T>;

	HOG(const size_t orientations, const size_t cellSize, const size_t blockSize);

	Vector encode(const HOG::Matrix &image) const;

	typename HOG<T>::DistanceMatrix getGroundDistance(const mtrc::numeric::DynamicMatrix<T> &image,
													  const T rotation_cost, const T move_cost, const T threshold = 0);

  private:
	size_t orientations;
	size_t cellSize;
	size_t blockSize;

	DistanceMatrix getSpatialDistance(size_t n_hog_bins, size_t blocks_per_image_rows, size_t blocks_per_image_columns);
	typename HOG<T>::DistanceMatrix getOrientationDistance(const T angleUnitCost = 20);
};

} // namespace mtrc

#include "hog.cpp"

#endif // METRIC_TRANSFORM_HOG_HPP
