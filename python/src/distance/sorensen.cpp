#include "metric/metric/catalog/vector/L1.hpp"

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <vector>

namespace py = pybind11;

template <typename Value, typename Container> void register_wrapper(py::module &m)
{
	using Metric = mtrc::Sorensen<Value>;
	using BContainer = mtrc::numeric::CompressedVector<Value>;

	auto cls = py::class_<Metric>(m, "Sorensen", "Sorensen–Dice coefficient");
	cls.def(py::init<>());
	cls.def("__call__", (Value (Metric::*)(const Container &, const Container &) const) & Metric::operator(),
			py::arg("a"), py::arg("b"));
	cls.def("__call__", (Value (Metric::*)(const BContainer &, const BContainer &) const) & Metric::operator(),
			py::arg("a"), py::arg("b"));
	cls.def("__repr__", [](const Metric &a) { return "<Sorensen>"; });
}

void export_metric_sorensen(py::module &m) { register_wrapper<double, std::vector<double>>(m); }
