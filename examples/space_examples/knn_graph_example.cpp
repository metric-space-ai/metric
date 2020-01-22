/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/

#include <iostream>
#include "../../modules/utils/graph.hpp"
#include "../../modules/distance.hpp"





//template <typename Sample, typename D = double, typename Distance = metric::Euclidian<D>>
//int updatekNN(metric::KNNGraph graph, Sample p1, Sample p2, D dist)
//{
//
//    //kNNList* kl = &kNN->list[p1];
//    //kNNItem* ki = kl->items;
//
//	D max_dist = 100;
//	size_t num_edges = 2;
//
//    if(max_dist > dist || num_edges < graph.neighbors_num()) 
//	{
//        //int i = 0;
//        //for(i=0; i <= kl->size; i++) {
//        //    if(ki->id == p2) { return 0 ;} //TODO:??
//
//        //    if (ki->dist > dist || i == kl->size) {
//        //        int moveamount = kNN->k - i -1;
//        //        if(moveamount > 0 ) { //TODO: needed?
//        //            // Move from ki to ki+1
//        //            memmove(ki+1,ki,moveamount*sizeof(kNNItem));
//        //        }
//        //        ki->id = p2;
//        //        ki->dist = dist;
//        //        ki->new_item = true; //TODO: not needed in all search types
//
//        //        if(kl->size < kNN->k) { kl->size++;}
//        //        kl->max_dist = kl->items[kl->size -1].dist; //TODO: optimize?
//        //        break;
//        //    }
//        //    ki++;
//        //}
//        return 1; // Did update
//    }
//    else 
//	{
//        return 0; // Did not update
//    }
//}


//
//template <typename Sample>
//void random_pair_NN_descent(std::vector<Sample> X, int max_size)
//{
//	REPEAT
//	 random_pair_division(X, max_size);
//	 delta = Changes(kNN);
//	UNTIL delta < 10%
//
//	REPEAT
//	 random_pair_division(X, max_size);
//	 NNDES(X,kNN);
//	 delta = Changes(kNN);
//	UNTIL delta < StopCriterion;
//}


int main()
{
	std::cout << "Graph space example have started" << std::endl;
	std::cout << "" << std::endl;
	
    size_t neighbors_num = 3;
		
    std::vector<std::vector<double>> table = {
        { 0, 1 },
        { 1, 1 },
        { 2, 2 },
        { 3, 3 },
        { 4, 3 },
        { 5, 3 },
        { 4, 6 },
        { 5, 1 },
        { 4, 1 },
        { 3, 2 },
        { 0, 3 },
        { 1, 3 },
        { 2, 3 },
        { 6, 6 },
        { 0, 2 },
        { 0, 9 },
        { 0, 4 },
        { 0, 5 },
        { 0, 6 },
        { 0, 7 },
        { 0, 8 },
    };
	
    //std::vector<std::vector<double>> table = {
    //    { 0, 1, 1, 1, 1, 1, 2, 3 },
    //    { 1, 1, 1, 1, 1, 2, 3, 4 },
    //    { 2, 2, 2, 1, 1, 2, 0, 0 },
    //    { 3, 3, 2, 2, 1, 1, 0, 0 },
    //    { 4, 3, 2, 1, 0, 0, 0, 0 },
    //    { 5, 3, 2, 1, 0, 0, 0, 0 },
    //    { 4, 6, 2, 2, 1, 1, 0, 0 },
    //    { 0, 1, 1, 1, 1, 1, 6, 1 },
    //    { 1, 1, 1, 1, 1, 2, 7, 2 },
    //    { 2, 2, 2, 1, 1, 2, 8, 3 },
    //    { 3, 3, 2, 2, 1, 1, 9, 4 },
    //    { 4, 3, 2, 1, 0, 0, 5, 5 },
    //    { 5, 3, 2, 1, 0, 0, 4, 6 },
    //    { 4, 6, 2, 2, 1, 1, 3, 7 }
    //};
	
    auto g = metric::KNNGraph<std::vector<double>, metric::Euclidian<double>>(table, neighbors_num, 2.5 * neighbors_num);
	
    std::cout << "graph:" << std::endl;
    std::cout << g.get_matrix() << std::endl;
	
	//
	
	std::vector<double> query = { 7, 5 };

    std::cout << std::endl;
    std::cout << "gnn search:" << std::endl;
	auto found = g.gnnn_search(query, 3);
	
    std::cout << "[ ";
	for (size_t i = 0; i < found.size(); i++)
	{
		std::cout << found[i] << " ";
	}
    std::cout << "]";

	
    return 0;
}
