#include "modules/space/tree.hpp"

#include <boost/python.hpp>
#include <vector>

namespace bp = boost::python;

/*
template <class recType, class Metric>
class Tree {
    template <class Archive, class Stream> void deserialize(Archive& input, Stream& stream);
    template <class Archive> void serialize(Archive& archive);

    std::tuple<std::size_t, bool> insert_if(const recType& p, Distance treshold);
    std::size_t insert_if(const std::vector<recType>& p, Distance treshold);
    bool erase(const recType& p);
    recType operator[](size_t id);
    Node_ptr nn(const recType& p) const;
    std::vector<std::pair<Node_ptr, Distance>> knn(const recType& p, unsigned k = 10) const;
    std::vector<std::pair<Node_ptr, Distance>> rnn(const recType& p, Distance distance = 1.0) const;
    size_t size();
    void traverse(const std::function<void(Node_ptr)>& f);
    int levelSize();
    void print() const;
    void print(std::ostream& ostr) const;
    std::map<int, unsigned> print_levels();
    std::vector<recType> toVector();
    std::string to_json(std::function<std::string(const recType&)> printer);
    std::string to_json();
    bool check_covering() const;
    void traverse_child(const std::function<void(Node_ptr)>& f);
    Node_ptr get_root();
    bool empty() const { return root == nullptr; }
    bool same_tree(const Node_ptr lhs, const Node_ptr rhs) const;
    bool operator==(const Tree& t);
    friend std::ostream& operator<<(std::ostream& ostr, const Tree& t);
    Distance distance_by_id(std::size_t id1, std::size_t id2) const;
    Distance distance(const recType & p1, const recType & p2) const;
    blaze::SymmetricMatrix<blaze::DynamicMatrix<Distance, blaze::rowMajor>> matrix() const;
*/

template <typename recType, typename Metric>
void register_wrapper_Tree() {
    using Tree = metric::Tree<recType, Metric>;
    using Container = std::vector<recType>;
    auto tree = bp::class_<Tree>("Tree", bp::no_init);
    tree.def(bp::init<int, Metric>(
        (
            bp::arg("truncate") = -1,
            bp::arg("distance") = Metric(),
        ),
        "Empty tree"
    ));
    tree.def(bp::init<const recType&, int, Metric>(
        (
            bp::arg("p"),
            bp::arg("truncate") = -1,
            bp::arg("distance") = Metric(),
        )
    ));
    tree.def(bp::init<const Container&, int, Metric>(
        (
            bp::arg("p"),
            bp::arg("truncate") = -1,
            bp::arg("distance") = Metric(),
        )
    ));
    std::vector<std::vector<std::size_t>> (Tree::*clustering1) (
        const std::vector<double>&,
        const std::vector<std::size_t>,
        const std::vector<recType>&
    ) = &Tree::clustering;

    std::vector<std::vector<std::size_t>> (Tree::*clustering2) (
        const std::vector<double>&,
        const std::vector<std::size_t>
    ) = &Tree::clustering;

    std::vector<std::vector<std::size_t>> (Tree::*clustering3) (
        const std::vector<double>&,
        const std::vector<recType>&
    ) = &Tree::clustering;

    std::size_t (Tree::*insert1) (const recType&) = &Tree::insert;
    bool (Tree::*insert2) (const std::vector<recType>&) = &Tree::insert;

    tree.def("clustering", clustering1);
    tree.def("clustering", clustering2);
    tree.def("clustering", clustering3);
    tree.def("insert", &Tree::insert1);
    tree.def("insert", &Tree::insert2);
}

void export_metric_Tree() {
    register_wrapper_Tree<std::vector<double>, void>();
}
