/* // test if all needed is included in framework.cpp
#include "framework.cpp"

int main()
{
    return 0;
}

// */

//*
#include <iostream>
#include <vector>
#include <deque>

#include "3rdparty/blaze/Blaze.h"

#include "3rdparty/Eigen/Dense"

#include "framework.cpp"



int main()
{

    using V = double;
    //using R = std::vector<V>;
    //using R = std::deque<V>;
    using R = blaze::DynamicVector<V>;
    //using R = Eigen::Array<double, Eigen::Dynamic, 1>;
    using S = std::vector<R>;
    //using M = metric::MetricExample<R>;
    //using M = metric::MetricBase<R>;
    using M = metric::Euclidean<R>;

    using V2 = float;
    //using R2 = std::vector<V>;
    using R2 = std::deque<V2>;
    using S2 = std::deque<R>;

    using R3 = Eigen::Array<double, 1, Eigen::Dynamic>;

    R rec = {5, 3};
    S data = {{1, 1}, {2, 2}, {2, 3}};
    auto m = M();

    //auto a = metric::Algorithm<M>(m);
    auto a = metric::Norm<M>(m);
    auto result = a.process_single(rec);
    auto result_batch = a.process_batch(data);
    auto result_sorted = a.sort_by_distance(data, {2, 3});
    auto result_norm = a(rec);
    auto result_norm_batch = a.norm_batch(data);

    //R2 rec2 = {5, 3};
    //auto result2 = a.process_single(rec2); // no viable conversion
    //auto result_norm2 = a(rec2); // no matching function

    S2 data2 = {{1, 1}, {2, 2}, {2, 3}};
    auto result_norm_batch_2 = a.norm_batch(data2);

    R3 rec3 = R3(2);
    R3 rec3zero = R3(2);
    rec3 << 5, 3;
    rec3zero << 0, 0;
    auto m3 = metric::EuclideanNoIter<R3>();
    std::cout << "\nEigen-lib distance non-specific code test: " << m3(rec3, rec3zero) << "\n";

    auto m2 = metric::EuclideanNoIter<R2>();
    std::cout << "\nSTL distance non-specific code test: " << m2({5, 3}, {0, 0}) << "\n";

    auto m_stl = metric::EuclideanTypeSpecific<R2>();
    auto m_blaze = metric::EuclideanTypeSpecific<R>();
    auto m_eigen = metric::EuclideanTypeSpecific<R3>();
    R2 rec2 = {5, 3};
    R2 rec2zero = {0, 0};
    R rec1zero = {0, 0};
    std::cout << "\nSTL-specific distance test: " << m_stl(rec2, rec2zero) << "\n";
    std::cout << "\nBlaze-specific distance test: " << m_blaze(rec, rec1zero) << "\n";
    std::cout << "\nEigen-specific distance test: " << m_eigen(rec3, rec3zero) << "\n";


    std::cout << "\nnorm itself:\n";
    std::cout << result_norm << "\n";

    std::cout << "\nresult for single record:\n";
    for (size_t i = 0; i<result.size(); ++i) {
        std::cout << result[i] << "\t";
    }
    std::cout << "\n";

    std::cout << "\nresult for set of records:\n";
    for (size_t i = 0; i<result_batch.size(); ++i) {
        for (size_t j = 0; j<result_batch[0].size(); ++j) {
            std::cout << result_batch[i][j] << " | " << result_sorted[i][j] << " \t";
        }
        std::cout << "\n";
    }

    std::cout << "\nnorm result for set of records:\n";
    for (size_t i = 0; i<result_norm_batch.size(); ++i) {
        std::cout << result_norm_batch[i] << "\t";
    }
    std::cout << "\n\n";


    // trying defaults

    {
        std::vector<std::vector<double>> v_in = {{1, 1}, {2, 2}, {2, 3}};

        auto default_alg = metric::Algorithm();
        auto default_result_sorted = default_alg.sort_by_distance(v_in, {3, 3});
        std::cout << "\nresult for set of records, default algorithm , default nan returning metric:\n";
        for (size_t i = 0; i<default_result_sorted.size(); ++i) {
            for (size_t j = 0; j<default_result_sorted[0].size(); ++j) {
                std::cout << default_result_sorted[i][j] << " \t";
            }
            std::cout << "\n";
        }

        auto default_alg_eucl = metric::Algorithm<metric::Euclidean<>>();
        auto default_result_sorted_eucl = default_alg_eucl.sort_by_distance(v_in, {3, 3});
        std::cout << "\nresult for set of records, default algorithm, Euclidean:\n";
        for (size_t i = 0; i<default_result_sorted_eucl.size(); ++i) {
            for (size_t j = 0; j<default_result_sorted_eucl[0].size(); ++j) {
                std::cout << default_result_sorted_eucl[i][j] << " \t";
            }
            std::cout << "\n";
        }
    }



    // trying different metric return and input types

    {
        using RecType = std::deque<long double>;
        //using RecType = blaze::DynamicVector<double>;

        std::deque<RecType> v_in = {{1, 1}, {2, 2}, {2, 3}};

        auto default_alg_eucl = metric::Algorithm<metric::Euclidean<RecType>>();
        auto default_result_sorted_eucl = default_alg_eucl.sort_by_distance(v_in, {3, 3});
        std::cout << "\nresult for set of records, default algorithm, Euclidean, custom return type:\n";
        for (size_t i = 0; i<default_result_sorted_eucl.size(); ++i) {
            for (size_t j = 0; j<default_result_sorted_eucl[0].size(); ++j) {
                std::cout << default_result_sorted_eucl[i][j] << " \t";
            }
            std::cout << "\n";
        }
    }




    return 0;
}

// */
