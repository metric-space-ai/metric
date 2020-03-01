#ifndef PANDA_METRIC_HOG_HPP
#define PANDA_METRIC_HOG_HPP

#include "../../3rdparty/blaze/Math.h"


namespace metric {
	template<typename T>
	class HOG {
		public:
			HOG();
			compute(Matrix& image);

	private:
		using Matrix = blaze::DynamicMatrix<T>;
	};
}

#endif //PANDA_METRIC_HOG_HPP
