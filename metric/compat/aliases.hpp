// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_COMPAT_ALIASES_HPP
#define _METRIC_COMPAT_ALIASES_HPP

#include "../space.hpp"

namespace metric::compat {

template <typename RecType, typename Metric>
using LegacyMatrix = metric::Matrix<RecType, Metric>;

template <typename RecType, typename Metric>
using LegacyMatrixSpace = metric::MatrixSpace<RecType, Metric>;

template <typename RecType, typename Metric>
using LegacyTree = metric::Tree<RecType, Metric>;

template <typename RecType, typename Metric>
using LegacyTreeSpace = metric::TreeSpace<RecType, Metric>;

template <typename Sample, typename Distance, typename WeightType = bool, bool isDense = false, bool isSymmetric = true>
using LegacyGraphSpace = metric::GraphSpace<Sample, Distance, WeightType, isDense, isSymmetric>;

template <typename RecType, typename Metric>
using LegacyFiniteSpace = metric::FiniteSpace<RecType, Metric>;

} // namespace metric::compat

#endif
