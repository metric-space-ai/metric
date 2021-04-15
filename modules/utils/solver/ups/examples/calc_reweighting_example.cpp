
#include "../ups_solver/init.hpp"
#include "../ups_solver/depth_to_normals.hpp"
#include "../ups_solver/normals_to_sh.hpp"
#include "../ups_solver/calc_reweighting.hpp"

#include "../../../../../3rdparty/blaze/Blaze.h"

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

    auto rho = initRho(I);
    std::cout << "rho[0]:\n" << rho[0] << "\n\n";
    std::cout << "rho[1]:\n" << rho[1] << "\n\n";
    std::cout << "rho[2]:\n" << rho[2] << "\n\n";


    auto s = initS<float>(I[0].size(), I.size());
    //s[0][0][0] = 10; // TODO remove

    auto i_rho = VariablesInitialization(I, M, rho);

    auto normals = depthToNormals(Z, M, K);
    blaze::DynamicMatrix<float> sh = normalsToSh(std::get<2>(normals));
    std::cout << "sh:\n" << sh << "\n\n";

    auto weights = calcReweighting(std::get<1>(i_rho), sh, s, std::get<0>(i_rho));

    std::cout << "weights:\n";
    for (size_t i = 0; i<weights.size(); ++i) {
        std::cout << "----\n";
        for (size_t j = 0; j<weights[0].size(); ++j) {
            std::cout << weights[i][j] << "\n";
            std::cout << std::get<0>(i_rho)[i][j] << "\n";
            std::cout << s[i][j] << "\n";
            std::cout << std::get<1>(i_rho)[j] << "\n";
        }
    }



    return 0;
}
