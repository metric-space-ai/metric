// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_SPACE_STREAMING_HPP
#define _METRIC_SPACE_STREAMING_HPP

#include <cstddef>
#include <iterator>
#include <type_traits>
#include <utility>
#include <vector>

#include <metric/core/concepts.hpp>
#include <metric/core/version.hpp>
#include <metric/record/id.hpp>

namespace mtrc::space::streaming {

struct ingest_options {
	// 1 reports every accepted record. 0 disables progress callbacks.
	std::size_t progress_interval{1};
	bool report_initial_progress{false};
	bool report_final_progress{true};
};

struct ingest_progress {
	std::size_t requested{};
	std::size_t accepted{};
	std::size_t old_size{};
	std::size_t current_size{};
	core::SpaceVersion version_before{};
	core::SpaceVersion version_current{};
	RecordId last_id{};
	bool has_last_id{};
	bool cancelled{};

	auto complete() const -> bool { return !cancelled && accepted == requested; }
	auto fraction() const -> double
	{
		return requested == 0 ? 1.0 : static_cast<double>(accepted) / static_cast<double>(requested);
	}
};

struct ingest_report {
	std::size_t requested{};
	std::size_t appended{};
	std::size_t old_size{};
	std::size_t new_size{};
	core::SpaceVersion version_before{};
	core::SpaceVersion version_after{};
	std::vector<RecordId> appended_ids;
	bool cancelled{};

	// Streaming append does not evaluate pair distances and never builds a dense all-pairs provider.
	std::size_t distance_evaluations{};
	bool materialized_dense_all_pairs{};

	auto changed() const -> bool { return version_before != version_after; }
	auto complete() const -> bool { return !cancelled && appended == requested; }
};

namespace detail {

template <typename Container, typename = void> struct has_size : std::false_type {};

template <typename Container>
struct has_size<Container, std::void_t<decltype(std::declval<const Container &>().size())>> : std::true_type {};

template <typename Container> auto container_size(const Container &records) -> std::size_t
{
	if constexpr (has_size<Container>::value) {
		return static_cast<std::size_t>(records.size());
	} else {
		return static_cast<std::size_t>(std::distance(std::begin(records), std::end(records)));
	}
}

struct no_progress {
	auto operator()(const ingest_progress &) const noexcept -> void {}
};

struct never_cancel {
	auto operator()(const ingest_progress &) const noexcept -> bool { return false; }
};

template <typename ProgressCallback>
auto notify_progress(ProgressCallback &callback, const ingest_progress &progress) -> void
{
	static_assert(std::is_invocable_v<ProgressCallback &, const ingest_progress &>,
				  "streaming progress callback must accept const ingest_progress&");
	callback(progress);
}

template <typename CancelCallback>
auto should_cancel(CancelCallback &callback, const ingest_progress &progress) -> bool
{
	if constexpr (std::is_invocable_r_v<bool, CancelCallback &, const ingest_progress &>) {
		return callback(progress);
	} else if constexpr (std::is_invocable_r_v<bool, CancelCallback &>) {
		return callback();
	} else {
		static_assert(std::is_invocable_r_v<bool, CancelCallback &, const ingest_progress &> ||
						  std::is_invocable_r_v<bool, CancelCallback &>,
					  "streaming cancel callback must return bool");
		return false;
	}
}

template <typename Space>
auto make_progress(const Space &space, const ingest_report &report) -> ingest_progress
{
	ingest_progress progress;
	progress.requested = report.requested;
	progress.accepted = report.appended;
	progress.old_size = report.old_size;
	progress.current_size = space.size();
	progress.version_before = report.version_before;
	progress.version_current = space.version();
	progress.cancelled = report.cancelled;
	if (!report.appended_ids.empty()) {
		progress.last_id = report.appended_ids.back();
		progress.has_last_id = true;
	}
	return progress;
}

template <typename Space, typename Container, typename ProgressCallback, typename CancelCallback,
		  typename std::enable_if<core::RecordMetricSpaceLike_v<Space>, int>::type = 0>
auto append_batch_impl(Space &space, const Container &records, ProgressCallback progress_callback,
					   CancelCallback cancel_callback, ingest_options options) -> ingest_report
{
	ingest_report report;
	report.requested = container_size(records);
	report.old_size = space.size();
	report.new_size = space.size();
	report.version_before = space.version();
	report.version_after = space.version();
	report.appended_ids.reserve(report.requested);

	if (options.report_initial_progress && options.progress_interval != 0) {
		notify_progress(progress_callback, make_progress(space, report));
	}

	for (const auto &record : records) {
		if (should_cancel(cancel_callback, make_progress(space, report))) {
			report.cancelled = true;
			break;
		}

		const auto id = space.insert(typename Space::record_type(record));
		report.appended_ids.push_back(id);
		++report.appended;
		report.new_size = space.size();
		report.version_after = space.version();

		if (options.progress_interval != 0 &&
			(report.appended % options.progress_interval == 0 || report.appended == report.requested)) {
			notify_progress(progress_callback, make_progress(space, report));
		}
	}

	report.new_size = space.size();
	report.version_after = space.version();

	if (report.cancelled && options.report_final_progress && options.progress_interval != 0) {
		notify_progress(progress_callback, make_progress(space, report));
	}

	return report;
}

} // namespace detail

template <typename Space, typename Container, typename ProgressCallback, typename CancelCallback,
		  typename std::enable_if<core::RecordMetricSpaceLike_v<Space>, int>::type = 0>
auto append_batch(Space &space, const Container &records, ProgressCallback progress_callback,
				  CancelCallback cancel_callback, ingest_options options = {}) -> ingest_report
{
	return detail::append_batch_impl(space, records, std::move(progress_callback), std::move(cancel_callback), options);
}

template <typename Space, typename Container,
		  typename std::enable_if<core::RecordMetricSpaceLike_v<Space>, int>::type = 0>
auto append_batch(Space &space, const Container &records, ingest_options options = {}) -> ingest_report
{
	return detail::append_batch_impl(space, records, detail::no_progress{}, detail::never_cancel{}, options);
}

template <typename Space, typename Container,
		  typename std::enable_if<core::RecordMetricSpaceLike_v<Space>, int>::type = 0>
auto ingest(Space &space, const Container &records, ingest_options options = {}) -> ingest_report
{
	return append_batch(space, records, options);
}

template <typename Provider>
auto refresh_after_append(Provider &provider, const ingest_report &report)
	-> decltype(provider.refresh_after_append(report))
{
	return provider.refresh_after_append(report);
}

} // namespace mtrc::space::streaming

namespace mtrc::space {
using streaming::append_batch;
using streaming::ingest;
using streaming::ingest_options;
using streaming::ingest_progress;
using streaming::ingest_report;
using streaming::refresh_after_append;
} // namespace mtrc::space

#endif
