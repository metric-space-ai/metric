// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_WORKFLOW_HPP
#define _METRIC_WORKFLOW_HPP

// Practical C++ entry point for the common METRIC workflow:
//
//   records -> metric discovery/selection -> finite space -> stats/modify results.
//
// This header is intentionally an aggregate. It does not create a monolithic API
// and it does not own algorithms. Ownership stays with the Level-1 namespaces:
// `record`, `metric`, `space`, `stats`, and `modify`.

#include <metric/record.hpp>

#include <metric/metric/admission.hpp>
#include <metric/metric/catalog.hpp>
#include <metric/metric/compose.hpp>
#include <metric/metric/custom.hpp>
#include <metric/metric/discovery.hpp>
#include <metric/metric/discovery_report.hpp>

#include <metric/core/errors.hpp>
#include <metric/core/metadata.hpp>
#include <metric/core/metric_space.hpp>
#include <metric/core/metric_traits.hpp>
#include <metric/core/result.hpp>

#include <metric/space/user_api.hpp>
#include <metric/stats.hpp>
#include <metric/modify.hpp>

#endif
