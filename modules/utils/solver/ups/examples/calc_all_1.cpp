
#include "../ups_solver/init.hpp"
#include "../ups_solver/depth_to_normals.hpp"
#include "../ups_solver/normals_to_sh.hpp"
#include "../ups_solver/calc_reweighting.hpp"
#include "../ups_solver/update_blocks.hpp"

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

    auto rho_i = initRho(I);
    std::cout << "rho[0]:\n" << rho_i[0] << "\n\n";
    std::cout << "rho[1]:\n" << rho_i[1] << "\n\n";
    std::cout << "rho[2]:\n" << rho_i[2] << "\n\n";


    auto s = initS<float>(I[0].size(), I.size());
    //s[0][0][0] = 10; // TODO remove

    auto i_rho = VariablesInitialization(I, M, rho_i);

    //auto normals = depthToNormals(Z, M, K);  // also works

    auto gradients = getMaskedGradients(Z, M, K);
    blaze::DynamicVector<float> z_vector_masked = std::get<0>(gradients);  // Matlab z = z(mask);
    blaze::DynamicVector<float> zx = std::get<1>(gradients);
    blaze::DynamicVector<float> zy = std::get<2>(gradients);
    blaze::DynamicVector<float> xx = std::get<3>(gradients);
    blaze::DynamicVector<float> yy = std::get<4>(gradients);
    blaze::CompressedMatrix<float> Dx = std::get<5>(gradients);
    blaze::CompressedMatrix<float> Dy = std::get<6>(gradients);

    auto normals = getNormalMap(z_vector_masked, zx, zy, K, xx, yy);

    blaze::DynamicMatrix<float> sh = normalsToSh(std::get<2>(normals));
    std::cout << "sh:\n" << sh << "\n\n";


    // calcJacobian test
    auto Jac = calcJacobian<float>(std::get<0>(normals), z_vector_masked, K, xx, yy, Dx, Dy);
    std::cout << std::endl << "J_dz:" << std::endl << std::get<1>(Jac) << std::endl << std::endl;



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


    auto nablaOp = getNabla<float>(M);

    //std::cout << (blaze::DynamicMatrix<unsigned short, blaze::columnMajor>)M << "\n\n";
    std::cout << std::get<0>(nablaOp) << "\n\n";
    std::cout << std::get<1>(nablaOp) << "\n";

    blaze::CompressedMatrix<float> G (std::get<0>(nablaOp).rows() + std::get<1>(nablaOp).rows(), std::get<0>(nablaOp).columns());
    auto view0 = blaze::submatrix(G, 0, 0, std::get<0>(nablaOp).rows(), std::get<0>(nablaOp).columns());
    auto view1 = blaze::submatrix(G, std::get<0>(nablaOp).rows(), 0, std::get<1>(nablaOp).rows(), std::get<1>(nablaOp).columns());
    view0 = std::get<0>(nablaOp);
    view1 = std::get<1>(nablaOp);

    std::cout << std::endl << "G:" << std::endl << G << std::endl << std::endl;



    float mu = 2e-6;

    bool regular = 1;
    float huber = 0.1;
    float pcg_maxit = 100;
    float pcg_tol = 1e-5;

    auto rho_upd = updateAlbedo(std::get<0>(i_rho), std::get<1>(i_rho), sh, s, weights, G, mu, huber, regular, pcg_tol, pcg_maxit);

    std::cout << std::endl << "rho updated:" << std::endl << rho_upd << std::endl << std::endl;




    //float nb_nonsingular = 1e-10;
    //auto s_upd = updateLighting(std::get<0>(i_rho), std::get<1>(i_rho), sh, s, weights, nb_nonsingular);
    auto s_upd = updateLighting(std::get<0>(i_rho), std::get<1>(i_rho), sh, s, weights);

    std::cout << std::endl << "residual: " << std::get<1>(s_upd) << std::endl << std::endl;



    float beta = 5e-4;

    auto theta = std::get<1>(normals);  // dz
    blaze::DynamicVector<float> u = blaze::DynamicVector<float>(theta.size(), 0);

    std::vector<blaze::DynamicVector<float>> drho = {};
    for (size_t ch = 0; ch < std::get<0>(i_rho)[0].size(); ++ch) {
        blaze::DynamicVector<float> drho_ch = G * std::get<1>(i_rho)[ch];
        drho.push_back(drho_ch);
    }

    auto z_upd = updateDepth<float>(std::get<0>(i_rho), std::get<1>(i_rho), s, theta, z_vector_masked, zx, zy, u, weights, drho, K, xx, yy, Dx, Dy, 3, beta);

    // z, zx, zy, dz, N_unnormalized, sh, J_dz, res_z
    std::cout << std::endl << std::endl << "updateDepth results:" << std::endl << std::endl;
    std::cout << std::endl << "z:" << std::endl << std::get<0>(z_upd) << std::endl << std::endl;
    std::cout << std::endl << "zx:" << std::endl << std::get<1>(z_upd) << std::endl << std::endl;
    std::cout << std::endl << "zy:" << std::endl << std::get<2>(z_upd) << std::endl << std::endl;
    std::cout << std::endl << "dz:" << std::endl << std::get<3>(z_upd) << std::endl << std::endl;
    std::cout << std::endl << "N_unnormalized:" << std::endl << std::get<4>(z_upd) << std::endl << std::endl;
    std::cout << std::endl << "sh:" << std::endl << std::get<5>(z_upd) << std::endl << std::endl;
    std::cout << std::endl << "J_dz:" << std::endl << std::get<6>(z_upd) << std::endl << std::endl;
    std::cout << std::endl << "res_z:" << std::endl << std::get<7>(z_upd) << std::endl << std::endl;



    return 0;
}
