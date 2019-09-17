# METRIC | SPACE
*A templated, header only C++14 implementation of a Metric Search Tree.*

## Overview

A Search Tree works like a std-container to store data of some structure. Basically a Metric Search Tree has the same principle as a binary search tree or a kd-tree, but it works for arbitrary (!) data structures. This Metric Search Tree is basically a Cover Tree Implementation. Additionally to the distiance (or similarity) between the data, a covering distance from level to level decides how the tree grows.

It can be used to find similar sets of data in millions of data sets, each containing hundreds of single values in only a few milliseconds.

## Simple example
Build the tree, search for a data record and investigate the tree structure.
```c++

/*** here are some data records ***/
std::vector<double> v0 = {0, 1, 1, 1, 1, 1, 2, 3};
std::vector<double> v1 = {1, 1, 1, 1, 1, 2, 3, 4};
std::vector<double> v2 = {2, 2, 2, 1, 1, 2, 0, 0};
std::vector<double> v3 = {3, 3, 2, 2, 1, 1, 0, 0};

/*** initialize the tree ***/
metric::Tree<std::vector<double>> cTree;

/*** add data records ***/
cTree.insert(v0);
cTree.insert(v1);
cTree.insert(v2);
cTree.insert(v3);

cTree.print();

// (2)
//  ├──(0)
//  |   └──(1)
//  └──(3)


/*** find the nearest neighbour of a data record ***/
std::vector<double> v4 = {2, 8, 2, 1, 0, 0, 0, 0};
auto nn = cTree.nn(v4);
std::cout << "nn for v4 is v" << nn->ID << std::endl;
// nn for v8 is v3


```


## Initialize a Tree
Construct an empty tree or fill it directly with data records.
```c++
typedef std::vector<double> recType;
typedef std::vector<recType> recList;

/*** Tree with default L2 metric (Euclidian distance measure) ***/
metric::Tree<recType> cTree;             // empty tree
metric::Tree<recType> cTree(recType v1); // with one data record
metric::Tree<recType> cTree(recList m1); // a container with records.

/** A Tree with a custom metric. ***/
metric::Tree<recType, customMetric> cTree;
```

 ## Search options
```c++
/*** logarithmic complexity ***/
auto nn = cTree.nn();                       // finds the nearest neighbour.
auto knn = cTree.knn(5);                    // finds the fives nearest neighbours
auto rnn = cTree.rnn(a_record, a_distance); // finds all neigbours in a_distance to a_record.

/*** linear complexity***/
// when data.sum() gives the sum of the data records elements  ...
cTree.traverse([&](auto node_p) {
    if (4 < node_p->data.sum() && node_p->data.sum() <= 5)
        std::cout << "ID: " << node_p->ID << std::endl;
};);

```

## Access the nodes
```c++
/*** access through dereference to the underlying data ***/
// nn->ID;          // gives the ID of the record. the ID is counted up like an vector index.
// nn->data;        // gives the data record of a node (every node contains data)
// nn->parent;      // gives the parent node in the tree
// nn->children[0]; // gives the first child node. (children is a std::vector)
// nn->parent_dist; // gives the distance to the parent.
// nn->level;       // gives the level of the node postion (higher is nearer to the root)

std::cout << "nearest neighbour of a_record info: " << std::endl;
std::cout << "  ID: " << nn->ID << std::endl;
std::cout << "  parent->ID: " << nn->parent->ID << std::endl;
std::cout << "  num children: " << nn->children.size() << std::endl;
std::cout << "  num siblings: " << nn->parent->children.size() << std::endl;
std::cout << "  distance to the parent: " << nn->parent_dist << std::endl;
std::cout << "  level of the node postion in the tree: " << nn->level << std::endl;
std::cout << "  siblings IDs: ";
/*** print the siblings IDs ***/
for (auto q : nn->parent->children)
{
	std::cout << q->ID << " ";
}

/*** access a single node by index ***/
auto data_record = cTree[1]; // internaly it just traverse throuh the tree and gives back the corresponding data record
                             // in linear complexity, avoid this.

std::cout << "data record #1: {";
for (auto i = 0; i < data_record.size() - 1; i++) 
{
	std::cout << data_record[i] << ", ";
}
std::cout << data_record[data_record.size() - 1] << "}" << std::endl;
```

## Use a custom container with custom metric use an "Eigen" Vector and L1 metric.

```c++
#include "metric_space.hpp"
#include <eigen3/Eigen/Core>

using recType = Eigen::VectorXd;

template <typename T>
struct recMetric {
    T operator()(const Eigen::VectorXd &p, const Eigen::VectorXd &q) const
    {
        return (p - q).cwise().abs().sum(); // L1 norm
    }
};

int main()
{
    metric::Tree<recType, recMetric> cTree;
    // the rest is the same like in the simple example

    return 0;
}
``` 

## Details
- `data record` is a set of values, a row in a table, a data point, etc. All records contain the same paramters, but with different values.

- `n` is a number of data records.

- `distance` is one value, that represents the similarty of two data records. Less distance means more similar. 
For geometric points, it is just the geometric distance. For high dimensional points it's exatly the same math, 
but called `Euclidian Distance` or `L2 Metric`. But there are other `Metrics` as well, who sometimes works better.

The Tree Implementation is based on the Cover Tree Approach (https://en.wikipedia.org/wiki/Cover_tree),
that means in comparsion to a binary search tree an additionally "covering" distance (that only depends on the tree level) is used togehter with the actual metric between the data records. It decides if a data record becomes a sibling or a child in the tree.

Benefits in comparsion to a brute force search over all data:
```
std::vector<data_record>  metric::tree<data_record>
add record:      0                       log(n)
find nn record:  n                       log(n)
```

## Implementation Cons
The Tree is currently not balanced and can grow degenerative by building single long branches or bushy levels. In both extreme cases it is not better than simple brute force over an array.

I'm working on a balancing, which hopefully solves this issue. For pratice check the tree.max_level or print the tree to check the growing. The max_level should not be much bigger than log(n).

The overhead of every data records is ca. 64 Byte to handle the nodes.

*Other CoverTree Implementations can be found here, but each is missing some functionality, that drives me to this new approach. It can be seen as bringing togehter all the good ideas.*
- https://gitlab.com/christoph-conrads/cover-tree
- https://github.com/DNCrane/Cover-Tree
- https://github.com/manzilzaheer/CoverTree