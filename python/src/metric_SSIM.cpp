#include "modules/distance.hpp"

#include <boost/python.hpp>
#include <vector>

namespace bp = boost::python;

template<typename DistanceType, typename Value>
void register_wrapper_SSIM() {
    using Metric = metric::SSIM<DistanceType, Value>;
    using ValueType = typename Value::value_type;
    using Container = typename std::vector<Value>;
    bp::class_<Metric>("SSIM")
        .def(bp::init<ValueType, ValueType>((bp::arg("dynamic_range_"), bp::arg("masking_"))))
        .def("__call__", &Metric::template operator()<Container>);
}

void export_metric_SSIM() {
    register_wrapper_SSIM<double, std::vector<double>>();
}
