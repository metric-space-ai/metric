#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include "modules/utils/solver/ups/ups_solver/ups_solver.hpp"

//#include <limits>

//#include <tuple>
#include <vector>
#include <iostream>



TEST_CASE("ups: overall test", "[utils]")
{

    blaze::CompressedMatrix<unsigned char, blaze::columnMajor> M {
        {0, 0, 1, 1, 0},
        {0, 1, 1, 1, 1},
        {0, 1, 1, 0, 0},
        {0, 0, 1, 0, 0}
    };

    blaze::DynamicMatrix<float, blaze::columnMajor> Z {
        {0.8, 0.8, 0.8, 0.8, 0.8},
        {0.8, 0.8, 0.8, 0.9, 0.8},
        {0.8, 1.0, 0.8, 0.8, 0.8},
        {0.8, 0.8, 0.8, 0.8, 0.8}};

    blaze::DynamicMatrix<float> K {
        {1, 0, 1},
        {0, 1, 1},
        {0, 0, 1}};


    // images

    blaze::DynamicMatrix<float, blaze::columnMajor> I1R {
        {0,  5,  10, 5,  0},
        {5,  10, 15, 10, 5},
        {10, 15, 20, 15, 10},
        {5,  10, 15, 10, 0}
    };

    blaze::DynamicMatrix<float, blaze::columnMajor> I2R {
        {0,  5,  10, 5,  0},
        {5,  10, 30, 20, 5},
        {10, 15, 20, 15, 10},
        {5,  10, 15, 10, 0}
    };

    std::vector<blaze::DynamicMatrix<float, blaze::columnMajor>> I1 {I1R, I1R, I1R};
    std::vector<blaze::DynamicMatrix<float, blaze::columnMajor>> I2 {I2R, I2R, I2R};
    std::vector<std::vector<blaze::DynamicMatrix<float, blaze::columnMajor>>> I {I1, I2};


    auto r = ups_solver(Z, M, K, I);

    std::cout << std::endl << "final result:" << std::endl
              << "z: " << std::endl << std::get<0>(r) << std::endl
              << "rho1: " << std::endl << std::get<1>(r)[0] << std::endl
              << "rho2: " << std::endl << std::get<1>(r)[1] << std::endl
              << "rho3: " << std::endl << std::get<1>(r)[2] << std::endl;

//    blaze::DynamicMatrix<float, blaze::columnMajor> z_ref = {  // c++ result on some stage of debugging
//        {            0,            0,       1.2021,      1.05274,            0 },
//        {            0,      1.04852,      1.53897,     0.717244,     0.732883 },
//        {            0,     0.995618,     0.793853,            0,            0 },
//        {            0,            0,     0.821557,            0,            0 }
//    };

    blaze::DynamicMatrix<float, blaze::columnMajor> z_ref = {  // Octave reference result
        {            0,            0,      1.20301,      1.05178,            0 },
        {            0,      1.04842,      1.53990,      0.71446,      0.73173 },
        {            0,      0.99260,      0.79102,            0,            0 },
        {            0,            0,      0.82003,            0,            0 }
    };

    blaze::DynamicMatrix<float> albedo_ref = {  // Octave reference result
        {0.00000,     0.00000,    17.97467,    53.25992,     0.00000},
        {0.00000,    25.61316,    40.16718,    31.92726,    36.94323},
        {0.00000,   261.87056,    74.76535,     0.00000,     0.00000},
        {0.00000,     0.00000,   110.63529,     0.00000,     0.00000}
    };

    auto z_diff = blaze::abs(std::get<0>(r) - z_ref);
    auto albedo1_diff = blaze::abs(std::get<1>(r)[0] - albedo_ref);
    auto albedo_channels_diff = blaze::abs(std::get<1>(r)[1] - std::get<1>(r)[0]) +
                                blaze::abs(std::get<1>(r)[2] - std::get<1>(r)[0]);

    std::cout << std::endl << "differences:" << std::endl
              << "z: " << std::endl << z_diff << std::endl
              << "albedo: " << std::endl << albedo1_diff << std::endl;

    REQUIRE(blaze::max(z_diff) < 0.1);
    REQUIRE(blaze::max(albedo1_diff) < 2);
    REQUIRE(blaze::max(albedo_channels_diff) < 0.000001);

}


