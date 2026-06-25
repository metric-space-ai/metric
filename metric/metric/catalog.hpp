// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_METRIC_CATALOG_HPP
#define _METRIC_METRIC_CATALOG_HPP

#include <metric/metric/catalog/vector/L1.hpp>
#include <metric/metric/catalog/vector/Standards.hpp>
#include <metric/metric/catalog/vector/Geometry.hpp>
#include <metric/metric/catalog/structured/EMD.hpp>
#include <metric/metric/catalog/structured/Edit.hpp>
#include <metric/metric/catalog/structured/Categorical.hpp>
#include <metric/metric/catalog/structured/Geometry.hpp>
#include <metric/metric/catalog/structured/TimeSeries.hpp>
#include <metric/metric/catalog/structured/TWED.hpp>
#include <metric/metric/catalog/distribution/Empirical.hpp>
#include <metric/metric/catalog/distribution/Probability.hpp>
#include <metric/metric/catalog/distribution/CramervonMises.hpp>
#include <metric/metric/catalog/distribution/KolmogorovSmirnov.hpp>
#include <metric/metric/catalog/distribution/RandomEMD.hpp>

// metric/metric/catalog/space/Riemannian.hpp is intentionally NOT included here.
// RiemannianDistance (AIRM) needs a generalized symmetric eigensolver (LAPACK
// dsygv via metric/utils/wrappers/lapack.hpp), so pulling it into this umbrella
// would leak an external-backend runtime dependency into the otherwise
// header-only public core (<metric/metric.hpp>). Consumers that want AIRM
// include <metric/metric/catalog/space/Riemannian.hpp> explicitly and link
// LAPACK; metric/metric/discovery.hpp then registers its admission status
// (the same opt-in pattern used for physically-quarantined metrics).

#endif
