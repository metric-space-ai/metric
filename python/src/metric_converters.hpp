#pragma once
#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include <boost/python.hpp>
#include <boost/python/object.hpp>
#include <boost/python/converter/implicit.hpp>
#include <boost/python/converter/registry.hpp>
#include <boost/python/module.hpp>
#include <numpy/arrayobject.h>
#include <numpy/arrayscalars.h>


class PyObjectHelper
{
    boost::python::object object;
public:
    PyObjectHelper(PyObject* obj_ptr)
        :object(boost::python::handle<>(boost::python::borrowed(obj_ptr)))
    {
    }

    std::string name() {
        return boost::python::extract<std::string>(this->object.attr("__class__").attr("__name__"));
    }

    bool isNumpyArray() {
        return this->name() == "ndarray";
    }

    unsigned numberOfDimensions() {
        return boost::python::extract<unsigned>(this->object.attr("ndim"));
    }
};


template<typename T> struct is_2dvector : public std::false_type {};

template<typename T, typename A1, typename A2>
struct is_2dvector<std::vector<std::vector<T, A1>, A2>> : public std::true_type {};


struct IterableConverter
{
    template <typename Container>
    IterableConverter& from_python()
    {
        boost::python::converter::registry::push_back(
            &IterableConverter::convertible<Container>,
            &IterableConverter::construct<Container>,
            boost::python::type_id<Container>()
        );

        return *this;
    }

    /*
    @brief Check if PyObject is iterable twice
    */
    template <typename Container>
    static void* convertible(PyObject* object)
    {
        PyObjectHelper helper(object);
        if constexpr(is_2dvector<Container>::value) {
            return PyObject_GetIter(object) && helper.isNumpyArray() && helper.numberOfDimensions() > 1 ? object : NULL;
        }
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
        if (PyObjectHelper(obj_ptr).isNumpyArray()) {
            return obj_ptr;
        }
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
        auto name = PyObjectHelper(obj_ptr).name();
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
        auto name = PyObjectHelper(obj_ptr).name();

        void* storage = ((boost::python::converter::rvalue_from_python_storage<ScalarType>*) data)->storage.bytes;

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
