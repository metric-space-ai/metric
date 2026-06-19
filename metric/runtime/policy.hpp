// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_RUNTIME_POLICY_HPP
#define _METRIC_RUNTIME_POLICY_HPP

#include <string>

namespace metric::runtime {

enum class accuracy {
	exact,
	approximate,
};

enum class materialization {
	lazy,
	materialized,
};

enum class execution {
	serial,
	parallel,
};

class policy {
  public:
	constexpr policy(accuracy accuracy_mode = accuracy::exact,
					 materialization materialization_mode = materialization::lazy,
					 execution execution_mode = execution::serial)
		: accuracy_(accuracy_mode)
		, materialization_(materialization_mode)
		, execution_(execution_mode)
	{
	}

	constexpr auto accuracy_mode() const noexcept -> accuracy { return accuracy_; }
	constexpr auto materialization_mode() const noexcept -> materialization { return materialization_; }
	constexpr auto execution_mode() const noexcept -> execution { return execution_; }

	constexpr auto is_exact() const noexcept -> bool { return accuracy_ == accuracy::exact; }
	constexpr auto is_approximate() const noexcept -> bool { return accuracy_ == accuracy::approximate; }
	constexpr auto uses_materialization() const noexcept -> bool
	{
		return materialization_ == materialization::materialized;
	}
	constexpr auto uses_parallel_execution() const noexcept -> bool { return execution_ == execution::parallel; }

	auto representation_preference() const -> std::string
	{
		return uses_materialization() ? "matrix_cache" : "implicit";
	}

	auto name() const -> std::string
	{
		std::string value = is_exact() ? "exact" : "approximate";
		value += uses_materialization() ? "_materialized" : "_lazy";
		value += uses_parallel_execution() ? "_parallel" : "_serial";
		return value;
	}

  private:
	accuracy accuracy_;
	materialization materialization_;
	execution execution_;
};

constexpr auto exact() -> policy
{
	return policy(accuracy::exact, materialization::lazy, execution::serial);
}

constexpr auto approximate() -> policy
{
	return policy(accuracy::approximate, materialization::lazy, execution::serial);
}

constexpr auto lazy(policy base = exact()) -> policy
{
	return policy(base.accuracy_mode(), materialization::lazy, base.execution_mode());
}

constexpr auto materialized(policy base = exact()) -> policy
{
	return policy(base.accuracy_mode(), materialization::materialized, base.execution_mode());
}

constexpr auto serial(policy base = exact()) -> policy
{
	return policy(base.accuracy_mode(), base.materialization_mode(), execution::serial);
}

constexpr auto parallel(policy base = exact()) -> policy
{
	return policy(base.accuracy_mode(), base.materialization_mode(), execution::parallel);
}

} // namespace metric::runtime

#endif
