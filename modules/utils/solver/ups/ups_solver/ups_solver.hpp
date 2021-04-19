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
        T delta = 0.00045,
        bool regular = 1,
        T huber = 0.1,
        T pcg_maxit = 100,
        T pcg_tol = 1e-5,
        T lambda = 1,
        size_t lins_maxit = 3, // depth update max iterations
        T beta = 5e-4
        // TODO add options
        )
{

    std::vector<std::vector<blaze::DynamicVector<T>>> s;
    if (sh_order == ho_low)
        s = initS<T>(I[0].size(), I.size(), {0, 0, -1, 0.2});
    else
        s = initS<T>(I[0].size(), I.size(), {0, 0, -1, 0.2, 0, 0, 0, 0, 0});
    std::cout << std::endl << "s:" << std::endl << s << std::endl;  // TODO remove

    std::vector<blaze::DynamicMatrix<T, blaze::columnMajor>> rho_init = initRho(I);
    std::cout << std::endl << "rho_init:" << std::endl << rho_init << std::endl;  // TODO remove

    auto i_rho = VariablesInitialization(I, Mask, rho_init);
    std::vector<std::vector<blaze::DynamicVector<T>>> flat_imgs = std::get<0>(i_rho);
    std::vector<blaze::DynamicVector<T>> rho = std::get<1>(i_rho);
    std::cout << std::endl << "rho:" << std::endl << rho << std::endl;  // TODO remove

    size_t nimages = flat_imgs.size();
    size_t nchannels = flat_imgs[0].size();
    size_t npix = flat_imgs[0][0].size();


    blaze::DynamicVector<T> allpixels (nimages*nchannels*npix);
    size_t pos = 0;
    for (size_t im = 0; im < nimages; ++im) {
        for (size_t ch = 0; ch < nchannels; ++ch) {
            blaze::subvector(allpixels, pos, npix) = flat_imgs[im][ch];
            pos += npix;
        }
    }
    std::cout << std::endl << "allpixels:" << std::endl << allpixels << std::endl << std::endl;  // TODO remove
    std::nth_element(allpixels.begin(), allpixels.begin() + allpixels.size()/2, allpixels.end());
    T pix_median = allpixels[allpixels.size()/2 - 1];
    if (allpixels.size() % 2 == 0) {
        pix_median = (pix_median + allpixels[allpixels.size()/2]) / 2;
    }
    allpixels = blaze::abs(allpixels - pix_median);
    pix_median = allpixels[allpixels.size()/2 - 1];
        if (allpixels.size() % 2 == 0) {
            pix_median = (pix_median + allpixels[allpixels.size()/2]) / 2;
        }
    T div_lambda = delta*pix_median / (nchannels*nimages);
    mu = mu / (div_lambda*nchannels);
    //std::cout << std::endl << "mu:" << std::endl << mu << std::endl;  // TODO remove


    auto nablaOp = getNabla<T>(Mask);
    blaze::CompressedMatrix<T> G (std::get<0>(nablaOp).rows() + std::get<1>(nablaOp).rows(), std::get<0>(nablaOp).columns());
    auto view0 = blaze::submatrix(G, 0, 0, std::get<0>(nablaOp).rows(), std::get<0>(nablaOp).columns());
    auto view1 = blaze::submatrix(G, std::get<0>(nablaOp).rows(), 0, std::get<1>(nablaOp).rows(), std::get<1>(nablaOp).columns());
    view0 = std::get<0>(nablaOp);
    view1 = std::get<1>(nablaOp);  // G written
    std::cout << std::endl << "G:" << std::endl << G << std::endl << std::endl;  // TODO remove


    /*  // TODO remove
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
    // */


    //*  // TODO debug and enable
    auto gradients = getMaskedGradients(Z_init, Mask, K);
    blaze::DynamicVector<T> z = std::get<0>(gradients);  // Matlab z = z(mask);
    blaze::DynamicVector<T> zx = std::get<1>(gradients);
    blaze::DynamicVector<T> zy = std::get<2>(gradients);
    blaze::DynamicVector<T> xx = std::get<3>(gradients);
    blaze::DynamicVector<T> yy = std::get<4>(gradients);
    blaze::CompressedMatrix<T> Dx = std::get<5>(gradients);
    blaze::CompressedMatrix<T> Dy = std::get<6>(gradients);

    auto normals = getNormalMap(z, zx, zy, K, xx, yy);
    blaze::DynamicMatrix<T> N_normalized = std::get<0>(normals);  // TODO use in sh initialization!
    blaze::DynamicVector<T> dz = std::get<1>(normals);
    blaze::DynamicMatrix<T> N_unnormalized = std::get<2>(normals);
    // */

    blaze::DynamicVector<T> theta = dz;

    blaze::DynamicVector<T> u = blaze::DynamicVector<T>(theta.size(), 0);

    std::vector<blaze::DynamicVector<T>> drho = {};
    for (size_t ch = 0; ch < flat_imgs[0].size(); ++ch) {
        blaze::DynamicVector<T> drho_ch = G * rho[ch];
        drho.push_back(drho_ch);
    }

    blaze::DynamicMatrix<T> normals_theta (N_unnormalized.rows(), N_unnormalized.columns());
    for (size_t c = 0; c < N_unnormalized.columns(); ++c) {
        blaze::column(normals_theta, c) = blaze::column(N_unnormalized, c) / theta; // TODO check if we can use normalized here
    }
    blaze::DynamicMatrix<T> sh = normalsToSh(normals_theta, sh_order);
    std::cout << std::endl << "sh:" << std::endl << sh << std::endl;  // TODO remove

    // TODO add initial energy computation if needed



    // main loop

    std::vector<std::vector<blaze::DynamicVector<T>>> weights;

    for (size_t it = 0; it < maxit; ++it) {

        std::cout << std::endl << std::endl << "---- iter " << it << std::endl << std::endl;  // TODO remove

        if (it == c2f_lighting && sh_order == ho_low) {  // switch lighting scheme sph harm order
            sh_order = ho_high;
            s = initS<T>(I[0].size(), I.size(), {0, 0, -1, 0.2, 0, 0, 0, 0, 0});
            for (size_t c = 0; c < N_unnormalized.columns(); ++c) {
                blaze::column(normals_theta, c) = blaze::column(N_unnormalized, c) / theta;
            }
            sh = normalsToSh(normals_theta, sh_order);
        }
        std::cout << std::endl << "sh:" << std::endl << sh << std::endl;  // TODO remove

        //*  // TODO debug rho update
        // albedo
        weights = calcReweighting(rho, sh, s, flat_imgs, lambda);
        rho = updateAlbedo(flat_imgs, rho, sh, s, weights, G, mu, huber, regular, pcg_tol, pcg_maxit);
        std::vector<blaze::DynamicVector<T>> drho = {};
        for (size_t ch = 0; ch < flat_imgs[0].size(); ++ch) {
            blaze::DynamicVector<T> drho_ch = G * rho[ch];
            drho.push_back(drho_ch);
        }
        std::cout << std::endl << "rho:" << std::endl << rho << std::endl;  // TODO remove
        // */

        // lighting
        weights = calcReweighting(rho, sh, s, flat_imgs, lambda);
        auto s_upd = updateLighting(flat_imgs, rho, sh, s, weights);
        s = std::get<0>(s_upd);
        std::cout << std::endl << "s:" << std::endl << s << std::endl;  // TODO remove

        //*  // TODO enable
        //depth
        weights = calcReweighting(rho, sh, s, flat_imgs, lambda);
        std::cout << std::endl << "before updating z:" << std::endl;
        std::cout << std::endl << "flat_imgs:" << std::endl << flat_imgs << std::endl;  // TODO remove
        std::cout << std::endl << "rho:" << std::endl << rho << std::endl;  // TODO remove
        std::cout << std::endl << "s:" << std::endl << s << std::endl;  // TODO remove
        std::cout << std::endl << "theta:" << std::endl << theta << std::endl;  // TODO remove
        std::cout << std::endl << "z:" << std::endl << z << std::endl;  // TODO remove
        std::cout << std::endl << "zx:" << std::endl << zx << std::endl;  // TODO remove
        std::cout << std::endl << "zy:" << std::endl << zy << std::endl;  // TODO remove
        std::cout << std::endl << "u:" << std::endl << u << std::endl;  // TODO remove
        std::cout << std::endl << "weights:" << std::endl << weights << std::endl;  // TODO remove
        std::cout << std::endl << "drho:" << std::endl << drho << std::endl;  // TODO remove
        std::cout << std::endl << "xx:" << std::endl << xx << std::endl;  // TODO remove
        std::cout << std::endl << "yy:" << std::endl << yy << std::endl;  // TODO remove
        std::cout << std::endl << "Dx:" << std::endl << Dx << std::endl;  // TODO remove
        std::cout << std::endl << "Dy:" << std::endl << Dy << std::endl;  // TODO remove
        auto z_upd = updateDepth<T>(flat_imgs, rho, s, theta, z, zx, zy, u, weights, drho, K, xx, yy, Dx, Dy, lins_maxit, beta);
        // z, zx, zy, dz, N_unnormalized, sh, J_dz, res_z
        z = std::get<0>(z_upd);  // TOSO consider in-place update of z, zx, zy
        zx = std::get<1>(z_upd);
        zy = std::get<2>(z_upd);
        dz = std::get<3>(z_upd);
        N_unnormalized = std::get<4>(z_upd);
        sh = std::get<5>(z_upd);
        blaze::CompressedMatrix<T> J_dz = std::get<6>(z_upd);
        T res_z = std::get<7>(z_upd);
        std::cout << std::endl << "after updating z:" << std::endl;
        std::cout << std::endl << "z:" << std::endl << z << std::endl;  // TODO remove
        // */



        // TODO continue here

    }  // for (size_t it = 0; it < maxit; ++it) // end main loop

}



#endif // _UPS_SOLVER_HPP
