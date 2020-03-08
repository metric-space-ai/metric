#include "stl_wrappers.hpp"
#include <boost/python.hpp>
#include <boost/python/object.hpp>
#include <boost/python/converter/implicit.hpp>
#include <boost/python/converter/registry.hpp>
#include <boost/python/module.hpp>
#include <numpy/arrayobject.h>
#include <numpy/arrayscalars.h>
#include <iostream>

namespace bp = boost::python;

struct IterableConverter
{
    template <typename Container>
    IterableConverter& from_python()
    {
        bp::converter::registry::push_back(
            &IterableConverter::convertible,
            &IterableConverter::construct<Container>,
            bp::type_id<Container>()
        );

        std::cout << "Register type_id " << bp::type_id<Container>().name() << std::endl;

        return *this;
    }

    /*
    @brief Check if PyObject is iterable.
    */
    static void* convertible(PyObject* object)
    {
        std::cout << "IterableConverter convertible " << (PyObject_GetIter(object) != NULL) << std::endl;
        return PyObject_GetIter(object) ? object : NULL;
    }

    /* @brief Convert iterable PyObject to C++ container type.

     Container Concept requirements:
       * Container::value_type is CopyConstructable.
       * Container can be constructed and populated with two iterators.
         I.e. Container(begin, end)
    */
    template <typename Container>
    static void construct(PyObject* object, bp::converter::rvalue_from_python_stage1_data* data)
    {
        // Object is a borrowed reference, so create a handle indicting it is
        // borrowed for proper reference counting.
        bp::handle<> handle(bp::borrowed(object));

        // Obtain a handle to the memory block that the converter has allocated
        // for the C++ type.
        typedef bp::converter::rvalue_from_python_storage<Container> storage_type;
        void* storage = reinterpret_cast<storage_type*>(data)->storage.bytes;

        typedef bp::stl_input_iterator<typename Container::value_type> iterator;

        // Allocate the C++ type into the converter's memory block, and assign
        // its handle to the converter's convertible variable.  The C++
        // container is populated by passing the begin and end iterators of
        // the python object to the container's constructor.
        new (storage) Container(
            iterator(bp::object(handle)), // begin
            iterator());                  // end
        data->convertible = storage;
    }
};

struct NumpyArrayConverter {

    template <typename ArrayType>
    NumpyArrayConverter& from_python() {
        bp::converter::registry::push_back(
            &NumpyArrayConverter::convertible,
            &NumpyArrayConverter::construct<ArrayType>,
            bp::type_id<ArrayType>()
        );

        std::cout << "Register type_id " << bp::type_id<ArrayType>().name() << std::endl;

        return *this;
    }

    static void* convertible(PyObject* obj_ptr) {
        std::string name = getObjType(obj_ptr);

        if ( name == "ndarray") {
            std::cout << "NumpyArrayConverter convertible " << name << " yes" << std::endl;
            return obj_ptr;
        }
        std::cout << "NumpyArrayConverter convertible " << name << " no" << std::endl;
        return 0;
    }

    template <typename ArrayType>
    static void construct(PyObject* obj_ptr, boost::python::converter::rvalue_from_python_stage1_data* data) {
        bp::object obj(bp::handle<>(bp::borrowed(obj_ptr)));
        void* storage = ((bp::converter::rvalue_from_python_storage<ArrayType>*) data)->storage.bytes;
        ArrayType* array = new (storage) ArrayType(obj);
        data->convertible = storage;
    }
};

struct NumpyScalarConverter {

    template <typename ScalarType>
    NumpyScalarConverter& from_python() {
        bp::converter::registry::push_back(
            &NumpyScalarConverter::convertible,
            &NumpyScalarConverter::construct<ScalarType>,
            bp::type_id<ScalarType>()
        );

        return *this;
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
        std::cout << "NumpyScalarConverter convertible " << name << " no" << std::endl;

        return 0;
    }

    template <typename ScalarType>
    static void construct(PyObject* obj_ptr, bp::converter::rvalue_from_python_stage1_data* data) {
        std::string name = getObjType(obj_ptr);

        void* storage = ((bp::converter::rvalue_from_python_storage<ScalarType>*) data)->storage.bytes;

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

void export_converters()
{
    IterableConverter()
        .from_python<std::vector<double>>()
        .from_python<std::vector<int>>()
        .from_python<std::vector<std::vector<double>>>()
        .from_python<std::vector<std::vector<int>>>();

    NumpyScalarConverter()
        .from_python<signed char>()
        .from_python<short>()
        .from_python<int>()
        .from_python<long>()
        .from_python<long long>()
        .from_python<unsigned char>()
        .from_python<unsigned short>()
        .from_python<unsigned int>()
        .from_python<unsigned long>()
        .from_python<unsigned long long>()
        .from_python<float>()
        .from_python<double>();

//    NumpyArrayConverter()
//        .from_python<std::vector<double>>()
//        .from_python<std::vector<int>>()
//        .from_python<std::vector<std::vector<double>>>()
//        .from_python<std::vector<std::vector<int>>>();

    NumpyArrayConverter()
        .from_python<WrapStlVector<double>>()
        .from_python<WrapStlMatrix<double>>()
        .from_python<WrapStlVector<WrapStlVector<double>>>();
}