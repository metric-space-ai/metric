/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2020 Panda Team
*/

#include "modules/correlation.hpp"
#include <boost/python.hpp>
#include <iostream>

namespace bp = boost::python;

struct simple_user_euclidian {

    double operator()(const std::vector<double>& a, const std::vector<double>& b) const
    {
        double sum = 0;
        for (size_t i = 0; i < a.size(); ++i) {
            sum += (a[i] - b[i]) * (a[i] - b[i]);
        }
        return std::sqrt(sum);
    }
    template <class Container>
    double operator()(const Container& aa, const Container& bb) const
    {
        const bp::list& a = bp::extract<bp::list>(aa);
        const bp::list& b = bp::extract<bp::list>(bb);
        double sum = 0;
        for (size_t i = 0; i < bp::len(a); ++i) {
            double a_i = bp::extract<double>(a[i]);
            double b_i = bp::extract<double>(b[i]);
            sum += (a_i - b_i) * (a_i - b_i);
        }
        return std::sqrt(sum);
    }
};

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

template <class Metric1, class Metric2>
void bind_MGC(const char* name) {
    using MGC = metric::MGC<NotUsed, Metric1, NotUsed, Metric2>;
    bp::class_<MGC>(name)
        .def("estimate", +[](MGC& self, bp::object& A, bp::object& B, const size_t BsampleSize = 250, const double threshold = 0.05, size_t maxIterations = 1000) {
            return self.estimate(python_wrap_stl(A), python_wrap_stl(B), BsampleSize, threshold, maxIterations);
        }, "return estimate of the correlation betweeen A and B")
        .def("calculate", +[](MGC& self, bp::object& A, bp::object& B) {
            return self.operator()(python_wrap_stl(A), python_wrap_stl(B));
        }, "return correlation betweeen A and B");
}

BOOST_PYTHON_MODULE(metric)
{

    bind_MGC<simple_user_euclidian,simple_user_euclidian>("MGC_SimpleEuclidian_SimpleEuclidian");

    bind_MGC<metric::Euclidian<double>,metric::Euclidian<double>>("MGC_Euclidian_Euclidian");
    bind_MGC<metric::Euclidian<double>,metric::Manhatten<double>>("MGC_Euclidian_Manhatten");

    bind_MGC<metric::Manhatten<double>,metric::Manhatten<double>>("MGC_Manhatten_Manhatten");
    bind_MGC<metric::Manhatten<double>,metric::Euclidian<double>>("MGC_Manhatten_Euclidian");
}

