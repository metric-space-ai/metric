#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include "modules/transform/wavelet.hpp"

#include <vector>


using DM = blaze::DynamicMatrix<double>;
using SM = blaze::CompressedMatrix<double>;

using VBM = std::vector<blaze::DynamicVector<double>>;
using VVM = std::vector<std::vector<double>>;

using BV = blaze::DynamicVector<double>;
using SV = std::vector<double>;


TEMPLATE_TEST_CASE("dwt2blaze", "[transform]", DM, SM) {

    TestType data2d = {
        { 0, 0, 0, 0, 0, 0, 1, 2, 3, 4 },
        { 0, 0, 0, 0, 0, 0, 8, 7, 6, 5 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 5, 1, 5, 0, 0, 0, 0, 0 },
        { 0, 0, 1, 5, 1, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, -2, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 1, 1, 1, 0, 0, 0, 0, 0, 0, 0 },
        { 1, 1, 1, 0, 0, 0, 0, 0, 0, 0 },
        { 1, 1, -1, 0, 0, 0, 0, 0, 0, 0 },
    };
    std::tuple<TestType, TestType, TestType, TestType> splitted;
    TestType restored;
    REQUIRE_NOTHROW(splitted = wavelet::dwt2(data2d, 4));
    REQUIRE_NOTHROW(restored = wavelet::idwt2(std::get<0>(splitted),
											  std::get<1>(splitted),
											  std::get<2>(splitted),
											  std::get<3>(splitted),
											  4, data2d.rows(), data2d.columns()));
    auto maxdiff = double(blaze::max(blaze::abs(restored - data2d)));
    REQUIRE(maxdiff <= std::numeric_limits<double>::epsilon()*1e6);
}



TEMPLATE_TEST_CASE("dwt2old", "[transform]", VBM, VVM)
{
    TestType data2d = {
        { 0, 0, 0, 0, 0, 0, 1, 2, 3, 4 },
        { 0, 0, 0, 0, 0, 0, 8, 7, 6, 5 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 5, 1, 5, 0, 0, 0, 0, 0 },
        { 0, 0, 1, 5, 1, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, -2, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 1, 1, 1, 0, 0, 0, 0, 0, 0, 0 },
        { 1, 1, 1, 0, 0, 0, 0, 0, 0, 0 },
        { 1, 1, -1, 0, 0, 0, 0, 0, 0, 0 },
    };
    std::tuple<TestType, TestType, TestType, TestType> splitted;
    TestType restored;
    REQUIRE_NOTHROW(splitted = wavelet::dwt2(data2d, 4));
    REQUIRE_NOTHROW(restored = wavelet::idwt2(std::get<0>(splitted), std::get<1>(splitted),
                                                std::get<2>(splitted), std::get<3>(splitted),
                                                        4, data2d.size(), data2d[0].size()));

    double maxdiff = 0;
    for (size_t i = 0; i < restored.size(); ++i) {
        for (size_t j = 0; j < restored[0].size(); ++j) {
            double diff = std::abs(restored[i][j] - data2d[i][j]);
            if (diff > maxdiff)
                maxdiff = diff;
        }
    }
    REQUIRE( maxdiff <= std::numeric_limits<double>::epsilon()*1e6 );
}



TEMPLATE_TEST_CASE("dwt_d", "[transform]", BV, SV)
{
    TestType data = {1, 2, 3, 4, 1, 0, 20, 0, 0, 5, -10, 0, -4, -2, 0, 0, 0, 0, 0, 1, 2, 3, 4};
    std::tuple<TestType, TestType> splitted;
    TestType restored;
    int wavelet = 4;
    REQUIRE_NOTHROW(splitted = wavelet::dwt(data, wavelet));
    REQUIRE_NOTHROW(restored = wavelet::idwt(std::get<0>(splitted),
                                            std::get<1>(splitted),
                                                    wavelet, data.size()));

    double maxdiff = 0;
    for (size_t i = 0; i < restored.size(); ++i) {
        double diff = std::abs(restored[i] - data[i]);
        if (diff > maxdiff)
            maxdiff = diff;
    }
    REQUIRE( maxdiff <= std::numeric_limits<double>::epsilon()*1e6 );
}