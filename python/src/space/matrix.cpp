#include "modules/space/matrix.hpp"
#include "modules/distance.hpp"
#include "metric_py.hpp"

#include <boost/python.hpp>
#include <vector>
#include <utility>
#include <iostream>

namespace bp = boost::python;

// TODO: add metric
template<typename recType, typename Metric>
void register_wrapper_matrix() {
    using Matrix = metric::Matrix<recType, Metric>;
    using Container = std::vector<recType>;
    size_t (Matrix::*insert1)(const recType&) = &Matrix::insert;
    std::vector<size_t> (Matrix::*insert2)(const Container&) = &Matrix::insert;
    std::pair<std::size_t, bool> (Matrix::*insert_if1)(const recType&, typename Matrix::distType) = &Matrix::insert_if;
    std::vector<std::pair<std::size_t, bool>> (Matrix::*insert_if2)(const Container&, typename Matrix::distType) = &Matrix::insert_if;

    bp::class_<Matrix>("Matrix", bp::no_init)
        .def(bp::init<Metric>(
            (
                bp::arg("d") = Metric()
            )
        ))
        .def(bp::init<const recType&, Metric>(
            (
                bp::arg("p"),
                bp::arg("d") = Metric()
            )
        ))
        .def(bp::init<const Container&, Metric>(
            (
                bp::arg("p"),
                bp::arg("d") = Metric()
            )
        ))
        .def("insert", insert1)
        .def("insert_if", insert_if1)
        .def("insert", insert2)
        .def("insert_if", insert_if2)
        .def("erase", &Matrix::erase)
        //.def("__getitem__", &Matrix::operator[]) // FIXME: broken in CPP
        .def("__setitem__", &Matrix::set)
        .def("__call__", &Matrix::operator())
        .def("erase", &Matrix::erase)
        .def("__len__", &Matrix::size)
        .def("nn", &Matrix::nn)
        .def("knn", &Matrix::knn)
        .def("rnn", &Matrix::rnn);
}

void export_metric_matrix() {
    register_wrapper_matrix<std::vector<double>, metric::Euclidian<double>>();
}
