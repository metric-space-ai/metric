// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_CORE_NEIGHBOR_HPP
#define _METRIC_CORE_NEIGHBOR_HPP

#include "record_id.hpp"

namespace metric::core {

template <typename Distance> struct Neighbor {
	using distance_type = Distance;

	RecordId id;
	Distance distance{};
};

} // namespace metric::core

namespace metric {
template <typename Distance> using Neighbor = core::Neighbor<Distance>;
} // namespace metric

#endif
