/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 Panda Team
*/

#include <vector>
#include <iostream>
#include "../../modules/space.hpp"

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
    std::cout << "nn of v8 is v" << nn->ID << std::endl;

    cTree.insert(v8);

    cTree.print();

    /*** batch insert ***/
    std::vector<std::vector<double>> table = {
        {0, 1, 1, 1, 1, 1, 2, 3},
        {1, 1, 1, 1, 1, 2, 3, 4},
        {2, 2, 2, 1, 1, 2, 0, 0},
        {3, 3, 2, 2, 1, 1, 0, 0},
        {4, 3, 2, 1, 0, 0, 0, 0},
        {5, 3, 2, 1, 0, 0, 0, 0},
        {4, 6, 2, 2, 1, 1, 0, 0},};

    metric::Tree<std::vector<double>> cTree2(table);

    std::vector<std::vector<double>> table2 = {
        {3, 7, 2, 1, 0, 0, 0, 0},
        {2, 8, 2, 1, 0, 0, 0, 0}};

    cTree2.insert(table2);

    cTree2.print();

    return 0;
}