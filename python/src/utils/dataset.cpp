#include "modules/utils/datasets.hpp"
#include <boost/python.hpp>
#include <tuple>

namespace py = boost::python;

void register_wrapper_datasets() {
    using Class = Datasets;
    auto dim = py::class_<Class>("Datasets");
    dim.def("get_mnist", +[](Class& self, const std::string filename){
        auto result = self.getMnist(filename);
        return py::make_tuple(std::get<0>(result), std::get<1>(result), std::get<2>(result));
    });
}

void export_metric_datasets() {
    register_wrapper_datasets();
}