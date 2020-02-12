#include "metric_py.hpp"
#include <numpy/arrayobject.h>

namespace bp = boost::python;

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


void export_numpy_array_converter() {
    NumpyArrayConverter<WrapStlVector<double>>();
}
