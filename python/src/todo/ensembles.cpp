#include "modules/mapping/ensembles.hpp"

#include <boost/python.hpp>
#include <functional>
#include <vector>
#include <variant>

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

    bp::class_<Mapping>("Bagging", bp::init<int, double, double, std::vector<double>, std::vector<WeakLearnerVariant>>())
        .def("train", train)
        .def("predict", predict);
}

void export_metric_ensembles() {
    using Record = std::vector<double>;
    using WeakLearner = metric::edmClassifier<Record, CSVM>;
    using WeakLearnerVariant = std::variant<metric::edmSVM<Record>, metric::edmClassifier<Record, CSVM>>;
    register_wrapper_Boosting<Record, WeakLearner, metric::SubsampleRUS<Record>>();
    register_wrapper_Bagging<Record, WeakLearnerVariant, metric::SubsampleRUS<Record>>();
}

template <class Record>
class DT {
public:
    DT(double entropy_threshold_ = 0, double gain_threshold_ = 0);

    template <typename ConType, typename VariantType>
    void train(ConType& payments, std::vector<VariantType>, std::function<int(const Record&)>& response);

    template <typename ConType, typename VariantType>
    void predict(ConType& input_data, std::vector<VariantType> dimensions, std::vector<int>& predictions);

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

BOOST_PYTHON_MODULE(_ensembles) {
    export_metric_ensembles();
    export_metric_DT();
}