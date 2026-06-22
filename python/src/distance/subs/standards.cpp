#include "metric/metric/catalog/vector/Standards.hpp"
#include "stl_wrappers.hpp"

#include <pybind11/functional.h>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <string>
#include <typeinfo>

namespace py = pybind11;

template <typename T> const std::string python__str__(const T &) { return "<" + std::string(typeid(T).name()) + ">"; }

template <typename Value, typename Container> void register_wrapper_euclidean(py::module &m)
{
	using Metric = mtrc::Euclidean<Value>;
	auto p1 = &Metric::template operator()<Container>;
	Value (Metric::*p2)(const Value &, const Value &) const = &Metric::operator();
	py::class_<Metric>(m, "Euclidean")
		.def(py::init<>())
		.def("__call__", p1)
		.def("__call__", p2)
		.def("__call__", &Metric::template operator()<std::vector<Value>>)
		.def("__repr__", &python__str__<Metric>);
}

template <typename Value, typename Container> void register_wrapper_manhattan(py::module &m)
{
	using Metric = mtrc::Manhattan<Value>;
	py::class_<Metric>(m, "Manhattan")
		.def(py::init<>())
		.def("__call__", &Metric::template operator()<Container>)
		.def("__call__", &Metric::template operator()<std::vector<Value>>)
		.def("__repr__", &python__str__<Metric>);
}

template <typename Value, typename Container> void register_wrapper_pnorm(py::module &m)
{
	using Metric = mtrc::P_norm<Value>;
	py::class_<Metric>(m, "P_norm")
		.def(py::init<Value>(), py::arg("p") = 1)
		.def_property_readonly("p", &Metric::exponent)
		.def("__call__", &Metric::template operator()<Container>)
		.def("__call__", &Metric::template operator()<std::vector<Value>>)
		.def("__repr__", &python__str__<Metric>);
}

template <typename Value, typename Container> void register_wrapper_euclidean_thresholded(py::module &m)
{
	using Metric = mtrc::Euclidean_thresholded<Value>;
	py::class_<Metric>(m, "Euclidean_thresholded")
		.def(py::init<>())
		.def(py::init<Value, Value>(), py::arg("thres"), py::arg("factor"))
		.def("__call__", &Metric::template operator()<Container>)
		.def("__call__", &Metric::template operator()<std::vector<Value>>)
		.def("__repr__", &python__str__<Metric>);
}

template <typename Value, typename Container> void register_wrapper_cosine(py::module &m)
{
	using Metric = mtrc::Cosine<Value>;
	py::class_<Metric>(m, "Cosine")
		.def(py::init<>())
		.def("__call__", &Metric::template operator()<Container>)
		.def("__call__", &Metric::template operator()<std::vector<Value>>)
		.def("__repr__", &python__str__<Metric>);
}

template <typename Value, typename Container> void register_wrapper_chebyshev(py::module &m)
{
	using Metric = mtrc::Chebyshev<Value>;
	py::class_<Metric>(m, "Chebyshev")
		.def(py::init<>())
		.def("__call__", &Metric::template operator()<Container>)
		.def("__call__", &Metric::template operator()<std::vector<Value>>)
		.def("__repr__", &python__str__<Metric>);
}

void export_metric_standards(py::module &m)
{
	register_wrapper_euclidean<double, NumpyToVectorAdapter<double>>(m);
	register_wrapper_manhattan<double, NumpyToVectorAdapter<double>>(m);
	register_wrapper_pnorm<double, NumpyToVectorAdapter<double>>(m);
	register_wrapper_euclidean_thresholded<double, NumpyToVectorAdapter<double>>(m);
	register_wrapper_cosine<double, NumpyToVectorAdapter<double>>(m);
	register_wrapper_chebyshev<double, NumpyToVectorAdapter<double>>(m);
}

#ifndef METRIC_PYTHON_EMBED_STANDARDS_IN_DISTANCE
PYBIND11_MODULE(standards, m) { export_metric_standards(m); }
#endif
