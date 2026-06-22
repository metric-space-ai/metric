// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_STATS_HPP
#define _METRIC_STATS_HPP

// User-facing aggregate for investigating an existing finite metric space.
// `mtrc::stats` is read-only: search, sampling, properties, correlation, and
// structural analysis never define a metric and never mutate the source space.

#include <metric/stats/correlate/compare.hpp>
#include <metric/stats/correlate/correlation.hpp>
#include <metric/stats/correlate/options.hpp>
#include <metric/stats/correlate/significance.hpp>

#include <metric/stats/properties/describe.hpp>
#include <metric/stats/properties/distribution.hpp>
#include <metric/stats/properties/entropy.hpp>
#include <metric/stats/properties/intrinsic_dimension.hpp>
#include <metric/stats/properties/local_volume.hpp>
#include <metric/stats/properties/profile.hpp>

#include <metric/stats/sample/options.hpp>
#include <metric/stats/sample/sample.hpp>

#include <metric/stats/search/nearest.hpp>
#include <metric/stats/search/neighbors.hpp>
#include <metric/stats/search/options.hpp>

#include <metric/stats/structural_analysis/clustering.hpp>
#include <metric/stats/structural_analysis/diagnostics.hpp>
#include <metric/stats/structural_analysis/groups.hpp>
#include <metric/stats/structural_analysis/options.hpp>
#include <metric/stats/structural_analysis/outliers.hpp>
#include <metric/stats/structural_analysis/representatives.hpp>

#endif
