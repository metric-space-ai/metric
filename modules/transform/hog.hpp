#ifndef PANDA_METRIC_HOG_HPP
#define PANDA_METRIC_HOG_HPP

#include "../../3rdparty/blaze/Math.h"


namespace metric {

	template<typename T>
	class HOG {
		public:
			using Matrix = blaze::DynamicMatrix<T>;
			using Vector = blaze::DynamicVector<T>;

			HOG(const size_t orientations, const size_t cellSize, const size_t blockSize);

			Vector encode(const HOG::Matrix &image) const;

			blaze::DynamicMatrix<T> groundDistance(blaze::DynamicMatrix<T> image, T rotation_cost, T move_cost);

		private:
			size_t orientations;
			size_t cellSize;
			size_t blockSize;

			Matrix spatial_dist(size_t n_hog_bins, size_t orientations, size_t blockSize, size_t blocks_per_image_rows, size_t blocks_per_image_columns);
			Matrix rotation_dist(size_t orientations, bool isSigned = false);
	};

}

#include "hog.cpp"

#endif //PANDA_METRIC_HOG_HPP
