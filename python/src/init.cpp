/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2020 Panda Team
*/

#include "metric_converters.hpp"
#include "stl_wrappers.hpp"

#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include <vector>
#include <iostream>

namespace bp = boost::python;


void export_converters()
{
    IterableConverter()
        .from_python<std::vector<double>>()
        .from_python<std::vector<int>>()
        .from_python<std::vector<std::vector<double>>>()
        .from_python<std::vector<std::vector<int>>>()
        ;

    NumpyScalarConverter()
        .from_python<signed char>()
        .from_python<short>()
        .from_python<int>()
        .from_python<long>()
        .from_python<long long>()
        .from_python<unsigned char>()
        .from_python<unsigned short>()
        .from_python<unsigned int>()
        .from_python<unsigned long>()
        .from_python<unsigned long long>()
        .from_python<float>()
        .from_python<double>();

    NumpyArrayConverter()
        .from_python<WrapStlVector<double>>()
        .from_python<WrapStlMatrix<double>>()
        .from_python<WrapStlVector<WrapStlVector<double>>>();
}

void export_containers() {
    typedef std::vector<double> VectorDouble;
    typedef std::vector<VectorDouble> VectorVectorDouble;
    typedef std::vector<int> VectorInt;
    typedef std::vector<VectorInt> VectorVectorInt;
    typedef std::vector<bool> VectorBool;
    typedef std::vector<unsigned long> VectorLong;
    typedef std::vector<VectorLong> VectorVectorLong;
    bp::class_<VectorDouble>("VectorDouble").def(bp::vector_indexing_suite<VectorDouble>());
    bp::class_<VectorVectorDouble>("VectorVectorDouble").def(bp::vector_indexing_suite<VectorVectorDouble>());
    bp::class_<VectorInt>("VectorInt").def(bp::vector_indexing_suite<VectorInt>());
    bp::class_<VectorVectorInt>("VectorVectorInt").def(bp::vector_indexing_suite<VectorVectorInt>());
    bp::class_<VectorBool>("VectorBool").def(bp::vector_indexing_suite<VectorBool>());
    bp::class_<VectorLong>("VectorLong").def(bp::vector_indexing_suite<VectorLong>());
    bp::class_<VectorVectorLong>("VectorVectorLong").def(bp::vector_indexing_suite<VectorVectorLong>());
}

void test1(int t) {
    std::cout << "test1 " << t << std::endl;
}

void test2(const std::vector<int>& t) {
    std::cout << "test2 " << t[0] << std::endl;
}

BOOST_PYTHON_MODULE(_metric) {
    export_converters();
    // exposing C++ return types
    export_containers();
}
