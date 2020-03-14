/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2020 Panda Team
*/
#include <boost/python.hpp>

namespace py = boost::python;

void export_metric_autoencoder();
void export_metric_DSPCC();
void export_metric_kmeans();
void export_metric_kmedoids();

BOOST_PYTHON_MODULE(_mapping) {
    export_metric_autoencoder();
    export_metric_DSPCC();
    export_metric_kmeans();
    export_metric_kmedoids();
}
