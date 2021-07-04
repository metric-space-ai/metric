
#ifndef _UPS_UPDATE_BLOCKS_HPP
#define _UPS_UPDATE_BLOCKS_HPP


#include "modules/utils/solver/ups/ups_solver/normals.hpp"

#include "modules/utils/solver/helper/lapwrappers.hpp"
#include "modules/utils/solver/pcg.hpp"

#include "modules/utils/solver/approxchol.hpp"

#include "3rdparty/blaze/Blaze.h"

#include <tuple>
#include <iostream>  // TODO remove when debugged



// ---------- service functions


/**
 * @brief computes Jacobian of normals by depth
 * @param normals - x, y, z components (in columns) of normals per (masked) pixel
 * @param z_vector_masked - depth per (masked) pixel
 * @param K - camera intrinsic matrix
 * @param xx - indices of masked pixels by x with respect to point where camera axis crosses the image
 * @param yy - same by y
 * @param Dx - gradient of z in x direction
 * @param Dy - same in y direction
 * @return Jacobian matrix of unnormalized normal on z and Jacobian matrix of norm of the normal (dz), respectively
 */
template <typename T>
std::tuple<std::vector<blaze::CompressedMatrix<T>>, blaze::CompressedMatrix<T>>
//std::tuple<std::vector<blaze::DynamicMatrix<T>>, blaze::DynamicMatrix<T>>
normalJacByDepth(
        const blaze::DynamicMatrix<T> & normals,
        const blaze::DynamicVector<T> & z_vector_masked,
        const blaze::DynamicMatrix<T> & K,
        const blaze::DynamicVector<T> & xx,
        const blaze::DynamicVector<T> & yy,
        const blaze::CompressedMatrix<T> & Dx,
        const blaze::CompressedMatrix<T> & Dy
        //const blaze::DynamicMatrix<T> & Dx,
        //const blaze::DynamicMatrix<T> & Dy
        )
{
    size_t npix = z_vector_masked.size();
    auto J_n_un_1 = K(0, 0) * Dx;
    auto J_n_un_2 = K(1, 1) * Dy;
    auto eye = blaze::IdentityMatrix<T, blaze::columnMajor>(npix);

    blaze::CompressedMatrix<T> xx_diag (xx.size(), xx.size());
    blaze::CompressedMatrix<T> yy_diag (xx.size(), xx.size());
    //blaze::DynamicMatrix<T> xx_diag (xx.size(), xx.size(), 0);
    //blaze::DynamicMatrix<T> yy_diag (xx.size(), xx.size(), 0);

    {
        auto diag = blaze::diagonal(xx_diag);
        diag = xx;
    }
    {
        auto diag = blaze::diagonal(yy_diag);
        diag = yy;
    }
    auto J_n_un_3 = -eye - xx_diag*Dx - yy_diag*Dy;

    blaze::CompressedMatrix<T> x_scale (npix, npix);
    blaze::CompressedMatrix<T> y_scale (npix, npix);
    blaze::CompressedMatrix<T> z_scale (npix, npix);
    //blaze::DynamicMatrix<T> x_scale (npix, npix, 0);
    //blaze::DynamicMatrix<T> y_scale (npix, npix, 0);
    //blaze::DynamicMatrix<T> z_scale (npix, npix, 0);

    {
        auto diag = blaze::diagonal(x_scale);
        diag = blaze::column(normals, 0);
    }
    {
        auto diag = blaze::diagonal(y_scale);
        diag = blaze::column(normals, 1);
    }
    {
        auto diag = blaze::diagonal(z_scale);
        diag = blaze::column(normals, 2);
    }

    blaze::CompressedMatrix<T> J_dz =
    //blaze::DynamicMatrix<T> J_dz =
            x_scale * J_n_un_1 +
            y_scale * J_n_un_2 +
            z_scale * J_n_un_3;

    std::vector<blaze::CompressedMatrix<T>> J_n_un (3);
    //std::vector<blaze::DynamicMatrix<T>> J_n_un (3);
    J_n_un[0] = J_n_un_1;
    J_n_un[1] = J_n_un_2;
    J_n_un[2] = J_n_un_3;

    return std::make_tuple(J_n_un, J_dz);
}


/**
 * @brief computes energy (being minimized)
 * @param flat_imgs - vector of vectorized masked images represented by vector of 3 color channels,
 * where each channel is DynamicVector of masked pixels
 * @param rho - albedo per color channels, vectorized by mask the same way as images
 * @param s - lighting coefficients per image, than per channel (4 or 9 depending on current lightig scheme)
 * @param sh - augmented lightig data, brightness share of oriented normal (of unit albedo) regardong given lighting source coefficient
 * (in columns, horizontally arranged first by image, than by channel)
 * @param theta - (lagged) norm of normals, used as lagged block descend method auxiliary vatiable
 * @param drho - gradients of rho by x and y, concatenated into single matrix, stored in vector by channels
 * @param dz - updated norm of normals, to be compared with theta
 * @param lambda - weight of shape from shading term
 * @param huber - Huber regularization factor
 * @param mu - the weight of smuthness on albedos
 * @param beta - stepsize on theta update
 * @return energy value
 */
template <typename T>
T energyCauchy(
        const std::vector<std::vector<blaze::DynamicVector<T>>> & flat_imgs,
        const std::vector<blaze::DynamicVector<T>> & rho,
        const std::vector<std::vector<blaze::DynamicVector<T>>> & s,
        const blaze::DynamicMatrix<T> & sh,
        const blaze::DynamicVector<T> & theta,
        const std::vector<blaze::DynamicVector<T>> & drho,
        const blaze::DynamicVector<T> & dz,
        //const blaze::DynamicVector<T> & u,
        const T lambda = 1.0,
        const T huber = 0.1,
        const T mu = 0.000002,
        const T beta = 0.0005
        )
{

    T energy = 0;
    for (size_t im = 0; im < s.size(); ++im) {
        for (size_t ch = 0; ch < s[0].size(); ++ch) {
            blaze::DynamicVector<T> tmp = (sh * s[im][ch])*rho[ch];
            energy += blaze::sum(blaze::pow(tmp - flat_imgs[im][ch], 2));
        }
    }
    energy = lambda/2*energy;
    //T energy_no_smooth = energy;

    // if(options.regular==1)
    T huber_summand = 0;
    for (size_t ch = 0; ch < drho.size(); ++ch) {
        for (size_t el = 0; el < drho[ch].size(); el++) {
            if (drho[ch][el] >= huber) {
                huber_summand += blaze::abs(drho[ch][el]) - huber*0.5;
            } else {
                huber_summand += 0.5*blaze::pow(drho[ch][el], 2) / huber;
            }
        }
    }
    energy += mu*huber_summand;

    //T objective = energy + 0.5*beta * blaze::sum(blaze::pow(theta - dz + u, 2));
    T objective = energy + 0.5*beta * blaze::sum(blaze::pow(theta - dz, 2));  // removed u

    //return std::make_tuple(energy, objective, energy_no_smooth);  // no need since we do not show per-iter progress output
    return objective;
}


/**
 * @brief weight updater
 * @param rho - albedo per color channels, vectorized by mask the same way as images
 * @param sh - augmented lightig data, brightness of oriented normal (of unit albedo) under current lighting (in columns,
 * horizontally arranged first by image, than by channel)
 * @param s - lightong coefficients per image, than per channel (4 or 9 depending on current lightig scheme)
 * @param flat_imgs - vector of vectorized masked images represented by vector of 3 color channels,
 * where each channel is DynamicVector of (masked) pixels
 * @param lambda - the weight of shape from shading term
 * @return updated weights used in Lagged Block Descent Method
 */
template <typename T>
std::vector<std::vector<blaze::DynamicVector<T>>>
reweight(
        const std::vector<blaze::DynamicVector<T>> & rho,
        const blaze::DynamicMatrix<float> & sh,
        const std::vector<std::vector<blaze::DynamicVector<T>>> & s,
        const std::vector<std::vector<blaze::DynamicVector<T>>> & flat_imgs,
        const T lambda = 1
        )
{
    size_t nimages = flat_imgs.size();
    size_t nchannels = flat_imgs[0].size();
    //size_t npix = flat_imgs[0][0].size();

    //auto rho_avg = blaze::DynamicVector<T>(rho[0].size(), 0);
    //for (size_t i = 0; i<rho.size(); ++i) {
    //    rho_avg += rho[i];
    //}
    //rho_avg /= rho.size();

    blaze::DynamicMatrix<T, blaze::columnMajor> s_mat (s[0][0].size(), nimages*nchannels);
    for (size_t i = 0; i < nimages*nchannels; ++i) {
        blaze::column(s_mat, i) = s[i/nchannels][i%nchannels];
    }
    auto rk_mat = sh * s_mat;

    std::vector<std::vector<blaze::DynamicVector<T>>> weights = {};
    size_t w_idx = 0;
    for (size_t i = 0; i < nimages; ++i) {
        std::vector<blaze::DynamicVector<T>> rk_ch = {};
        for (size_t c = 0; c<nchannels; ++c) {
            blaze::DynamicVector<T> v = blaze::column(rk_mat, w_idx);
            v = v * rho[c];  // elementwize
            v = v - flat_imgs[i][c];
            v = blaze::pow(v, 2) / (lambda*lambda) + 1;
            v = 1 / v;
            rk_ch.push_back(v);
            ++w_idx;
        }
        weights.push_back(rk_ch);
    }

    return weights;
}




// ----------- block updaters


/**
 * @brief albedo updater
 * @param rho - albedo, in&out parameter being updated
 * @param flat_imgs - vectorized images
 * @param N - augmented lighting, same as sh (refer to energyCauche for details)
 * @param s - lighting coefficients per image, than per channel (4 or 9 depending on current lightig scheme)
 * @param reweight - updated weights
 * @param G - gradients of albedo per x and per y, concatenated in single matrix
 * @param huber - Huber regularization factor
 * @param mu - the weight of smuthness on albedos
 * @param tol - PCG tolerance
 * @param maxit - maximum number of iterations for PCG
 * for more details on most parameters see energyCauchy or reweight functions above
 */
template <typename T>
void updateAlbedo(
        std::vector<blaze::DynamicVector<T>> & rho, // in & out
        const std::vector<std::vector<blaze::DynamicVector<T>>> & flat_imgs,
        const blaze::DynamicMatrix<T> & N,  // sh
        const std::vector<std::vector<blaze::DynamicVector<T>>> & s,
        const std::vector<std::vector<blaze::DynamicVector<T>>> & reweight,
        const blaze::CompressedMatrix<T> & G,
        //const blaze::DynamicMatrix<T> & G,
        const T huber,
        const T mu,
        const bool regular,
        const float tol,  // TODO check type!!
        const float maxit    // TODO check type!!
        )
{

    size_t nimages = flat_imgs.size();
    size_t nchannels = flat_imgs[0].size();
    size_t npix = flat_imgs[0][0].size();

    for (size_t ch = 0; ch < nchannels; ++ch) {
        //blaze::DynamicMatrix<T> A (npix, npix, 0);
        blaze::CompressedMatrix<T, blaze::columnMajor> A (npix, npix);
        if (regular) {
            blaze::CompressedMatrix<T> Dk (G.rows(), G.rows());
            //blaze::DynamicMatrix<T> Dk (G.rows(), G.rows(), 0);
            auto c = blaze::abs(G * rho[ch]);
            blaze::DynamicVector<T>  diag_values = map( c, [huber](T el) { return 1.0/(el > huber ? el : huber); } );
            blaze::diagonal(Dk) = diag_values;
            A = A + mu * blaze::trans(G) * Dk * G;
        }
        //auto b = blaze::CompressedVector<T>(rho[0].size());
        auto b = blaze::DynamicVector<T>(rho[0].size(), 0);
        for (size_t im = 0; im < nimages; ++im) {
            auto Ns = N * s[im][ch];
            blaze::DynamicVector<T> nnNs = map( Ns, [](T el) { return el > 0 ? el : 0; } );
            blaze::DynamicVector<T> diag_values = blaze::sqrt(2*reweight[im][ch])*nnNs;
            blaze::CompressedMatrix<T> Ai (npix, npix);
            //blaze::DynamicMatrix<T> Ai (npix, npix, 0);
            blaze::diagonal(Ai) = diag_values;
            A = A + blaze::trans(Ai) * Ai;
            b = b + blaze::trans(Ai) * (blaze::sqrt(2*reweight[im][ch])*flat_imgs[im][ch]);
        }

        //* // TODO make good Pre
        std::vector<size_t> pcgIts = {};
//        auto Pre = blaze::IdentityMatrix<T>(A.rows());
        blaze::CompressedMatrix<T> Pre (A.rows(), A.rows());
        for (size_t p = 0; p<npix; ++p) { // temporary code
            T d_val = A(p, p);
            d_val = d_val > 0 ? d_val : 1;
            Pre(p, p) = d_val; // > 0 ? d_val : 1;
        }

        //metric::LLmatp<T> llmat (A);
        //metric::LDLinv<T> ldli = metric::approxChol(llmat);
        //metric::SolverB<T> F = [=](const blaze::DynamicVector<T>& b) { return ldli.LDLsolver(b); };

        //blaze::DynamicMatrix<T> AA = A;  // TODO remove
//        auto start_time = std::chrono::steady_clock::now(); // TODO remove after debugging & profiling
        //blaze::DynamicVector<T> X = metric::pcg<T>(A, b, Pre, pcgIts, tol, maxit);
        blaze::DynamicVector<T> X = metric::pcg<T>(A, b, Pre, pcgIts, tol, maxit);
        //blaze::DynamicVector<T> X = metric::pcg<T>(A, b, F, pcgIts, tol, maxit);
        //blaze::DynamicVector<T> X = blaze::solve(AA, b);
//        auto end_time = std::chrono::steady_clock::now();
//        std::cout << "PCG for albedo completed in " <<
//                     double(std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count()) / 1000000 << " s" <<
//                     std::endl;
        // */

        /*
        std::vector<size_t> pcgIts = {};
        auto fn = metric::approxchol_lap(A, pcgIts, 1e-2F);
        auto start_time = std::chrono::steady_clock::now();
        //blaze::DynamicVector<T> X = metric::pcg<T>(A, b, Pre, pcgIts, tol, maxit);
        blaze::DynamicVector<T> X = fn(b);
        auto end_time = std::chrono::steady_clock::now();
        std::cout << "PCG for albedo completed in " <<
                     double(std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count()) / 1000000 << " s" <<
                     std::endl << std::endl;
        // */

        rho[ch] = X;
    }

}



/**
 * @brief lighting per image updater
 * @param s - lighting coefficients per image, than per channel (4 or 9 depending on current lightig scheme),
 * being updated (in&out parameter)
 * @param flat_imgs - vectorized images
 * @param rho - albedo, in&out parameter being updated
 * @param N - augmented lighting, same as sh (refer to energyCauche for details)
 * @param reweight - updated weights
 * @param tol - PCG tolerance
 * @param maxit - maximum number of iterations for PCG
 * for more details on most parameters see energyCauchy or reweight functions above
 */
template <typename T>
void updateLighting(
        std::vector<std::vector<blaze::DynamicVector<T>>> & s,  // in & out
        const std::vector<std::vector<blaze::DynamicVector<T>>> & flat_imgs,
        const std::vector<blaze::DynamicVector<T>> & rho,
        const blaze::DynamicMatrix<T> & N,  // sh
        const std::vector<std::vector<blaze::DynamicVector<T>>> & reweight,
        const float tol,  //T nb_nonsingular
        const float maxit    // TODO check type!! PCG requires double for some reason

        )
{

    size_t nimages = flat_imgs.size();
    size_t nchannels = flat_imgs[0].size();

    for (size_t im = 0; im < nimages; ++im) {
        for (size_t ch = 0; ch < nchannels; ++ch) {

            auto r_sq = blaze::sqrt(reweight[im][ch]);
            blaze::DynamicVector<T> rho_i_c = r_sq * rho[ch];
            blaze::DynamicVector<T> img_i_c = r_sq * flat_imgs[im][ch];
            blaze::DynamicVector<T> l = N * s[im][ch];
            for (size_t i = 0; i < l.size(); ++i) {
                if (l[i] < 0) {
                    rho_i_c[i] = 0;
                    img_i_c[i] = 0;
                }
            }

            blaze::DynamicMatrix<T> rhon (N.rows(), N.columns());
            for (size_t c = 0; c < N.columns(); ++c) {
                blaze::column(rhon, c) = rho_i_c * blaze::column(N, c);  // elementwise
            }

            auto A = blaze::trans(rhon)*rhon;
            auto b = blaze::trans(rhon)*img_i_c;

            //auto PInv = blaze::inv(blaze::trans(A) * A) * blaze::trans(A);  // no, we need pinv since matrix can be singular!
            //blaze::DynamicVector<T> s_ch = PInv * b;

            //blaze::DynamicVector<T> s_ch = blaze::solve(A, b);

            std::vector<size_t> pcgIts = {};
            auto Pre = blaze::IdentityMatrix<T>(A.rows());
//            blaze::CompressedMatrix<T> Pre (A.rows(), A.rows());
//            for (size_t p = 0; p<A.rows(); ++p) {  // temporary code
//                T d_val = A(p, p);
//                d_val = d_val > 0 ? d_val : 1;
//                Pre(p, p) = d_val; // > 0 ? d_val : 1;
//            }

//            auto start_time = std::chrono::steady_clock::now(); // TODO remove after profiling
            blaze::DynamicVector<T> s_ch = metric::pcg<T>(A, b, Pre, pcgIts, tol, maxit);
//            auto end_time = std::chrono::steady_clock::now();
//            std::cout << "PCG for lighting completed in " <<
//                         double(std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count()) / 1000000 << " s" <<
//                         std::endl;

            s[im][ch] = s_ch;
        }
    }

}




/**
 * @brief z map updater
 * @param N_unnormalized - map of unnormalized normals (x, y, z components (in columns) of normals per (masked) pixel)
 * (in&out parameter being updated)
 * @param theta - (lagged) norm of normals, used as lagged block descend method auxiliary vatiable (in%out)
 * @param z_vector_masked - depth map per (masked) pixel (in&out)
 * @param zx - equals to Dx * z_vector_masked (in&out)
 * @param zy - same by y (in&out)
 * @param flat_imgs - vector of vectorized masked images represented by vector of 3 color channels,
 * where each channel is DynamicVector of (masked) pixels
 * @param rho - albedo per color channels, vectorized by mask the same way as images
 * @param s - lighting coefficients per image, than per channel (4 or 9 depending on current lightig scheme)
 * @param reweight - updated weights
 * @param drho - gradients of rho by x and y, concatenated into single matrix, stored in vector by channels
 * @param K - camera intrinsic matrix
 * @param xx - indices of masked pixels by x with respect to point where camera axis crosses the image
 * @param yy - same by y
 * @param Dx - gradient of z in x direction
 * @param Dy - same in y direction
 * @param maxit - maximum number of iterations of inner loop
 * @param beta - stepsize on theta update
 * @param t_ - initial step size for line search
 * @param maxit_linesearch - maximum number of line search
 * @param pcg_tol - PCG tolerance
 * @param pcg_maxit - maximum number of iterations for PCG
 * @param sh_order - current lighting approximation scheme: 4 components for ho_low, 9 components for ho_high
 * for more details on some parameters see energyCauchy or reweight functions above
 */
template <typename T>
void updateDepth(
        blaze::DynamicMatrix<T> & N_unnormalized,  // out
        blaze::DynamicVector<T> & theta,  // in & out
        blaze::DynamicVector<T> & z_vector_masked,  // in & out
        blaze::DynamicVector<T> & zx,  // in & out
        blaze::DynamicVector<T> & zy,  // in & out
        const std::vector<std::vector<blaze::DynamicVector<T>>> & flat_imgs,
        const std::vector<blaze::DynamicVector<T>> & rho,
        const std::vector<std::vector<blaze::DynamicVector<T>>> & s,
        //const blaze::DynamicVector<T> & u,
        const std::vector<std::vector<blaze::DynamicVector<T>>> & reweight,
        const std::vector<blaze::DynamicVector<T>> & drho,
        const blaze::DynamicMatrix<T> & K,
        const blaze::DynamicVector<T> & xx,
        const blaze::DynamicVector<T> & yy,
        const blaze::CompressedMatrix<T> & Dx,
        const blaze::CompressedMatrix<T> & Dy,
        //const blaze::DynamicMatrix<T> & Dx,
        //const blaze::DynamicMatrix<T> & Dy,
        const size_t maxit = 3,  // option options.LinS.maxit,
        const T beta = 5e-4,
        const T t_ = 10,
        const size_t maxit_linesearch = 1000,
        const float pcg_tol = 1e-10,
        const float pcg_maxit = 1000,
        const harmo_order sh_order = ho_low
        )
{

    size_t nimages = flat_imgs.size();
    size_t nchannels = flat_imgs[0].size();
    size_t npix = flat_imgs[0][0].size();

    T t = t_;

    blaze::DynamicVector<T> z0 = z_vector_masked;
    blaze::DynamicVector<T> z_last;

    N_unnormalized = pixNormals(z_vector_masked, zx, zy, xx, yy, K);
    blaze::DynamicVector<T> dz = blaze::sqrt(blaze::sum<blaze::rowwise>(N_unnormalized % N_unnormalized)); // TODO compare to Eps if needed
    blaze::DynamicMatrix<T> N_normalized = normalizePixNormals(N_unnormalized, dz);

    auto Jac = normalJacByDepth<float>(N_normalized, z_vector_masked, K, xx, yy, Dx, Dy);
    auto J_dz = std::get<1>(Jac);
    auto J_n_un = std::get<0>(Jac);

    blaze::DynamicMatrix<T> normals_theta (N_unnormalized.rows(), N_unnormalized.columns());
    for (size_t c = 0; c < N_unnormalized.columns(); ++c) {
        blaze::column(normals_theta, c) = blaze::column(N_unnormalized, c) / theta;
    }

    auto sh = normalsToSh(normals_theta, sh_order);

    T tab_objective = energyCauchy(flat_imgs, rho, s, sh, theta, drho, dz);  // removed u

    blaze::DynamicVector<T> z;
    //T res_z;

    for (size_t it = 0; it < maxit; ++it) {

        //std::cout << "started z update (inner) iteration " << it << std::endl;
        std::vector<std::vector<blaze::DynamicVector<T>>> reweighted_rho = {};  // input for calcEnergyPhotometricTerm computation
        std::vector<std::vector<blaze::DynamicVector<T>>> reweighted_img = {};
        for (size_t im = 0; im < nimages; ++im) {
            reweighted_rho.push_back(std::vector<blaze::DynamicVector<T>>());
            reweighted_img.push_back(std::vector<blaze::DynamicVector<T>>());
            for (size_t ch = 0; ch < nchannels; ++ch) {
                auto r_sq = blaze::sqrt(reweight[im][ch]);
                blaze::DynamicVector<T> rho_i_c = r_sq * rho[ch];
                blaze::DynamicVector<T> img_i_c = r_sq * flat_imgs[im][ch];
                blaze::DynamicVector<T> l = sh * s[im][ch];
                for (size_t i = 0; i < l.size(); ++i) {
                    if (l[i] < 0) {
                        rho_i_c[i] = 0;
                        img_i_c[i] = 0;
                    }
                }
                reweighted_rho[im].push_back(rho_i_c);
                reweighted_img[im].push_back(img_i_c);
            }
        }

        // ------
        // calcEnergyPhotometricTerm Matlab function

        blaze::DynamicMatrix<T> normals_theta (N_unnormalized.rows(), N_unnormalized.columns());
        for (size_t c = 0; c < N_unnormalized.columns(); ++c) {
            blaze::column(normals_theta, c) = blaze::column(N_unnormalized, c) / theta;
        }

        blaze::CompressedMatrix<T> theta_1_diag (theta.size(), theta.size());
        //blaze::DynamicMatrix<T> theta_1_diag (theta.size(), theta.size(), 0);
        blaze::diagonal(theta_1_diag) = 1/theta;

        blaze::CompressedMatrix<T> J_n_0 = theta_1_diag * J_n_un[0];
        blaze::CompressedMatrix<T> J_n_1 = theta_1_diag * J_n_un[1];
        blaze::CompressedMatrix<T> J_n_2 = theta_1_diag * J_n_un[2];
        //blaze::DynamicMatrix<T> J_n_0 = theta_1_diag * J_n_un[0];
        //blaze::DynamicMatrix<T> J_n_1 = theta_1_diag * J_n_un[1];
        //blaze::DynamicMatrix<T> J_n_2 = theta_1_diag * J_n_un[2];

        // ---- calcJacobianWrtNormals Matlab function call ----

        T w0 = sqrt(3/(4*M_PI));
        //T w3 = sqrt(1/(4*M_PI));  // not used in the reference code, TODO check again

        blaze::CompressedMatrix<T> J_sh_0 = w0 * J_n_0;
        blaze::CompressedMatrix<T> J_sh_1 = w0 * J_n_1;
        blaze::CompressedMatrix<T> J_sh_2 = w0 * J_n_2;
        blaze::CompressedMatrix<T> J_sh_3 (J_n_0.rows(), J_n_0.columns());
        //blaze::DynamicMatrix<T> J_sh_0 = w0 * J_n_0;
        //blaze::DynamicMatrix<T> J_sh_1 = w0 * J_n_1;
        //blaze::DynamicMatrix<T> J_sh_2 = w0 * J_n_2;
        //blaze::DynamicMatrix<T> J_sh_3 (J_n_0.rows(), J_n_0.columns(), 0);

        std::vector<blaze::CompressedMatrix<T>> J_sh = {J_sh_0, J_sh_1, J_sh_2, J_sh_3};
        //std::vector<blaze::DynamicMatrix<T>> J = {J_sh_0, J_sh_1, J_sh_2, J_sh_3};

        if (sh_order != ho_low) {
            T w4 = 3*sqrt(5/(12*M_PI));
            T w5 = 3*sqrt(5/(12*M_PI));
            T w7 = 3/2*sqrt(5/(12*M_PI));
            T w8 = 0.5*sqrt(5/(4*M_PI));

            // add diag
            blaze::CompressedMatrix<T> N0 (normals_theta.rows(), normals_theta.rows());
            blaze::CompressedMatrix<T> N1 (normals_theta.rows(), normals_theta.rows());
            blaze::CompressedMatrix<T> N2 (normals_theta.rows(), normals_theta.rows());
            //blaze::DynamicMatrix<T> N0 (normals_theta.rows(), normals_theta.rows(), 0);
            //blaze::DynamicMatrix<T> N1 (normals_theta.rows(), normals_theta.rows(), 0);
            //blaze::DynamicMatrix<T> N2 (normals_theta.rows(), normals_theta.rows(), 0);
            blaze::diagonal(N0) = blaze::column(normals_theta, 0);
            blaze::diagonal(N1) = blaze::column(normals_theta, 1);
            blaze::diagonal(N2) = blaze::column(normals_theta, 2);
            blaze::CompressedMatrix<T> J_sh_4 = w4 * (N1 * J_n_0 + N0 * J_n_1);
            blaze::CompressedMatrix<T> J_sh_5 = w4 * (N2 * J_n_0 + N0 * J_n_2);
            blaze::CompressedMatrix<T> J_sh_6 = w5 * (N2 * J_n_1 + N1 * J_n_2);
            blaze::CompressedMatrix<T> J_sh_7 = 2 * w7 * (N0 * J_n_0 - N1 * J_n_1);
            blaze::CompressedMatrix<T> J_sh_8 = 6 * w8 * N2 * J_n_2;
            //blaze::DynamicMatrix<T> J_sh_4 = w4 * (N1 * J_n_0 + N0 * J_n_1);
            //blaze::DynamicMatrix<T> J_sh_5 = w4 * (N2 * J_n_0 + N0 * J_n_2);
            //blaze::DynamicMatrix<T> J_sh_6 = w5 * (N2 * J_n_1 + N1 * J_n_2);
            //blaze::DynamicMatrix<T> J_sh_7 = 2 * w7 * (N0 * J_n_0 - N1 * J_n_1);
            //blaze::DynamicMatrix<T> J_sh_8 = 6 * w8 * N2 * J_n_2;
            J_sh.push_back(J_sh_4);
            J_sh.push_back(J_sh_5);
            J_sh.push_back(J_sh_6);
            J_sh.push_back(J_sh_7);
            J_sh.push_back(J_sh_8);
        }
        // ---- end of calcJacobianWrtNormals Matlab function ----

        auto sh = normalsToSh(normals_theta, sh_order);

        //blaze::DynamicVector<T> cost_aug = sqrt(0.5*beta) * (theta - dz + u);
        blaze::DynamicVector<T> cost_aug = sqrt(0.5*beta) * (theta - dz);  // no u anymore

        //Matlab code: J_aug = -spdiags(repmat(sqrt(0.5*params.beta),size(theta,1),1), 0, size(theta,1), size(theta,1)) * J_dz;  % jacobian of lagrangian
        blaze::CompressedMatrix<T> J_aug = -J_dz * sqrt(0.5*beta);  // TODO check simplification by tests!
        //blaze::DynamicMatrix<T> J_aug = -J_dz * sqrt(0.5*beta);  // TODO check simplification by tests!

        blaze::CompressedMatrix<T> J_cauchy (nimages*nchannels*npix, npix);
        //blaze::DynamicMatrix<T> J_cauchy (nimages*nchannels*npix, npix, 0);

        //blaze::DynamicVector<size_t> row_vec (nimages*nchannels*npix, 0);
        //blaze::DynamicVector<size_t> col_vec (nimages*nchannels*npix, 0);
        //blaze::DynamicVector<T> val_vec (nimages*nchannels*npix, 0);
        std::vector<size_t> row_vec;
        std::vector<size_t> col_vec;
        std::vector<T> val_vec;
        row_vec.reserve(nimages*nchannels*npix);
        col_vec.reserve(nimages*nchannels*npix);
        val_vec.reserve(nimages*nchannels*npix);
        size_t idx = 0;
        blaze::DynamicVector<T> cost_cauchy (nimages*nchannels*npix, 0);

        for (size_t im = 0; im < nimages; ++im) {
            for (size_t ch = 0; ch < nchannels; ++ch) {

                blaze::DynamicVector<T> cost_im_ch = reweighted_rho[im][ch]*(sh * s[im][ch]) - reweighted_img[im][ch];
                 blaze::subvector(cost_cauchy, idx, npix) = cost_im_ch;
                 idx += cost_im_ch.size();

                 //Jacobian of photometric term
                 blaze::CompressedMatrix<T> J_sh_ =
                 //blaze::DynamicMatrix<T> J_sh_ =
                         J_sh[0] * s[im][ch][0] +
                         J_sh[1] * s[im][ch][1] +
                         J_sh[2] * s[im][ch][2] +
                         J_sh[3] * s[im][ch][3];
                 if (sh_order != ho_low) {
                     J_sh_ = J_sh_ +
                             J_sh[4] * s[im][ch][4] +
                             J_sh[5] * s[im][ch][5] +
                             J_sh[6] * s[im][ch][6] +
                             J_sh[7] * s[im][ch][7] +
                             J_sh[8] * s[im][ch][8];
                 }

                 blaze::CompressedMatrix<T> rho_w_diag (npix, npix);
                 //blaze::DynamicMatrix<T> rho_w_diag (npix, npix, 0);
                 blaze::diagonal(rho_w_diag) = reweighted_rho[im][ch];
                 blaze::CompressedMatrix<T, blaze::columnMajor> rho_w_J_sh_ = rho_w_diag * J_sh_;
                 //blaze::DynamicMatrix<T, blaze::columnMajor> rho_w_J_sh_ = rho_w_diag * J_sh_;

                 //size_t cnt = 0;
                 for (size_t c = 0; c < rho_w_J_sh_.columns(); ++c) {
                     for (typename blaze::CompressedMatrix<T, blaze::columnMajor>::Iterator it=rho_w_J_sh_.begin(c); it!=rho_w_J_sh_.end(c); ++it) {
                         row_vec.push_back(it->index() + (3*im + ch)*npix); // TODO optimize J_cauchy filling
                         col_vec.push_back(c);
                         val_vec.push_back(it->value());
                         //++cnt;
                     }
                 }
            }
        }

        for (size_t el = 0; el < row_vec.size(); el++) {
            J_cauchy(row_vec[el], col_vec[el]) = val_vec[el];  // TODO optimize!!
        }

        // ------
        // end of calcEnergyPhotometricTerm Matlab function

        T t_step = 2/(2 + 1/t);
        size_t count = 1;

        auto F = blaze::trans(J_aug)*J_aug + blaze::trans(J_cauchy) * J_cauchy;
        auto b = -blaze::trans(J_aug)*cost_aug - blaze::trans(J_cauchy) * cost_cauchy;

        //blaze::DynamicMatrix<T> L;  // TODO debug and enable
        //blaze::DynamicMatrix<T> F_dense = F;
        //blaze::llh(F_dense, L);  // result slightly differs, consider parameters using of Cholesky decomposition

        //std::cout << std::endl << "L:" << std::endl << L << std::endl;

        std::vector<size_t> pcgIts = {};
//        auto Pre = blaze::IdentityMatrix<T>(F.rows());  // TODO replace with good preconditioner
        blaze::CompressedMatrix<T> Pre (F.rows(), F.rows());
        for (size_t p = 0; p<F.rows(); ++p) {  // temporary code
            T d_val = F(p, p);
            d_val = d_val > 0 ? d_val : 1;
            Pre(p, p) = d_val; // > 0 ? d_val : 1;
        }
//        auto start_time = std::chrono::steady_clock::now(); // TODO disable
        blaze::DynamicVector<T> z_step = metric::pcg<T>(F, b, Pre, pcgIts, pcg_tol, pcg_maxit);
//        auto end_time = std::chrono::steady_clock::now();
//        std::cout << "PCG for depth completed in " <<
//                     double(std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count()) / 1000000 << " s" <<
//                     std::endl;

        //z_step = {
        //    0.79248,
        //   -0.30904,
        //    3.98192,
        //    2.83238,
        //   -1.32703,
        //    0.21779,
        //   -0.40463,
        //    0.28324,
        //    0.56881
        //};  // TODO remove


        while (true) {

            z_vector_masked = z0 + t_step*z_step;

            //depthToNormals
            blaze::DynamicVector<T> zx_ = Dx*z_vector_masked;
            blaze::DynamicVector<T> zy_ = Dy*z_vector_masked;

            N_unnormalized = pixNormals(z_vector_masked, zx_, zy_, xx, yy, K);
            dz = blaze::sqrt(blaze::sum<blaze::rowwise>(N_unnormalized % N_unnormalized)); // TODO compare to Eps if needed

            for (size_t c = 0; c < N_unnormalized.columns(); ++c) {
                blaze::column(normals_theta, c) = blaze::column(N_unnormalized, c) / theta;
            }
            sh = normalsToSh(normals_theta, sh_order);

            //auto ec = energyCauchy(flat_imgs, rho, s, sh, theta, drho, dz, u);
            //auto ec = energyCauchy(flat_imgs, rho, s, sh, theta, drho, dz);
            //T objective = std::get<1>(ec);
            T objective = energyCauchy(flat_imgs, rho, s, sh, theta, drho, dz);  // removed u

            ++count;

            if (objective > tab_objective && count < maxit_linesearch) {
                t = t*0.5;
                t_step = 2/(2+1/t);

            } else {

                //if (objective > tab_objective) {
                    //std::cout << std::endl << "Not found descent step in z update: step " << t_step << std::endl;
                //}
                z_last = z0;
                z0 = z_vector_masked;

                //depthToNormals
                zx = Dx*z0;
                zy = Dy*z0;
                N_unnormalized = pixNormals(z0, zx, zy, xx, yy, K);
                dz = blaze::sqrt(blaze::sum<blaze::rowwise>(N_unnormalized % N_unnormalized)); // TODO compare to Eps if needed
                blaze::DynamicMatrix<T> N_normalized = normalizePixNormals(N_unnormalized, dz);

                for (size_t c = 0; c < N_unnormalized.columns(); ++c) {
                    blaze::column(normals_theta, c) = blaze::column(N_unnormalized, c) / theta;
                }

                Jac = normalJacByDepth<float>(N_normalized, z0, K, xx, yy, Dx, Dy);
                J_dz = std::get<1>(Jac);
                J_n_un = std::get<0>(Jac);  // for next iter

                sh = normalsToSh(normals_theta, sh_order);

                break;

            } // else (objective > tab_objective && count < maxit_linesearch)
        } // while (true)

        //res_z = blaze::norm(1/t_step* F * (z0 - z_last) - b);
        //res_z = res_z > std::numeric_limits<T>::epsilon() ? res_z : std::numeric_limits<T>::epsilon();  // drop negative?

    } // for (size_t it = 0; it < maxit; ++it)

    theta = dz;  // in the original MAtlab code theta is updated outside the function
}




#endif // _UPS_UPDATE_BLOCKS_HPP

