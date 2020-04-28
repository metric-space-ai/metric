/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 Michael Welsch
*/

#include "../../modules/space.hpp"
#include "assets/assets.cpp"
#include "../../modules/utils.hpp" // for only DistanceType

#include "../../3rdparty/blaze/Math.h"

#include <iostream>
#include <vector>
#include <random>
#include <thread>

/*** define custom metric ***/

template <typename>
struct recMetric_Blaze;

template <typename VT>
struct recMetric_Blaze<blaze::CompressedVector<VT>> {

    using RecordType = blaze::CompressedVector<VT>;
    using ValueType = VT;

    metric::DistanceType operator()(const blaze::CompressedVector<VT>& p, const blaze::CompressedVector<VT>& q) const
    {
        return assets::sed(assets::zeroPad(p), assets::zeroPad(q), metric::DistanceType(0), metric::DistanceType(1));
    }
};

/*** simulation helper functions ***/
template <class MT>
void insert_random(metric::Tree<MT>& cTree, int samples, int dimension)
{
    // random generator
    std::random_device rnd_device;
    std::mt19937 mersenne_engine(rnd_device());
    std::uniform_real_distribution<double> dist(-1, 1);
    auto gen = std::bind(dist, mersenne_engine);

    for (int i = 0; i < samples; ++i) {
        std::vector<double> vec = assets::linspace(gen(), gen(), dimension);  // gererator random lines.
        blaze::CompressedVector<double> comp_vec(dimension);
        comp_vec = assets::smoothDenoise(vec, 0.1);  // denoise and sparse the data
        cTree.insert(comp_vec);
    }
    return;
}

/*** fill a tree with 1 Mio records and search for nearest neighbour **/
int main()
{
	std::cout << "Advanced space example have started" << std::endl;
	std::cout << "" << std::endl;

    metric::Tree<recMetric_Blaze<blaze::CompressedVector<double>>> cTree;

    int n_records = 250;
    int rec_dim = 10;
    int threads = 4;

    /*** parallel insert ***/
    std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();  // start inserting
    std::vector<std::thread> inserter;
    for (int i = 0; i < threads; ++i) {
        inserter.push_back(std::thread(insert_random<recMetric_Blaze<blaze::CompressedVector<double>>>,
            std::ref(cTree), n_records, rec_dim));
    }
    for (int i = 0; i < threads; ++i) {
        inserter[i].join();
    }
    std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();  // end inserting

    /*** search for a similar data record ***/
    std::vector<double> vec1 = assets::linspace(0.3, -0.3, rec_dim);
    auto a_record = assets::smoothDenoise(vec1, 0.1);
    auto nn = cTree.nn(a_record);  // nearest neigbour
    (void)nn;
    std::chrono::high_resolution_clock::time_point t3 = std::chrono::high_resolution_clock::now();  // end searching
    auto insert_time = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
    auto nn_time = std::chrono::duration_cast<std::chrono::microseconds>(t3 - t2).count();
	
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
    std::cout << std::endl;
    std::cout << std::endl;

    std::cout << "tree of " << (n_records * threads) << " curves build in " << insert_time / 1000.0 / 1000.0
              << " seconds, "
              << "nn find in " << nn_time / 1000.0 / 1000.0 << " seconds, " << std::endl;

    return 0;
}
