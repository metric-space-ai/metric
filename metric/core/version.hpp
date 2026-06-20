// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_CORE_VERSION_HPP
#define _METRIC_CORE_VERSION_HPP

#include <cstddef>

namespace metric::core {

using SpaceVersion = std::size_t;

inline constexpr SpaceVersion initial_space_version{0};

inline constexpr auto next_space_version(SpaceVersion version) -> SpaceVersion { return version + 1; }

} // namespace metric::core

namespace metric {
using core::initial_space_version;
using core::next_space_version;
using core::SpaceVersion;
} // namespace metric

#endif
