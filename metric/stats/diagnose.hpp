// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_STATS_DIAGNOSE_HPP
#define _METRIC_STATS_DIAGNOSE_HPP

#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <metric/core/concepts.hpp>
#include <metric/core/metric_traits.hpp>
#include <metric/core/result.hpp>
#include <metric/metric/discovery.hpp>
#include <metric/stats/properties/profile.hpp>
#include <metric/stats/search/nearest.hpp>
#include <metric/stats/structural_analysis/outliers.hpp>

namespace mtrc::stats {

struct diagnose_options {
	diagnose_options() = default;

	properties::profile_options profile{};
	bool include_neighbor_check{true};
	std::size_t neighbor_count{1};
	bool include_outlier_scores{true};
	std::size_t outlier_neighbor_count{1};
};

template <typename Distance> struct SpaceDiagnosis {
	using distance_type = Distance;

	properties::StatsProfile<Distance> profile;
	NeighborSet<Distance> neighbor_check;
	OutlierResult<Distance> outliers;

	core::metric_law metric_law{core::metric_law::unknown};
	metric::admission_status admission_status{metric::admission_status::quarantine};
	bool discoverable_metric{false};

	bool has_neighbor_check{false};
	bool has_outlier_scores{false};
	bool exact{true};
	std::string algorithm{"diagnose_space"};
	std::string representation{"metric_space"};
	std::vector<std::string> notes;
};

namespace diagnose_detail {

inline auto require_diagnose_options(const diagnose_options &options) -> void
{
	if (options.include_neighbor_check && options.neighbor_count == 0) {
		throw std::invalid_argument("diagnose_space neighbor_count must be >= 1 when neighbor check is enabled");
	}
	if (options.include_outlier_scores && options.outlier_neighbor_count == 0) {
		throw std::invalid_argument("diagnose_space outlier_neighbor_count must be >= 1 when outlier scores are enabled");
	}
}

inline auto append_size_note(std::vector<std::string> &notes, std::size_t record_count) -> void
{
	if (record_count == 0) {
		notes.push_back("space is empty; neighbor and outlier sections are skipped");
	} else if (record_count == 1) {
		notes.push_back("space has one record; neighbor and outlier sections need at least two records");
	}
}

} // namespace diagnose_detail

// Cohesive read-only diagnosis of one finite metric space. The report combines
// the existing profile, one deterministic neighbor check, k-NN outlier scores,
// and the metric law/admission status of the space's metric type. Sections that
// require at least two records are skipped explicitly for empty/singleton spaces
// and recorded in `notes`; invalid option values still throw.
template <typename Space, typename std::enable_if<MetricSpaceLike_v<Space>, int>::type = 0>
auto diagnose_space(const Space &space, diagnose_options options = {}) -> SpaceDiagnosis<typename Space::distance_type>
{
	using distance_type = typename Space::distance_type;
	using metric_type = typename Space::metric_type;

	diagnose_detail::require_diagnose_options(options);

	SpaceDiagnosis<distance_type> result;
	result.metric_law = core::metric_traits<metric_type>::law;
	result.admission_status = metric::admission_status_v<metric_type>;
	result.discoverable_metric = metric::admission_is_true_metric(result.admission_status);
	result.profile = properties::profile(space, std::move(options.profile));
	result.exact = result.profile.exact;

	diagnose_detail::append_size_note(result.notes, space.size());
	if (!result.discoverable_metric) {
		result.notes.push_back("space metric is not admitted as a discoverable true metric");
	}

	if (space.size() < 2) {
		return result;
	}

	const auto available_neighbors = space.size() - 1;
	if (options.include_neighbor_check) {
		const auto count = std::min(options.neighbor_count, available_neighbors);
		result.neighbor_check = search::knn(space, space.id(0), count);
		result.has_neighbor_check = true;
		result.exact = result.exact && result.neighbor_check.exact;
	}

	if (options.include_outlier_scores) {
		const auto count = std::min(options.outlier_neighbor_count, available_neighbors);
		result.outliers = structural_analysis::nearest_neighbor_outliers(space, count);
		result.has_outlier_scores = true;
		result.exact = result.exact && result.outliers.exact;
	}

	return result;
}

} // namespace mtrc::stats

namespace mtrc {
using stats::diagnose_options;
using stats::diagnose_space;
template <typename Distance> using SpaceDiagnosis = stats::SpaceDiagnosis<Distance>;
} // namespace mtrc

#endif
