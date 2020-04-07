/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2020 Panda Team
*/

#pragma once

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <vector>


/**
    Tiny adapter that provides NumpyArray with std::vector like interface
*/
template<typename T>
class NumpyToVectorAdapter: public pybind11::array_t<T> {
public:
    using pybind11::array_t<T>::array_t;

    NumpyToVectorAdapter(pybind11::array_t<T> obj)
        : pybind11::array_t<T>(obj)
    {
    }

    bool empty() const {return this->size() == 0;}

    T* begin() const {
        return pybind11::detail::array_begin<T>(this->request());
    }

    T* end() const {
        return pybind11::detail::array_end<T>(this->request());
    }

    const T& operator[](size_t index) const {
        // TODO: init upon creation
        auto r = pybind11::array_t<T>::template unchecked<1>();
        return r[index];
    }
};

//template<typename T>
//class WrapStlMatrix: public base_python_object {
//public:
//    typedef std::vector<T> value_type;
//    WrapStlMatrix() = default;
//    WrapStlMatrix(base_python_object& obj)
//        : base_python_object(obj) {
//    }
//
//    size_t size() const {
//        return boost::python::len(*this);
//    }
//
//    bool empty() const {
//        return size() == 0;
//    }
//
//	boost::python::stl_input_iterator<T> begin() const {
//        return boost::python::stl_input_iterator<T>(*this);
//    }
//
//	boost::python::stl_input_iterator<T> end() const {
//        return boost::python::stl_input_iterator<T>();
//    }
//
//    WrapStlMatrix operator[](int index) const {
//        base_python_object wr = boost::python::extract<base_python_object>(base_python_object::operator[](index));
//        return WrapStlMatrix(wr);
//    }
//};
