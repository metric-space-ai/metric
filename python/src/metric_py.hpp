/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2020 Panda Team
*/

#pragma once
#include <boost/python.hpp>
#include <boost/python/numpy.hpp>
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
namespace bpn = boost::python::numpy;

using base_python_object = bp::api::object;

template<typename T, class Allocator = std::allocator<T>>
class WrapStlVector: public base_python_object {
    public:
        typedef T value_type;
        WrapStlVector() = default;
        WrapStlVector(base_python_object obj)
            : base_python_object(obj)
        {
        }
        size_t size() const {return bp::len(*this);}

        bool empty() const {return size() == 0;}

        bp::stl_input_iterator<T> begin() const {
            return bp::stl_input_iterator<T>(*this);
        }

        bp::stl_input_iterator<T> end() const {
            return bp::stl_input_iterator<T>();
        }

        template<typename U = T, typename std::enable_if<std::is_floating_point<U>::value>::type* = nullptr>
        U operator[](int index) const  {
            U wr = bp::extract<U>(base_python_object::operator[](index));
            return wr;
        }

        template<typename U = T, typename std::enable_if<std::is_integral<U>::value>::type* = nullptr>
        U operator[](int index) const  {
            U wr = bp::extract<U>(base_python_object::operator[](index));
            return wr;
        }

        template<typename U = T, typename std::enable_if<!std::is_floating_point<U>::value>::type* = nullptr>
        WrapStlVector<typename U::value_type> operator[](int index) const {
            base_python_object wr = bp::extract<base_python_object>(base_python_object::operator[](index));
            return WrapStlVector<typename U::value_type>(wr);
        }

};

template<typename T>
class WrapStlMatrix: public base_python_object {
    public:
        typedef std::vector<T> value_type;
        WrapStlMatrix() = default;
        WrapStlMatrix(base_python_object& obj)
            : base_python_object(obj)
        {
        }
        size_t size() const {return bp::len(*this);}

        bool empty() const {return size() == 0;}

        bp::stl_input_iterator<T> begin() const {
            return bp::stl_input_iterator<T>(*this);
        }

        bp::stl_input_iterator<T> end() const {
            return bp::stl_input_iterator<T>();
        }

        WrapStlMatrix operator[](int index) const {
            base_python_object wr = bp::extract<base_python_object>(base_python_object::operator[](index));
            return WrapStlMatrix(wr);
        }
};

template<class T>
struct MPLHelpType
{
    typedef T agrument_type;
};

static std::string getObjType(const bp::api::object& obj) {
    boost::python::extract<boost::python::object> objectExtractor(obj);
    boost::python::object o=objectExtractor();
    std::string obj_type = boost::python::extract<std::string>(o.attr("__class__").attr("__name__"));
    return obj_type;
}

static std::string getObjType(PyObject* obj_ptr) {
       boost::python::object obj(boost::python::handle<>(boost::python::borrowed(obj_ptr)));
       return getObjType(obj);
}
