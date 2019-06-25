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

#include "../metric_correlation.hpp"
#include "blaze/Math.h"

template <typename T>
void matrix_print(const std::vector<std::vector<T>> &mat)
{

    std::cout << "[";
    for (int i = 0; i < mat.size(); i++)
    {
        for (int j = 0; j < mat[i].size() - 1; j++)
        {
            std::cout << mat[i][j] << ", ";
        }
        std::cout << mat[i][mat[i].size() - 1] << " ]" << std::endl;
        ;
    }
}

template <typename T>
void vector_print(const std::vector<T> &vec)
{

    std::cout << "[";
    for (int i = 0; i < vec.size(); i++)
    {
        std::cout << vec[i] << ", ";
    }
    std::cout << vec[vec.size() - 1] << " ]" << std::endl;
}

struct simple_user_euclidian
{
    double operator()(const std::vector<double> &a, const std::vector<double> &b) const
    {
        double sum = 0;
        for (size_t i = 0; i < a.size(); ++i)
        {
            sum += (a[i] - b[i]) * (a[i] - b[i]);
        }
        return std::sqrt(sum);
    }
};

struct blaze_euclidean
{
    double operator()(const blaze::DynamicVector<double> &p, const blaze::DynamicVector<double> &q) const
    {
        return blaze::l2Norm(p - q);
    }
};

std::vector<std::vector<double>> read_csv(const std::string filename, const long length = -1)
{
    std::vector<std::vector<double>> v;

    std::ifstream in(filename);
    if (!in.is_open())
    {
        std::cout << "Failed to open " << filename << std::endl;
        return v;
    }

    size_t count = 0;
    std::string record;
    while (std::getline(in, record))
    {
        if ((length > 0) && (count++ >= length))
        {
            break;
        }

        std::istringstream is(record);
        std::vector<double> row((std::istream_iterator<double>(is)),
                                std::istream_iterator<double>());
        v.push_back(row);
    }

    // for ( const auto &row : v )
    // {
    //     for ( double x : row ) std::cout << x << ' ';
    //     std::cout << std::endl;
    // }

    return v;
}

int main()
{
	std::cout << "we have started" << std::endl;
	std::cout << '\n';

    using RecType = std::vector<double>;
    using RecType2 = blaze::DynamicVector<double>;

    std::vector<RecType> data1 = read_csv("assets/dataset1.csv", atoi("limit"));
    std::vector<RecType> data2 = read_csv("assets/dataset2.csv", atoi("limit"));
    if (data1.empty() || data2.empty())
    {
        return EXIT_FAILURE;
    }

    std::cout << "rows: " << data1.size() << ", cols: " << data1[0].size() << std::endl;
    std::cout << "rows: " << data2.size() << ", cols: " << data2[0].size() << std::endl;

    std::vector<RecType2> d1(data1.size());
    std::vector<RecType2> d2(data2.size());

    for (size_t i = 0; i < data1.size(); ++i)
    {
        blaze::DynamicVector<double> tmp(1UL, data1[i].size());
        for (size_t j = 0; j < data1[i].size(); ++j)
        {
            tmp[j] = data1[i][j];
        }
        d1[i] = tmp;
    }

    for (size_t i = 0; i < data2.size(); ++i)
    {
        blaze::DynamicVector<double> tmp(1UL, data2[i].size());
        for (size_t j = 0; j < data2[i].size(); ++j)
        {
            tmp[j] = data2[i][j];
        }
        d2[i] = tmp;
    }

    /* Build functors (function objects) with user types and metrics */
    typedef simple_user_euclidian Met;
    typedef blaze_euclidean Met2;

    /* Set up the correlation function */
    auto mgc_corr = metric::correlation::MGC<RecType, Met, RecType, Met>();

    /* Compute distance matrix 1 */
    // auto dist1= metric::correlation::blaze_distance_matrix<Met2>()(d1);
    // auto dist2= metric::correlation::blaze_distance_matrix<Met2>()(d2);

    // double result2= metric::correlation::MGC_direct()(dist1,dist2);

    /* Compute and benchmark */
	auto t1 = std::chrono::steady_clock::now();
	auto result = mgc_corr.estimate(data1, data2, 100, 1.0, 100); // A1 = std::vector<...>, A2 = std::deque<...>
	auto t2 = std::chrono::steady_clock::now();
	std::cout << result << " (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000 << "s)" << std::endl;

	// out: 

	// (Time for Center Matrices = 0.004921s)
	// (Time for Center ranked distance Matrices = 0.066706s)
	// (Time for Transpose Matrices = 0.006052s)
	// (Time for Covariance Matrices = 0.01338s)
	// (Time for clean up Matrices = 0.000603s)
	// (Time for normalize Matrices = 0.005805s)
	// (Time for clean up Matrices = 0.000181s)
	// (Time for significane Matrices = 0.070254s)
	// (Time for optimal Matrices = 0.002556s)
	// 1 - nan(ind) 0.653303 0.653303
	// (Time for Center Matrices = 0.004444s)
	// (Time for Center ranked distance Matrices = 0.053503s)
	// (Time for Transpose Matrices = 0.001877s)
	// (Time for Covariance Matrices = 0.005545s)
	// (Time for clean up Matrices = 0.000778s)
	// (Time for normalize Matrices = 0.001573s)
	// (Time for clean up Matrices = 2.3e-05s)
	// (Time for significane Matrices = 0.062688s)
	// (Time for optimal Matrices = 0.005361s)
	// 2 0.5 0.648665 0.650984
	// 0.650984 (Time = 0.528097s)


	t1 = std::chrono::steady_clock::now();
	result = mgc_corr(data1, data2);
	t2 = std::chrono::steady_clock::now();
	std::cout << result << " (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000 << "s)" << std::endl;

	// out:

	// (Time for Center Matrices = 9.1524s)
	// (Time for Center ranked distance Matrices = 396.751s)
	// (Time for Transpose Matrices = 112.499s)
	// (Time for Covariance Matrices = 163.922s)
	// (Time for clean up Matrices = 13.7064s)
	// (Time for normalize Matrices = 8.96766s)
	// (Time for clean up Matrices = 0.142683s)
	// (Time for significane Matrices = 184.483s)
	// (Time for optimal Matrices = 5.64593s)
	// 0.65593 (Time = 1020.32s)


	t1 = std::chrono::steady_clock::now();
	result = metric::correlation::MGC<RecType2, Met2, RecType2, Met2>()(d1, d2);
	t2 = std::chrono::steady_clock::now();
	std::cout << result << " (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000 << "s)" << std::endl;
	 
	// out:

	// (Time for Center Matrices = 7.02497s)
	// (Time for Center ranked distance Matrices = 324.589s)
	// (Time for Transpose Matrices = 104.01s)
	// (Time for Covariance Matrices = 176.117s)
	// (Time for clean up Matrices = 12.7903s)
	// (Time for normalize Matrices = 12.0978s)
	// (Time for clean up Matrices = 0.326226s)
	// (Time for significane Matrices = 218.289s)
	// (Time for optimal Matrices = 6.77513s)
	// 0.65593 (Time = 928.156s)

    return 0;
}
