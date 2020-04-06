# METRIC | SPACE
*A templated, header only C++17 implementation of a Metric Search Tree.*

## Overview

Module provides classes for the
representation of metric spaces including basic operations
such as the search for neighboring elements.

|**Module**|**File**|**Class**|**Constructor**|**operator()**|**opeator[]**|**inser**|**insert_if**|**nn**|**knn**|**rnn**|**erase**
|:------:|:-----:|:-----|:-----|:-----|:-----|:-----|:-----|:-----|:-----|:-----|:-----|
|Space|matrix.hpp|Matrix|metric::Matrix<float, metric::Euclidean\<float>> m;|auto distance = m(id1,id2)|auto value = m[id]|std::size_t id = m.insert(1.0); <br> std::vector\<std::size_t>ids = m.insert(std::vector\<float>{1,2,3});|std::pair\<std::size_t,bool> id = m.insert_if(1.0, 10); <br>std::vecto<std::pair<std::size_t, bool>>ids = m.insert_if(std::vector\<float>{1,2,3}, 10);|size_t id = m.nn(1.5)|std::vector\<std::size_t> ids = m.knn(1.5, 10);|std::vector\<std::size_t> ids = m.rnn(1.5, 11.5);|m.erase(0)|
|Space|tree.hpp  |Tree  |metric::Tree<float, metric::Euclidean\<float>> t;  |auto distance = t(id1,id2)|auto value = t[id]|std::size_t id = t.insert(1.0); <br> std::vector\<std::size_t>ids = t.insert(std::vector\<float>{1,2,3});|std::pair\<std::size_t,bool> id = t.insert_if(1.0, 10); <br>std::vecto<std::pair<std::size_t, bool>>ids = t.insert_if(std::vector\<float>{1,2,3}, 10);|size_t id = t.nn(1.5)|std::vector\<std::size_t> ids = t.knn(1.5, 10);|std::vector\<std::size_t> ids = t.rnn(1.5, 11.5);|t.erase(0)|
| Space|knn_graph.hpp|KNNGraph|metric::KNNGraph<float, metric::Euclidean\<float>> g(data, neighbours_num, 3*neighbours_num); | auto distance = g(id1, id2) | auto value = g[id]; | std::size_t id = g.insert(1.0); <br> std::vector\<std::size_t>ids = g.insert(std::vector\<float>{1,2,3}); | std::pair\<std::size_t,bool> id = g.insert_if(1.0, 10); <br>std::vector\<std::pair<std::size_t, bool>>ids = g.insert_if(std::vector\<float>{1,2,3}, 10); | size_t id = g.nn(1.5) | std::vector\<std::size_t> ids = t.knn(1.5, 10); | std::vector\<std::size_t> ids = t.rnn(1.5, 11.5); | t.erase(0) |

## Space types

- **Tree**.
A Search Tree works like a std-container to store data of some structure. 
Basically a Metric Search Tree has the same principle as a binary search tree or a kd-tree, 
but it works for arbitrary data structures. This Metric Search Tree is basically a Cover Tree Implementation. 
Additionally to the distiance (or similarity) between the data, a covering distance from level to 
level decides how the tree grows.

- **Graph**. Graph is a general concept for the explicit representation of a metric
space for matrix and tree.
The edges in this graph represent the distances between the individual elements.

## Tree
#### Simple example
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

*For a full example and more details see `examples/space_examples/simple_example.cpp`*



#### Initialize a Tree
Construct an empty tree or fill it directly with data records.
```c++
typedef std::vector<double> RecType;
typedef std::vector<RecType> recList;

/*** Tree with default L2 metric (Euclidean distance measure) ***/
metric::Tree<RecType> cTree;             // empty tree
metric::Tree<RecType> cTree(RecType v1); // with one data record
metric::Tree<RecType> cTree(recList m1); // a container with records.

/** A Tree with a custom metric. ***/
metric::Tree<RecType, customMetric> cTree;
```

 #### Search options
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

*For a full example and more details see `examples/space_examples/search_and_access_example.cpp`*



#### Access the nodes
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

*For a full example and more details see `examples/space_examples/search_and_access_example.cpp`*


#### Use a custom container with custom metric use an "Eigen" Vector and L1 metric.

Custom metric:
```c++
using RecType = Eigen::VectorXd;

template <typename T>
struct recMetric {
    T operator()(const Eigen::VectorXd &p, const Eigen::VectorXd &q) const
    {
        return (p - q).lpNorm<1>(); // L1 norm
    }
};
``` 

Then we can create the data and the tree:
```c++
RecType x(8);
	
std::vector<RecType> table;

x << 0, 1, 1, 1, 1, 1, 2, 3;
table.push_back(x);
x << 1, 1, 1, 1, 1, 2, 3, 4;
table.push_back(x);
x << 2, 2, 2, 1, 1, 2, 0, 0;
table.push_back(x);
x << 3, 3, 2, 2, 1, 1, 0, 0;
table.push_back(x);
x << 4, 3, 2, 1, 0, 0, 0, 0;
table.push_back(x);
x << 5, 3, 2, 1, 0, 0, 0, 0;
table.push_back(x);
x << 4, 6, 2, 2, 1, 1, 0, 0;
table.push_back(x);
		
metric::Tree<RecType, recMetric<double>> cTree(table);
``` 

#### Details
- `data record` is a set of values, a row in a table, a data point, etc. All records contain the same paramters, but with different values.

- `n` is a number of data records.

- `distance` is one value, that represents the similarty of two data records. Less distance means more similar. 
For geometric points, it is just the geometric distance. For high dimensional points it's exatly the same math, 
but called `Euclidean Distance` or `L2 Metric`. But there are other `Metrics` as well, who sometimes works better.

The Tree Implementation is based on the Cover Tree Approach (https://en.wikipedia.org/wiki/Cover_tree),
that means in comparsion to a binary search tree an additionally "covering" distance (that only depends on the tree level) is used togehter with the actual metric between the data records. It decides if a data record becomes a sibling or a child in the tree.

Benefits in comparsion to a brute force search over all data:
```
std::vector<data_record>  metric::tree<data_record>
add record:      0                       log(n)
find nn record:  n                       log(n)
```

#### Implementation Cons
The Tree is currently balanced by ideas of http://proceedings.mlr.press/v37/izbicki15.pdf. However, it can still grow degenerative by building single long branches or bushy levels if the distance values are too small << 0.1. One can avoid it by simple rescaling the distance by a constant factor. If all distances are somewhere in the range of 0.1 ... 100 it works best.

The overhead of every data records is ca. 64 Byte to handle the nodes.

*Other CoverTree Implementations can be found here, but each is missing some functionality, that drives me to this new approach. It can be seen as bringing togehter all the good ideas.*
- https://gitlab.com/christoph-conrads/cover-tree
- https://github.com/DNCrane/Cover-Tree
- https://github.com/manzilzaheer/CoverTree


## Graph

#### Simple example


```c++
size_t w = 3; 
size_t h = 3; 
	
auto g1 = metric::Grid4(h, w);
auto g2 = metric::Grid6(h, w);
auto g3 = metric::Grid8(h, w);
auto g4 = metric::Margulis(h * w);
auto g5 = metric::Paley(h * w);
auto g6 = metric::LPS(h * w);

```

#### Access the nodes

```c++
size_t node = 1;
size_t max_depth = 4;

std::vector<std::vector<size_t>> neighbors = g1.getNeighbours(node, max_depth);

// out
// 0 | 1
// 1 | 0
// 1 | 2
// 1 | 4
// 2 | 3
// 2 | 5
// 2 | 7
// 3 | 6
// 3 | 8

```

#### Custom Graph

Suppose we have a function that creates grid's vector:

```c++    
std::vector<std::pair<size_t, size_t>> createGrid4(size_t width, size_t height)
{
    std::vector<std::pair<size_t, size_t>> edgesPairs;

    for (size_t i = 0; i < height; ++i) {
        for (size_t j = 0; j < width; ++j) {

            int ii0 = 0, ii1 = 0;
            int jj0 = 0, jj1 = 0;

            if (i > 0) {
                ii0 = -1;
            }
            if (j > 0) {
                jj0 = -1;
            }

            if (i < height - 1) {
                ii1 = 1;
            }
            if (j < width - 1) {
                jj1 = 1;
            }

            for (int ii = ii0; ii <= ii1; ++ii) {
                for (int jj = jj0; jj <= jj1; ++jj) {
                    if ((ii == 0) || (jj == 0)) {
                        edgesPairs.emplace_back(i * width + j, (i + ii) * width + (j + jj));
                    }
                }
            }
        }
    }
    return edgesPairs;
}
```

Then we can create Graph object from custom values:

```c++    

std::vector<std::pair<size_t, size_t>> edges = createGrid4(h, w);

auto g_custom = metric::Graph<char, true, false>(edges);  // edge value type = bool, isDense = false, isSymmetric = true

std::cout << g_custom.get_matrix() << std::endl;

// out
// ( 0 1 0 1 0 0 0 0 0 )
// ( 1 0 1 0 1 0 0 0 0 )
// ( 0 1 0 0 0 1 0 0 0 )
// ( 1 0 0 0 1 0 1 0 0 )
// ( 0 1 0 1 0 1 0 1 0 )
// ( 0 0 1 0 1 0 0 0 1 )
// ( 0 0 0 1 0 0 0 1 0 )
// ( 0 0 0 0 1 0 1 0 1 )
// ( 0 0 0 0 0 1 0 1 0 )

```

---

## Run
*You need STL and C++17 support to compile.*

METRIC | SPACE works headonly. Just include the header into your project.

```cpp
#include "modules/space.hpp"
```

or directly include one of specified distance from the following:

```cpp
#include "modules/space/tree.hpp"
#include "modules/space/matrix.hpp"
```



#### CMake compilation

Inside folder with your cpp file or inside `examples/space_examples/` run the following commands:

_Windows_

```bash
mkdir build
cd build
cmake .. -A x64 -T llvm
```
Then open solution in the Microsoft Visual Studio

_Linux_

Just run cmake
```bash
mkdir build
cd build
cmake ..
make
```

#### Direct compilation, using compiler

```bash
$ clang++ ./examples/space_examples/simple_example.cpp -std=c++17
```
