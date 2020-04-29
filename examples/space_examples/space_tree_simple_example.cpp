/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 Panda Team
*/


// select here any single option to choose example data type:
#define USE_STL // STL containers
//#define USE_BLAZE // Blaze DynamicVector
//#define USE_EIGEN // Eigen Array
//#define USE_ARMA // Armadillo Row, requires linker parameter: -larmadillo

#include "../../modules/distance.hpp"
#include "../../modules/space.hpp"

#if defined(USE_BLAZE)
#include "../../3rdparty/blaze/Blaze.h"
#elif defined(USE_EIGEN)
#include "../../3rdparty/Eigen/Dense"
#elif defined(USE_ARMA)
#include <armadillo>
#endif

#include <vector>
#include <iostream>

int main()
{
#if defined(USE_STL)
    using Record = std::deque<double>;
#elif defined(USE_BLAZE)
    using Record = blaze::DynamicVector<double>; // works ok
#elif defined(USE_EIGEN)
    using Record = Eigen::Array<double, 1, Eigen::Dynamic>;
#elif defined(USE_ARMA)
    using Record = arma::Row<double>;
#endif

    std::cout << "Simple space example have started" << std::endl;
    std::cout << "" << std::endl;

    /*** here are some data records ***/

#ifdef USE_EIGEN  // Eigen requires other initialization code than other types
    Record v0 (8); // Eigen
    Record v1 (8);
    Record v2 (8);
    Record v3 (8);
    v0 << 0, 1, 1, 1, 1, 1, 2, 3; // Eigen
    v1 << 1, 1, 1, 1, 1, 2, 3, 4;
    v2 << 2, 2, 2, 1, 1, 2, 0, 0;
    v3 << 3, 3, 2, 2, 1, 1, 0, 0;
#else
    Record v0 = { 0, 1, 1, 1, 1, 1, 2, 3 }; // STL, Blaze, Armadillo
    Record v1 = { 1, 1, 1, 1, 1, 2, 3, 4 };
    Record v2 = { 2, 2, 2, 1, 1, 2, 0, 0 };
    Record v3 = { 3, 3, 2, 2, 1, 1, 0, 0 };
#endif

    /*** initialize the tree ***/
    metric::Tree<metric::Euclidean<Record>> cTree;

    /*** add data records ***/
    cTree.insert(v0);
    cTree.insert(v1);
    cTree.insert(v2);
    cTree.insert(v3);


    /*** find the nearest neighbour of a data record ***/
#if defined(USE_EIGEN)
    Record v8 (8); // Eigen
    v8 << 2, 8, 2, 1, 0, 0, 0, 0;
#else
    Record v8 = { 2, 8, 2, 1, 0, 0, 0, 0 };
#endif

    auto nn = cTree.nn(v8);
    std::cout << "nearest neighbour of v8 is v" << nn->ID << std::endl;
    std::cout << std::endl;


    cTree.print();

    std::cout << std::endl;
    std::cout << std::endl;

    //

    /*** batch insert ***/
#if defined(USE_EIGEN)
    std::vector<std::vector<double>> table_v = {
        { 0, 1, 1, 1, 1, 1, 2, 3 },
        { 1, 1, 1, 1, 1, 2, 3, 4 },
        { 2, 2, 2, 1, 1, 2, 0, 0 },
        { 3, 3, 2, 2, 1, 1, 0, 0 },
        { 4, 3, 2, 1, 0, 0, 0, 0 },
        { 5, 3, 2, 1, 0, 0, 0, 0 },
        { 4, 6, 2, 2, 1, 1, 0, 0 },
    };
    std::vector<Record> table;
    for (size_t i = 0; i < table_v.size(); ++i) {
        auto r = Record::Map(table_v[i].data(), table_v[i].size());
        table.push_back(r);
    }

    std::vector<std::vector<double>> table2_v = { {3, 7, 2, 1, 0, 0, 0, 0 }, { 2, 8, 2, 1, 0, 0, 0, 0 } };
    std::vector<Record> table2;
    for (size_t i = 0; i < table2_v.size(); ++i) {
        auto r = Record::Map(table2_v[i].data(), table2_v[i].size());
        table2.push_back(r);
    }
#else
        std::vector<Record> table = {
            { 0, 1, 1, 1, 1, 1, 2, 3 },
            { 1, 1, 1, 1, 1, 2, 3, 4 },
            { 2, 2, 2, 1, 1, 2, 0, 0 },
            { 3, 3, 2, 2, 1, 1, 0, 0 },
            { 4, 3, 2, 1, 0, 0, 0, 0 },
            { 5, 3, 2, 1, 0, 0, 0, 0 },
            { 4, 6, 2, 2, 1, 1, 0, 0 },
        };
    std::vector<Record> table2 = { {3, 7, 2, 1, 0, 0, 0, 0 }, { 2, 8, 2, 1, 0, 0, 0, 0 } };
#endif

    metric::Tree<metric::Euclidean<Record>> cTree2(table);

    cTree2.insert(table2);

    cTree2.print();

    return 0;
}
