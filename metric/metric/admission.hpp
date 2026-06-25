// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_METRIC_ADMISSION_HPP
#define _METRIC_METRIC_ADMISSION_HPP

#include <metric/core/metric_traits.hpp>

namespace mtrc::metric {

// ---------------------------------------------------------------------------
// Law-based helpers (kept for backward compatibility).
//
// `is_admitted_metric_v` is true exactly when the metric_traits law is a true
// metric, which covers both the `admitted` and `restricted_metric` admission
// statuses below.
// ---------------------------------------------------------------------------

template <typename Metric> constexpr auto is_admitted_metric_v = core::metric_traits<Metric>::law == core::metric_law::metric;

template <typename Metric> constexpr auto is_quarantined_metric_v = core::metric_traits<Metric>::law != core::metric_law::metric;

// ---------------------------------------------------------------------------
// Metric Discovery admission status.
//
// `metric_law` answers "what law does this computation satisfy" (distance /
// metric / pseudo_metric / unknown). `admission_status` answers the governance
// question Metric Discovery cares about: may this class be discovered as a
// METRIC-provided metric, and if not, what is its retirement path?
//
//   admitted          - true metric on its natural record domain; no tunable
//                       parameter or per-fit precondition can break the law
//                       (only structural requirements such as alignment apply).
//   restricted_metric - true metric, but only under enforced parameter/domain
//                       gates (e.g. Minkowski p >= 1, nonnegative inputs, a
//                       metric ground cost, a positive calibrated scale). Still
//                       routable by metric-only algorithms because the gate is
//                       enforced on construction and on every evaluation.
//   quarantine        - not a true metric as shipped, but a metric variant
//                       exists in the same literature family (or the class has
//                       a legitimate non-metric home elsewhere). Kept for a
//                       compatibility window pending admission/relocation;
//                       never deleted by family name alone.
//   rejected          - the specific computation is not a metric and the
//                       family's metric variant is a separate, already-present
//                       class, so this computation is scheduled for removal.
//
// admitted and restricted_metric are exactly the statuses whose metric_traits
// law == metric. quarantine and rejected are exactly the statuses whose law
// != metric. `admission_is_true_metric` encodes that relation, and
// metric/metric/discovery.hpp static_asserts it for every registered class.
// ---------------------------------------------------------------------------

enum class admission_status {
	admitted,
	restricted_metric,
	quarantine,
	rejected,
};

constexpr auto admission_is_true_metric(admission_status status) -> bool
{
	return status == admission_status::admitted || status == admission_status::restricted_metric;
}

inline auto admission_status_name(admission_status status) -> const char *
{
	switch (status) {
	case admission_status::admitted:
		return "admitted";
	case admission_status::restricted_metric:
		return "restricted_metric";
	case admission_status::quarantine:
		return "quarantine";
	case admission_status::rejected:
		return "rejected";
	}
	return "quarantine";
}

// Primary template: derive a safe default from the declared metric law. A
// user-provided callable that truthfully declares metric_law::metric is treated
// as `admitted`; everything else defaults to `quarantine` (never silently
// `rejected`, which is a deliberate removal decision). Catalog and quarantine
// classes specialize this in metric/metric/discovery.hpp.
template <typename Metric, typename = void> struct metric_admission {
	static constexpr admission_status status = core::metric_traits<Metric>::law == core::metric_law::metric
												   ? admission_status::admitted
												   : admission_status::quarantine;
};

template <typename Metric> constexpr admission_status admission_status_v = metric_admission<Metric>::status;

template <typename Metric> constexpr bool is_admitted_v = metric_admission<Metric>::status == admission_status::admitted;

template <typename Metric>
constexpr bool is_restricted_metric_v = metric_admission<Metric>::status == admission_status::restricted_metric;

template <typename Metric>
constexpr bool is_quarantined_status_v = metric_admission<Metric>::status == admission_status::quarantine;

template <typename Metric> constexpr bool is_rejected_v = metric_admission<Metric>::status == admission_status::rejected;

// A class is discoverable as a METRIC metric exactly when it is admitted or a
// restricted metric. This MUST agree with the metric_traits law; discovery.hpp
// static_asserts the agreement for every registered class.
template <typename Metric>
constexpr bool is_discoverable_metric_v = admission_is_true_metric(metric_admission<Metric>::status);

} // namespace mtrc::metric

#endif
