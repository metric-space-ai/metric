// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef _METRIC_COMPAT_ALIASES_HPP
#define _METRIC_COMPAT_ALIASES_HPP

#include "../space.hpp"

namespace mtrc::compat {

template <typename RecType, typename Metric> using LegacyMatrix = mtrc::Matrix<RecType, Metric>;

template <typename RecType, typename Metric> using LegacyMatrixSpace = mtrc::MatrixSpace<RecType, Metric>;

template <typename RecType, typename Metric> using LegacyTree = mtrc::Tree<RecType, Metric>;

template <typename RecType, typename Metric> using LegacyTreeSpace = mtrc::TreeSpace<RecType, Metric>;

template <typename Sample, typename Distance, typename WeightType = bool, bool isDense = false, bool isSymmetric = true>
using LegacyGraphSpace = mtrc::GraphSpace<Sample, Distance, WeightType, isDense, isSymmetric>;

template <typename RecType, typename Metric> using LegacyFiniteSpace = mtrc::FiniteSpace<RecType, Metric>;

} // namespace mtrc::compat

#endif
