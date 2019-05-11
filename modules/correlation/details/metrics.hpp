#ifndef _METRICS_MGC_HPP
#define _METRICS_MGC_HPP

#include <cmath>


namespace metric {

	template<typename Container>
	struct generic_euclidian {
		typedef typename Container::value_type T;
		static_assert(
				std::is_floating_point<T>::value, "T must be a float type");

		T operator()(const Container &a, const Container &b) const {
			T sum = 0;
			for (auto it1 = a.begin(), it2 = b.begin(); it1 != a.end() || it2 != b.end(); ++it1, ++it2) {
				sum += (*it1 - *it2) * (*it1 - *it2);
			}
			return std::sqrt(sum);
		}
	};

	template<typename Container>
	struct generic_manhatten {
		typedef typename Container::value_type T;
		static_assert(
				std::is_floating_point<T>::value, "T must be a float type");

		T operator()(const Container &a, const Container &b) const {
			T sum = 0;
			for (auto it1 = a.begin(), it2 = b.begin(); it1 != a.end() || it2 != b.end(); ++it1, ++it2) {
				sum += std::abs(*it1 - *it2);
			}
			return sum;
		}
	};

}

#endif // header guard