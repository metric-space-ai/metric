/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2020 Panda Team
*/

#pragma once
#include <boost/mpl/vector.hpp>
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
