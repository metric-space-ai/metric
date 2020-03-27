/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2020 Panda Team
*/

#pragma once

#include <pybind11/pybind11.h>
#include <vector>

using base_python_object = boost::python::api::object;

template<typename T, class Allocator = std::allocator<T>>
class WrapStlVector: public base_python_object {
    public:
        typedef T value_type;
        WrapStlVector() = default;
        WrapStlVector(base_python_object obj)
            : base_python_object(obj)
        {
        }
        size_t size() const {return boost::python::len(*this);}

        bool empty() const {return size() == 0;}

        boost::python::stl_input_iterator<T> begin() const {
            return boost::python::stl_input_iterator<T>(*this);
        }

        boost::python::stl_input_iterator<T> end() const {
            return boost::python::stl_input_iterator<T>();
        }

        template<typename U = T, typename std::enable_if<std::is_floating_point<U>::value>::type* = nullptr>
        U operator[](int index) const  {
            U wr = boost::python::extract<U>(base_python_object::operator[](index));
            return wr;
        }

        template<typename U = T, typename std::enable_if<std::is_integral<U>::value>::type* = nullptr>
        U operator[](int index) const  {
            U wr = boost::python::extract<U>(base_python_object::operator[](index));
            return wr;
        }

        template<typename U = T, typename std::enable_if<!std::is_floating_point<U>::value>::type* = nullptr>
        WrapStlVector<typename U::value_type> operator[](int index) const {
            base_python_object wr = boost::python::extract<base_python_object>(base_python_object::operator[](index));
            return WrapStlVector<typename U::value_type>(wr);
        }
};

template<typename T>
class WrapStlMatrix: public base_python_object {
public:
    typedef std::vector<T> value_type;
    WrapStlMatrix() = default;
    WrapStlMatrix(base_python_object& obj)
        : base_python_object(obj) {
    }

    size_t size() const {
        return boost::python::len(*this);
    }

    bool empty() const {
        return size() == 0;
    }

	boost::python::stl_input_iterator<T> begin() const {
        return boost::python::stl_input_iterator<T>(*this);
    }

	boost::python::stl_input_iterator<T> end() const {
        return boost::python::stl_input_iterator<T>();
    }

    WrapStlMatrix operator[](int index) const {
        base_python_object wr = boost::python::extract<base_python_object>(base_python_object::operator[](index));
        return WrapStlMatrix(wr);
    }
};
