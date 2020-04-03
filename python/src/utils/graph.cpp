#include "modules/utils/graph.hpp"
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>

namespace py = pybind11;

// here we need another approach (dynamic class resolution)
//template <typename WeightType = bool, bool isDense = false, bool isSymmetric = true>
//metric::Graph<WeightType, isDense, isSymmetric> createGraph(bool isDense = false, bool isSymmetric = true)
//{
//    return metric::Graph<WeightType, isDense, isSymmetric>();
//}

void register_wrapper_graph(py::module& m) {
    using Class = metric::Graph<>;
    auto cls = py::class_<Class>(m, "Graph", "Graph based on blaze-lib");
    cls.def(py::init<>());
    cls.def(py::init<size_t>(),
        py::arg("nodes_number")
    );
    cls.def(py::init<const std::vector<std::pair<size_t, size_t>>&>(),
        py::arg("edges_pairs")
    );
    cls.def_property_readonly("nodes_number", &Class::getNodesNumber);
    cls.def_property_readonly("is_valid", &Class::isValid);
    cls.def("neighbours", &Class::template getNeighbours<>,
        py::arg("node_index"),
        py::arg("max_deep")
    );
    cls.def("get_matrix", &Class::get_matrix);
    cls.def("build", &Class::buildEdges,
        py::arg("edges_pairs")
    );
}

void register_wrapper_grids(py::module& m) {
    {
        using Class = metric::Grid4;
        py::class_<Class, metric::Graph<>>(m, "Grid4")
            .def(py::init<size_t>(), py::arg("nodes_number"))
            .def(py::init<size_t, size_t>(), py::arg("width"), py::arg("height"));
    }
    {
        using Class = metric::Grid6;
        py::class_<Class, metric::Graph<>>(m, "Grid6")
            .def(py::init<size_t>(), py::arg("nodes_number"))
            .def(py::init<size_t, size_t>(), py::arg("width"), py::arg("height"));
    }
    {
        using Class = metric::Grid8;
        py::class_<Class, metric::Graph<>>(m, "Grid8")
            .def(py::init<size_t>(), py::arg("nodes_number"))
            .def(py::init<size_t, size_t>(), py::arg("width"), py::arg("height"));
    }
}

void export_metric_graphs(py::module& m) {
    register_wrapper_graph(m);
    register_wrapper_grids(m);
}