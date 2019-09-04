/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/

#include <vector>
#include <iostream>
#include <chrono>
#include "../../modules/distance.hpp"

int main()
{
	
	/******************** examples for k-related Distances **************************/

    std::cout << "Sorensen Distance example have started" << std::endl;
    std::cout << "" << std::endl;

    typedef double V_type;

    V_type vt0 = 0;
    V_type vt1 = 1;
    V_type vt2 = 2;
    V_type vt3 = 3;
	

    // blaze vectors
    blaze::DynamicVector<V_type> bdv1 {vt0, vt1, vt2, vt0};
    blaze::DynamicVector<V_type> bdv2 {vt0, vt1, vt3};
	
	auto t1 = std::chrono::steady_clock::now();	
	auto result = metric::sorensen(bdv1, bdv2);
	auto t2 = std::chrono::steady_clock::now();
    std::cout << "Sorensen Distance result from function on DynamicVector: " << result
		<< " (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000 << " ms)" << std::endl;
	std::cout << "" << std::endl;

	
	// 
    blaze::StaticVector<V_type, 4UL> bsv1 {vt0, vt1, vt2, vt0};
    blaze::StaticVector<V_type, 4UL> bsv2 {vt0, vt1, vt3, vt0};
	
	t1 = std::chrono::steady_clock::now();
	result = metric::sorensen(bsv1, bsv2);
	t2 = std::chrono::steady_clock::now();
    std::cout << "Sorensen Distance result from function on StativVector: " << result
		<< " (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000 << " ms)" << std::endl;
	std::cout << "" << std::endl;
	
	
	// 
    blaze::HybridVector<V_type, 4UL> bhv1 {vt0, vt1, vt2, vt0};
    blaze::HybridVector<V_type, 4UL> bhv2 {vt0, vt1, vt3, vt0};
	
	t1 = std::chrono::steady_clock::now();
	result = metric::sorensen(bhv1, bhv2);
	t2 = std::chrono::steady_clock::now();
    std::cout << "Sorensen Distance result from function on HybridVector: " << result
		<< " (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000 << " ms)" << std::endl;
	std::cout << "" << std::endl;
	
	
	// 
    blaze::CompressedVector<V_type> bcv1 {vt0, vt1, vt2, vt0};
    blaze::CompressedVector<V_type> bcv2 {vt0, vt1, vt3};
	
	t1 = std::chrono::steady_clock::now();
	result = metric::sorensen(bcv1, bcv2);
	t2 = std::chrono::steady_clock::now();
    std::cout << "Sorensen Distance result from function on CompressedVector: " << result
		<< " (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000 << " ms)" << std::endl;
	std::cout << "" << std::endl;
	

	// STL
    std::vector<V_type> obj1 = {vt0, vt1, vt2, vt0};
    std::vector<V_type> obj2 = {vt0, vt1, vt3};
	
	t1 = std::chrono::steady_clock::now();
	result = metric::sorensen(obj1, obj2);
	t2 = std::chrono::steady_clock::now();
    std::cout << "Sorensen Distance result from function on STL Vector: " << result
		<< " (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000 << " ms)" << std::endl;
	std::cout << "" << std::endl;

    return 0;
}
