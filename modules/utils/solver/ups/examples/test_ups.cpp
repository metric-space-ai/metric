
#include "modules/utils/solver/ups/ups_solver/ups_solver.hpp"
//#include "../helpers/console_output.hpp"

#include "3rdparty/blaze/Blaze.h"

#include <tuple>
#include <vector>
#include <iostream>


int main()
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


    return 0;

}
