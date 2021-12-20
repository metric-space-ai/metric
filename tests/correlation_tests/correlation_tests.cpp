/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2019 Panda Team
*/
#include <catch2/catch.hpp>

#include <algorithm>
#include <deque>
#include "modules/correlation.hpp"
#include "modules/distance.hpp"
#include "modules/utils/graph/connected_components.hpp"

#include <random>
#include <limits>

using namespace Catch::literals;

template <typename T>
std::vector<T> generateVector(size_t size)
{
    std::default_random_engine g;
    std::normal_distribution<T> nd(0, 1);

    std::vector<T> vector(size);

    for (auto i = 0; i < size; ++i) {
        vector[i] = nd(g);
    }

    return vector;
}

template <typename T>
std::vector<std::vector<T>> generateMatrix(size_t rows, size_t cols)
{
    std::vector<std::vector<T>> matrix;
    matrix.reserve(rows);

    for (auto i = 0; i < rows; ++i) {
        matrix.emplace_back(generateVector<T>(cols));
    }

    return matrix;
}

template <typename T>
blaze::DynamicMatrix<T> generateRandomMatrix(size_t rows, size_t cols)
{
    std::default_random_engine g;
    std::normal_distribution<T> nd(0, 1);

    blaze::DynamicMatrix<T> matrix(rows, cols);

    for (auto i = 0; i < rows; ++i) {
        for (auto j = 0; j < cols; ++j) {
            matrix(i, j) = nd(g);
        }
    }

    return matrix;
}

TEMPLATE_TEST_CASE("Metrics", "correlation", float, double)
{
    auto vectorNull = generateVector<TestType>(0);

    metric::Euclidean<TestType> Euclidean;
    metric::Manhatten<TestType> manhatten;

    REQUIRE(Euclidean(vectorNull, vectorNull) == 0);
    REQUIRE(manhatten(vectorNull, vectorNull) == 0);

    std::vector<TestType> vector0(1000, 0);
    REQUIRE(Euclidean(vector0, vector0) == 0);
    REQUIRE(manhatten(vector0, vector0) == 0);

    auto vector = generateVector<TestType>(2000);
    REQUIRE(Euclidean(vector, vector) == 0);
    REQUIRE(manhatten(vector, vector) == 0);
}

TEST_CASE("CRACKER")
{
    blaze::DynamicMatrix<bool> M(10, 10, true);
    blaze::row(M, 2) = false;

    auto result = metric::graph::largest_connected_component(M)[0];
}

//TEST_CASE("MGC")
//{
//
//    double t = 1e-13;
//    std::vector<std::vector<double>> A1 = { { -1.08661677587398 }, { -1.00699896410939 }, { -0.814135753976830 },
//        { -0.875364720432552 }, { -0.659607023272462 }, { -0.798949992922930 }, { -0.431585448024267 },
//        { -0.619123703544758 }, { -0.351492263653510 }, { -0.394814371972061 }, { -0.309693618374598 },
//        { -0.352009525808777 }, { -0.0803413535982411 }, { 0.0103940699342647 }, { -0.130735385695596 },
//        { -0.138214899507693 }, { 0.0279270082022143 }, { 0.141670765995995 }, { 0.112221224566625 },
//        { 0.376767573021755 }, { 0.186729429735154 }, { 0.597349318463320 }, { 0.451380104139401 },
//        { 0.639237742050564 }, { 0.797420868050314 }, { 0.690091614630087 }, { 0.921722674141222 },
//        { 0.852593762434809 }, { 0.954771723842945 }, { 1.03297970279357 } };
//
//    std::deque<std::array<float, 1>> B1 = { { 2.70625143351230 }, { 1.41259513494005 }, { 0.666086793692617 },
//        { 0.647856446084279 }, { 0.887764969338737 }, { 0.286220905202707 }, { 0.543682026943014 },
//        { 0.0402339224257120 }, { 0.105812168910424 }, { 0.0230915137205610 }, { 0.00298976085950325 },
//        { 0.00366997150982423 }, { 0.000384825484363474 }, { 7.27293780465119e-05 }, { 2.50809340229209e-07 },
//        { 0.00306636655437742 }, { 0.000456283181338950 }, { 0.00801756105329616 }, { 1.17238339150888e-09 },
//        { 0.0803830108071682 }, { 0.0774478107095828 }, { 0.0474847202878941 }, { 0.0818772460512609 },
//        { 0.486406609209630 }, { 0.197547677770060 }, { 0.628321368933714 }, { 1.02400551043736 },
//        { 0.552591658802459 }, { 1.52144482984914 }, { 3.43908991254968 } };
//
//    typedef std::vector<double> Rec1;
//    typedef std::array<float, 1> Rec2;
//
//    typedef metric::Euclidean<double> Met1;
//    typedef metric::Manhatten<float> Met2;
//
//    auto mgc_corr = metric::MGC<Rec1, Met1, Rec2, Met2>();
//
//    auto result = mgc_corr(A1, B1);
//    REQUIRE(result == 0.28845660296530595_a);
//
//    auto mgc_corr2 = metric::MGC<Rec2, Met2, Rec1, Met1>();
//    result = mgc_corr2(B1, A1);
//    REQUIRE(result == 0.28845660296530595_a);
//
//    typedef std::vector<double> Rec;
//    auto mgc = metric::MGC<Rec, Met1, Rec, Met1>();
//    auto m1 = generateMatrix<double>(4, 4);
//    auto m2 = generateMatrix<double>(4, 4);
//    mgc(m1, m2);
//
//    /* MGC corelation */
//    std::vector<double> correlationReference = {0.3406052387919164,
//												0.36670317239506234,
//												0.35527060120466675,
//												0.3899053662191226,
//												0.2884565911772877,
//												0.37837904236096026,
//												0.4589100366738734,
//												0.3811881240213428,
//												0.44657481646290537};
//    auto correlation = mgc.xcorr(A1, B1, 4);
//
//    REQUIRE(correlationReference.size() == correlation.size());
//
//	for (auto i = 0; i < correlation.size(); ++i) {
//		REQUIRE(correlationReference[i] == Approx(correlation[i]));
//	}
//}

TEMPLATE_TEST_CASE("MGC_Estimate", "[correlation]", float, double)
{
    const int dataSize = 1e6;
    std::default_random_engine generator;
    std::normal_distribution<TestType> normal(0, 0.5);

    std::vector<std::vector<TestType>> dataX;
    std::vector<std::vector<TestType>> dataY;

    std::uniform_real_distribution<TestType> uniform(0, 1);
    for (auto i = 0; i < dataSize; ++i) {
        TestType x1 = uniform(generator);
        TestType x2 = uniform(generator);
        TestType x3 = uniform(generator);
        TestType y1 = x1 * x1 + normal(generator);
        TestType y2 = x2 * x2 * x2 + normal(generator);
        TestType y3 = std::pow(x3, 0.5) + normal(generator);

        dataX.emplace_back(std::initializer_list<TestType> { x1, x2, x3 });
        dataY.emplace_back(std::initializer_list<TestType> { y1, y2, y3 });
    }

    typedef std::vector<TestType> Rec;
    typedef metric::Euclidean<TestType> Met;

    auto mgc = metric::MGC<Rec, Met, Rec, Met>();

    auto result = mgc.estimate(dataX, dataY);
}

TEST_CASE("MGC_construct")
{
    // this test has no asserts, it just checks compilation

    metric::MGC<float, metric::Euclidean<float>, float, metric::Euclidean<float>> m1;
    m1(std::vector<float>{}, std::vector<float>{});

    metric::MGC<float, metric::Euclidean<float>, std::vector<float>, metric::Manhatten<float>> m2;
    m2(std::vector<float> {}, std::vector<std::vector<float>> {});

    metric::MGC<float, metric::Euclidean<float>, std::vector<double>, metric::Manhatten<double>> m3;
    m3(std::vector<float> {}, std::vector<std::vector<double>> {});

    metric::MGC<float, metric::Euclidean<float>,
                std::vector<double>, metric::Manhatten<double>> m4(metric::Euclidean<float>{});
    m4(std::vector<float> {}, std::vector<std::vector<double>> {});

    metric::MGC<float, metric::Euclidean<float>,
                std::vector<double>, metric::Manhatten<double>> m5(metric::Euclidean<float> {},
                                                                   metric::Manhatten<double>{});
    m5(std::vector<float> {}, std::vector<std::vector<double>> {});

    metric::Euclidean<float> e1;
    metric::Manhatten<double> e2;

    metric::MGC<float, metric::Euclidean<float>, std::vector<double>, metric::Manhatten<double>> m6(e1, e2);
    m6(std::vector<float> {}, std::vector<std::vector<double>> {});

    std::function<float(float, float)> f1 = [](float, float) ->float {return 0;};
    using func_metric = std::function<float(float, float)>;
    metric::MGC<float, func_metric, float, func_metric> m7(f1, f1);
    m7(std::vector<float>{}, std::vector<float>{});
}
