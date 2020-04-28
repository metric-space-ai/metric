/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 Panda Team
*/

#include "../../modules/distance.hpp"
#include "../../modules/space.hpp"

//#include "../../3rdparty/blaze/Blaze.h"


#include <vector>
#include <iostream>

int main()
{
    //using Record = blaze::DynamicVector<double>; // works ok
    using Record = std::deque<double>;

    std::cout << "Simple space example have started" << std::endl;
    std::cout << "" << std::endl;

    /*** here are some data records ***/
    Record v0 = { 0, 1, 1, 1, 1, 1, 2, 3 };
    Record v1 = { 1, 1, 1, 1, 1, 2, 3, 4 };
    Record v2 = { 2, 2, 2, 1, 1, 2, 0, 0 };
    Record v3 = { 3, 3, 2, 2, 1, 1, 0, 0 };


    /*** initialize the tree ***/
    metric::Tree<metric::Euclidean<Record>> cTree;

    /*** add data records ***/
    cTree.insert(v0);
    cTree.insert(v1);
    cTree.insert(v2);
    cTree.insert(v3);


    /*** find the nearest neighbour of a data record ***/
    Record v8 = { 2, 8, 2, 1, 0, 0, 0, 0 };

    auto nn = cTree.nn(v8);
    std::cout << "nearest neighbour of v8 is v" << nn->ID << std::endl;
    std::cout << std::endl;


    cTree.print();

    std::cout << std::endl;
    std::cout << std::endl;

    //

    /*** batch insert ***/
    std::vector<Record> table = {
        { 0, 1, 1, 1, 1, 1, 2, 3 },
        { 1, 1, 1, 1, 1, 2, 3, 4 },
        { 2, 2, 2, 1, 1, 2, 0, 0 },
        { 3, 3, 2, 2, 1, 1, 0, 0 },
        { 4, 3, 2, 1, 0, 0, 0, 0 },
        { 5, 3, 2, 1, 0, 0, 0, 0 },
        { 4, 6, 2, 2, 1, 1, 0, 0 },
    };

    metric::Tree<metric::Euclidean<Record>> cTree2(table);

    std::vector<Record> table2 = { { 3, 7, 2, 1, 0, 0, 0, 0 }, { 2, 8, 2, 1, 0, 0, 0, 0 } };

    cTree2.insert(table2);

    cTree2.print();

    return 0;
}
