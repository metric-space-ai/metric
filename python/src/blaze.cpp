#include <blaze/Blaze.h>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>

#include <vector>
#include <string>

namespace py = pybind11;


template <typename Value>
void wrap_blaze_compressed_matrix(py::module& m) {
    using Matrix = blaze::CompressedMatrix<Value>;

    auto cls = py::class_<Matrix>(m, "CompressedMatrix");
    cls.def(py::init<>());
    cls.def(py::init<size_t, size_t>(),
        py::arg("m"),
        py::arg("n")
    );
    cls.def(py::init<size_t, size_t, size_t>(),
        py::arg("m"),
        py::arg("n"),
        py::arg("nonzeros")
    );
    cls.def(py::init<size_t, size_t, std::vector<size_t>>(),
        py::arg("m"),
        py::arg("n"),
        py::arg("nonzeros")
    );
    cls.def(py::init([](py::array_t<Value, py::array::c_style> array) {
        /* Request a buffer descriptor from Python */
        py::buffer_info info = array.request();
        auto data = array.template unchecked<2>();

        /* Some sanity checks ... */
        if (info.shape[1] != 3)
            throw std::runtime_error("Incompatible buffer dimension!");

        const size_t nr_points = data.shape(0);
        size_t rows = 0;
        size_t cols = 0;
        // calculate max row and col
        for (size_t i = 0; i < nr_points; i++) {
            rows = std::max(rows, size_t(data(i, 0)));
            cols = std::max(cols, size_t(data(i, 1)));
        }
        auto matrix = new Matrix(rows, cols, nr_points);
        // we can't guarantee order in input data, so we use insert here
        for (size_t i = 0; i < nr_points; i++) {
            matrix->insert(data(i, 0), data(i, 1), data(i, 2));
        }
        return matrix;
    }), py::arg().noconvert());

    cls.def("__repr__", [](Matrix& self) {
        return "CompressedMatrix<" + std::to_string(self.rows()) + "x" + std::to_string(self.columns()) + ">";
    });
}

template <typename Value>
void wrap_blaze_dynamic_matrix(py::module& m) {
    using Matrix = blaze::DynamicMatrix<Value>;

    py::class_<Matrix>(m, "DynamicMatrix");
}

void export_blaze_matrices(py::module& m) {
    wrap_blaze_compressed_matrix<double>(m);
    wrap_blaze_dynamic_matrix<double>(m);
//    py::implicitly_convertible<py::array_t<double>, blaze::CompressedMatrix<double>>();

}