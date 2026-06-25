// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_MAPPINGS_MAPPING_HPP
#define _METRIC_MAPPINGS_MAPPING_HPP

#include <metric/core/concepts.hpp>

namespace mtrc::modify::map {

template <typename Mapping, typename Space, typename std::enable_if<Mapping_v<Mapping, Space>, int>::type = 0>
auto derive_from(const Mapping &mapping, const Space &space) -> decltype(mapping.derive_from(space))
{
	return mapping.derive_from(space);
}

template <typename Transform, typename Space, typename std::enable_if<DerivedSpaceTransform_v<Transform, Space>, int>::type = 0>
auto transform(const Transform &mapping_artifact, const Space &space) -> decltype(mapping_artifact.transform(space))
{
	return mapping_artifact.transform(space);
}

} // namespace mtrc::modify::map

#endif
