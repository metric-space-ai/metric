#include "modules/mapping/ensembles/DT.hpp"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include <pybind11/functional.h>
#include <functional>
#include <vector>
#include <variant>
#include <algorithm>

namespace py = pybind11;

// TODO: Python Distance wrapper, Python Accessor wrapper
class Dimension {
public:
    /**
 * @brief Construct a new Dimension object
 *
 * @param accessor_ field accessor
 * @param m metric object
 */
    Dimension(py::object accessor, py::object distance)
        : _accessor(accessor), _distance(distance)
    {
    }
    /**
     * @brief Calculate distance between fields in records
     *
     * @param r1 data record
     * @param r2 data record
     * @return distance between fileds in records r1 and r2
     */
    double get_distance(py::object r1, py::object r2) const
    {
        return this->_distance(this->_accessor(r1), this->_accessor(r2)).cast<double>();
    }

private:
    py::object _accessor;
    py::object _distance;
};

//template<typename Record, typename ReturnType>
//class FunctionWrapper
//{
//    py::object obj;
//public:
//    FunctionWrapper(py::object obj)
//        : obj(obj)
//    {
//    }
//
//    ReturnType operator()(const Record& record) {
//        return this->obj(record).cast<ReturnType>();
//    }
//};

template <typename Record>
void register_wrapper_DT(py::module& m) {
    using Class = metric::DT<Record>;
    using Container = std::vector<Record>;
    using Callback = std::function<int(const Record&)>;
    using Dims = std::vector<std::variant<Dimension>>;

    void (Class::*train)(const Container&, Dims, Callback&) = &Class::train;
    void (Class::*predict)(const Container&, Dims, std::vector<int>&) = &Class::predict;

    auto dt = py::class_<Class>(m, "DT");
    dt.def(py::init<double, double>(),
        py::arg("entropy_threshold") = 0,
        py::arg("gain_threshold") = 0
    );
    dt.def("train", +[](Class& self, const Container& data, const std::vector<Dimension>& dims, Callback& response){
            std::vector<std::variant<Dimension>> variantDims;   // FIXME: better way to convert variants (fix CPP maybe?)
            for (auto& dim : dims) {
                variantDims.push_back(dim);
            }
            return self.train(data, variantDims, response);
        },
        py::arg("payments"),
        py::arg("dimensions"),
        py::arg("response")
    );
    dt.def("predict", +[](Class& self, const Container& data, const std::vector<Dimension>& dims){
            std::vector<int> output;
            std::vector<std::variant<Dimension>> variantDims;   // FIXME: better way to convert variants (fix CPP maybe?)
            for (auto& dim : dims) {
                variantDims.push_back(dim);
            }
            self.predict(data, variantDims, output);
            return output;
        },
        py::arg("data"),
        py::arg("dimensions")
    );

    py::class_<Dimension>(m, "Dimension")
        .def(py::init<py::object, py::object>(), py::arg("accessor"), py::arg("distance"));
}

void export_metric_DT(py::module& m) {
    register_wrapper_DT<py::object>(m);

//    IterableConverter()
//        .from_python<std::vector<Dimension>>()
//        .from_python<std::vector<py::object>>()
//        ;
}

PYBIND11_MODULE(_dt, m) {
    export_metric_DT(m);
}