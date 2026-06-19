// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_CORE_PARAMETERS_HPP
#define _METRIC_CORE_PARAMETERS_HPP

#include <cstddef>

namespace metric {

struct count {
	std::size_t value{};

	constexpr auto size() const noexcept -> std::size_t { return value; }
};

} // namespace metric

#endif
