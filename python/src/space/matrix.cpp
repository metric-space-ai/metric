#include "modules/space/matrix.hpp"
#include "modules/distance/k-related/Standards.hpp"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include <vector>
#include <utility>
#include <string>

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
void register_wrapper_matrix(py::module& m, const std::string& postfix) {
    using Matrix = metric::Matrix<recType, Metric>;
    using Container = std::vector<recType>;

    m.def("create_matrix", &createMatrix<Container, recType, Metric>,
        "Distance matrix factory",
        py::arg("metric"),
        py::arg("items")
    );
    m.def("create_matrix", &createMatrix<recType, recType, Metric>,
        py::arg("metric"),
        py::arg("item")
    );
    m.def("create_matrix", &createMatrixEmpty<recType, Metric>,
        py::arg("metric")
    );

    size_t (Matrix::*insert1)(const recType&) = &Matrix::insert;
    std::vector<size_t> (Matrix::*insert2)(const Container&) = &Matrix::insert;
    std::pair<std::size_t, bool> (Matrix::*insert_if1)(const recType&, typename Matrix::distType) = &Matrix::insert_if;
    std::vector<std::pair<std::size_t, bool>> (Matrix::*insert_if2)(const Container&, typename Matrix::distType) = &Matrix::insert_if;

    const std::string name = std::string("Matrix_") + postfix;
    py::class_<Matrix>(m, name.c_str())
        .def(py::init<>(), "empty matrix")
        .def(py::init<const recType&>(),
            "Construct a new Matrix with one data record",
            py::arg("p")
        )
        .def(py::init<const Container&>(),
            "Construct a new Matrix with set of data records",
            py::arg("items")
        )
        .def("__repr__", [](const Matrix& self){ return "<Matrix object>"; })
        .def("insert", insert1,
            "append data record to the matrix",
            py::arg("item")
        )
        .def("insert_if", insert_if1,
            "append data record into the Matrix only if distance bigger than a threshold",
            py::arg("item"),
            py::arg("threshold")
        )
        .def("insert", insert2,
            "append set of data records to the matrix",
            py::arg("items")
        )
        .def("insert_if", insert_if2,
            "append data records into the Matrix only if distance bigger than a threshold",
            py::arg("items"),
            py::arg("threshold")
        )
        .def("erase", &Matrix::erase,
            "erase data record from Matrix by ID",
            py::arg("index")
        )
        .def("__getitem__", &Matrix::operator[],
            "access a data record by ID",
            py::arg("index")
        )
        .def("__setitem__", &Matrix::set,
            "change data record by ID",
            py::arg("index"),
            py::arg("record")
        )
        .def("__call__", &Matrix::operator(),
            "access a distance by two IDs",
            py::arg("i"),
            py::arg("j")
        )
        .def("__len__", &Matrix::size,
            "amount of data records"
        )
        .def("nn", &Matrix::nn,
            "find nearest neighbour of data record",
            py::arg("p")
        )
        .def("knn", &Matrix::knn,
            "find  K nearest neighbours of data record",
            py::arg("p"),
            py::arg("k") = 10
        )
        .def("rnn", &Matrix::rnn,
            "find all nearest neighbour in range [0;distance]",
            py::arg("p"),
            py::arg("range") = 1.0
        );
}

void export_metric_matrix(py::module& m) {
    register_wrapper_matrix<std::vector<double>, metric::Euclidian<double>>(m, "Euclidean");
    register_wrapper_matrix<std::vector<double>, metric::Manhatten<double>>(m, "Manhatten");
    register_wrapper_matrix<std::vector<double>, metric::P_norm<double>>(m, "P_norm");
}
