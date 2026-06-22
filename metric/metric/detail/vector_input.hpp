/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#ifndef _METRIC_DISTANCE_DETAIL_VECTOR_INPUT_HPP
#define _METRIC_DISTANCE_DETAIL_VECTOR_INPUT_HPP

#include <cstddef>
#include <stdexcept>
#include <string>

namespace mtrc::detail {

template <typename Container>
void require_aligned_vector_inputs(const Container &a, const Container &b, const char *name)
{
	if (a.size() != b.size()) {
		throw std::invalid_argument(std::string(name) + " requires inputs with equal size");
	}
}

template <typename TrainingData>
auto aligned_vector_dimension(const TrainingData &data, const char *name) -> std::size_t
{
	if (data.empty()) {
		throw std::invalid_argument(std::string(name) + " requires non-empty training data");
	}
	return data[0].size();
}

} // namespace mtrc::detail

#endif
