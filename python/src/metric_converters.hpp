#pragma once
#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include <boost/python.hpp>
#include <boost/python/object.hpp>
#include <boost/python/converter/implicit.hpp>
#include <boost/python/converter/registry.hpp>
#include <boost/python/module.hpp>
#include <numpy/arrayobject.h>
#include <numpy/arrayscalars.h>
#include <iostream>


inline std::string getObjClassName(const boost::python::api::object& obj) {
    boost::python::extract<boost::python::object> objectExtractor(obj);
    boost::python::object o=objectExtractor();
    std::string obj_type = boost::python::extract<std::string>(o.attr("__class__").attr("__name__"));
    return obj_type;
}

inline std::string getObjClassName(PyObject* obj_ptr) {
       boost::python::object obj(boost::python::handle<>(boost::python::borrowed(obj_ptr)));
       return getObjClassName(obj);
}

struct IterableConverter
{
    template <typename Container>
    IterableConverter& from_python()
    {
        boost::python::converter::registry::push_back(
            &IterableConverter::convertible,
            &IterableConverter::construct<Container>,
            boost::python::type_id<Container>()
        );

        return *this;
    }

    /*
    @brief Check if PyObject is iterable.
    */
    static void* convertible(PyObject* object)
    {
        return PyObject_GetIter(object) ? object : NULL;
    }

    /* @brief Convert iterable PyObject to C++ container type.

     Container Concept requirements:
       * Container::value_type is CopyConstructable.
       * Container can be constructed and populated with two iterators.
         I.e. Container(begin, end)
    */
    template <typename Container>
    static void construct(PyObject* object, boost::python::converter::rvalue_from_python_stage1_data* data)
    {
        // Object is a borrowed reference, so create a handle indicting it is
        // borrowed for proper reference counting.
        boost::python::handle<> handle(boost::python::borrowed(object));

        // Obtain a handle to the memory block that the converter has allocated
        // for the C++ type.
        typedef boost::python::converter::rvalue_from_python_storage<Container> storage_type;
        void* storage = reinterpret_cast<storage_type*>(data)->storage.bytes;

        typedef boost::python::stl_input_iterator<typename Container::value_type> iterator;

        // Allocate the C++ type into the converter's memory block, and assign
        // its handle to the converter's convertible variable.  The C++
        // container is populated by passing the begin and end iterators of
        // the python object to the container's constructor.
        new (storage) Container(
            iterator(boost::python::object(handle)), // begin
            iterator());                  // end
        data->convertible = storage;
    }
};

struct NumpyArrayConverter {

    template <typename ArrayType>
    NumpyArrayConverter& from_python() {
        boost::python::converter::registry::push_back(
            &NumpyArrayConverter::convertible,
            &NumpyArrayConverter::construct<ArrayType>,
            boost::python::type_id<ArrayType>()
        );

        return *this;
    }

    static void* convertible(PyObject* obj_ptr) {
        const std::string name = getObjClassName(obj_ptr);

        if ( name == "ndarray") {
            std::cout << "NumpyArrayConverter convertible " << name << " yes" << std::endl;
            return obj_ptr;
        }
        std::cout << "NumpyArrayConverter convertible " << name << " no" << std::endl;
        return 0;
    }

    template <typename ArrayType>
    static void construct(PyObject* obj_ptr, boost::python::converter::rvalue_from_python_stage1_data* data) {
        boost::python::object obj(boost::python::handle<>(boost::python::borrowed(obj_ptr)));
        void* storage = ((boost::python::converter::rvalue_from_python_storage<ArrayType>*) data)->storage.bytes;
        ArrayType* array = new (storage) ArrayType(obj);
        data->convertible = storage;
    }
};

struct NumpyScalarConverter {

    template <typename ScalarType>
    NumpyScalarConverter& from_python() {
        boost::python::converter::registry::push_back(
            &NumpyScalarConverter::convertible,
            &NumpyScalarConverter::construct<ScalarType>,
            boost::python::type_id<ScalarType>()
        );

        return *this;
    }

    static void* convertible(PyObject* obj_ptr) {
        std::string name = getObjClassName(obj_ptr);

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

    template <typename ScalarType>
    static void construct(PyObject* obj_ptr, boost::python::converter::rvalue_from_python_stage1_data* data) {
        void* storage = ((boost::python::converter::rvalue_from_python_storage<ScalarType>*) data)->storage.bytes;

        ScalarType * scalar = new (storage) ScalarType;
        if (PyArray_IsScalar(obj_ptr, Float32))
            (*scalar) = PyArrayScalar_VAL(obj_ptr, Float32);
        else if (PyArray_IsScalar(obj_ptr, Float64))
            (*scalar) = PyArrayScalar_VAL(obj_ptr, Float64);
        else if (PyArray_IsScalar(obj_ptr, Int8))
            (*scalar) = PyArrayScalar_VAL(obj_ptr, Int8);
        else if (PyArray_IsScalar(obj_ptr, Int16))
            (*scalar) = PyArrayScalar_VAL(obj_ptr, Int16);
        else if (PyArray_IsScalar(obj_ptr, Int32))
            (*scalar) = PyArrayScalar_VAL(obj_ptr, Int32);
        else if (PyArray_IsScalar(obj_ptr, Int64))
            (*scalar) = PyArrayScalar_VAL(obj_ptr, Int64);
        else if (PyArray_IsScalar(obj_ptr, UInt8))
            (*scalar) = PyArrayScalar_VAL(obj_ptr, UInt8);
        else if (PyArray_IsScalar(obj_ptr, UInt16))
            (*scalar) = PyArrayScalar_VAL(obj_ptr, UInt16);
        else if (PyArray_IsScalar(obj_ptr, UInt32))
            (*scalar) = PyArrayScalar_VAL(obj_ptr, UInt32);
        else if (PyArray_IsScalar(obj_ptr, UInt64))
            (*scalar) = PyArrayScalar_VAL(obj_ptr, UInt64);

        data->convertible = storage;
    }
};
