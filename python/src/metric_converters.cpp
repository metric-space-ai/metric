#include <boost/python.hpp>

namespace bp = boost::python;

struct iterable_converter
{
    template <typename Container>
    iterable_converter&
    from_python()
    {
        bp::converter::registry::push_back(
            &iterable_converter::convertible,
            &iterable_converter::construct<Container>,
            bp::type_id<Container>()
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
            iterator());                      // end
        data->convertible = storage;
    }
};

void export_converters()
{
    iterable_converter()
        .from_python<std::vector<double>>()
        .from_python<std::vector<std::vector<double>>>();
}