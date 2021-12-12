/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 Panda Team
*/

#include <vector>
#include <iostream>
#include "modules/space.hpp"
#include "modules/distance.hpp"
int main()
{
	std::cout << "Search and access example have started" << std::endl;
	std::cout << "" << std::endl;

    /*** batch insert ***/
    std::vector<std::vector<double>> table = {
        { 0, 1, 1, 1, 1, 1, 2, 3 },
        { 1, 1, 1, 1, 1, 2, 3, 4 },
        { 2, 2, 2, 1, 1, 2, 0, 0 },
        { 3, 3, 2, 2, 1, 1, 0, 0 },
        { 4, 3, 2, 1, 0, 0, 0, 0 },
        { 5, 3, 2, 1, 0, 0, 0, 0 },
        { 4, 6, 2, 2, 1, 1, 0, 0 },
    };

    metric::Tree<std::vector<double>, metric::Euclidean<double>> cTree(table);

    cTree.print();
    std::cout << std::endl;

	// search
	
    std::vector<double> a_record = { 2, 8, 2, 1, 0, 0, 0, 0 };
	double a_number = 5;
	double a_distance = 6;

	/*** logarithmic complexity ***/

	auto nn = cTree.nn(a_record);					// finds the nearest neighbour.
    std::cout << "nearest neighbour of a_record is: " << nn->ID << std::endl;
    std::cout << std::endl;

	auto knn = cTree.knn(a_record, a_number);		// finds the a_number nearest neighbours	
    for (auto i = 0; i < knn.size(); i++) {
		std::cout << "#" << i << " nearest neighbour (from " << a_number << ") of a_record is: " << knn[i].first->ID << ", distance: " << knn[i].second << std::endl;
    }
    std::cout << std::endl;

	auto rnn = cTree.rnn(a_record, a_distance);		// finds all neigbours in a_distance to a_record.
    for (auto i = 0; i < rnn.size(); i++) {
		std::cout << "#" << i << " nearest neighbour (in a distance " << a_distance << ") of a_record is: " << rnn[i].first->ID << ", distance: " << rnn[i].second << std::endl;
    }
    std::cout << std::endl;


	// access nodes
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
    std::cout << std::endl;
    std::cout << std::endl;

	/*** access a single node by index ***/
	auto data_record = cTree[1]; // internaly it just traverse throuh the tree and gives back the corresponding data record
								 // in linear complexity, avoid this.

    std::cout << "data record #1: {";
	for (auto i = 0; i < data_record.size() - 1; i++) 
	{
		std::cout << data_record[i] << ", ";
	}
    std::cout << data_record[data_record.size() - 1] << "}" << std::endl;

    return 0;
}
