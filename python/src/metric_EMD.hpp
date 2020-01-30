/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2020 Panda Team
*/

namespace metric_EMD {

class EMD_detailsScope {};

template <class T>
std::vector<std::vector<T>>(*ground_distance_matrix_of_2dgrid_1)(size_t,size_t) = &metric::EMD_details::ground_distance_matrix_of_2dgrid<T>;

template <class T>
std::vector<std::vector<T>>(*ground_distance_matrix_of_2dgrid_2)(std::vector<std::vector<T>>) = &metric::EMD_details::ground_distance_matrix_of_2dgrid<T>;

};
