// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_RUNTIME_EXECUTION_HPP
#define _METRIC_RUNTIME_EXECUTION_HPP

#include <stdexcept>
#include <string>
#include <utility>

#include "../core/metric_traits.hpp"
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

inline auto require_exact_outliers(policy runtime_policy) -> void
{
	if (!runtime_policy.is_exact()) {
		throw std::invalid_argument("approximate outlier runtime policy is not implemented");
	}
}

inline auto require_exact_denoise(policy runtime_policy) -> void
{
	if (!runtime_policy.is_exact()) {
		throw std::invalid_argument("approximate denoise runtime policy is not implemented");
	}
}

inline auto require_exact_representatives(policy runtime_policy) -> void
{
	if (!runtime_policy.is_exact()) {
		throw std::invalid_argument("approximate representative runtime policy is not implemented");
	}
}

inline auto require_exact_compress(policy runtime_policy) -> void
{
	if (!runtime_policy.is_exact()) {
		throw std::invalid_argument("approximate compression runtime policy is not implemented");
	}
}

inline auto require_exact_describe(policy runtime_policy) -> void
{
	if (!runtime_policy.is_exact()) {
		throw std::invalid_argument("approximate structure-description runtime policy is not implemented");
	}
}

inline auto require_exact_compare(policy runtime_policy) -> void
{
	if (!runtime_policy.is_exact()) {
		throw std::invalid_argument("approximate comparison runtime policy is not implemented");
	}
}

inline auto require_exact_embed(policy runtime_policy) -> void
{
	if (!runtime_policy.is_exact()) {
		throw std::invalid_argument("approximate embedding runtime policy is not implemented");
	}
}

inline auto require_exact_reduce(policy runtime_policy) -> void
{
	if (!runtime_policy.is_exact()) {
		throw std::invalid_argument("approximate reduction runtime policy is not implemented");
	}
}

inline auto require_exact_map(policy runtime_policy) -> void
{
	if (!runtime_policy.is_exact()) {
		throw std::invalid_argument("approximate mapping runtime policy is not implemented");
	}
}

inline auto require_lazy_embed(policy runtime_policy) -> void
{
	if (runtime_policy.uses_materialization()) {
		throw std::invalid_argument("materialized embedding runtime policy is not implemented");
	}
}

inline auto require_lazy_reduce(policy runtime_policy) -> void
{
	if (runtime_policy.uses_materialization()) {
		throw std::invalid_argument("materialized reduction runtime policy is not implemented");
	}
}

inline auto require_lazy_map(policy runtime_policy) -> void
{
	if (runtime_policy.uses_materialization()) {
		throw std::invalid_argument("materialized mapping runtime policy is not implemented");
	}
}

template <typename Metric>
constexpr auto supports_parallel_metric(policy runtime_policy) noexcept -> bool
{
	return !runtime_policy.uses_parallel_execution() || core::metric_thread_safe_v<Metric>;
}

template <typename Metric>
inline auto require_parallel_metric(policy runtime_policy) -> void
{
	if (!supports_parallel_metric<Metric>(runtime_policy)) {
		throw std::invalid_argument("parallel runtime policy requires a thread-safe metric");
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

inline auto outlier_representation(policy runtime_policy) -> std::string
{
	return execution_representation(runtime_policy);
}

inline auto denoise_representation(policy runtime_policy) -> std::string
{
	return execution_representation(runtime_policy);
}

inline auto representative_representation(policy runtime_policy) -> std::string
{
	return execution_representation(runtime_policy);
}

inline auto compression_representation(policy runtime_policy) -> std::string
{
	return execution_representation(runtime_policy);
}

inline auto describe_representation(policy runtime_policy) -> std::string
{
	return execution_representation(runtime_policy);
}

inline auto compare_representation(policy runtime_policy) -> std::string
{
	return execution_representation(runtime_policy);
}

inline auto embed_representation(policy runtime_policy) -> std::string
{
	return execution_representation(runtime_policy);
}

inline auto reduce_representation(policy runtime_policy) -> std::string
{
	return execution_representation(runtime_policy);
}

inline auto map_representation(policy runtime_policy) -> std::string
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

template <typename Metric>
inline auto diagnostics_for_metric(policy runtime_policy = exact(), std::string representation = {},
								   std::string intent = {}) -> RuntimeDiagnostics
{
	auto result = diagnostics(runtime_policy, std::move(representation), std::move(intent));
	if (result.supported && !supports_parallel_metric<Metric>(runtime_policy)) {
		result.supported = false;
		result.reason = "parallel runtime policy requires a thread-safe metric";
	}
	return result;
}

template <typename Space>
inline auto diagnostics_for_space(const Space &, policy runtime_policy = exact(), std::string representation = {},
								  std::string intent = {}) -> RuntimeDiagnostics
{
	return diagnostics_for_metric<typename Space::metric_type>(runtime_policy, std::move(representation),
															   std::move(intent));
}

} // namespace metric::runtime

#endif
