#include "modules/space/matrix.hpp"
#include "metric_py.hpp"

#include <boost/python.hpp>
#include <vector>
#include <pair>
#include <iostream>

namespace bp = boost::python;

// TODO: add metric
template<typename recType, typename Metric>
void register_wrapper_matrix() {
    using Matrix = metric::Matrix<recType, Metric>;
    using Container = std::vector<recType>;
    size_t (Matrix::*insert1)(const recType&) = &Matrix::insert;
    std::pair<std::size_t, bool> (Matrix::*insert_if1)(const recType&, Matrix::distType) = &Matrix::insert_if;
    auto insert2 = &Matrix::insert<Container>;
    auto inset_if2 = &Matrix::insert_if<Container>;

    bp::class_<Matrix>("Matrix", bp::init<const Container&>())
        .def(bp::init<const recType&>()
        .def("insert", insert1)
        .def("insert_if", insert_if1)
        .def("insert", insert2)
        .def("insert_if", inset_if2)
        .def("erase", &Matrix::erase)
        .def("set", &Matrix::set)
        .def("__getitem__", &Matrix::operator[])
        .def("__setitem__", &Metric::set)
        .def("__call__", &Matrix::operator())
        .def("erase", &Matrix::erase)
        .def("size", &Matrix::size)
        .def("nn", &Matrix::nn)
        .def("knn", &Matrix::knn)
        .def("rnn", &Matrix::rnn);
}

void export_metric_matrix() {
    register_wrapper_matrix<std::vector<double>>();
}
