/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/

#include <vector>
#include <random>
#include <cmath>
#include "modules/distance.hpp"
#include <iostream>
#include <fstream>
#include <random>  // for random vector generation

template <typename T>
void save_ds(std::vector<std::vector<T>> ds, std::string filename)
{
    std::ofstream f;
    typedef std::numeric_limits<T> dbl;
    f.precision(dbl::max_digits10);

    f.open(filename);
    for (size_t i = 0; i < ds.size(); i++) {
        for (size_t j = 0; j < ds[i].size() - 1; j++)
            f << ds[i][j] << ";";
        f << ds[i][ds[i].size() - 1] << std::endl;
    }
    f.close();
}

template <typename T>
std::vector<std::vector<T>> load_ds(std::string filename)
{
    std::vector<std::vector<T>> res;
    std::ifstream f;
    f.open(filename);
    std::string line;
    while (f.good()) {
        std::getline(f, line, '\n');
        std::istringstream ln_str(line);
        std::string data;
        std::vector<T> ln_num;
        while (getline(ln_str, data, ';')) {
            ln_num.push_back(atof(data.c_str()));
        }
        res.push_back(ln_num);
    }
    res.pop_back();
    f.close();
    return res;
}

int main()
{

    typedef std::numeric_limits<long double> ldbl;
    std::cout.precision(ldbl::max_digits10);

    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_int_distribution<int> uni(0, 4);
    std::uniform_real_distribution<long double> uni_real(-1, 1);  // interval [a, b)

    std::vector<long double> cases1 { 1, 3, 2, 4, 2 };
    std::vector<long double> cases2 { 4, 3, 2, 1, 4 };

    // entropies for fixed vector

    std::cout << "entropies for {{5,5}, {2,2}, {3,3}, {5,1}}:\n";

    std::vector<std::vector<double>> v = { { 5, 5 }, { 2, 2 }, { 3, 3 }, { 5, 1 } };
    auto e = entropy(v, 3, 2.0, metric::Chebyshev<double>());
    std::cout << "Chebyshev: " << e << std::endl;

    e = entropy(v, 3, 2.0, metric::P_norm<double>(3));
    std::cout << "General Minkowsky, 3: " << e << std::endl;

    e = entropy(v, 3, 2.0, metric::P_norm<double>(2));
    std::cout << "General Minkowsky, 2: " << e << std::endl;

    e = entropy(v, 3, 2.0, metric::Euclidian<double>());
    std::cout << "Euclidean: " << e << std::endl;

    e = entropy_kl(v, 3, 2.0, metric::Euclidian<double>());
    std::cout << "Euclidean Kozachenko-Leonenko: " << e << std::endl;

    e = entropy(v, 3, 2.0, metric::P_norm<double>(1));
    std::cout << "General Minkowsky, 1: " << e << std::endl;

    e = entropy(v, 3, 2.0, metric::Manhatten<double>());
    std::cout << "Manhatten: " << e << std::endl;

    e = metric::entropy(v);
    std::cout << "Default: " << e << std::endl;

    std::vector<std::vector<long double>> v_ld = { { 5, 5 }, { 2, 2 }, { 3, 3 }, { 5, 1 } };
    for (size_t i = 0; i < 10; i++) {
        e = entropy(v_ld, 3, 2.0L, metric::P_norm<long double>(3));
        std::cout << "General Minkowsky, 3: " << e << std::endl;
    }

    int ds_len = 20;
    std::vector<std::vector<double>> v1 = { { 5, 5 }, { 2, 2 }, { 3, 3 }, { 5, 5 } };
    std::vector<std::vector<double>> v2 = { { 5, 5 }, { 2, 2 }, { 3, 3 }, { 1, 1 } };
    //* // VOI, normalized VOI

    std::cout << std::endl;
    std::cout << "negative entropy" << std::endl;
    std::cout << std::endl;

    std::vector<std::vector<float>> v31 = { { 0 }, { 0 }, { 0 }, { 0 } };
    std::vector<std::vector<float>> v32 = { { 0 }, { 0 }, { 0 }, { 0 } };

    auto eX = entropy(v31, 3, 2.0f, metric::Chebyshev<long double>());
    auto eY = entropy(v32, 3, 2.0f, metric::Chebyshev<long double>());

    auto mi = metric::mutualInformation(v31, v32);

    auto voi = eX + eY - 2 * mi;

    std::cout << "eX = " << eX << std::endl;
    std::cout << "eY = " << eY << std::endl;
    std::cout << "mi = " << mi << std::endl;
    std::cout << "voi = " << voi << std::endl;

    for (size_t i = 0; i < 5; i++)
        std::cout << "voi = " << metric::variationOfInformation(v31, v32) << std::endl;
    for (size_t i = 0; i < 5; i++)
        std::cout << "voi_normalized = " << metric::variationOfInformation_normalized(v31, v32) << std::endl;

    auto f_voi = metric::VOI<long double>();
    for (size_t i = 0; i < 5; i++)
        std::cout << "voi_functor = " << f_voi(v31, v32) << std::endl;

    auto f_voi_norm = metric::VOI_normalized();
    for (size_t i = 0; i < 5; i++)
        std::cout << "voi_norm_functor = " << f_voi_norm(v31, v32) << std::endl;

    auto f_voi_kl = metric::VOI_kl<long double>();
    for (size_t i = 0; i < 5; i++)
        std::cout << "KL voi_functor = " << f_voi_kl(v31, v32) << std::endl;

    auto f_voi_norm_kl = metric::VOI_normalized_kl();
    for (size_t i = 0; i < 5; i++)
        std::cout << "KL voi_norm_functor = " << f_voi_norm_kl(v31, v32) << std::endl;

    return 0;
}
