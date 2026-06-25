// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_STRATEGIES_CORRELATION_HPP
#define _METRIC_STRATEGIES_CORRELATION_HPP

#include <cstddef>

namespace mtrc::stats::correlate {

// Strategy selector for compare()/correlate(): requests the MGC dependence statistic.
// MGC is a non-metric multiscale graph correlation; see correlation.hpp for its
// statistical semantics (sample statistic in [-1, 1], no p-value).
struct mgc_options {
	std::size_t sample_count{250};
	double estimate_threshold{0.05};
	std::size_t max_iterations{1000};
};

} // namespace mtrc::stats::correlate

namespace mtrc {
// Re-export alongside compare()/correlate() so callers can write mtrc::mgc_options.
using stats::correlate::mgc_options;
} // namespace mtrc

#endif
