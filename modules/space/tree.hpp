/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018, Michael Welsch

*/

#ifndef _METRIC_SPACE_TREE_HPP
#define _METRIC_SPACE_TREE_HPP

#include <atomic>
#include <cmath>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <numeric>
#include <shared_mutex>
#include <stack>
#include <string>
#include <tuple>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include "../../3rdparty/blaze/Math.h"
#include "../../3rdparty/blaze/math/Matrix.h"
#include "../../3rdparty/blaze/math/adaptors/SymmetricMatrix.h"
namespace metric {
/*
  _ \         _|             |  |       \  |        |       _)
  |  |  -_)   _| _` |  |  |  |   _|    |\/ |   -_)   _|   _| |   _|
  ___/ \___| _| \__,_| \_,_| _| \__|   _|  _| \___| \__| _|  _| \__|

*/
/*** standard euclidian (L2) Metric ***/
template <typename, typename>
struct SerializedNode;

template <typename, typename>
class Node;

struct unsorted_distribution_exception : public std::exception {
};
struct bad_distribution_exception : public std::exception {
};

/*
  __ __|
     |   _ | -_)   -_)
    _| _|  \___| \___|

*/
/*** Cover Tree Implementation ***/
template <class recType, class Metric>
class Tree {

public:
    typedef Node<recType, Metric> NodeType;
    typedef Node<recType, Metric>* Node_ptr;
    typedef Tree<recType, Metric> TreeType;
    using rset_t = std::tuple<Node_ptr, std::vector<Node_ptr>, std::vector<Node_ptr>>;
    using Distance = typename std::result_of<Metric(recType, recType)>::type;

    /***
      @brief cluster tree nodes according to distribution
      @param distribution vector with percents of amount of nodes, this vector should be sorted,
      otherwise metric_space::unsorted_distribution_exception would be thrown. If distribution vector
      containd value less than zero or greate than 1, the metric_space::bad_distribution_exception would be thrown.

      @param points vector with data values

      @param indexes indexes in points vector, as a source set only data records with corresponding indices will be
      used.
      @return vector of vector of node IDs according to distribution
     */
    std::vector<std::vector<std::size_t>> clustering(const std::vector<double>& distribution,
        const std::vector<std::size_t>& indexes, const std::vector<recType>& points);

    /***
      @brief cluster tree nodes according to distribution

      @param distribution vector with percents of amount of nodes, this vector should be sorted,
      otherwise metric_space::unsorted_distribution_exception would be thrown. If distribution vector
      containd value less than zero or greate than 1, the metric_space::bad_distribution_exception would be thrown.

      @param IDS id's of nodes in tree, these nodes would be used as a source set.

      @return vector of vector of node IDs according to distribution
    */

    std::vector<std::vector<std::size_t>> clustering(
        const std::vector<double>& distribution, const std::vector<std::size_t>& IDS);

    /***
        @brief cluster tree nodes according to distribution

        @param distribution vector with percents of amount of nodes, this vector should be sorted,
        otherwise metric_space::unsorted_distribution_exception would be thrown. If distribution vector
        containd value less than zero or greate than 1, the metric_space::bad_distribution_exception would be thrown.

        @param points vector with data values.  these values would be used as a source set.
    */

    std::vector<std::vector<std::size_t>> clustering(
        const std::vector<double>& distribution, const std::vector<recType>& points);

    /**
     * @brief deserialize tree from Archive
     *
     * @param input serialization object
     * @param stream  underlying input stream
     */
    template <class Archive, class Stream>
    void deserialize(Archive& input, Stream& stream);

    /**
     * @brief Serialize tree to archive
     *
     * @param archive serialization object
     */
    template <class Archive>
    void serialize(Archive& archive);

    /*** Constructors ***/

    /**
     * @brief Construct an empty Tree object
     *
     * @param truncate truncate paramter
     * @param d metric object
     */
    Tree(int truncate = -1, Metric d = Metric());  // empty tree

    /**
     * @brief Construct a Tree object with one data record as root
     *
     * @param p data record
     * @param truncate truncate parameter
     * @param d metric object
     */
    Tree(const recType& p, int truncate = -1, Metric d = Metric());  // cover tree with one data record as root

    /**
     * @brief Construct a Tree object from data vector
     *
     * @param p vector of data records to store in tree
     * @param truncate truncate paramter
     * @param d metric object
     */
    template <class Container>
    Tree(const Container& p, int truncateArg = -1, Metric d = Metric());

    /**
     * @brief Destroy the Tree object
     *
     */
    ~Tree();  // Destuctor

    /*** Access Operations ***/

    /**
     * @brief Insert date record to the cover tree
     *
     * @param p data record
     * @return ID of inserted node
     */
    std::size_t insert(const recType& p);

    /**
     * @brief Insert set of data records to the cover tree
     *
     * @param p vector of data records
     * @return true if inserting successful
     * @return false if inserting unsuccessful
     */

    bool insert(const std::vector<recType>& p);

    /**
     * @brief inser data record to the tree if distance between NN and new point is greater than a threshold
     *
     * @param p new data record
     * @param treshold distance threshold
     * @return ID of inserted node and true if inserting successful
     * @return ID of NN node and false if inserting unsuccessful
     */
    std::tuple<std::size_t, bool> insert_if(const recType& p, Distance treshold);

    /**
     * @brief inser set of data records to the tree if distance between root and new point is greater than a threshold
     *
     * @param p vector of new data records
     * @param treshold distance threshold
     * @return std::size_t amount of inserted points
     */
    std::size_t insert_if(const std::vector<recType>& p, Distance treshold);

    /**
     * @brief erase data record from cover tree
     *
     * @param p data record to erase
     * @return true if erase successful
     * @return false if erase unsuccessful
     */
    bool erase(const recType& p);

    /**
     * @brief access data record by ID
     *
     * @param id data record ID
     * @return data record with ID == id
     * @throws std::runtime_error when tree has no element with ID
     */
    recType operator[](size_t id);

    /*** Nearest Neighbour search ***/

    /**
     * @brief find nearest neighbour of data record
     *
     * @param p searching data record
     * @return Node containing nearest neigbour to p
     */
    Node_ptr nn(const recType& p) const;

    /**
     * @brief find K-nearest neighbour of data record
     *
     * @param p searching data record
     * @param k amount of nearest neighbours
     * @return vector of pair of node pointer and distance to searching point
     */
    std::vector<std::pair<Node_ptr, Distance>> knn(const recType& p, unsigned k = 10) const;

    /**
     * @brief find all nearest neighbour in range [0;distance]
     *
     * @param p searching point
     * @param distance max distance to searching point
     * @return vector of pair of node pointer and distance to searching point
     */
    std::vector<std::pair<Node_ptr, Distance>> rnn(const recType& p, Distance distance = 1.0) const;

    /*** utilitys ***/

    /**
     * @brief tree size
     *
     * @return return amount of nodes
     */
    size_t size();

    /**
     * @brief traverse tree and apply callback function to each node
     *
     * @param f node callback
     */
    void traverse(const std::function<void(Node_ptr)>& f);

    /** Dev Tools **/

    /**
     * @brief  return the max_level of the tree (= root level)
     *
     * @return level of the root node
     */
    int levelSize();

    /**
     * @brief pretty print tree to stdout
     *
     */
    void print() const;

    /**
     * @brief pretty print tree to provided stream
     *
     * @param ostr
     */
    void print(std::ostream& ostr) const;

    /**
     * @brief  print and return levels information
     *
     * @return map { level -> amount of nodes at this level}
     */
    std::map<int, unsigned> print_levels();

    /**
     * @brief convert tree to vector
     *
     * @return vector of data records  stored in the tree
     */
    std::vector<recType> toVector();

    /**
     * @brief serialize tree to JSON, with custom data record serialzer
     *
     * @param printer function converting data record to JSON
     * @return JSON representation of tree
     */
    std::string to_json(std::function<std::string(const recType&)> printer);

    /**
     * @brief serialize tree to JSON
     *
     * @return JSON representation of tree
     */
    std::string to_json();

    /**
     * @brief check tree covering invariant
     *
     * @return true if tree is ok
     * @return false if tree is corrupted
     */
    bool check_covering() const;

    /**
     * @brief traverse tree except root node
     *
     * @param f callback for nodes
     */
    void traverse_child(const std::function<void(Node_ptr)>& f);
    /**
     * @brief Get the root node
     *
     * @return root node
     */
    Node_ptr get_root() { return root; }

    /**
     * @brief check is tree empty
     *
     * @return true if tree has no nodes
     * @return false if tree has at least one node
     */
    bool empty() const { return root == nullptr; }

    /**
     * @brief compare two trees
     *
     * @param lhs root node of the first tree
     * @param rhs root node of the second tree
     * @return true if lhs and rhs has the same structure
     * @return false if lhs and rhs differs
     */
    bool same_tree(const Node_ptr lhs, const Node_ptr rhs) const;

    /**
     * @brief compare tree with another
     *
     * @param t another tree
     * @return true if trees equivalent
     * @return false if trees differs
     */
    bool operator==(const Tree& t) const { return same_tree(root, t.root); }

    /**
     * @brief writing tree to stream, same as print(std::ostream)
     *
     * @param ostr output stream
     * @param t tree
     * @return stream reference
     */
    friend std::ostream& operator<<(std::ostream& ostr, const Tree& t)
    {
        t.print(ostr);
        return ostr;
    }
    /**
     * @brief Computes graph distance between two nodes
     * @param id1  - ID of the first node
     * @param id2  - ID of the second node
     * @return weighted sum of edges between nodes
     */
    Distance distance_by_id(std::size_t id1, std::size_t id2) const;

    /**
     * @brief Computes graph distance between two records
     * @param p1  - first record
     * @param p2  - second record
     * @return weighted sum of edges between nodes nearest to p1 and p2
     */
    Distance distance(const recType & p1, const recType & p2) const;

    /**
     * @brief convert cover tree to distance matrix
     * @return blaze::SymmetricMatrix with distance
     *
     */
    blaze::SymmetricMatrix<blaze::DynamicMatrix<Distance, blaze::rowMajor>> matrix() const;
private:
    friend class Node<recType, Metric>;

    /*** Types ***/
    Metric metric_;

    /*** Properties ***/
    Distance base = 2;  // Base for estemating the covering of the tree
    Node_ptr root;  // Root of the tree
    std::atomic<int> min_scale;  // Minimum scale
    std::atomic<int> max_scale;  // Minimum scale
    int truncate_level = -1;  // Relative level below which the tree is truncated
    std::atomic<std::size_t> nextID = 0;  // Next node ID
    mutable std::shared_timed_mutex global_mut;  // lock for changing the root
    std::vector<std::pair<recType, Node_ptr>> data;

    std::unordered_map<std::size_t, std::size_t> index_map;  // ID -> data index mapping

    // /*** Imlementation Methodes ***/

    Node_ptr insert(Node_ptr p, Node_ptr x);

    template <typename pointOrNodeType>
    std::tuple<std::vector<int>, std::vector<Distance>> sortChildrenByDistance(Node_ptr p, pointOrNodeType x) const;

    bool grab_sub_tree(Node_ptr proot, const recType& center, std::unordered_set<std::size_t>& parsed_points,
        const std::vector<std::size_t>& distribution_sizes, std::size_t& cur_idx,
        std::vector<std::vector<std::size_t>>& result);

    bool grab_tree(Node_ptr start_point, const recType& center, std::unordered_set<std::size_t>& parsed_points,
        const std::vector<std::size_t>& distribution_sizes, std::size_t& cur_idx,
        std::vector<std::vector<std::size_t>>& result);

    double find_neighbour_radius(const std::vector<std::size_t>& IDS, const std::vector<recType>& points);
    double find_neighbour_radius(const std::vector<std::size_t>& IDS);

    double find_neighbour_radius(const std::vector<recType>& points);

    //  template <typename pointOrNodeType>
    Node_ptr insert_(Node_ptr p, Node_ptr x);

    void nn_(Node_ptr current, Distance dist_current, const recType& p, std::pair<Node_ptr, Distance>& nn) const;
    std::size_t knn_(Node_ptr current, Distance dist_current, const recType& p,
        std::vector<std::pair<Node_ptr, Distance>>& nnList, std::size_t nnSize) const;
    void rnn_(Node_ptr current, Distance dist_current, const recType& p, Distance distance,
        std::vector<std::pair<Node_ptr, Distance>>& nnList) const;

    void print_(NodeType* node_p, std::ostream& ostr) const;

    Node_ptr merge(Node_ptr p, Node_ptr q);
    std::pair<Node_ptr, std::vector<Node_ptr>> mergeHelper(Node_ptr p, Node_ptr q);
    auto findAnyLeaf() -> Node_ptr;
    void extractNode(Node_ptr node);

    template <class Archive>
    void serialize_aux(Node_ptr node, Archive& archvie);

    Node_ptr rebalance(Node_ptr p, Node_ptr x);
    rset_t rebalance_(Node_ptr p, Node_ptr q, Node_ptr x);

    std::vector<std::vector<std::size_t>> clustering_impl(
        const std::vector<double>& distribution, const recType& center, double radius);

    bool update_idx(std::size_t& cur_idx, const std::vector<std::size_t>& distribution_sizes,
        std::vector<std::vector<std::size_t>>& result);

    Distance metric(const recType& p1, const recType& p2) const { return metric_(p1, p2); }
    Distance metric_by_id(const std::size_t id1, const std::size_t id2) {
        return metric_(data[index_map[id1]].first, data[index_map[id2]].first);
    }
    template <class Archive>
    auto deserialize_node(Archive& istr) -> SerializedNode<recType, Metric>;

    std::size_t add_data(const recType & p, Node_ptr ptr) {
        data.push_back(std::pair{p,ptr});
        auto id = nextID++;
        index_map[id] = data.size()-1;
        return id;
    }
    const recType & get_data(std::size_t ID) {
        return data[index_map[ID]].first;
    }
    void remove_data(std::size_t ID) {
        auto p = data.begin();
        auto pi = index_map.find(ID);
        std::size_t i = pi->second;
        std::advance(p, i);
        data.erase(p);
        index_map.erase(pi);
        for(auto &kv : index_map) {
            if(kv.first <= i)
                continue;
            kv.second -= 1;
        }
    }
    std::pair<Distance, std::size_t> distance_to_root(Node_ptr p) const;
    std::pair<Distance, std::size_t> distance_to_level(Node_ptr &p, int level) const;
    Distance distance_by_node(Node_ptr p1, Node_ptr p2) const;
    std::pair<Distance, std::size_t> graph_distance(Node_ptr p1, Node_ptr p2) const; 
};
}  // namespace metric
#include "tree.cpp"  // include the implementation

#endif  //_METRIC_SPACE_TREE_HPP
