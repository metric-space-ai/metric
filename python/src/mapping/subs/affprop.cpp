#include "modules/mapping/affprop.hpp"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include <vector>

namespace py = pybind11;

template <typename RecType, typename Metric>
void wrap_affprop(py::module& m) {
    using Class = metric::AffProp<RecType, Metric>;
    using Value = typename Metric::distance_type;

    auto cls = py::class_<Class>(m, "AffProp", "Affinity Propagation");
    cls.def(py::init<>());
    cls.def(py::init<Value, int, Value, Value>(),
        py::arg("preference") = 0.5,
        py::arg("maxiter") = 200,
        py::arg("tol") = 1.0e-6,
        py::arg("damp") = 0.5
    );
    cls.def("__call__", &Class::operator(),
        py::arg("dm")
    );
}

void export_metric_affprop(py::module& m) {
    using Value = double;
    using Container = std::vector<Value>;
    using Functor = std::function<Value(const Container&, const Container&)>;

    boost::mpl::for_each<metric::MetricTypes, boost::mpl::make_identity<boost::mpl::_1>>([&](auto metr) {
        using Metric = typename decltype(metr1)::type;
        wrap_affprop<Container, Metric>(m);
    });
    wrap_affprop<Container, Functor>;
}

PYBIND11_MODULE(affprop, m) {
    export_metric_affprop(m);
}