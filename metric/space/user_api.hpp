// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_SPACE_USER_API_HPP
#define _METRIC_SPACE_USER_API_HPP

// User-facing aggregate for working with a finite metric space from a caller's point of view.
//
// These headers sit above the low-level storage/index/select machinery and give a coherent surface for
// the common workflow: build a space, manage its record set, read its pairwise values, query it, carve
// it into sub-spaces or merge spaces, and recover stale caches/indexes after a mutation. They reuse the
// existing mtrc::space::storage and mtrc::stats::search components; they do not introduce new algorithms.
//
//   mtrc::space::SpaceBuilder / space_builder   - safer staged construction (builder.hpp)
//   mtrc::space::records                         - batch insert / erase / replace, id validation (records.hpp)
//   mtrc::space::distances                       - pair value / row / pair iteration / providers (distances.hpp)
//   mtrc::space::query                           - nearest / k_nearest / within helpers (query.hpp)
//   mtrc::space (lineage)                        - sub-space / merge lineage + merge_checked (lineage.hpp)
//   mtrc::space::cache                           - stale detection and rebuild (cache.hpp)
//   mtrc::space::persistence                     - native finite-space artifacts (persistence.hpp)

#include <metric/space/builder.hpp>
#include <metric/space/cache.hpp>
#include <metric/space/distances.hpp>
#include <metric/space/lineage.hpp>
#include <metric/space/partition.hpp>
#include <metric/space/persistence.hpp>
#include <metric/space/query.hpp>
#include <metric/space/records.hpp>

#endif
