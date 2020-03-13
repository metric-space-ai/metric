#include "modules/space/tree.hpp"
#include "modules/distance/k-related/Standards.hpp"

#include <boost/python.hpp>
#include <vector>

namespace py = boost::python;
/*  TODO:
    template <class Archive, class Stream> void deserialize(Archive& input, Stream& stream);
    template <class Archive> void serialize(Archive& archive);

    void traverse(const std::function<void(Node_ptr)>& f);
    void print(std::ostream& ostr) const;
    std::string to_json(std::function<std::string(const recType&)> printer);
    void traverse_child(const std::function<void(Node_ptr)>& f);
    bool same_tree(const Node_ptr lhs, const Node_ptr rhs) const;
    friend std::ostream& operator<<(std::ostream& ostr, const Tree& t);
    blaze::SymmetricMatrix<blaze::DynamicMatrix<Distance, blaze::rowMajor>> matrix() const;
*/

template <typename recType, typename Metric>
void register_wrapper_Tree() {
    using Tree = metric::Tree<recType, Metric>;
    using Container = std::vector<recType>;
    using Distance = typename Tree::Distance;
    auto tree = py::class_<Tree, boost::noncopyable>("Tree", py::no_init);
    tree.def(py::init<int>(
        (
            py::arg("truncate") = -1
        ),
        "Empty tree"
    ));
// FIXME: broken in C++ add_value
//    tree.def(py::init<const recType&, int>(
//        (
//            py::arg("p"),
//            py::arg("truncate") = -1
//        )
//    ));
    tree.def(py::init<const Container&, int>(
        (
            py::arg("p"),
            py::arg("truncate") = -1
        )
    ));
    std::vector<std::vector<std::size_t>> (Tree::*clustering1) (
        const std::vector<double>&,
        const std::vector<std::size_t>&,
        const std::vector<recType>&
    ) = &Tree::clustering;

// FIXME: broken in C++
//    std::vector<std::vector<std::size_t>> (Tree::*clustering2) (
//        const std::vector<double>&,
//        const std::vector<std::size_t>&
//    ) = &Tree::clustering;

    std::vector<std::vector<std::size_t>> (Tree::*clustering3) (
        const std::vector<double>&,
        const std::vector<recType>&
    ) = &Tree::clustering;

    std::tuple<std::size_t, bool> (Tree::*insert_if1) (const recType&, Distance) = &Tree::insert_if;
    std::size_t (Tree::*insert_if2) (const std::vector<recType>&, Distance) = &Tree::insert_if;

    tree.def("clustering", clustering1);
//    tree.def("clustering", clustering2);
    tree.def("clustering", clustering3);
    tree.def("insert", insert_if1);
    tree.def("insert", insert_if2);

    tree.def("erase", &Tree::erase);
    tree.def("__getitem__", &Tree::operator[]);
    tree.def("nn", &Tree::nn, py::return_internal_reference<>());
    tree.def("knn", &Tree::knn);
    tree.def("rnn", &Tree::rnn);
    tree.def("__len__", &Tree::size);
    tree.def("empty", &Tree::size);
    tree.def("to_vector", &Tree::toVector);
    tree.def("check_covering", &Tree::check_covering);
    tree.def("distance_by_id", &Tree::distance_by_id);
    tree.def("distance", &Tree::distance);

    std::string (Tree::*to_json1)(std::function<std::string(const recType&)>) = &Tree::to_json;
    std::string (Tree::*to_json2)() = &Tree::to_json;
    tree.def("to_json", to_json1);
    tree.def("to_json", to_json2);

    void (Tree::*print)() const = &Tree::print;

    tree.def("level_size", &Tree::levelSize);
    tree.def("print", print);
    tree.def("print_levels", &Tree::print_levels);
    tree.def("root", &Tree::get_root, py::return_internal_reference<>());
    tree.def("__eq__", &Tree::operator==);
}

void export_metric_Tree() {
    register_wrapper_Tree<std::vector<double>, metric::Euclidian<double>>();
}
