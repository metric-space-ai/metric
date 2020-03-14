#include "modules/distance/k-structured/SSIM.hpp"

#include <boost/python.hpp>
#include <vector>

namespace py = boost::python;

template<typename DistanceType, typename Value>
void register_wrapper_SSIM() {
    using Metric = metric::SSIM<DistanceType, Value>;
    using ValueType = typename Value::value_type;
    using Container = typename std::vector<Value>;
    py::class_<Metric>("SSIM")
        .def(py::init<ValueType, ValueType>((py::arg("dynamic_range_"), py::arg("masking_"))))
        .def("__call__", &Metric::template operator()<Container>);
}

void export_metric_SSIM() {
    register_wrapper_SSIM<double, std::vector<double>>();
}
