#include "modules/distance/k-related/L1.hpp"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include <vector>

namespace py = pybind11;


template<typename Value>
void register_wrapper(py::module& m) {
    {
        using Specialization = double (*)(const blaze::CompressedVector<Value>&, const blaze::CompressedVector<Value>&);
        m.def<Specialization>("sorensen", &metric::sorensen<Value>,
            "Sørensen–Dice coefficient"
        );
    }

    {
        using Specialization = double (*)(const std::vector<Value>&, const std::vector<Value>&);
        m.def<Specialization>("sorensen", &metric::sorensen<std::vector<Value>>,
            "Sørensen–Dice coefficient",
            py::arg("a"),
            py::arg("b")
        );
    }
}

void export_metric_sorensen(py::module& m) {
    register_wrapper<double>(m);
}
