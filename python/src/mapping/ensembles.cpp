#include "modules/mapping/ensembles.hpp"
#include <boost/python.hpp>
#include <functional>
#include <vector>

namespace bp = boost::python;

template <typename Record, typename WeakLearner, typename Subsampler>
void register_wrapper_Boosting() {
    using Mapping = metric::Boosting<Record, WeakLearner, Subsampler>;
    using Container = std::vector<Record>;
    using Features = std::vector<std::function<double(Record)>>;
    using Callback = std::function<bool(Record)>;

    void (Mapping::*train)(Container&, Features&, Callback&, bool) = &Mapping::train;
    void (Mapping::*predict)(Container&, Features&, std::vector<bool>&) = &Mapping::predict;

    bp::class_<Mapping>("Boosting", bp::init<int, double, double, WeakLearner>())
        .def("train", train)
        .def("predict", predict);
}

template <typename Record, typename WeakLearnerVariant, typename Subsampler>
void register_wrapper_Bagging() {
    using Mapping = metric::Bagging<Record, WeakLearnerVariant, Subsampler>;
    using Container = std::vector<Record>;
    using Features = std::vector<std::function<double(Record)>>;
    using Callback = std::function<bool(Record)>;

    void (Mapping::*train)(Container&, Features&, Callback&, bool) = &Mapping::train;
    void (Mapping::*predict)(Container&, Features&, std::vector<bool>&) = &Mapping::predict;

    bp::class_<Mapping>("Bagging", bp::init<int, double, double, std::vector<double>, std::vector<WeakLearner>>())
        .def("train", train)
        .def("predict", predict);
}

void export_metric_ensembles() {
    using Record = std::vector<double>;
    register_wrapper_Boosting<Record, metric::edmClassifier<Record, CSVM>, metric::SubsampleRUS<Record>>();
    register_wrapper_Bagging<Record, metric::edmClassifier<Record, CSVM>, metric::SubsampleRUS<Record>>();
}
