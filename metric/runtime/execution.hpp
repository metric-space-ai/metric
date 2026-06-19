// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_RUNTIME_EXECUTION_HPP
#define _METRIC_RUNTIME_EXECUTION_HPP

#include <stdexcept>
#include <string>

#include "policy.hpp"

namespace metric::runtime {

inline auto require_exact_neighbors(policy runtime_policy) -> void
{
	if (!runtime_policy.is_exact()) {
		throw std::invalid_argument("approximate neighbor runtime policy is not implemented");
	}
}

inline auto neighbor_representation(policy runtime_policy) -> std::string
{
	return runtime_policy.uses_materialization() ? "matrix_cache" : "metric_space";
}

} // namespace metric::runtime

#endif
