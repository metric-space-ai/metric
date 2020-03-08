#include "modules/mapping/ensembles.hpp"
#include <boost/python.hpp>
#include <functional>
#include <vector>
#include <variant>

namespace bp = boost::python;

template <typename Record>
void register_wrapper_DT() {
    using Mapping = metric::DT<Record>;
    using Container = std::vector<Record>;
    using Callback = std::function<int(const Record&)>;

    void (Mapping::*train)(Container&, std::vector<Record>, Callback&) = &Mapping::train;
    void (Mapping::*predict)(Container&, std::vector<Record>, std::vector<int>&) = &Mapping::predict;

    bp::class_<Mapping>("DT", bp::init<double, double>())
        .def("train", train)
        .def("predict", predict);
}

void export_metric_DT() {
    typedef std::variant<double, std::vector<double>, std::vector<std::vector<double>>, std::string> V;  // field type
    typedef std::vector<V> Record;
    register_wrapper_DT<Record>();
}