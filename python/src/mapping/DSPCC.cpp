#include "modules/mapping/DSPCC.hpp"

#include <boost/python.hpp>
#include <vector>
#include <tuple>
#include <deque>

namespace py = boost::python;

template <typename recType, typename Metric>
void register_wrapper_DSPCC() {
    using Mapping = metric::DSPCC<recType, Metric>;
    using Matrix = std::vector<std::vector<recType>>;
    using Queue = std::deque<std::vector<recType>>;

    auto dspcc = py::class_<Mapping>("DSPCC", py::init<const std::vector<recType>&, size_t, size_t, float, size_t>(
        (
            py::arg("training_dataset"),
            py::arg("n_features_") = 1,
            py::arg("n_subbands_") = 4,
            py::arg("time_freq_balance_") = 0.5,
            py::arg("n_top_features_") = 16
        )
    ));
    Matrix (Mapping::*encode) (const std::vector<recType>&) = &Mapping::time_freq_PCFA_encode;
    dspcc.def("time_freq_PCFA_encode", encode);
    dspcc.def("time_freq_PCFA_decode", &Mapping::time_freq_PCFA_decode);
    dspcc.def("encode", &Mapping::encode);
    dspcc.def("decode", &Mapping::decode);
}

void export_metric_DSPCC() {
    register_wrapper_DSPCC<std::vector<double>, void>();
}
