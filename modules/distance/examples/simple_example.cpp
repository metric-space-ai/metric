/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/

#include <vector>
#include <iostream>
#include <chrono>
#include "assets/test_data.cpp"
#include "../metric_distance.hpp"
//#include "../details/k-random/Edit.hpp"

#include "blaze/Blaze.h"

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

    metric::distance::P_norm<double> distance(2);

    //-----------------------------------------------
    // The ground distance - thresholded Euclidean distance.
    // Since everything is ints, we multiply by COST_MULT_FACTOR.

    typedef int edm_Type;

    // const edm_Type COST_MULT_FACTOR = 1000;
    // const edm_Type THRESHOLD = 3 * COST_MULT_FACTOR; //1.412*COST_MULT_FACTOR;
    //                                                  // std::vector< std::vector<int> > cost_mat; // here it's defined as global for Rubner's interfaces.
    //                                                  // If you do not use Rubner's interface it's a better idea
    //                                                  // not to use globals.
    //                                                  // unsigned int im1_R = i0.size();
    //                                                  // unsigned int im1_C = i0.size();

    size_t im1_R = img1.size() / 6;
    size_t im1_C = img1[0].size() / 6;
    size_t n = im1_R * im1_C;

    // serialize_mat2vec
    std::vector<edm_Type> i1;
    std::vector<edm_Type> i2;
    std::vector<edm_Type> i3;
    std::vector<edm_Type> i4;

    for (size_t i = 0; i < im1_R; ++i)
    {
        for (size_t j = 0; j < im1_C; ++j)
        {
            i1.push_back(img1[i][j]);
            i2.push_back(img2[i][j]);
            i3.push_back(img3[i][j]);
            i4.push_back(img4[i][j]);
        }
    }

    auto cost_mat = metric::distance::EMD_details::ground_distance_matrix_of_2dgrid<edm_Type>(im1_C, im1_R);
//    auto cost_mat = metric::distance::EMD_details::ground_distance_matrix_of_2dgrid<long>(im1_C, im1_R); // not matching types

    auto maxCost = metric::distance::EMD_details::max_in_distance_matrix(cost_mat);

    std::cout << "bis hier" << std::endl;

    //matrix_print(cost_mat);

    metric::distance::EMD<edm_Type> distance2(cost_mat, maxCost);
//    metric::distance::EMD<long> distance2(cost_mat, maxCost);  // not matching types


    //auto result1 = distance(v0, v2);

    auto t1 = std::chrono::steady_clock::now();

    // geht davon aus, dass i1 und i2 serialisierte Vectoren der Bildmatrizen sind und cost_mat eine passende Distanzematrix enthält, die jedoch die orgiginalen Pixelpositionen berücksichtigt.
    auto result1 = distance2(i1, i2);
    auto t2 = std::chrono::steady_clock::now();
    std::cout << "-------r" << std::endl;
    auto result2 = distance2(i2, i1);
    auto t3 = std::chrono::steady_clock::now();

    std::cout << "result1: " << result1 << " (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()) / 1000000 << "s)" << std::endl;
    std::cout << "result2: " << result2 << " (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(t3 - t2).count()) / 1000000 << "s)" << std::endl;
    //     auto result2 = distance2(i1, i3, cost_mat);

    //     auto t3 = std::chrono::steady_clock::now();

    //    // std::cout << "result1: " << double(result1)/1e8 << ",  result2: " << double(result2)/1e8 << std::endl;
    // std::cout << "result2: " << result2 << " (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(t3 - t2).count()) / 1000000 << "s)" << std::endl;



    // SSIM
    metric::distance::SSIM<std::vector<int>> ssimFactor;

    auto result3_SSIM = ssimFactor(img1, img2);
    auto result4_SSIM = ssimFactor(img1, img3);

    std::cout << "result3_SSIM: " << result3_SSIM << ",  result4_SSIM: " << result4_SSIM << std::endl;



    // Edit
    std::string str1 = "1011001100110011001111110011001100110011011110011001100110011001110001100110011001101";
    std::string str2 = "1000011001100110011011100110011001100110111001100110011001100111000110011001100110011";

    // std::vector<int> vec2 = {1,2,3,4,5,6,7,8,9,10};
    // std::vector<int> vec1 = {};

    metric::distance::Edit<std::string> distance3;

    auto t4 = std::chrono::steady_clock::now();
    auto result5_EDIT = distance3(str1, str2);

    std::cout << "result5_EDIT: " << result5_EDIT << " (Time = " << double(std::chrono::duration_cast<std::chrono::microseconds>(t4 - t3).count()) / 1000000 << "s)" << std::endl;




    // test simple k-related distanses
    std::cout << "\nk-related metrics: \n";

    typedef double V_type;

    V_type vt0 = 0;
    V_type vt1 = 1;
    V_type vt2 = 2;
    V_type vt21 = 3;


    std::vector<V_type> obj1 = {vt0, vt1, vt2, vt0};
    std::vector<V_type> obj2 = {vt0, vt1, vt21, vt0};
    std::vector<V_type> obj3 = {vt0, vt1, vt21};



//    metric::distance::Sorensen_t<V_type> sor;
//    std::cout << "sorensen metric result: " << sor(obj1, obj2) << "\n";

//    metric::distance::Sorensen_s sor2;
//    std::cout << "sorensen metric result: " << sor2(obj1, obj2) << "\n";

//    metric::distance::Sorensen<V_type> sor3;
//    std::cout << "sorensen metric result: " << sor3(obj1, obj3) << "\n";

    // blaze vectors
    blaze::DynamicVector<V_type> bdv1 {vt0, vt1, vt2, vt0};
    blaze::DynamicVector<V_type> bdv3 {vt0, vt1, vt21};
//    std::cout << "sorensen metric result on DynamicVector: " << sor3(bdv1, bdv3) << "\n";

    blaze::StaticVector<V_type, 4UL> bsv1 {vt0, vt1, vt2, vt0};
    blaze::StaticVector<V_type, 4UL> bsv2 {vt0, vt1, vt21, vt0}; // static vectros cannot be of different length!!
//    std::cout << "sorensen metric result on StaticVector: " << sor3(bsv1, bsv2) << "\n";

    blaze::HybridVector<V_type, 4UL> bhv1 {vt0, vt1, vt2, vt0};
    blaze::HybridVector<V_type, 4UL> bhv2 {vt0, vt1, vt21, vt0}; // static vectros cannot be of different length!!
//    std::cout << "sorensen metric result on HybridVector: " << sor3(bhv1, bhv2) << "\n";

    blaze::CompressedVector<V_type> bcv1 {vt0, vt1, vt2, vt0};
    blaze::CompressedVector<V_type> bcv3 {vt0, vt1, vt21};
//    std::cout << "sorensen metric result on CompressedVector: " << sor3(bcv1, bcv3) << "\n";

    std::cout << "sorensen metric result from function on CompressedVector: " << metric::distance::sorensen(bcv1, bcv3) << "\n";
    std::cout << "sorensen metric result from function on STL Vector: " << metric::distance::sorensen(obj1, obj3) << "\n";
    std::cout << "sorensen metric result from function on DynamicVector: " << metric::distance::sorensen(bdv1, bdv3) << "\n";
    std::cout << "sorensen metric result from function on StativVector: " << metric::distance::sorensen(bsv1, bsv2) << "\n";
    std::cout << "sorensen metric result from function on HybridVector: " << metric::distance::sorensen(bhv1, bhv2) << "\n";

    // TODO test STL deque etc


    // test for template parameters of existing metrics

    metric::distance::Euclidian<float> eu;
    std::cout << "euclidean metric result: " << eu(obj1, obj2) << "\n";


    //



    return 0;
}
