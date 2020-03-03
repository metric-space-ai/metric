/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2020 Panda Team
*/

#include "metric_types.hpp"
#include "metric_py.hpp"

namespace metric_MGC {

class NotUsed {};

class MGCInterface {
public:
    virtual ~MGCInterface() = default;
    virtual double estimate(const WrapStlMatrix<double>& obj1, const WrapStlMatrix<double>& obj2, const size_t BsampleSize, const double threshold, size_t maxIterations) = 0;
    virtual double operator()(const WrapStlMatrix<double>& obj1, const WrapStlMatrix<double>& obj2) = 0;
};

template <class T1, class T2>
class MGCImpl: public metric::MGC<NotUsed,T1,NotUsed,T2>, public MGCInterface {
    virtual double estimate(const WrapStlMatrix<double>& obj1, const WrapStlMatrix<double>& obj2, const size_t BsampleSize, const double threshold, size_t maxIterations) override {
        return metric::MGC<NotUsed,T1,NotUsed,T2>::estimate(obj1, obj2);
    }
    virtual double operator()(const WrapStlMatrix<double>& obj1, const WrapStlMatrix<double>& obj2) override {
        return metric::MGC<NotUsed,T1,NotUsed,T2>::operator()(obj1, obj2);
    }
};

class metrics {
    using MetricTable =
        std::map<
                std::string,
                std::map<
                    std::string,
                    std::shared_ptr<MGCInterface>
                >
        >;

    static MetricTable table;
    static MetricTable CreateMetricTable() {
        MetricTable t;

        size_t i = 0;
        boost::mpl::for_each<MetricTypes, MPLHelpType<boost::mpl::_1> >([&](auto type_wrapper) {
        using Type1 = typename decltype(type_wrapper)::agrument_type;
            assert(MetricTypeNames.size() > i);
            auto name1 = MetricTypeNames[i++];
            size_t j = 0;
            boost::mpl::for_each<MetricTypes, MPLHelpType<boost::mpl::_1> >([&](auto type_wrapper) {
                assert(MetricTypeNames.size() > j);
                auto name2 = MetricTypeNames[j++];
                using Type2 = typename decltype(type_wrapper)::agrument_type;
                auto& tt = t[name1];
                tt.emplace(name2, std::make_shared<MGCImpl<Type1,Type2>>());
            });
        });

        return t;
    }


public:
    MGCInterface* mgc = nullptr;

    explicit metrics(const std::string& Metric1, const std::string& Metric2)
    {
        auto t = table.find(Metric1);
        if (t == table.end()) throw std::runtime_error("Metric1 not found");
        auto tt = t->second.find(Metric2);
        if (tt == t->second.end()) throw std::runtime_error("Metric2 not found");
        mgc = tt->second.get();
    }
};

metrics::MetricTable metrics::table = metrics::CreateMetricTable();

};


namespace bp = boost::python;

void export_metric_MGC()
{

bp::class_<metric_MGC::metrics>("MGC", bp::init<std::string,std::string>())
    .def("estimate", +[](metric_MGC::metrics& self, bp::object& A, bp::object& B, const size_t BsampleSize = 250, const double threshold = 0.05, size_t maxIterations = 1000) {
        return self.mgc->estimate(WrapStlMatrix<double>(A), WrapStlMatrix<double>(B), BsampleSize, threshold, maxIterations);
    }, "return estimate of the correlation betweeen A and B")
    .def("__call__", +[](metric_MGC::metrics& self, bp::object& A, bp::object& B) {
        return self.mgc->operator()(WrapStlMatrix<double>(A), WrapStlMatrix<double>(B));
    }, "return correlation betweeen A and B")
    ;
}


/*
    template <typename Container1, typename Container2>
    double operator()(const Container1& a, const Container2& b) const;

    template <typename Container1, typename Container2>
    double estimate(const Container1& a, const Container2& b, const size_t sampleSize = 250,
                    const double threshold = 0.05, size_t maxIterations = 1000);

    double mean(const std::vector<double>& data);
    double variance(const std::vector<double>& data, const double mean);
    std::vector<double> icdf(const std::vector<double>& prob, const double mu, const double sigma);
    double erfcinv(const double z);
    double erfinv_imp(const double p, const double q);
    double polyeval(const std::vector<double>& poly, const double z);
    double peak2ems(const std::vector<double>& data);
    std::vector<double> linspace(double a, double b, int n);
*/