#include "../../metric_converters.hpp"
#include "modules/mapping/ensembles/DT.hpp"

#include <boost/python.hpp>
#include <functional>
#include <vector>
#include <variant>
#include <algorithm>

namespace py = boost::python;

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
        return py::extract<double>(this->_distance(this->_accessor(r1), this->_accessor(r2)));
    }

private:
    py::object _accessor;
    py::object _distance;
};

template<typename Record, typename ReturnType>
class FunctionWrapper
{
    py::object obj;
public:
    FunctionWrapper(py::object obj)
        : obj(obj)
    {
    }

    ReturnType operator()(const Record& record) {
        return py::extract<ReturnType>(this->obj(record));
    }
};

template <typename Record>
void register_wrapper_DT() {
    using Class = metric::DT<Record>;
    using Container = std::vector<Record>;
    using Callback = std::function<int(const Record&)>;
    using Dims = std::vector<std::variant<Dimension>>;

    void (Class::*train)(const Container&, Dims, Callback&) = &Class::train;
    void (Class::*predict)(const Container&, Dims, std::vector<int>&) = &Class::predict;

    auto dt = py::class_<Class>("DT", py::init<double, double>(
        (
            py::arg("entropy_threshold") = 0,
            py::arg("gain_threshold") = 0
        )
    ));
    dt.def("train", +[](Class& self, const Container& data, const std::vector<Dimension>& dims, py::object response){
        std::vector<std::variant<Dimension>> variantDims;   // FIXME: better way to convert variants (fix CPP maybe?)
        for (auto& dim : dims) {
            variantDims.push_back(dim);
        }
        Callback resp = FunctionWrapper<Record, int>(response);
        return self.train(data, variantDims, resp);
    },
        (
            py::arg("payments"),
            py::arg("dimensions"),
            py::arg("response")
        )
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
        (
            py::arg("data"),
            py::arg("dimensions")
        )
    );

    py::class_<Dimension>("Dimension", py::init<py::object, py::object>(
        (
            py::arg("accessor"),
            py::arg("distance")
        )
    ));
}

void export_metric_DT() {
    register_wrapper_DT<py::object>();

    IterableConverter()
        .from_python<std::vector<Dimension>>()
        .from_python<std::vector<py::object>>()
        ;
}

BOOST_PYTHON_MODULE(_dt) {
    export_metric_DT();
}