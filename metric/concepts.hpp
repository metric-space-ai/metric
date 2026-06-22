// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_CONCEPTS_HPP
#define _METRIC_CONCEPTS_HPP

#include <type_traits>
#include <utility>

namespace mtrc {

template <typename Callable, typename Record, typename = void> struct is_metric_callable : std::false_type {};

template <typename Callable, typename Record>
struct is_metric_callable<Callable, Record,
						  std::void_t<decltype(std::declval<const Callable &>()(
							  std::declval<const Record &>(), std::declval<const Record &>()))>> : std::true_type {};

template <typename Callable, typename Record>
constexpr bool is_metric_callable_v = is_metric_callable<Callable, Record>::value;

template <typename Callable, typename Record>
using metric_distance_t =
	decltype(std::declval<const Callable &>()(std::declval<const Record &>(), std::declval<const Record &>()));

template <typename Record, typename Callable> class Metric {
  public:
	using record_type = Record;
	using callable_type = Callable;
	using distance_type = metric_distance_t<callable_type, record_type>;

	explicit Metric(callable_type callable) : callable_(std::move(callable))
	{
		static_assert(is_metric_callable_v<callable_type, record_type>,
					  "mtrc::Metric requires a callable accepting two records");
	}

	auto operator()(const record_type &lhs, const record_type &rhs) const -> distance_type
	{
		return callable_(lhs, rhs);
	}

	auto callable() const -> const callable_type & { return callable_; }

  private:
	callable_type callable_;
};

template <typename Record, typename Callable>
auto make_metric(Callable callable) -> Metric<Record, typename std::decay<Callable>::type>
{
	return Metric<Record, typename std::decay<Callable>::type>(std::forward<Callable>(callable));
}

} // namespace mtrc

#endif
