#include "modules/mapping/DSPCC.hpp"

#include <boost/python.hpp>
#include <vector>
#include <tuple>
#include <deque>

namespace bp = boost::python;

template <typename recType, typename Metric>
void register_wrapper_DSPCC() {
    using Mapping = metric::DSPCC<recType, Metric>;
    using Matrix = std::vector<std::vector<recType>>;
    using Queue = std::deque<std::vector<recType>>;
    auto dspcc = bp::class_<Mapping>("DSPCC", bp::init<const std::vector<recType>&, size_t, size_t, float, size_t>(
        (
            bp::arg("training_dataset"),
            bp::arg("n_features_") = 1,
            bp::arg("n_subbands_") = 4,
            bp::arg("time_freq_balance_") = 0.5,
            bp::arg("n_top_features_") = 16
        )
    ));
    Matrix (Mapping::*encode1) (const std::vector<recType>&) = &Mapping::time_freq_PCFA_encode;
    Matrix (Mapping::*encode2) (const std::tuple<Queue, Queue>&) = &Mapping::time_freq_PCFA_encode;
    dspcc.def("time_freq_PCFA_encode", encode1);
    dspcc.def("time_freq_PCFA_encode", encode2);
    dspcc.def("time_freq_PCFA_decode", &Mapping::time_freq_PCFA_decode);
    dspcc.def("mixed_code_serialize", &Mapping::mixed_code_serialize);
    dspcc.def("encode", &Mapping::mixed_code_serialize);
    dspcc.def("decode", &Mapping::mixed_code_serialize);
}

void export_metric_DSPCC() {
    register_wrapper_DSPCC<std::vector<double>, void>();
}
