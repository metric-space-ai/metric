#include "modules/space/matrix.hpp"
#include "metric_py.hpp"

#include <boost/python.hpp>
#include <vector>
#include <iostream>

namespace bp = boost::python;

// TODO: add metric
template<typename recType>
void register_wrapper_matrix() {
    using Matrix = metric::Matrix<recType>;
    // using Container = std::vector<recType>;
    // bool (Matrix::*append1)(const recType&) = &Matrix::append;
    // bool (Matrix::*append_if1)(const recType&, float) = &Matrix::append_if;
    // bool (Matrix::*append2)(const Container&) = &Matrix::append;
    // bool (Matrix::*append_if2)(const Container&, float) = &Matrix::append_if;

    bp::class_<Matrix>("Matrix", bp::init<const std::vector<recType>&>())
        // FIXME: missing in CPP
        //.def(bp::init<const recType&>()
        //.def("append", append1)
        //.def("append_if", append_if1)
        //.def("append", append2)
        //.def("append_if", append_if2)
        // .def("erase", &Matrix::erase)
        //.def("set", &Matrix::set)
        //.def("__getitem__", &Matrix::operator[])  // FIXME: this is broken in CPP
        .def("__call__", &Matrix::operator())
        .def("size", &Matrix::size);
}


void export_metric_matrix() {
    register_wrapper_matrix<std::vector<double>>();
}
