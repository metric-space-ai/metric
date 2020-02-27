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

std::string getObjType(const bp::api::object& obj) {
    boost::python::extract<boost::python::object> objectExtractor(obj);
    boost::python::object o=objectExtractor();
    std::string obj_type = boost::python::extract<std::string>(o.attr("__class__").attr("__name__"));
    return obj_type;
}

std::string getObjType(PyObject* obj_ptr) {
       boost::python::object obj(boost::python::handle<>(boost::python::borrowed(obj_ptr)));
       return getObjType(obj);
}

void export_converters();
void export_metric_KOC();
void export_metric_sorensen();
void export_metric_MGC();
void export_metric_Edit();
void export_metric_EMD();
void export_metric_Entropy();
void export_metric_standards();
void export_metric_SSIM();
void export_metric_TWED();
void export_metric_matrix();
void export_metric_kohonen();
void export_metric_dbscan();

BOOST_PYTHON_MODULE(metric) {

    export_converters();

    // exposing C++ return types
    bp::class_<VectorDouble>("VectorDouble").def(bp::vector_indexing_suite<VectorDouble>());
    bp::class_<VectorVectorDouble>("VectorVectorDouble").def(bp::vector_indexing_suite<VectorVectorDouble>());
    bp::class_<VectorInt>("VectorInt").def(bp::vector_indexing_suite<VectorInt>());
    bp::class_<VectorVectorInt>("VectorVectorInt").def(bp::vector_indexing_suite<VectorVectorInt>());
    bp::class_<VectorBool>("VectorBool").def(bp::vector_indexing_suite<VectorBool>());

    export_metric_KOC();
    export_metric_matrix();
    export_metric_sorensen();
    export_metric_MGC();
    export_metric_Edit();
    export_metric_EMD();
    export_metric_Entropy();
    export_metric_standards();
    export_metric_SSIM();
    export_metric_TWED();
//    export_metric_kohonen();
    export_metric_dbscan();

// TODO #include "metric_affprop.cpp"

}
