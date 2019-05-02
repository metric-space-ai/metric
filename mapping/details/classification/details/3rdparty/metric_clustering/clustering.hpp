/*This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.


Signal Empowering Technology presents

███╗   ███╗███████╗████████╗██████╗ ██╗ ██████╗     ██████╗██╗     ██╗   ██╗███████╗████████╗███████╗██████╗ ██╗███╗   ██╗ ██████╗         
████╗ ████║██╔════╝╚══██╔══╝██╔══██╗██║██╔════╝    ██╔════╝██║     ██║   ██║██╔════╝╚══██╔══╝██╔════╝██╔══██╗██║████╗  ██║██╔════╝         
██╔████╔██║█████╗     ██║   ██████╔╝██║██║         ██║     ██║     ██║   ██║███████╗   ██║   █████╗  ██████╔╝██║██╔██╗ ██║██║  ███╗        
██║╚██╔╝██║██╔══╝     ██║   ██╔══██╗██║██║         ██║     ██║     ██║   ██║╚════██║   ██║   ██╔══╝  ██╔══██╗██║██║╚██╗██║██║   ██║        
██║ ╚═╝ ██║███████╗   ██║   ██║  ██║██║╚██████╗    ╚██████╗███████╗╚██████╔╝███████║   ██║   ███████╗██║  ██║██║██║ ╚████║╚██████╔╝        
╚═╝     ╚═╝╚══════╝   ╚═╝   ╚═╝  ╚═╝╚═╝ ╚═════╝     ╚═════╝╚══════╝ ╚═════╝ ╚══════╝   ╚═╝   ╚══════╝╚═╝  ╚═╝╚═╝╚═╝  ╚═══╝ ╚═════╝                                                                                                                                                                                                                      
                                                                            Copyright (c) 2018 M.Welsch <michael@welsch.one>

                                                                                             

templated clustering functions


*/

#ifndef CLUSTERING_H
#define CLUSTERING_H

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <tuple>
#include <cmath>
#include <vector>
#include <numeric>
#include <algorithm>
#include <deque>
#include <string>
#include <random>
#include <assert.h>


namespace clustering {


// --------------------------------------------------------------
// KMEANS
// --------------------------------------------------------------

    /*
    k-means clustering algorithm. 
    Input (vector of fixed-size vector, clustersize)
    for example:
    std::vector<std::array<float, 5>> data{
            {0, 0, 0, 0, 0},
            {1.74120000000000, 4.07812000000000, -0.0927036000000, 41.7888000000000, 41.7888000000000},
            {7.75309000000000, 16.2466000000000, 3.03956000000000, 186.074000000000, 186.074000000000},
            {2.85493000000000, 3.25380000000000, 2.50559000000000, 68.5184000000000, 68.5184000000000},
            {5.81414000000000, 8.14015000000000, 3.22950000000000, 139.539000000000, 139.539000000000},
            {2.57927000000000, 2.63399000000000, 2.46802000000000, 61.9026000000000, 61.9026000000000}};
    auto [means, idx] = kmeans(data, 4); // clusters the data in 4 groups.
    means: A vector holding the means (same type as input data)
    idx: A vector containing the cluster index
    */

// --------------------------------------------------------------
// KMEDOIDS
// --------------------------------------------------------------

// --------------------------------------------------------------
// DBSCAN
// --------------------------------------------------------------

// --------------------------------------------------------------
// AFFPROP
// --------------------------------------------------------------

} // namespace clustering

#include "clustering.tpp"
#endif // CLUSTERING_H
