#ifndef _UPS_SOLVER_HPP
#define _UPS_SOLVER_HPP

#include "modules/utils/solver/ups/ups_solver/init.hpp"
#include "modules/utils/solver/ups/ups_solver/depth_to_normals.hpp"
#include "modules/utils/solver/ups/ups_solver/normals_to_sh.hpp"
#include "modules/utils/solver/ups/ups_solver/calc_reweighting.hpp"
#include "modules/utils/solver/ups/ups_solver/update_blocks.hpp"
#include "modules/utils/solver/ups/ups_solver/vec_to_image.hpp"
#include "modules/utils/solver/ups/helpers/console_output.hpp"


#include "3rdparty/blaze/Blaze.h"

#include <tuple>
#include <vector>
#include <iostream>




template <typename T>
std::tuple<blaze::DynamicMatrix<T>, std::vector<blaze::DynamicMatrix<T>>, std::vector<std::vector<blaze::DynamicVector<T>>>>
ups_solver(
        const blaze::DynamicMatrix<T, blaze::columnMajor> & Z_init,
        //blaze::CompressedMatrix<bool, blaze::columnMajor> Mask,
        blaze::DynamicMatrix<bool, blaze::columnMajor> Mask,
        blaze::DynamicMatrix<T> K,
        std::vector<std::vector<blaze::DynamicMatrix<T, blaze::columnMajor>>> I,
        harmo_order sh_order = ho_low,
        size_t maxit = 6, //20,  // main loop max iteration number
        size_t c2f_lighting = 8,  // iter number where we switch to high sh_order
        T mu = 2e-6,
        T delta = 0.00045,
        T huber = 0.1,
        bool regular = true,
        T tol = 1e-5, // 1e-10,
        size_t pcg_maxit = 50, //1000, //1e3,
        T beta = 5e-4, // TODO trace, make structure!!!
        T kappa = 1.5,
        T lambda = 1
        //T delta = 4.5e-4 // parameter for computing  the weight for cauchy estimator.
        )
{

    bool console_debug_output = true;//false; //true;  // TODO remove

    size_t img_h = Z_init.rows();
    size_t img_w = Z_init.columns();

    std::vector<std::vector<blaze::DynamicVector<T>>> s;
    if (sh_order == ho_low)
        s = initS<T>(I[0].size(), I.size(), {0, 0, -1, 0.2});
    else
        s = initS<T>(I[0].size(), I.size(), {0, 0, -1, 0.2, 0, 0, 0, 0, 0});

    std::vector<blaze::DynamicMatrix<T, blaze::columnMajor>> rho_init = initRho(I);

    auto i_rho = VariablesInitialization(I, Mask, rho_init);
    std::vector<std::vector<blaze::DynamicVector<T>>> flat_imgs = std::get<0>(i_rho);
    std::vector<blaze::DynamicVector<T>> rho = std::get<1>(i_rho);
    if (console_debug_output) {
        std::cout << "rho_init: " << std::endl << rho << std::endl << std::endl;

    }

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
    //std::cout << "max pixel value: " << blaze::max(allpixels) << std::endl;
    //std::cout << "1st pixel value: " << allpixels[0] << std::endl;
    std::nth_element(allpixels.begin(), allpixels.begin() + allpixels.size()/2, allpixels.end());
    //std::cout << "1st pixel value after partial sort: " << allpixels[0] << std::endl;
    T pix_median = allpixels[allpixels.size()/2 - 1];
    if (allpixels.size() % 2 == 0) {
        pix_median = (pix_median + allpixels[allpixels.size()/2]) / 2;
    }
    allpixels = blaze::abs(allpixels - pix_median);
    std::nth_element(allpixels.begin(), allpixels.begin() + allpixels.size()/2, allpixels.end());
    pix_median = allpixels[allpixels.size()/2 - 1];
    if (allpixels.size() % 2 == 0) {
        pix_median = (pix_median + allpixels[allpixels.size()/2]) / 2;
    }
    T div_lambda = delta*pix_median / (nchannels*nimages);
    mu = mu / (div_lambda*nchannels);


    auto nablaOp = getNabla<T>(Mask);
    if (console_debug_output) {
        std::cout << std::endl << "mask, nabla operator matrices: "  // TODO remove
                  << std::endl << Mask << std::endl << std::endl
                  << std::get<0>(nablaOp) << std::endl
                  << std::get<1>(nablaOp) << std::endl;
    }

    blaze::CompressedMatrix<T> G (std::get<0>(nablaOp).rows() + std::get<1>(nablaOp).rows(), std::get<0>(nablaOp).columns());
    //blaze::DynamicMatrix<T> G (std::get<0>(nablaOp).rows() + std::get<1>(nablaOp).rows(), std::get<0>(nablaOp).columns(), 0);
    auto view0 = blaze::submatrix(G, 0, 0, std::get<0>(nablaOp).rows(), std::get<0>(nablaOp).columns());
    auto view1 = blaze::submatrix(G, std::get<0>(nablaOp).rows(), 0, std::get<1>(nablaOp).rows(), std::get<1>(nablaOp).columns());
    view0 = std::get<0>(nablaOp);
    view1 = std::get<1>(nablaOp);  // G written
    if (console_debug_output) {
        std::cout << std::endl << "G:" << std::endl << G << std::endl << std::endl;
    }

    //auto normals = depthToNormals(Z_init, Mask, K);
    auto gradients = getMaskedGradients(Z_init, Mask, K);
    blaze::DynamicVector<T> z_vector_masked = std::get<0>(gradients);  // Matlab z = z(mask);
    blaze::DynamicVector<T> zx = std::get<1>(gradients);
    blaze::DynamicVector<T> zy = std::get<2>(gradients);
    blaze::DynamicVector<T> xx = std::get<3>(gradients);
    blaze::DynamicVector<T> yy = std::get<4>(gradients);
    blaze::CompressedMatrix<T> Dx = std::get<5>(gradients);
    blaze::CompressedMatrix<T> Dy = std::get<6>(gradients);
    //blaze::DynamicMatrix<T> Dx = std::get<5>(gradients);
    //blaze::DynamicMatrix<T> Dy = std::get<6>(gradients);


    auto normals_map = getNormalMap(z_vector_masked, zx, zy, K, xx, yy);
    blaze::DynamicMatrix<T> N_normalized = std::get<0>(normals_map);
    blaze::DynamicVector<T> dz = std::get<1>(normals_map);
    blaze::DynamicMatrix<T> N_unnormalized = std::get<2>(normals_map);

    blaze::DynamicVector<T> theta = dz;

    blaze::DynamicVector<T> u = blaze::DynamicVector<T>(theta.size(), 0);

    std::vector<blaze::DynamicVector<T>> drho = {};
    //for (size_t ch = 0; ch < std::get<0>(i_rho)[0].size(); ++ch) {
    //    blaze::DynamicVector<T> drho_ch = G * std::get<1>(i_rho)[ch];
    //    drho.push_back(drho_ch);
    //}

    blaze::DynamicMatrix<T> normals_theta (N_unnormalized.rows(), N_unnormalized.columns());
    for (size_t c = 0; c < N_unnormalized.columns(); ++c) {
        blaze::column(normals_theta, c) = blaze::column(N_unnormalized, c) / theta;
    }
    blaze::DynamicMatrix<T> sh;
    sh = normalsToSh(normals_theta, sh_order);

    // TODO add initial energy computation if needed


    std::vector<std::vector<blaze::DynamicVector<T>>> weights;

    for (size_t it = 0; it < maxit; ++it) {

        if (it == c2f_lighting && sh_order == ho_low) {  // switch lighting scheme sph harmо order
            sh_order = ho_high;
            s = initS<T>(I[0].size(), I.size(), {0, 0, -1, 0.2, 0, 0, 0, 0, 0});
            for (size_t c = 0; c < N_unnormalized.columns(); ++c) {
                blaze::column(normals_theta, c) = blaze::column(N_unnormalized, c) / theta;
            }
            sh = normalsToSh(normals_theta, sh_order);
        }

        // albedo update
        if (console_debug_output) {  // TODO remove
            std::cout << std::endl << "flat_imgs:" << std::endl << flat_imgs << std::endl;
            std::cout << std::endl << "rho:" << std::endl << rho << std::endl;
            std::cout << std::endl << "sh:" << std::endl << sh << std::endl;
            std::cout << std::endl << "s:" << std::endl << s << std::endl;
            std::cout << std::endl << "weights:" << std::endl << weights << std::endl;
            std::cout << std::endl << "G:" << std::endl << G << std::endl;
            std::cout << std::endl << "huber:" << std::endl << huber << std::endl;
            std::cout << std::endl << "mu:" << std::endl << mu << std::endl;
        }
        weights = calcReweighting(rho, sh, s, flat_imgs, lambda);
        rho = updateAlbedo(flat_imgs, rho, sh, s, weights, G, huber, mu, regular, tol, pcg_maxit);
        drho = {};
        for (size_t ch = 0; ch < rho.size(); ++ch) {
            blaze::DynamicVector<T> drho_ch = G * rho[ch];
            drho.push_back(drho_ch);
        }
        //drho = drho_upd;
        if (console_debug_output) {
            std::cout << "rho: " << std::endl << rho << std::endl << std::endl;
        }

        // lighting update
        weights = calcReweighting(rho, sh, s, flat_imgs, lambda);
        auto s_upd = updateLighting(flat_imgs, rho, sh, s, weights, tol, pcg_maxit);
        s = std::get<0>(s_upd);
        auto res_s = std::get<1>(s_upd);

        // depth update
        weights = calcReweighting(rho, sh, s, flat_imgs, lambda);
        auto depth_upd = updateDepth<T>(flat_imgs, rho, s, theta, z_vector_masked, zx, zy, u, weights, drho, K, xx, yy, Dx, Dy,
                                     3, beta, 10, 1000, 1e-10, 1000, sh_order);  // TODO pass
        z_vector_masked = std::get<0>(depth_upd);
        zx = std::get<1>(depth_upd);
        zy = std::get<2>(depth_upd);
        dz = std::get<3>(depth_upd);
        N_unnormalized = std::get<4>(depth_upd);
        sh = std::get<5>(depth_upd);
        //blaze::CompressedMatrix<T> J_dz = std::get<6>(depth_upd);
        //T res_z = std::get<7>(depth_upd);

        // aux update
        theta = dz;
        for (size_t c = 0; c < N_unnormalized.columns(); ++c) {
            blaze::column(normals_theta, c) = blaze::column(N_unnormalized, c) / theta;
        }
        sh = normalsToSh(normals_theta, sh_order);

        // dual update
        if (beta > 10) {  // TODO pass beta_thres = 10
            u = u + (theta - dz);
        }

        // increment of stepsize
        beta *= kappa;
        if (beta > 10) {  // TODO pass beta_thres = 10
            u = u / kappa; // TODO trace MAtlab code and update beta, kappa
        }

        std::cout << "iter " << it << " completed" << std::endl;

    }  // for (size_t it = 0; it < maxit; ++it) // end main loop

    // output
    auto z_out = vecToImage<T>(z_vector_masked, Mask, img_h, img_w);
    std::vector<blaze::DynamicMatrix<T>> rho_out = {};
    for (size_t ch = 0; ch < rho.size(); ++ch) {
        rho_out.push_back(vecToImage<T>(rho[ch], Mask, img_h, img_w));
    }

    return std::make_tuple(z_out, rho_out, s);
}



#endif // _UPS_SOLVER_HPP
