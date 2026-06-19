// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_RUNTIME_EXECUTION_HPP
#define _METRIC_RUNTIME_EXECUTION_HPP

#include <stdexcept>
#include <string>
#include <utility>

#include "policy.hpp"

namespace metric::runtime {

struct RuntimeDiagnostics {
	std::string policy_name;
	bool exact{true};
	bool parallel{false};
	bool materialized{false};
	std::string representation;
	std::string intent;
	bool supported{true};
	std::string reason;
};

inline auto require_exact_neighbors(policy runtime_policy) -> void
{
	if (!runtime_policy.is_exact()) {
		throw std::invalid_argument("approximate neighbor runtime policy is not implemented");
	}
}

inline auto require_exact_groups(policy runtime_policy) -> void
{
	if (!runtime_policy.is_exact()) {
		throw std::invalid_argument("approximate grouping runtime policy is not implemented");
	}
}

inline auto execution_representation(policy runtime_policy) -> std::string
{
	return runtime_policy.uses_materialization() ? "matrix_cache" : "metric_space";
}

inline auto neighbor_representation(policy runtime_policy) -> std::string
{
	return execution_representation(runtime_policy);
}

inline auto group_representation(policy runtime_policy) -> std::string
{
	return execution_representation(runtime_policy);
}

inline auto diagnostics(policy runtime_policy = exact(), std::string representation = {}, std::string intent = {})
	-> RuntimeDiagnostics
{
	const auto supported = runtime_policy.is_exact();
	RuntimeDiagnostics result;
	result.policy_name = runtime_policy.name();
	result.exact = runtime_policy.is_exact();
	result.parallel = runtime_policy.uses_parallel_execution();
	result.materialized = runtime_policy.uses_materialization();
	result.representation = representation.empty() ? execution_representation(runtime_policy) : std::move(representation);
	result.intent = std::move(intent);
	result.supported = supported;
	if (!supported) {
		result.reason = "approximate runtime policies are not implemented for promoted C++ execution yet";
	}
	return result;
}

} // namespace metric::runtime

#endif
