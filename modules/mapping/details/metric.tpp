
/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/

#pragma once

#include <type_traits>
#include <math.h>

namespace metric {

	namespace mapping {

		namespace SOM_details {
		/*	template <typename Container>
			class default_euclidian;
		}
		*/


			template <typename Container>
			struct default_euclidian {
				typedef typename Container::value_type T;

				static_assert(std::is_floating_point<T>::value, "T must be a float type");


				T operator()(const Container &a, const Container &b) const {
					T sum = 0;
					for (auto it1 = a.begin(), it2 = b.begin(); it1 != a.end() || it2 != b.end(); ++it1, ++it2) {
						sum += (*it1 - *it2) * (*it1 - *it2);
					}

					return std::sqrt(sum);
				}
			};
		}
	}
}
