/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2020 Panda Team
*/

#include "metric_py.hpp"

#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

namespace bp = boost::python;

typedef std::vector<double> VectorDouble;
typedef std::vector<VectorDouble> VectorVectorDouble;
typedef std::vector<int> VectorInt;
typedef std::vector<VectorInt> VectorVectorInt;
typedef std::vector<bool> VectorBool;

void export_converters();

BOOST_PYTHON_MODULE(metric) {
    export_converters();

    // exposing C++ return types
    bp::class_<VectorDouble>("VectorDouble").def(bp::vector_indexing_suite<VectorDouble>());
    bp::class_<VectorVectorDouble>("VectorVectorDouble").def(bp::vector_indexing_suite<VectorVectorDouble>());
    bp::class_<VectorInt>("VectorInt").def(bp::vector_indexing_suite<VectorInt>());
    bp::class_<VectorVectorInt>("VectorVectorInt").def(bp::vector_indexing_suite<VectorVectorInt>());
    bp::class_<VectorBool>("VectorBool").def(bp::vector_indexing_suite<VectorBool>());
}
