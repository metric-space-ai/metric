/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2019 Panda Team
*/
#include <vector>
#include <deque>
#include <array>
#include <string>

#include <iostream>
#include <fstream>

#include <chrono>

#include "../../modules/correlation.hpp"
#include "../../3rdparty/blaze/Math.h"

template <typename T>
void matrix_print(const std::vector<std::vector<T>>& mat)
{

    std::cout << "[";
    for (int i = 0; i < mat.size(); i++) {
        for (int j = 0; j < mat[i].size() - 1; j++) {
            std::cout << mat[i][j] << ", ";
        }
        std::cout << mat[i][mat[i].size() - 1] << " ]" << std::endl;
        ;
    }
}

template <typename T>
void vector_print(const std::vector<T>& vec)
{

    std::cout << "[";
    for (int i = 0; i < vec.size(); i++) {
        std::cout << vec[i] << ", ";
    }
    std::cout << vec[vec.size() - 1] << " ]" << std::endl;
}

struct simple_user_Euclidean {
    double operator()(const std::vector<double>& a, const std::vector<double>& b) const
    {
        double sum = 0;
        for (size_t i = 0; i < a.size(); ++i) {
            sum += (a[i] - b[i]) * (a[i] - b[i]);
        }
        return std::sqrt(sum);
    }
};

struct blaze_euclidean {
    double operator()(const blaze::DynamicVector<double>& p, const blaze::DynamicVector<double>& q) const
    {
        return blaze::l2Norm(p - q);
    }
};

std::vector<std::vector<double>> read_csv(const std::string filename, const long length = -1)
{
    std::vector<std::vector<double>> v;

    std::ifstream in(filename);
    if (!in.is_open()) {
        std::cout << "Failed to open " << filename << std::endl;
        return v;
    }

    size_t count = 0;
    std::string record;
    while (std::getline(in, record)) {
        if ((length > 0) && (count++ >= length)) {
            break;
        }

        std::istringstream is(record);
        std::vector<double> row((std::istream_iterator<double>(is)), std::istream_iterator<double>());
        v.push_back(row);
    }

    return v;
}

int main()
{
	std::cout << "Advanced correlations example have started" << std::endl;
	std::cout << "" << std::endl;

    using RecType = std::vector<double>;

    std::vector<RecType> data1 = read_csv("assets/dataset1.csv", atoi("limit"));
    std::vector<RecType> data2 = read_csv("assets/dataset2.csv", atoi("limit"));
    if (data1.empty() || data2.empty()) {
        return EXIT_FAILURE;
    }

    std::cout << "dataset1 rows: " << data1.size() << ", cols: " << data1[0].size() << std::endl;
    std::cout << "dataset2 rows: " << data2.size() << ", cols: " << data2[0].size() << std::endl;
	std::cout << "" << std::endl;

    /* Build functors (function objects) with user types and metrics */
    typedef simple_user_Euclidean Met;

    /* Set up the correlation function */
    auto mgc_corr = metric::MGC<RecType, Met, RecType, Met>();

    /* Compute and benchmark */
    std::cout << "estimating correlation..." << std::endl;
    auto t1 = std::chrono::steady_clock::now();
    auto result = mgc_corr.estimate(data1, data2, 100, 1.0, 100);
    auto t2 = std::chrono::steady_clock::now();
    std::cout << "Multiscale graph correlation estimate: " 
              << result
              << " (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000
              << "s)" << std::endl;
	std::cout << std::endl;

    // out:
	// 1 -nan(ind) 0.653303 0.653303
	// 2 0.5 0.648665 0.650984
	// Multiscale graph correlation estimate: 0.650984 (Time = 0.823063s)
	
    std::cout << "computing correlation..." << std::endl;
    t1 = std::chrono::steady_clock::now();
    result = mgc_corr(data1, data2);
    t2 = std::chrono::steady_clock::now();    
    std::cout << "Multiscale graph correlation: " 
		      << result
              << " (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000
              << "s)" << std::endl;
	std::cout << std::endl;

    // out:
    // Multiscale graph correlation: 0.65593 (Time = 418.153s)

	// for blaze 

    using RecType2 = blaze::DynamicVector<double>;
    typedef blaze_euclidean Met2;

    std::vector<RecType2> d1(data1.size());
    std::vector<RecType2> d2(data2.size());

    for (size_t i = 0; i < data1.size(); ++i) {
        blaze::DynamicVector<double> tmp(1UL, data1[i].size());
        for (size_t j = 0; j < data1[i].size(); ++j) {
            tmp[j] = data1[i][j];
        }
        d1[i] = tmp;
    }

    for (size_t i = 0; i < data2.size(); ++i) {
        blaze::DynamicVector<double> tmp(1UL, data2[i].size());
        for (size_t j = 0; j < data2[i].size(); ++j) {
            tmp[j] = data2[i][j];
        }
        d2[i] = tmp;
    }
	
    /* Set up the correlation function */
    auto mgc_corr_blaze = metric::MGC<RecType2, Met2, RecType2, Met2>();

    std::cout << "estimating correlation (for blaze)..." << std::endl;
    t1 = std::chrono::steady_clock::now();
    result = mgc_corr_blaze.estimate(d1, d2);
    t2 = std::chrono::steady_clock::now();
    std::cout << "Multiscale graph correlation estimate (for blaze): " 
		      << result
              << " (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000
              << "s)" << std::endl;
	std::cout << std::endl;

    // out:
	// 1 -nan(ind) 0.66177 0.66177
	// 2 0.5 0.651861 0.656815
	// 3 0.417722 0.640442 0.651357
	// 4 0.359967 0.649279 0.650838
	// 5 0.322612 0.656676 0.652006
	// 6 0.262383 0.669431 0.65491
	// 7 0.250356 0.64777 0.65389
	// 8 0.197723 0.668255 0.655686
	// 9 0.19001 0.658393 0.655986
	// 10 0.177464 0.653838 0.655772
	// 11 0.1615 0.650828 0.655322
	// 12 0.15152 0.652565 0.655092
	// 13 0.146486 0.655824 0.655149
	// 14 0.143091 0.659013 0.655425
	// 15 0.139359 0.661811 0.65585
	// 16 0.136373 0.659662 0.656089
	// 17 0.125772 0.649607 0.655707
	// 18 0.120311 0.652674 0.655539
	// 19 0.117996 0.660476 0.655799
	// 20 0.115587 0.656575 0.655837
	// Multiscale graph correlation estimate (for blaze): 0.655837 (Time = 5.03296s)

    std::cout << "computing correlation (for blaze)..." << std::endl;
    t1 = std::chrono::steady_clock::now();
    result = mgc_corr_blaze(d1, d2);
    t2 = std::chrono::steady_clock::now();
    std::cout << "Multiscale graph correlation (for blaze): " 
		      << result
              << " (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000
              << "s)" << std::endl;
	std::cout << std::endl;

    // out:
    // Multiscale graph correlation (for blaze): 0.65593 (Time = 399.995s)

    return 0;
}
