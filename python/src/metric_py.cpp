/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2020 Panda Team
*/

#include "modules/correlation.hpp"
#include <boost/python.hpp>
#include <boost/variant.hpp>
#include <iostream>

namespace bp = boost::python;

using base_python_object = bp::api::object;
class python_wrap_stl: public base_python_object {
    public:
        typedef std::vector<double> value_type;
        python_wrap_stl(base_python_object& obj)
            : base_python_object(obj)
        {
        }
        size_t size() const {return bp::len(*this);}

        bp::stl_input_iterator<double> begin() const {
            return bp::stl_input_iterator<double>(*this);
        }

        bp::stl_input_iterator<double> end() const {
            return bp::stl_input_iterator<double>();
        }

        python_wrap_stl operator[](int index) const {
            base_python_object wr = bp::extract<base_python_object>(base_python_object::operator[](index));
            return python_wrap_stl(wr);
        }

};

class NotUsed {};

class metrics {
    using MGC_Euclidian_Euclidian = metric::MGC<NotUsed,metric::Euclidian<double>,NotUsed,metric::Euclidian<double>>;
    using MGC_Euclidian_Manhatten = metric::MGC<NotUsed,metric::Euclidian<double>,NotUsed,metric::Manhatten<double>>;
    using MGC_Manhatten_Manhatten = metric::MGC<NotUsed,metric::Manhatten<double>,NotUsed,metric::Manhatten<double>>;
    using MGC_Manhatten_Euclidian = metric::MGC<NotUsed,metric::Manhatten<double>,NotUsed,metric::Euclidian<double>>;

    using mgc_variant = boost::variant<
            MGC_Euclidian_Euclidian,
            MGC_Euclidian_Manhatten,
            MGC_Manhatten_Manhatten,
            MGC_Manhatten_Euclidian
        >;
    static const char constexpr* euclidian = "euclidian";
    static const char constexpr* manhatten = "manhatten";
public:
    std::shared_ptr<mgc_variant> mgc;
    metrics(const std::string& Metric1, const std::string& Metric2)
    {
        if (Metric1 == euclidian && Metric2 == euclidian) mgc = std::make_shared<mgc_variant>(MGC_Euclidian_Euclidian());
        else if (Metric1 == euclidian && Metric2 == manhatten) mgc = std::make_shared<mgc_variant>(MGC_Euclidian_Manhatten());

        else if (Metric1 == manhatten && Metric2 == manhatten) mgc = std::make_shared<mgc_variant>(MGC_Manhatten_Manhatten());
        else if (Metric1 == manhatten && Metric2 == euclidian) mgc = std::make_shared<mgc_variant>(MGC_Manhatten_Euclidian());

        else throw std::runtime_error("No such metrics");
    }
};

BOOST_PYTHON_MODULE(metric)
{
    bp::class_<metrics>("MGC", bp::init<std::string,std::string>())
        .def("estimate", +[](metrics& self, bp::object& A, bp::object& B, const size_t BsampleSize = 250, const double threshold = 0.05, size_t maxIterations = 1000) {
            double ret = std::numeric_limits<double>::infinity();
            boost::apply_visitor([&](auto& obj) { ret = obj.estimate(python_wrap_stl(A), python_wrap_stl(B), BsampleSize, threshold, maxIterations); }, *self.mgc);
            return ret;
        }, "return estimate of the correlation betweeen A and B")
        .def("__call__", +[](metrics& self, bp::object& A, bp::object& B) {
            double ret = std::numeric_limits<double>::infinity();
            boost::apply_visitor([&](auto& obj) { ret = obj.operator()(python_wrap_stl(A), python_wrap_stl(B)); }, *self.mgc);
            return ret;
        }, "return correlation betweeen A and B");

}

