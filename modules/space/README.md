#metric_space
A templated, header only and hopefully in future (not yet) thread safe C++14 implementation of a Metric Search Tree. A Search Tree works like a std-container to store data of some structure. Basically a Metric Search Tree has the same principle as a binary search tree or a kd-tree, but it works for arbitrary (!) data structures. This Metric Search Tree is basically a Cover Tree Implementation. Additionally to the distiance (or similarity) between the data, a covering distance from level to level decides how the tree grows.

It can be used to find similar sets of data in millions of data sets, each containing hundreds of single values in only a few milliseconds.

## simple example
build the tree, search for a data record and invastigate the tree structure.
```c++
#include "metric_space.hpp"
#include <iostream>
#include <vector>

int main()
{
    /*** here are some data records ***/
    std::vector<double> v0 = {0, 1, 1, 1, 1, 1, 2, 3};
    std::vector<double> v1 = {1, 1, 1, 1, 1, 2, 3, 4};
    std::vector<double> v2 = {2, 2, 2, 1, 1, 2, 0, 0};
    std::vector<double> v3 = {3, 3, 2, 2, 1, 1, 0, 0};
    std::vector<double> v4 = {4, 3, 2, 1, 0, 0, 0, 0};
    std::vector<double> v5 = {5, 3, 2, 1, 0, 0, 0, 0};
    std::vector<double> v6 = {4, 6, 2, 2, 1, 1, 0, 0};
    std::vector<double> v7 = {3, 7, 2, 1, 0, 0, 0, 0};

    /*** initialize the tree ***/
    metric::Tree<std::vector<double>> cTree;

    /*** add data records ***/
    cTree.insert(v0);
    cTree.insert(v1);
    cTree.insert(v2);
    cTree.insert(v3);
    cTree.insert(v4);
    cTree.insert(v5);
    cTree.insert(v6);
    cTree.insert(v7);

    /*** find the nearest neighbour of a data record ***/
    std::vector<double> v8 = {2, 8, 2, 1, 0, 0, 0, 0};
    auto nn = cTree.nn(v8);
    std::cout << "nn of v8 is v" << nn->ID << std::endl; // --> nn of v8 is v7

    cTree.insert(v8);

    cTree.print();

    //(2)
    // ├──(0)
    // |   └──(1)
    // ├──(3)
    // |   ├──(4)
    // |   |   └──(5)
    // |   └──(6)
    // └──(7)
    //     └──(8)

    return 0;
}

```

compile it for example with
clang++ ./main.cpp -o ./main  -std=c++14


## Initialize a Tree
Construct an empty tree or fill it directly with data records.
```c++
typedef std::vector<double> recType;
typedef std::vector<recType> recList

    /*** Tree with default L2 metric (Euclidian distance measure) ***/
    metric::Tree<recType>
        cTree;                           // empty tree
metric::Tree<recType> cTree(recType v1); // with one data record
metric::Tree<recType> cTree(recList m1); // a container with records.

/** A Tree with a custom metric. ***/
metric::Tree<recType, customMetric> cTree;
// ...
```

    ##search options
```c++
    /*** logarithmic complexity ***/
    auto nn = cTree.nn()              // finds the nearest neighbour.
              auto knn = cTree.knn(5) // finds the fives nearest neighbours
                         auto rnn = cTree
                                        .rnn(a_record, a_distance) // finds all neigbours in a_distance to a_record.

                                    /*** linear complexity***/
                                    // when data.sum() gives the sum of the data records elements  ...
                                    cTree.traverse([&](auto node_p) {
                                        if (4 < node_p->data.sum() && node_p->data.sum() <= 5)
                                            std::cout << "ID: " << node_p->ID << std::endl;
                                    };);

```

    ##access the nodes
```c++
    /*** access through dereference to the underlying data ***/
    nn->ID                      // gives the ID of the record. the ID is counted up like an vector index.
        nn->data                // gives the data record of a node (every node contains data)
            nn->parent          // gives the parent node in the tree
                nn->children[0] // gives the first child node. (children is a std::vector)
    nn->parent_dist             // gives the distance to the parent.
        nn->level               // gives the level of the node postion (higher is nearer to the root)

    /*** print the siblings IDs ***/
    for (auto q : nn->parent->children)
{
    std::cout << q->ID << std::endl;
}

/*** access a single node by index ***/
auto data_record = cTree[1]; // internaly it just traverse throuh the tree and gives back the corresponding data record
                             // in linear complexity, avoid this.
```

    ##use a custom container with custom metric use an "Eigen" Vector and L1 metric.

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
``` ##advanced example Find one similar curve under 1 Mio Curves.Use a time elastic distance
    metric(a sparsed TWED variant->see rts reporsitory) and
    parallel insert

```c++
#include "assets/assets.cpp"
#include "metric_space.hpp"
#include <blaze/Math.h>
#include <iostream>
#include <random>
#include <thread>
#include <vector>

    /*** define custom metric ***/
    template <typename T>
    struct recMetric_Blaze {
    T operator()(const blaze::CompressedVector<T> &p, const blaze::CompressedVector<T> &q) const
    {
        return assets::sed(assets::zeroPad(p), assets::zeroPad(q), T(0), T(1));
    }
};

/*** simulation helper functions ***/
template <class recType, class Metric>
void insert_random(metric::Tree<recType, Metric> &cTree, int samples, int dimension)
{
    // random generator
    std::random_device rnd_device;
    std::mt19937 mersenne_engine(rnd_device());
    std::uniform_real_distribution<double> dist(-1, 1);
    auto gen = std::bind(dist, mersenne_engine);

    for (int i = 0; i < samples; ++i) {
        std::vector<double> vec = assets::linspace(gen(), gen(), dimension); // gererator random lines.
        blaze::CompressedVector<double> comp_vec(dimension);
        comp_vec = assets::smoothDenoise(vec, 0.1); // denoise and sparse the data
        cTree.insert(comp_vec);
    }
    return;
}

/*** fill a tree with 1 Mio records and search for nearest neighbour **/
int main()
{

    metric::Tree<blaze::CompressedVector<double>, recMetric_Blaze<double>> cTree;

    int rec_count = 250000;
    int rec_dim = 100;
    int threads = 4;

    /*** parallel insert ***/
    std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now(); // start inserting
    std::vector<std::thread> inserter;
    for (int i = 0; i < threads; ++i) {
        inserter.push_back(std::thread(insert_random<blaze::CompressedVector<double>, recMetric_Blaze<double>>,
                                       std::ref(cTree), rec_count, rec_dim));
    }
    for (int i = 0; i < threads; ++i) {
        inserter[i].join();
    }
    std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now(); // end inserting

    /*** search for a similar data record ***/
    std::vector<double> vec1 = assets::linspace(0.3, -0.3, rec_dim);
    auto comp_vec = assets::smoothDenoise(vec1, 0.1);
    auto nn = cTree.nn(comp_vec); // nearest neigbour

    std::chrono::high_resolution_clock::time_point t3 = std::chrono::high_resolution_clock::now(); // end searching
    auto insert_time = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
    auto nn_time = std::chrono::duration_cast<std::chrono::microseconds>(t3 - t2).count();

    std::cout << "tree of " << (rec_count * threads) << " curves build in " << insert_time / 1000.0 / 1000.0
              << " seconds, "
              << "nn find in " << nn_time / 1000.0 / 1000.0 << " seconds, " << std::endl;

    return 0;
}
```
## details
data record = a set of values, a row in a table, a data point, etc. all records contain the same paramters, but with different values.

n = number of data records.

distance = one value, that represents the similarty of two data records. less distance = more similar. for geometric points, it is just the geometric distance. for high dimensional points it's exatly the same math, but called euclidian distance or L2 Metric. But there are other Metrics as well, who sometimes works better.

The Tree Implementation is based on the Cover Tree Approach (https://en.wikipedia.org/wiki/Cover_tree),
that means in comparsion to a binary search tree an additionally "covering" distance (that only depends on the tree level) is used togehter with the actual metric between the data records. It decides if a data record becomes a sibling or a child in the tree.

Benefits in comparsion to a brute force search over all data
```
            std::vector<data_record>  metric::tree<data_record>
add record:      0                       log(n)
find nn record:  n                       log(n)
```

Implementation Cons:
The Tree is currently not balanced and can grow degenerative by building single long branches or bushy levels. In both extreme cases it is not better than simple brute force over an array.

I'm working on a balancing, which hopefully solves this issue. for pratice check the tree.max_level or print the tree to check the growing. The max_level should not be much bigger than log(n).

The overhead of every data records is ca. 64 Byte to handle the nodes.

Other CoverTree Implementations can be found here, but each is missing some functionality, that drives me to this new approach. It can be seen as bringing togehter all the good ideas.
https://gitlab.com/christoph-conrads/cover-tree
https://github.com/DNCrane/Cover-Tree
https://github.com/manzilzaheer/CoverTree