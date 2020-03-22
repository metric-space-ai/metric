#include "modules/mapping/ensembles.hpp"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include <functional>
#include <vector>
#include <variant>

namespace py = pybind11;

template <typename Record, typename WeakLearner, typename Subsampler>
void register_wrapper_Boosting(py::module& m) {
    using Mapping = metric::Boosting<Record, WeakLearner, Subsampler>;
    using Container = std::vector<Record>;
    using Features = std::vector<std::function<double(Record)>>;
    using Callback = std::function<bool(Record)>;

    void (Mapping::*train)(Container&, Features&, Callback&, bool) = &Mapping::train;
    void (Mapping::*predict)(Container&, Features&, std::vector<bool>&) = &Mapping::predict;

    py::class_<Mapping>(m, "Boosting")
        .def(py::init<int, double, double, WeakLearner>(),
            py::arg("ensemble_size_"),
            py::arg("share_overall"),
            py::arg("share_minor"),
            py::arg("weak_classifier")
        )
        .def("train", train)
        .def("predict", predict);
}

template <typename Record, typename WeakLearnerVariant, typename Subsampler>
void register_wrapper_Bagging(py::module& m) {
    using Mapping = metric::Bagging<Record, WeakLearnerVariant, Subsampler>;
    using Container = std::vector<Record>;
    using Features = std::vector<std::function<double(Record)>>;
    using Callback = std::function<bool(Record)>;

    void (Mapping::*train)(Container&, Features&, Callback&, bool) = &Mapping::train;
    void (Mapping::*predict)(Container&, Features&, std::vector<bool>&) = &Mapping::predict;

    py::class_<Mapping>(m, "Bagging")
        .def(py::init<int, double, double, std::vector<double>, std::vector<WeakLearnerVariant>>(),
            py::arg("ensemble_size"),
            py::arg("share_overall"),
            py::arg("share_minor"),
            py::arg("type_weight"),
            py::arg("weak_classifiers")
        )
        .def("train", train)
        .def("predict", predict);
}

void export_metric_ensembles(py::module& m) {
    using Record = std::vector<double>;
    using WeakLearner = metric::edmClassifier<Record, CSVM>;
    using WeakLearnerVariant = std::variant<metric::edmSVM<Record>, metric::edmClassifier<Record, CSVM>>;
    register_wrapper_Boosting<Record, WeakLearner, metric::SubsampleRUS<Record>>(m);
    register_wrapper_Bagging<Record, WeakLearnerVariant, metric::SubsampleRUS<Record>>(m);
}

PYBIND11_MODULE(_ensembles, m) {
    export_metric_ensembles(m);
}