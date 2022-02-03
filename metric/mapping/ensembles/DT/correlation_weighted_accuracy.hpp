/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 Michael Welsch, Signal Empowering Technology
*/

/*
correlation weighted accuracy


*/

#ifndef _METRIC_MAPPING_ENSEMBLES_DT_CWA_HPP
#define _METRIC_MAPPING_ENSEMBLES_DT_CWA_HPP

namespace metric {

/**
 * @brief Correlation weighted accuracy
 *
 * @param a
 * @param b
 * @return
 */
template <typename Container> double correlation_weighted_accuracy(Container a, Container b);

} // namespace metric

#include "correlation_weighted_accuracy.cpp"

#endif
