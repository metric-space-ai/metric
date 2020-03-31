#include "modules/space/matrix.hpp"
#include "modules/distance/k-related/Standards.hpp"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include <vector>
#include <utility>
#include <string>
#include <typeinfo>

namespace py = pybind11;

template<typename Container, typename recType, typename Metric>
metric::Matrix<recType, Metric> createMatrix(const Metric& m, const Container& data) {
    return metric::Matrix<recType, Metric>(data, m);
}

template<typename recType, typename Metric>
metric::Matrix<recType, Metric> createMatrixEmpty(const Metric& m) {
    return metric::Matrix<recType, Metric>(m);
}

template<typename recType, typename Metric>
void register_wrapper_matrix(py::module& m) {
    using Matrix = metric::Matrix<recType, Metric>;
    using Container = std::vector<recType>;

    m.def("create_matrix", &createMatrix<Container, recType, Metric>,
        py::arg("metric"),
        py::arg("data")
    );
    m.def("create_matrix", &createMatrix<recType, recType, Metric>,
        py::arg("metric"),
        py::arg("data")
    );
    m.def("create_matrix", &createMatrixEmpty<recType, Metric>,
        py::arg("metric")
    );

    size_t (Matrix::*insert1)(const recType&) = &Matrix::insert;
    std::vector<size_t> (Matrix::*insert2)(const Container&) = &Matrix::insert;
    std::pair<std::size_t, bool> (Matrix::*insert_if1)(const recType&, typename Matrix::distType) = &Matrix::insert_if;
    std::vector<std::pair<std::size_t, bool>> (Matrix::*insert_if2)(const Container&, typename Matrix::distType) = &Matrix::insert_if;

    const std::string name = std::string("Matrix__") + std::string(typeid(Metric).name());
    py::class_<Matrix>(m, name.c_str())
        .def(py::init<>(), "empty matrix")
        .def(py::init<const recType&>(), py::arg("p"))
        .def(py::init<const Container&>(), py::arg("p"))
        .def("insert", insert1)
        .def("insert_if", insert_if1)
        .def("insert", insert2)
        .def("insert_if", insert_if2)
        .def("erase", &Matrix::erase)
        .def("__getitem__", &Matrix::operator[])
        .def("__setitem__", &Matrix::set)
        .def("__call__", &Matrix::operator())
        .def("erase", &Matrix::erase)
        .def("__len__", &Matrix::size)
        .def("nn", &Matrix::nn)
        .def("knn", &Matrix::knn)
        .def("rnn", &Matrix::rnn);
}

void export_metric_matrix(py::module& m) {
    register_wrapper_matrix<std::vector<double>, metric::Euclidian<double>>(m);
    register_wrapper_matrix<std::vector<double>, metric::Manhatten<double>>(m);
    register_wrapper_matrix<std::vector<double>, metric::P_norm<double>>(m);
}
