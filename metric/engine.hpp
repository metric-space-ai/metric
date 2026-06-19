// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_ENGINE_HPP
#define _METRIC_ENGINE_HPP

#include "core/concepts.hpp"
#include "core/metric_space.hpp"
#include "core/metric_traits.hpp"
#include "core/neighbor.hpp"
#include "core/parameters.hpp"
#include "core/record_id.hpp"
#include "core/result.hpp"
#include "intent/compress.hpp"
#include "intent/compare.hpp"
#include "intent/describe.hpp"
#include "intent/denoise.hpp"
#include "intent/embed.hpp"
#include "intent/groups.hpp"
#include "intent/map.hpp"
#include "intent/neighbors.hpp"
#include "intent/outliers.hpp"
#include "intent/representatives.hpp"
#include "intent/reduce.hpp"
#include "mappings/clustered_space.hpp"
#include "mappings/mapping.hpp"
#include "mappings/pcfa.hpp"
#include "operators/clustering.hpp"
#include "operators/correlation.hpp"
#include "operators/nearest.hpp"
#include "representations/cover_tree_index.hpp"
#include "representations/diagnostics.hpp"
#include "representations/graph_topology.hpp"
#include "representations/implicit.hpp"
#include "representations/knn_graph_index.hpp"
#include "representations/matrix_cache.hpp"
#include "runtime/cache.hpp"
#include "runtime/execution.hpp"
#include "runtime/policy.hpp"
#include "strategies/clustering.hpp"
#include "strategies/correlation.hpp"
#include "strategies/representatives.hpp"
#include "strategies/reduction.hpp"
#include "strategies/search.hpp"

#endif
