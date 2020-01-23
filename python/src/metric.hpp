/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2020 Panda Team
*/

#pragma once
#include <boost/python.hpp>
#include <boost/python/scope.hpp>

// define list of metric and thier names
using MetricTypes = boost::mpl::vector<
    metric::Euclidian<double>
    , metric::Manhatten<double>
    //, metric::Chebyshev<double>
    , metric::P_norm<double> // TODO: constructor argument
>;
std::vector<std::string> MetricTypeNames = {
    "euclidean",
    "manhatten",
    //"chebyshev",
    "p_norm",
};

namespace bp = boost::python;

using base_python_object = bp::api::object;
class python_wrap_stl: public base_python_object {
    public:
        typedef std::vector<double> value_type;
        python_wrap_stl() = default;
        python_wrap_stl(base_python_object& obj)
            : base_python_object(obj)
        {
        }
        size_t size() const {return bp::len(*this);}

        bool empty() const {return size() == 0;}

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

template<class T>
struct MPLHelpType
{
    typedef T agrument_type;
};

static std::string getType(const bp::api::object& obj) {
    boost::python::extract<boost::python::object> objectExtractor(obj);
    boost::python::object o=objectExtractor();
    std::string obj_type = boost::python::extract<std::string>(o.attr("__class__").attr("__name__"));
    return obj_type;
}
