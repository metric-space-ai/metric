// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_MAPPINGS_MAPPING_HPP
#define _METRIC_MAPPINGS_MAPPING_HPP

#include "../core/concepts.hpp"

namespace metric::mappings {

template <typename Mapping, typename Space, typename std::enable_if<Mapping_v<Mapping, Space>, int>::type = 0>
auto fit(const Mapping &mapping, const Space &space) -> decltype(mapping.fit(space))
{
	return mapping.fit(space);
}

template <typename Model, typename Space, typename std::enable_if<MappingModel_v<Model, Space>, int>::type = 0>
auto transform(const Model &model, const Space &space) -> decltype(model.transform(space))
{
	return model.transform(space);
}

} // namespace metric::mappings

#endif
