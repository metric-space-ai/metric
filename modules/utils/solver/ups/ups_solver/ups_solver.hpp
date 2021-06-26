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
        const blaze::DynamicMatrix<bool, blaze::columnMajor> & Mask,
        const blaze::DynamicMatrix<T> & K,
        const std::vector<std::vector<blaze::DynamicMatrix<T, blaze::columnMajor>>> & I,
        const harmo_order sh_order_ = ho_low,
        const size_t maxit = 20,  // main loop max iteration number
        const size_t c2f_lighting = 8,  // iter number where we switch to high sh_order
        const T mu_ = 2e-6,
        const T delta = 0.00045,
        const T huber = 0.1,
        const bool regular = true,
        const T tol = 1e-5, // 1e-10,
        const size_t pcg_maxit = 50, //1000, //1e3,
        const T beta_ = 5e-4, // TODO trace, make structure!!!
        const T kappa_ = 1.5,
        const T lambda = 1
        //T delta = 4.5e-4 // parameter for computing  the weight for cauchy estimator.
        )
{

    bool console_debug_output = false; //true;  // TODO remove

    size_t img_h = Z_init.rows();
    size_t img_w = Z_init.columns();

    harmo_order sh_order = sh_order_;
    T mu = mu_;
    T beta = beta_;
    T kappa = kappa_;

    std::vector<std::vector<blaze::DynamicVector<T>>> s;
    if (sh_order == ho_low)
        s = initS<T>(I[0].size(), I.size(), {0, 0, -1, 0.2});
    else
        s = initS<T>(I[0].size(), I.size(), {0, 0, -1, 0.2, 0, 0, 0, 0, 0});

    //std::vector<blaze::DynamicMatrix<T, blaze::columnMajor>> rho_init = initRho(I);
    // ----
    // rho as median of I over images for each channel
    std::vector<blaze::DynamicMatrix<T, blaze::columnMajor>> rho_init = {};
    for (size_t ch_idx=0; ch_idx<I[0].size(); ++ch_idx) {
        auto rho_ch = blaze::DynamicMatrix<T, blaze::columnMajor>(I[0][0].rows(), I[0][0].columns(), 0);
        rho_init.push_back(rho_ch);
    }
    for (size_t i = 0; i<I[0][0].rows(); ++i) {
        for (size_t j = 0; j<I[0][0].columns(); ++j) {
            for (size_t ch_idx=0; ch_idx<I[0].size(); ++ch_idx) {
                std::vector<T> p = {};
                for (size_t img_idx=0; img_idx<I.size(); ++img_idx) {
                    p.push_back(I[img_idx][ch_idx](i, j));
                }
                std::nth_element(p.begin(), p.begin() + p.size()/2, p.end());
                T median = p[p.size()/2 - 1];
                if (p.size() % 2 == 0) {
                    median = (median + p[p.size()/2]) / 2;
                }
                rho_init[ch_idx](i, j) = median;
            }
        }
    }
    // ----

    //auto i_rho = VariablesInitialization(I, Mask, rho_init);
    //std::vector<std::vector<blaze::DynamicVector<T>>> flat_imgs = std::get<0>(i_rho);
    //std::vector<blaze::DynamicVector<T>> rho = std::get<1>(i_rho);
    // ----

    size_t nimages = size(I);
    size_t nchannels = size(I[0]);  // we assume each image has equal number of channeks and resolution
    size_t nrows = I[0][0].rows();
    size_t ncols = I[0][0].columns();

    auto idc = indicesCwStd(Mask);

    //std::cout << "idc:\n" << idc << "\n\n";  // TODO remove

    // I -> flat_imga
    std::vector<std::vector<blaze::DynamicVector<T>>> flat_imgs;
    for (size_t i = 0; i < nimages; ++i) {
        std::vector<blaze::DynamicVector<T>> I_out = {};
        for (size_t c = 0; c<nchannels; ++c) {
            blaze::DynamicVector<T> flat_img = flattenToCol(I[i][c]);
            I_out.push_back(blaze::elements(flat_img, idc));  // apply mask and save
        }
        flat_imgs.push_back(I_out);
    }

    // rho_init -> rho
    std::vector<blaze::DynamicVector<T>> rho = {};
    for (size_t c = 0; c<nchannels; ++c) {
        blaze::DynamicVector<T> flat_rho = flattenToCol(rho_init[c]);
        rho.push_back(blaze::elements(flat_rho, idc));  // apply mask and save
    }

    // ----

    if (console_debug_output) {
        std::cout << "rho_init: " << std::endl << rho << std::endl << std::endl;
    }

    //nimages = flat_imgs.size();
    //nchannels = flat_imgs[0].size();
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

//    //auto normals = depthToNormals(Z_init, Mask, K);
//    auto gradients = getMaskedGradients(Z_init, Mask, K);
//    blaze::DynamicVector<T> z_vector_masked = std::get<0>(gradients);  // Matlab z = z(mask);
//    blaze::DynamicVector<T> zx = std::get<1>(gradients);
//    blaze::DynamicVector<T> zy = std::get<2>(gradients);
//    blaze::DynamicVector<T> xx = std::get<3>(gradients);
//    blaze::DynamicVector<T> yy = std::get<4>(gradients);
//    blaze::CompressedMatrix<T> Dx = std::get<5>(gradients);
//    blaze::CompressedMatrix<T> Dy = std::get<6>(gradients);
//    //blaze::DynamicMatrix<T> Dx = std::get<5>(gradients);
//    //blaze::DynamicMatrix<T> Dy = std::get<6>(gradients);

    // ----
    auto nM = getNabla<T>(Mask, Forward, DirichletHomogeneous);
    blaze::CompressedMatrix<T> Dx = std::get<0>(nM);
    blaze::CompressedMatrix<T> Dy = std::get<1>(nM);

    std::vector<size_t> z_idc = indicesCwStd(Mask);
    blaze::DynamicVector<T> z_vector = flattenToCol(Z_init);
    blaze::DynamicVector<T> z_vector_masked = blaze::elements(z_vector, z_idc);  // Matlab z = z(mask);

    blaze::DynamicVector<T> zx = Dx * z_vector_masked;  // matrix multiplication
    blaze::DynamicVector<T> zy = Dy * z_vector_masked;

    auto xxyy = indices2dCw(Mask);
    blaze::DynamicVector<T> xx = std::get<0>(xxyy) - K(0, 2);
    blaze::DynamicVector<T> yy = std::get<1>(xxyy) - K(1, 2);
    // ----


//    auto normals_map = getNormalMap(z_vector_masked, zx, zy, K, xx, yy);
//    blaze::DynamicMatrix<T> N_normalized = std::get<0>(normals_map);
//    blaze::DynamicMatrix<T> N_unnormalized = std::get<2>(normals_map);
    blaze::DynamicMatrix<T> N_unnormalized = pix_normals(z_vector_masked, zx, zy, xx, yy, K);
//    blaze::DynamicVector<T> dz = std::get<1>(normals_map);
    blaze::DynamicVector<T> dz = blaze::sqrt(blaze::sum<blaze::rowwise>(N_unnormalized % N_unnormalized)); // TODO compare to Eps if needed

    blaze::DynamicMatrix<T> N_normalized = normalize_normals(N_unnormalized, dz);

//    std::cout << "N_unnormalized: " << std::endl << N_unnormalized << std::endl;  // TODO remove
//    std::cout << "N_unnormalized2: " << std::endl << N_unnormalized2 << std::endl;
//    std::cout << "dz: " << std::endl << dz << std::endl;
//    std::cout << "dz2: " << std::endl << dz2 << std::endl;
//    std::cout << "N_normalized: " << std::endl << N_normalized << std::endl;  // TODO remove
//    std::cout << "N_normalized2: " << std::endl << N_normalized2 << std::endl;
//    std::cout << "N_unnormalized diff: " << std::endl << N_unnormalized - N_unnormalized2 << std::endl;  // TODO remove
//    std::cout << "dz diff: " << std::endl << dz - dz2 << std::endl;  // TODO remove
//    std::cout << "N_normalized diff: " << std::endl << N_normalized - N_normalized2 << std::endl;  // TODO remove

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

        if (it == c2f_lighting - 1 && sh_order == ho_low) {  // switch lighting scheme sph harmо order
            sh_order = ho_high;
            for (size_t i = 0; i < nimages; ++i) {
                for (size_t c = 0; c < nchannels; ++c) {
                    blaze::DynamicVector<T> v (9, 0);
                    blaze::subvector(v, 0, s[i][c].size()) = s[i][c];
                    s[i][c] = v;
                }
            }
            //s = initS<T>(I[0].size(), I.size(), {0, 0, -1, 0.2, 0, 0, 0, 0, 0});
            for (size_t c = 0; c < N_unnormalized.columns(); ++c) {
                blaze::column(normals_theta, c) = blaze::column(N_unnormalized, c) / theta;
            }
            sh = normalsToSh(normals_theta, sh_order);
        }

        // albedo update
        weights = calcReweighting(rho, sh, s, flat_imgs, lambda);
        if (console_debug_output) {  // TODO remove
            std::cout << std::endl << "before albedo update, iter " << it << ":" << std::endl;
            std::cout << std::endl << "flat_imgs:" << std::endl << flat_imgs << std::endl;
            std::cout << std::endl << "rho:" << std::endl << rho << std::endl;
            std::cout << std::endl << "sh:" << std::endl << sh << std::endl;
            std::cout << std::endl << "s:" << std::endl << s << std::endl;
            std::cout << std::endl << "weights:" << std::endl << weights << std::endl;
            std::cout << std::endl << "G:" << std::endl << G << std::endl;
            std::cout << std::endl << "huber:" << std::endl << huber << std::endl;
            std::cout << std::endl << "mu:" << std::endl << mu << std::endl;
        }
        rho = updateAlbedo(flat_imgs, rho, sh, s, weights, G, huber, mu, regular, tol, pcg_maxit);
        drho = {};
        for (size_t ch = 0; ch < rho.size(); ++ch) {
            blaze::DynamicVector<T> drho_ch = G * rho[ch];
            drho.push_back(drho_ch);
        }
        //drho = drho_upd;
        if (console_debug_output) {
            std::cout << "rho updated: " << std::endl << rho << std::endl << std::endl;
        }

        // lighting update
        weights = calcReweighting(rho, sh, s, flat_imgs, lambda);
        if (console_debug_output) {  // TODO remove
            std::cout << std::endl << "before lighting update, iter " << it << ":" << std::endl;
            std::cout << std::endl << "flat_imgs:" << std::endl << flat_imgs << std::endl;
            std::cout << std::endl << "rho:" << std::endl << rho << std::endl;
            std::cout << std::endl << "sh:" << std::endl << sh << std::endl;
            std::cout << std::endl << "s:" << std::endl << s << std::endl;
            std::cout << std::endl << "weights:" << std::endl << weights << std::endl;
            std::cout << std::endl << "tol:" << std::endl << tol << std::endl;
            std::cout << std::endl << "pcg_maxit:" << std::endl << pcg_maxit << std::endl;
        }
        auto s_upd = updateLighting(flat_imgs, rho, sh, s, weights, tol, pcg_maxit);
        s = std::get<0>(s_upd);
        //auto res_s = std::get<1>(s_upd);
        if (console_debug_output) {
            std::cout << "s updated: " << std::endl << s << std::endl << std::endl;
        }
        //std::cout << "s updated" << std::endl;  // TODO remove

        // depth update
        weights = calcReweighting(rho, sh, s, flat_imgs, lambda);
        if (console_debug_output) {  // TODO remove
            std::cout << std::endl << "before depth update, iter " << it << ":" << std::endl;
            std::cout << std::endl << "flat_imgs:" << std::endl << flat_imgs << std::endl;
            std::cout << std::endl << "rho:" << std::endl << rho << std::endl;
            std::cout << std::endl << "sh:" << std::endl << sh << std::endl;
            std::cout << std::endl << "s:" << std::endl << s << std::endl;
            std::cout << std::endl << "weights:" << std::endl << weights << std::endl;
            std::cout << std::endl << "z_vector_masked:" << std::endl << z_vector_masked << std::endl;
            std::cout << std::endl << "zx:" << std::endl << zx << std::endl;
            std::cout << std::endl << "zy:" << std::endl << zy << std::endl;
            std::cout << std::endl << "u:" << std::endl << u << std::endl;
            std::cout << std::endl << "drho:" << std::endl << drho << std::endl;
            std::cout << std::endl << "xx:" << std::endl << xx << std::endl;
            std::cout << std::endl << "yy:" << std::endl << yy << std::endl;
            std::cout << std::endl << "Dx:" << std::endl << Dx << std::endl;
            std::cout << std::endl << "Dy:" << std::endl << Dy << std::endl;
            std::cout << std::endl << "beta:" << std::endl << beta << std::endl;
        }
        //std::cout << "weights updated" << std::endl;  // TODO remove
        auto depth_upd = updateDepth<T>(flat_imgs, rho, s, theta, z_vector_masked, zx, zy, u, weights, drho, K, xx, yy, Dx, Dy,
                                     3, beta, 10, 1000, 1e-10, 1000, sh_order);  // TODO pass
        z_vector_masked = std::get<0>(depth_upd);
        zx = std::get<1>(depth_upd);
        zy = std::get<2>(depth_upd);
        dz = std::get<3>(depth_upd);
        N_unnormalized = std::get<4>(depth_upd);
        sh = std::get<5>(depth_upd);  // TODO remove
        //blaze::CompressedMatrix<T> J_dz = std::get<6>(depth_upd);
        //T res_z = std::get<7>(depth_upd);
        if (console_debug_output) {
            std::cout << "z_vector_masked updated: " << std::endl << z_vector_masked << std::endl << std::endl;
            std::cout << "zx updated: " << std::endl << zx << std::endl << std::endl;
            std::cout << "zy updated: " << std::endl << zy << std::endl << std::endl;
            std::cout << "dz updated: " << std::endl << dz << std::endl << std::endl;
            std::cout << "N_unnormalized updated: " << std::endl << N_unnormalized << std::endl << std::endl;
            std::cout << "sh updated: " << std::endl << sh << std::endl << std::endl;
        }

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
    if (console_debug_output) {  // TODO remove
        std::cout << std::endl << "final:" << std::endl;
        std::cout << std::endl << "z_vector_masked:" << std::endl << z_vector_masked << std::endl;
        std::cout << std::endl << "rho:" << std::endl << rho << std::endl;
        //std::cout << std::endl << "sh:" << std::endl << sh << std::endl;
        //std::cout << std::endl << "s:" << std::endl << s << std::endl;
    }

    auto z_out = vecToImage<T>(z_vector_masked, Mask, img_h, img_w);
    std::vector<blaze::DynamicMatrix<T>> rho_out = {};
    for (size_t ch = 0; ch < rho.size(); ++ch) {
        rho_out.push_back(vecToImage<T>(rho[ch], Mask, img_h, img_w));
    }

    return std::make_tuple(z_out, rho_out, s);
}



#endif // _UPS_SOLVER_HPP
