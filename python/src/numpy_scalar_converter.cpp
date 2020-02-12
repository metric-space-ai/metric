#include "metric_py.hpp"

#include <boost/python/object.hpp>
#include <boost/python/converter/implicit.hpp>
#include <boost/python/converter/registry.hpp>
#include <boost/python/module.hpp>
#include <numpy/arrayobject.h>
#include <numpy/arrayscalars.h>

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

void export_numpy_scalar_converter() {
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

}
