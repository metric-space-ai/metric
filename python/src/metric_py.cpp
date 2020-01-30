/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2020 Panda Team
*/

#include "modules/correlation.hpp"
#include "modules/distance.hpp"
#include "metric_py.hpp"

#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include <boost/python/object.hpp>
#include <boost/python/converter/implicit.hpp>
#include <boost/python/converter/registry.hpp>
#include <boost/python/module.hpp>
#include <numpy/arrayobject.h>
#include <numpy/arrayscalars.h>
#include "metric_MGC.hpp"
#include "metric_Edit.hpp"
#include "metric_Entropy.hpp"
#include "metric_EMD.hpp"
//#include "metric_affprop.hpp"

typedef std::vector<double> VectorDouble;
typedef std::vector<VectorDouble> VectorVectorDouble;
typedef std::vector<int> VectorInt;
typedef std::vector<VectorInt> VectorVectorInt;

template <typename ScalarType>
struct NumpyScalarConverter {

    NumpyScalarConverter() {
        using namespace boost::python;
        converter::registry::push_back(&convertible, &construct, type_id<ScalarType>());
    }

    static void* convertible(PyObject* obj_ptr) {
        std::string name = getObjType(obj_ptr);

        if (
                name == "float32" ||
                name == "float64" ||
                name == "int8" ||
                name == "int16" ||
                name == "int32" ||
                name == "int64" ||
                name == "uint8" ||
                name == "uint16" ||
                name == "uint32" ||
                name == "uint64"
            ) {
            return obj_ptr;
        }
        return 0;
    }

    static void construct(PyObject* obj_ptr, boost::python::converter::rvalue_from_python_stage1_data* data) {
        using namespace boost::python;
        std::string name = getObjType(obj_ptr);

        void* storage = ((converter::rvalue_from_python_storage<ScalarType>*) data)->storage.bytes;

        ScalarType * scalar = new (storage) ScalarType;

        if (name == "float32")
            (*scalar) = PyArrayScalar_VAL(obj_ptr, Float32);
        else if (name == "float64")
            (*scalar) = PyArrayScalar_VAL(obj_ptr, Float64);
        else if (name == "int8")
            (*scalar) = PyArrayScalar_VAL(obj_ptr, Int8);
        else if (name == "int16")
            (*scalar) = PyArrayScalar_VAL(obj_ptr, Int16);
        else if (name == "int32")
            (*scalar) = PyArrayScalar_VAL(obj_ptr, Int32);
        else if (name == "int64")
            (*scalar) = PyArrayScalar_VAL(obj_ptr, Int64);
        else if (name == "uint8")
            (*scalar) = PyArrayScalar_VAL(obj_ptr, UInt8);
        else if (name == "uint16")
            (*scalar) = PyArrayScalar_VAL(obj_ptr, UInt16);
        else if (name == "uint32")
            (*scalar) = PyArrayScalar_VAL(obj_ptr, UInt32);
        else if (name == "uint64")
            (*scalar) = PyArrayScalar_VAL(obj_ptr, UInt64);

        data->convertible = storage;
    }
};

BOOST_PYTHON_MODULE(metric) {

    bp::class_<VectorDouble>("VectorDouble").def(bp::vector_indexing_suite<VectorDouble>());
    bp::class_<VectorVectorDouble>("VectorVectorDouble").def(bp::vector_indexing_suite<VectorVectorDouble>());
    bp::class_<VectorInt>("VectorInt").def(bp::vector_indexing_suite<VectorInt>());
    bp::class_<VectorVectorInt>("VectorVectorInt").def(bp::vector_indexing_suite<VectorVectorInt>());

    NumpyScalarConverter<signed char>();
    NumpyScalarConverter<short>();
    NumpyScalarConverter<int>();
    NumpyScalarConverter<long>();
    NumpyScalarConverter<long long>();
    NumpyScalarConverter<unsigned char>();
    NumpyScalarConverter<unsigned short>();
    NumpyScalarConverter<unsigned int>();
    NumpyScalarConverter<unsigned long>();
    NumpyScalarConverter<unsigned long long>();
    NumpyScalarConverter<float>();
    NumpyScalarConverter<double>();


#include "metric_MGC.cpp"
#include "metric_Edit.cpp"
#include "metric_Entropy.cpp"
#include "metric_EMD.cpp"
//#include "metric_affprop.cpp"

}

