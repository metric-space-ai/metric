/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2020 Panda Team
*/
#include <boost/python.hpp>

namespace py = boost::python;

void export_metric_MGC();

BOOST_PYTHON_MODULE(_correlation) {
    export_metric_MGC();
}
