/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2020 Panda Team
*/

//#include "metric_converters.hpp"
//#include "stl_wrappers.hpp"

#include <pybind11/pybind11.h>
#include <pybind11/stl_bind.h>

#include <vector>
#include <iostream>

namespace py = pybind11;


/*void export_converters()
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
}*/

/*void export_containers() {
    typedef std::vector<double> VectorDouble;
    typedef std::vector<int> VectorInt;
    typedef std::vector<bool> VectorBool;
    typedef std::vector<unsigned char> VectorUChar;
    typedef std::vector<unsigned int> VectorUInt;
    typedef std::vector<unsigned long> VectorULong;
    typedef std::vector<VectorDouble> VectorVectorDouble;
    typedef std::vector<VectorInt> VectorVectorInt;
    typedef std::vector<VectorULong> VectorVectorULong;

    py::class_<VectorDouble>("VectorDouble").def(py::vector_indexing_suite<VectorDouble>());
    py::class_<VectorInt>("VectorInt").def(py::vector_indexing_suite<VectorInt>());
    py::class_<VectorBool>("VectorBool").def(py::vector_indexing_suite<VectorBool>());
    py::class_<VectorUChar>("VectorUChar").def(py::vector_indexing_suite<VectorUChar>());
    py::class_<VectorUInt>("VectorUInt").def(py::vector_indexing_suite<VectorUInt>());
    py::class_<VectorULong>("VectorULong").def(py::vector_indexing_suite<VectorULong>());

    py::class_<VectorVectorDouble>("VectorVectorDouble").def(py::vector_indexing_suite<VectorVectorDouble>());
    py::class_<VectorVectorInt>("VectorVectorInt").def(py::vector_indexing_suite<VectorVectorInt>());
    py::class_<VectorVectorULong>("VectorVectorULong").def(py::vector_indexing_suite<VectorVectorULong>());

}*/

void export_blaze_matrices(py::module& m);


std::vector<long> data(const std::vector<long>& ro_a, std::vector<long>& rw_a) {
    std::vector<long> result = {1, 2, 3};
    std::cout << &ro_a << " x " << &rw_a << " -> " << &result << std::endl;
    result.insert(result.end(), ro_a.begin(), ro_a.end());
    rw_a.front() = 777;
    return result;
}

PYBIND11_MAKE_OPAQUE(std::vector<long>);

PYBIND11_MODULE(metric, m) {
    py::bind_vector<std::vector<long>>(m, "LongVector", py::buffer_protocol())
        .def("ptr", [](std::vector<long>& self){ return (unsigned long)&self; });
    py::bind_vector<std::vector<double>>(m, "DoubleVector", py::buffer_protocol());
    //export_converters();
    // exposing C++ return types
    //export_containers();

    export_blaze_matrices(m);
    m.def("test", &data,
        py::arg("a"),
        py::arg("b")
    );
}
