/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2020 Panda Team
*/

#include "metric_py.hpp"

#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
//#include <boost/python/object.hpp>
//#include <boost/python/converter/implicit.hpp>
//#include <boost/python/converter/registry.hpp>
//#include <boost/python/module.hpp>
//#include "metric_affprop.hpp"

namespace bp = boost::python;

typedef std::vector<double> VectorDouble;
typedef std::vector<VectorDouble> VectorVectorDouble;
typedef std::vector<int> VectorInt;
typedef std::vector<VectorInt> VectorVectorInt;

std::string getObjType(const bp::api::object& obj) {
    boost::python::extract<boost::python::object> objectExtractor(obj);
    boost::python::object o=objectExtractor();
    std::string obj_type = boost::python::extract<std::string>(o.attr("__class__").attr("__name__"));
    return obj_type;
}

std::string getObjType(PyObject* obj_ptr) {
       boost::python::object obj(boost::python::handle<>(boost::python::borrowed(obj_ptr)));
       return getObjType(obj);
}

/*
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
*/

/*
template <typename ArrayType>
struct NumpyArrayConverter {

    NumpyArrayConverter() {
        using namespace boost::python;
        converter::registry::push_back(&convertible, &construct, type_id<ArrayType>());
    }

    static void* convertible(PyObject* obj_ptr) {
        std::string name = getObjType(obj_ptr);

        if ( name == "ndarray") {
            return obj_ptr;
        }
        return 0;
    }

    static void construct(PyObject* obj_ptr, boost::python::converter::rvalue_from_python_stage1_data* data) {
        using namespace boost::python;

        bp::object obj(boost::python::handle<>(bp::borrowed(obj_ptr)));

        void* storage = ((converter::rvalue_from_python_storage<ArrayType>*) data)->storage.bytes;

        ArrayType * array = new (storage) ArrayType(obj);

        data->convertible = storage;
    }
};
*/

void export_numpy_scalar_converter();
void export_numpy_array_converter();

void export_metric_MGC();
void export_metric_Edit();
void export_metric_Entropy();
void export_metric_VOI_kl();
void export_metric_VOI_normalized();
void export_metric_EMD();
void export_metric_EMD_details();

BOOST_PYTHON_MODULE(metric) {

    bp::class_<VectorDouble>("VectorDouble").def(bp::vector_indexing_suite<VectorDouble>());
    bp::class_<VectorVectorDouble>("VectorVectorDouble").def(bp::vector_indexing_suite<VectorVectorDouble>());
    bp::class_<VectorInt>("VectorInt").def(bp::vector_indexing_suite<VectorInt>());
    bp::class_<VectorVectorInt>("VectorVectorInt").def(bp::vector_indexing_suite<VectorVectorInt>());

    export_numpy_scalar_converter();

    export_numpy_array_converter();

    export_metric_MGC();
    export_metric_Edit();
    export_metric_Entropy();
    export_metric_VOI_kl();
    export_metric_VOI_normalized();
    export_metric_EMD();
    export_metric_EMD_details();

// TODO #include "metric_affprop.cpp"

}

