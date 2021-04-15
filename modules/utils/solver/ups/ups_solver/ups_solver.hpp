#ifndef _UPS_SOLVER_HPP
#define _UPS_SOLVER_HPP

#include "../ups_solver/init.hpp"
#include "../ups_solver/depth_to_normals.hpp"
#include "../ups_solver/normals_to_sh.hpp"
#include "../ups_solver/calc_reweighting.hpp"
#include "../ups_solver/update_blocks.hpp"

#include "../../../../../3rdparty/blaze/Blaze.h"

#include <tuple>
#include <vector>
#include <iostream>




template <typename T>
void  // TODO replace
ups_solver(
        const blaze::DynamicMatrix<T, blaze::columnMajor> & Z_init,
        blaze::CompressedMatrix<unsigned char, blaze::columnMajor> Mask,
        blaze::DynamicMatrix<T> K,
        std::vector<std::vector<blaze::DynamicMatrix<T, blaze::columnMajor>>> I,
        harmo_order sh_order = ho_low,
        size_t maxit = 20,  // main loop max iteration number
        size_t c2f_lighting = 8,  // iter nomber where we switch to high sh_order
        T mu = 2e-6,
        T delta = 0.00045
        // TODO add options
        )
{

    std::vector<std::vector<blaze::DynamicVector<T>>> s;
    if (sh_order == ho_low)
        s = initS<T>(I[0].size(), I.size(), {0, 0, -1, 0.2});
    else
        s = initS<T>(I[0].size(), I.size(), {0, 0, -1, 0.2, 0, 0, 0, 0, 0});

    std::vector<blaze::DynamicMatrix<T, blaze::columnMajor>> rho_init = initRho(I);

    auto i_rho = VariablesInitialization(I, Mask, rho_init);
    std::vector<std::vector<blaze::DynamicVector<T>>> flat_imgs = std::get<0>(i_rho);
    std::vector<blaze::DynamicVector<T>> rho = std::get<1>(i_rho);

    size_t nimages = flat_imgs.size();
    size_t nchannels = flat_imgs[0].size();
    size_t npix = flat_imgs[0][0].size();


    blaze::DynamicVector<T> allpixels (nimages*nchannels*npix);
    size_t pos = 0;
    for (size_t im = 0; im < nimages; ++im) {
        for (size_t ch = 0; ch < nchannels; ++ch) {
            blaze::subvector(allpixels, pos, npix) = flat_imgs[im][ch];
        }
    }
    std::nth_element(allpixels.begin(), allpixels.begin() + allpixels.size()/2, allpixels.end());
    T median = allpixels[allpixels.size()/2 - 1];
    if (allpixels.size() % 2 == 0) {
        median = (median + allpixels[allpixels.size()/2]) / 2;
    }
    T div_lambda = delta*median;
    // TODO update mu depending on image brightness median


    auto nablaOp = getNabla<T>(Mask);
    blaze::CompressedMatrix<T> G (std::get<0>(nablaOp).rows() + std::get<1>(nablaOp).rows(), std::get<0>(nablaOp).columns());
    auto view0 = blaze::submatrix(G, 0, 0, std::get<0>(nablaOp).rows(), std::get<0>(nablaOp).columns());
    auto view1 = blaze::submatrix(G, std::get<0>(nablaOp).rows(), 0, std::get<1>(nablaOp).rows(), std::get<1>(nablaOp).columns());
    view0 = std::get<0>(nablaOp);
    view1 = std::get<1>(nablaOp);  // G written
    //std::cout << std::endl << "G:" << std::endl << G << std::endl << std::endl;

    auto normals = depthToNormals(Z_init, Mask, K);
    blaze::DynamicMatrix<T> N_normalized = std::get<0>(normals);
    blaze::DynamicVector<T> dz = std::get<1>(normals);
    blaze::DynamicMatrix<T> N_unnormalized = std::get<2>(normals);
    blaze::DynamicVector<T> zx = std::get<3>(normals);
    blaze::DynamicVector<T> zy = std::get<4>(normals);
    blaze::DynamicVector<T> xx = std::get<5>(normals);
    blaze::DynamicVector<T> yy = std::get<6>(normals);
    blaze::CompressedMatrix<T> Dx = std::get<7>(normals);
    blaze::CompressedMatrix<T> Dy = std::get<8>(normals);

    blaze::DynamicVector<T> theta = dz;

    blaze::DynamicVector<T> u = blaze::DynamicVector<T>(theta.size(), 0);

    std::vector<blaze::DynamicVector<T>> drho = {};
    for (size_t ch = 0; ch < std::get<0>(i_rho)[0].size(); ++ch) {
        blaze::DynamicVector<T> drho_ch = G * std::get<1>(i_rho)[ch];
        drho.push_back(drho_ch);
    }

    blaze::DynamicMatrix<T> normals_theta (N_unnormalized.rows(), N_unnormalized.columns());
    for (size_t c = 0; c < N_unnormalized.columns(); ++c) {
        blaze::column(normals_theta, c) = blaze::column(N_unnormalized, c) / theta;
    }
    blaze::DynamicMatrix<T> sh = normalsToSh(normals_theta, sh_order);

    // TODO add initial energy computation if needed


    std::vector<std::vector<blaze::DynamicVector<T>>> weights;

    for (size_t it = 0; it < maxit; ++it) {

        if (it == c2f_lighting && sh_order == ho_low) {  // switch lighting scheme sph harm order
            sh_order = ho_high;
            s = initS<T>(I[0].size(), I.size(), {0, 0, -1, 0.2, 0, 0, 0, 0, 0});
            for (size_t c = 0; c < N_unnormalized.columns(); ++c) {
                blaze::column(normals_theta, c) = blaze::column(N_unnormalized, c) / theta;
            }
            sh = normalsToSh(normals_theta, sh_order);
        }

        weights = calcReweighting(rho, sh, s, flat_imgs);

        // TODO update blocks

    }  // for (size_t it = 0; it < maxit; ++it) // end main loop

}



#endif // _UPS_SOLVER_HPP
