#include "modules/space/tree.hpp"

#include <boost/python.hpp>
#include <vector>

namespace bp = boost::python;

/*
template <class recType, class Metric>
class Tree {
*/
    typedef Node<recType, Metric> NodeType;
    typedef Node<recType, Metric>* Node_ptr;
    typedef Tree<recType, Metric> TreeType;
    using rset_t = std::tuple<Node_ptr, std::vector<Node_ptr>, std::vector<Node_ptr>>;
    using Distance = typename std::result_of<Metric(recType, recType)>::type;

    std::vector<std::vector<std::size_t>> clustering(const std::vector<double>& distribution, const std::vector<std::size_t>& indexes, const std::vector<recType>& points);
    std::vector<std::vector<std::size_t>> clustering(const std::vector<double>& distribution, const std::vector<std::size_t>& IDS);
    std::vector<std::vector<std::size_t>> clustering(const std::vector<double>& distribution, const std::vector<recType>& points);

    template <class Archive, class Stream> void deserialize(Archive& input, Stream& stream);
    template <class Archive> void serialize(Archive& archive);

    /*** Access Operations ***/
    std::size_t insert(const recType& p);

    bool insert(const std::vector<recType>& p);

    std::tuple<std::size_t, bool> insert_if(const recType& p, Distance treshold);
    std::size_t insert_if(const std::vector<recType>& p, Distance treshold);
    bool erase(const recType& p);
    recType operator[](size_t id);

    Node_ptr nn(const recType& p) const;
    std::vector<std::pair<Node_ptr, Distance>> knn(const recType& p, unsigned k = 10) const;
    std::vector<std::pair<Node_ptr, Distance>> rnn(const recType& p, Distance distance = 1.0) const;

    /*** utilitys ***/
    size_t size();
    void traverse(const std::function<void(Node_ptr)>& f);

    int levelSize();
    void print() const;
    std::map<int, unsigned> print_levels();
    std::vector<recType> toVector();
    std::string to_json(std::function<std::string(const recType&)> printer);

    std::string to_json();
    bool check_covering() const;

    void traverse_child(const std::function<void(Node_ptr)>& f);
    Node_ptr get_root() { return root; }

    bool empty() const { return root == nullptr; }

    bool same_tree(const Node_ptr lhs, const Node_ptr rhs) const;
    bool operator==(const Tree& t) const { return same_tree(root, t.root); }
    friend std::ostream& operator<<(std::ostream& ostr, const Tree& t);
*/

template <typename recType, typename Metric>
void register_wrapper_Tree() {
    using Tree = metric::Tree<recType, Metric>;
    using Matrix = std::vector<std::vector<recType>>;
    auto tree = bp::class_<Tree>("Tree", bp::no_init);
    Tree(int truncate = -1, Metric d = Metric());
    Tree(const recType& p, int truncate = -1, Metric d = Metric());
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
    tree.def("clustering",)
    // template <class Container> Tree(const Container& p, int truncateArg = -1, Metric d = Metric()); // TODO

    Matrix (Mapping::*encode1) (const std::vector<recType>&) = &Mapping::time_freq_PCFA_encode;
    Matrix (Mapping::*encode2) (const std::tuple<std::deque<std::vector<recType>>, std::deque<std::vector<recType>>>) = &Mapping::time_freq_PCFA_encode;
    dspcc.def("time_freq_PCFA_encode", encode1);
    dspcc.def("time_freq_PCFA_encode", encode2);
    dspcc.def("time_freq_PCFA_decode", &Mapping::time_freq_PCFA_decode);
    dspcc.def("mixed_code_serialize", &Mapping::mixed_code_serialize);
    dspcc.def("encode", &Mapping::mixed_code_serialize);
    dspcc.def("decode", &Mapping::mixed_code_serialize);
}

void export_metric_Tree() {
    register_wrapper_Tree<std::vector<double>, void>();
}
