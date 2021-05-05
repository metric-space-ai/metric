#include <catch2/catch.hpp>

#include <limits>

#include "modules/distance/d-spaced/Riemannian.hpp"

#include <iostream>


using namespace metric;

using Matrix = blaze::DynamicMatrix<double>;
using Vector = blaze::DynamicVector<double>;


TEST_CASE("sygv_c")
{
    std::vector<double> A = { 1, 2, 2, 4 };
    std::vector<double> B = { 5, 6, 6, 8 };

    std::vector<double> W(2);
    std::vector<double> work(5);

    int info;
    dsygv(1, 'N', 'U', 2, A.data(), 2, B.data(), 2, W.data(), work.data(), 5, info);

    //std::cout << W[0] << " " << W[1] << std::endl;
}

TEST_CASE("sygv_blaze")
{
    Matrix A{{1, 2}, {2, 4}};
    Matrix B{{5, 6}, {6, 8}};
    Vector w{0, 0};

    sygv<double, double>(A, B, w);

    //std::cout << w[0] << " " << w[1] << std::endl;
}

TEST_CASE("riemannian_distance")
{
    Matrix A{{1, 2}, {2, 4}};
    Matrix B{{5, 6}, {6, 8}};

    auto rd = metric::RiemannianDistance<void, metric::Euclidean<double>>();
    std::cout << rd.matDistance(A, B) << std::endl;

    std::vector<std::vector<double>> ds1 {{0, 1}, {0, 0}, {1, 1}, {1, 0}};
    std::vector<std::vector<double>> ds2 {{0, 0}, {1, 1}, {2, 2}, {2, 1}};

    rd(ds1, ds2);
    rd.estimate(ds1, ds2);
}