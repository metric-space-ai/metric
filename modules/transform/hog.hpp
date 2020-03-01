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
			Vector compute(const HOG::Matrix &image) const;

		private:
			size_t orientations;
			size_t cellSize;
			size_t blockSize;

	};

}

#include "hog.cpp"

#endif //PANDA_METRIC_HOG_HPP
