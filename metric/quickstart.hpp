// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_QUICKSTART_HPP
#define _METRIC_QUICKSTART_HPP

// Lean entry point for the common happy path: records + a metric -> a finite
// metric space -> a first result, in a handful of lines. It pulls only the
// curated surface a newcomer needs, instead of the ~60-header <metric/engine.hpp>
// aggregate:
//
//   #include <metric/quickstart.hpp>
//   auto records = mtrc::record::import_records(words);
//   auto space   = mtrc::make_space(records, mtrc::Edit<char>{});
//   auto nn      = mtrc::find_neighbors(space, query, 3);
//   std::cout << nn << "\n";                 // operator<< from <metric/core/result.hpp>
//
// Reach for <metric/workflow.hpp> (the full stable workflow surface) or
// <metric/engine.hpp> (everything) when you outgrow this.

// records: import / read_csv / validate / export
#include <metric/record.hpp>

// metrics: the true-metric catalog + custom/composed metrics
#include <metric/metric/catalog.hpp>
#include <metric/metric/compose.hpp>
#include <metric/metric/custom.hpp>

// space: make_space + the opt-in SpaceBuilder / build_checked
#include <metric/core/metric_space.hpp>
#include <metric/core/result.hpp>
#include <metric/space/builder.hpp>

// the everyday verbs (all reachable at mtrc::)
#include <metric/stats/search/neighbors.hpp>
#include <metric/stats/structural_analysis/groups.hpp>
#include <metric/stats/structural_analysis/options.hpp>
#include <metric/stats/structural_analysis/outliers.hpp>
#include <metric/stats/properties/describe.hpp>
#include <metric/stats/properties/entropy.hpp>
#include <metric/stats/properties/intrinsic_dimension.hpp>

// legible output / export
#include <metric/utils/render.hpp>

#endif
