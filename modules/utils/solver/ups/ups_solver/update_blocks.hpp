
#ifndef _UPS_UPDATE_ALBEDO_HPP
#define _UPS_UPDATE_ALBEDO_HPP

#include "normals_to_sh.hpp"
#include "depth_to_normals.hpp"
#include "calc_energy_cauchy.hpp"

#include "modules/utils/solver/helper/lapwrappers.hpp"
#include "modules/utils/solver/pcg.hpp"

#include "modules/utils/solver/approxchol.hpp"

#include "3rdparty/blaze/Blaze.h"

#include <tuple>
#include <iostream>  // TODO remove



template <typename T>
std::vector<blaze::DynamicVector<T>>
updateAlbedo(
        const std::vector<std::vector<blaze::DynamicVector<T>>> & flat_imgs,
        const std::vector<blaze::DynamicVector<T>> & rho,
        const blaze::DynamicMatrix<T> & N,  // sh
        const std::vector<std::vector<blaze::DynamicVector<T>>> & s,
        const std::vector<std::vector<blaze::DynamicVector<T>>> & reweight,
        const blaze::CompressedMatrix<T> & G,
        //const blaze::DynamicMatrix<T> & G,
        T huber,
        T mu,
        bool regular,
        float tol,  // TODO check type!!
        float maxit    // TODO check type!!
        )
{

    size_t nimages = flat_imgs.size();
    size_t nchannels = flat_imgs[0].size();
    size_t npix = flat_imgs[0][0].size();

    std::vector<blaze::DynamicVector<T>> rho_out = {};

    for (size_t ch = 0; ch < nchannels; ++ch) {
        //blaze::CompressedMatrix<T> A (npix, npix);
        //blaze::DynamicMatrix<T> A (npix, npix, 0);
        blaze::CompressedMatrix<T, blaze::columnMajor> A (npix, npix);
        if (regular) {
            blaze::CompressedMatrix<T> Dk (G.rows(), G.rows());
            //blaze::DynamicMatrix<T> Dk (G.rows(), G.rows(), 0);
            auto c = blaze::abs(G * rho[ch]);
            blaze::DynamicVector<T>  diag_values = map( c, [huber](T el) { return 1.0/(el > huber ? el : huber); } );
            for (size_t i = 0; i < G.rows(); ++i) {
                Dk(i, i) = diag_values[i];
            }
            //std::cout << "Dk:" << std::endl << Dk << std::endl << std::endl;
            //std::cout << "Dk max:" << std::endl << blaze::max(Dk) << std::endl << std::endl;
            A = A + mu * blaze::trans(G) * Dk * G;
            //std::cout << "A:" << std::endl << A << std::endl << std::endl;
            //std::cout << "A max:" << std::endl << blaze::max(A) << std::endl << std::endl;
            //std::cout << "G max:" << std::endl << blaze::max(G) << std::endl << std::endl;
            //std::cout << "mu:" << std::endl << mu << std::endl << std::endl;
        }
        //auto b = blaze::CompressedVector<T>(rho[0].size());
        auto b = blaze::DynamicVector<T>(rho[0].size(), 0);
        for (size_t im = 0; im < nimages; ++im) {
            // reweight[i][ch]
            auto Ns = N * s[im][ch];
            blaze::DynamicVector<T> nnNs = map( Ns, [](T el) { return el > 0 ? el : 0; } );
            blaze::DynamicVector<T> diag_values = blaze::sqrt(2*reweight[im][ch])*nnNs;
            //std::cout << "diag_values:" << std::endl << diag_values << std::endl << std::endl;
            blaze::CompressedMatrix<T> Ai (npix, npix);
            //blaze::DynamicMatrix<T> Ai (npix, npix, 0);
            for (size_t i = 0; i < npix; ++i) {
                Ai(i, i) = diag_values[i];
            }
            //std::cout << "Ai:" << std::endl << Ai << std::endl << std::endl;
            //std::cout << "Ai max:" << std::endl << blaze::max(Ai) << std::endl << std::endl; // slightly differs from reference, TODO debug
            A = A + blaze::trans(Ai) * Ai;
            b = b + blaze::trans(Ai) * (blaze::sqrt(2*reweight[im][ch])*flat_imgs[im][ch]);
        }
        //std::cout << "A:" << std::endl << A << std::endl << std::endl;
        //std::cout << "resulting A max:" << std::endl << blaze::max(A) << std::endl << std::endl;
        //std::cout << "b" << std::endl << b << std::endl << std::endl;

        //* // TODO debug and enable
        std::vector<size_t> pcgIts = {};
        //auto Pre = blaze::IdentityMatrix<T>(A.rows());
        blaze::CompressedMatrix<T> Pre (A.rows(), A.rows());
        for (size_t p = 0; p<npix; ++p) {
            T d_val = A(p, p);
            d_val = d_val > 0 ? d_val : 1;
            Pre(p, p) = d_val; // > 0 ? d_val : 1;
        }
        //std::cout << "Pre" << std::endl << Pre << std::endl << std::endl;
        //std::cout << std::endl << "starting PCG for albedo..." << std::endl;

        //metric::LLmatp<T> llmat (A);
        //metric::LDLinv<T> ldli = metric::approxChol(llmat);
        //metric::SolverB<T> F = [=](const blaze::DynamicVector<T>& b) { return ldli.LDLsolver(b); };

        //blaze::DynamicMatrix<T> AA = A;  // TODO remove
        auto start_time = std::chrono::steady_clock::now(); // TODO disable
        //blaze::DynamicVector<T> X = metric::pcg<T>(A, b, Pre, pcgIts, tol, maxit);
        blaze::DynamicVector<T> X = metric::pcg<T>(A, b, Pre, pcgIts, tol, maxit);
        //blaze::DynamicVector<T> X = metric::pcg<T>(A, b, F, pcgIts, tol, maxit);
        //blaze::DynamicVector<T> X = blaze::solve(AA, b);  // TODO disable
        auto end_time = std::chrono::steady_clock::now();
        std::cout << "PCG for albedo completed in " <<
                     double(std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count()) / 1000000 << " s" <<
                     std::endl << std::endl;
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

        //std::cout << "X:" << std::endl << X << std::endl << std::endl;
        rho_out.push_back(X);
    }

    return rho_out;
}




template <typename T>
std::tuple<std::vector<std::vector<blaze::DynamicVector<T>>>, T>
updateLighting(
        const std::vector<std::vector<blaze::DynamicVector<T>>> & flat_imgs,
        const std::vector<blaze::DynamicVector<T>> & rho,
        const blaze::DynamicMatrix<T> & N,  // sh
        const std::vector<std::vector<blaze::DynamicVector<T>>> & s,
        const std::vector<std::vector<blaze::DynamicVector<T>>> & reweight,
        float tol,  //T nb_nonsingular  // TODO check type!!
        float maxit    // TODO check type!!

        )
{

    size_t nimages = flat_imgs.size();
    size_t nchannels = flat_imgs[0].size();
    size_t npix = flat_imgs[0][0].size();

    std::vector<std::vector<blaze::DynamicVector<T>>> s_out = {};
    T res_s = 0;
    //std::vector<std::vector<blaze::DynamicVector<T>>> reweighted_rho = {};
    //std::vector<std::vector<blaze::DynamicVector<T>>> reweighted_img = {};

    for (size_t im = 0; im < nimages; ++im) {
        //reweighted_rho.push_back(std::vector<blaze::DynamicVector<T>>());  // TODO consider removal
        //reweighted_img.push_back(std::vector<blaze::DynamicVector<T>>());
        s_out.push_back(std::vector<blaze::DynamicVector<T>>());
        for (size_t ch = 0; ch < nchannels; ++ch) {
            auto r_sq = blaze::sqrt(reweight[im][ch]);
            blaze::DynamicVector<T> rho_i_c = r_sq * rho[ch];
            blaze::DynamicVector<T> img_i_c = r_sq * flat_imgs[im][ch];
            //reweighted_rho[im].push_back(rho_i_c);
            //reweighted_img[im].push_back(img_i_c);

            blaze::DynamicVector<T> l = N * s[im][ch];
            for (size_t i = 0; i < l.size(); ++i) {
                if (l[i] < 0) {
                    rho_i_c[i] = 0;
                    img_i_c[i] = 0;
                }
            }
            //std::cout << std::endl << "rho:" << std::endl << rho[ch] << std::endl;  // TODO remove
            //std::cout << std::endl << "img:" << std::endl << flat_imgs[im][ch] << std::endl;  // TODO remove
            //std::cout << std::endl << "weighted_rho:" << std::endl << rho_i_c << std::endl;  // TODO remove
            //std::cout << std::endl << "weighted_img:" << std::endl << img_i_c << std::endl;

            blaze::DynamicMatrix<T> rhon (N.rows(), N.columns());
            for (size_t c = 0; c < N.columns(); ++c) {
                blaze::column(rhon, c) = rho_i_c * blaze::column(N, c);  // elementwise
            }

            auto A = blaze::trans(rhon)*rhon;
            auto b = blaze::trans(rhon)*img_i_c;
            //std::cout << std::endl << "A:" << std::endl << A << std::endl;
            //std::cout << std::endl << "b:" << std::endl << b << std::endl;

            //auto PInv = blaze::inv(blaze::trans(A) * A) * blaze::trans(A);  // no, we need pinv since matrix can be singular!
            //blaze::DynamicVector<T> s_ch = PInv * b;

            //blaze::DynamicVector<T> s_ch = blaze::solve(A, b);  // TODO disable

            std::vector<size_t> pcgIts = {};  // TODO enable
            auto Pre = blaze::IdentityMatrix<T>(A.rows());
            blaze::DynamicVector<T> s_ch = metric::pcg<T>(A, b, Pre, pcgIts, tol, maxit);  // TODO enable


            s_out[im].push_back(s_ch);
            res_s += blaze::norm(A * s_ch - b);

            //std::cout << std::endl << "PInv:" << std::endl << PInv << std::endl;
            //std::cout << std::endl << "updated s:" << std::endl << s_ch << std::endl;
        }
    }

    //std::cout << std::endl << "updated lighting" << std::endl;

    return std::make_tuple(s_out, res_s);

}







// calcEnergyPhotometricTerm(rho_w, I_w, s, N_unnormalized, params, u, dz, theta, options, J_n_un, J_dz)

template <typename T>
std::tuple<
        //std::vector<std::vector<blaze::DynamicVector<T>>>,
        blaze::DynamicVector<T>,
        blaze::DynamicVector<T>,
        //blaze::CompressedMatrix<T>,
        //blaze::CompressedMatrix<T>
        blaze::DynamicMatrix<T>,
        blaze::DynamicMatrix<T>
        >
calcEnergyPhotometricTerm(
        const std::vector<std::vector<blaze::DynamicVector<T>>> & rho_w,
        const std::vector<std::vector<blaze::DynamicVector<T>>> & I_w,
        const std::vector<std::vector<blaze::DynamicVector<T>>> & s,
        const blaze::DynamicVector<T> & theta,
        const blaze::DynamicVector<T> & dz,
        const blaze::DynamicMatrix<T> & N_unnormalized,
        const blaze::DynamicVector<T> & u,
        //const std::vector<blaze::CompressedMatrix<T>> & J_n_un,
        //const blaze::CompressedMatrix<T> & J_dz,
        const std::vector<blaze::DynamicMatrix<T>> & J_n_un,
        const blaze::DynamicMatrix<T> & J_dz,
        // params, options
        T beta, // = 5e-4
        harmo_order ho = ho_low
        )
{
    //std::cout << std::endl << "calcEnergyPhotometricTerm started" << std::endl;

    size_t nimages = I_w.size();
    size_t nchannels = I_w[0].size();
    size_t npix = I_w[0][0].size();

    blaze::DynamicMatrix<T> normals_theta (N_unnormalized.rows(), N_unnormalized.columns());
    for (size_t c = 0; c < N_unnormalized.columns(); ++c) {
        blaze::column(normals_theta, c) = blaze::column(N_unnormalized, c) / theta;
    }  // TODO pass theta!

    //blaze::CompressedMatrix<T> theta_1_diag (theta.size(), theta.size());
    blaze::DynamicMatrix<T> theta_1_diag (theta.size(), theta.size(), 0);
    blaze::diagonal(theta_1_diag) = 1/theta;
    //std::cout << std::endl << "theta_1_diag:" << std::endl << theta_1_diag << std::endl;

    //blaze::CompressedMatrix<T> J_n_0 = theta_1_diag * J_n_un[0];
    //blaze::CompressedMatrix<T> J_n_1 = theta_1_diag * J_n_un[1];
    //blaze::CompressedMatrix<T> J_n_2 = theta_1_diag * J_n_un[2];
    blaze::DynamicMatrix<T> J_n_0 = theta_1_diag * J_n_un[0];
    blaze::DynamicMatrix<T> J_n_1 = theta_1_diag * J_n_un[1];
    blaze::DynamicMatrix<T> J_n_2 = theta_1_diag * J_n_un[2];

    //std::cout << std::endl << "J_n_0:" << std::endl << J_n_0 << std::endl;
    //std::cout << std::endl << "J_n_1:" << std::endl << J_n_1 << std::endl;
    //std::cout << std::endl << "J_n_2:" << std::endl << J_n_2 << std::endl;

    auto J_sh = calcJacobianWrtNormals(normals_theta, ho, J_n_0, J_n_1, J_n_2);

    //std::cout << std::endl << "J_sh:" << std::endl << J_sh << std::endl;

    auto sh = normalsToSh(normals_theta, ho);

    blaze::DynamicVector<T> cost_aug = sqrt(0.5*beta) * (theta - dz + u);

    //std::cout << std::endl << "cost_aug:" << std::endl << cost_aug << std::endl;

    //Matlab code: J_aug = -spdiags(repmat(sqrt(0.5*params.beta),size(theta,1),1), 0, size(theta,1), size(theta,1)) * J_dz;  % jacobian of lagrangian
    //blaze::CompressedMatrix<T> J_aug = -J_dz * sqrt(0.5*beta);  // TODO check simplification by tests!
    blaze::DynamicMatrix<T> J_aug = -J_dz * sqrt(0.5*beta);  // TODO check simplification by tests!

    //blaze::CompressedMatrix<T> J_cauchy (nimages*nchannels*npix, npix);
    blaze::DynamicMatrix<T> J_cauchy (nimages*nchannels*npix, npix, 0);

    //blaze::DynamicVector<size_t> row_vec (nimages*nchannels*npix, 0);
    //blaze::DynamicVector<size_t> col_vec (nimages*nchannels*npix, 0);
    //blaze::DynamicVector<T> val_vec (nimages*nchannels*npix, 0);
    std::vector<size_t> row_vec = {};
    std::vector<size_t> col_vec = {};
    std::vector<T> val_vec = {};
    size_t idx = 0;
    //std::vector<std::vector<blaze::DynamicVector<T>>> cost_cauchy_mat = {};
    blaze::DynamicVector<T> cost_cauchy (nimages*nchannels*npix, 0);

    for (size_t im = 0; im < nimages; ++im) {

        //std::vector<blaze::DynamicVector<T>> cost_im = {};
        for (size_t ch = 0; ch < nchannels; ++ch) {
             blaze::DynamicVector<T> cost_im_ch = rho_w[im][ch]*(sh * s[im][ch]) - I_w[im][ch];
             //cost_im.push_back(cost_im_ch);
             //std::cout << std::endl << "cost_im_ch:" << std::endl << cost_im_ch << std::endl;
             blaze::subvector(cost_cauchy, idx, npix) = cost_im_ch;
             idx += cost_im_ch.size();

             //Jacobian of photometric term
             //blaze::CompressedMatrix<T> J_sh_ =
             blaze::DynamicMatrix<T> J_sh_ =
                     J_sh[0] * s[im][ch][0] +
                     J_sh[1] * s[im][ch][1] +
                     J_sh[2] * s[im][ch][2] +
                     J_sh[3] * s[im][ch][3];
             if (ho != ho_low) {
                 J_sh_ = J_sh_ +
                         J_sh[4] * s[im][ch][4] +
                         J_sh[5] * s[im][ch][5] +
                         J_sh[6] * s[im][ch][6] +
                         J_sh[7] * s[im][ch][7] +
                         J_sh[8] * s[im][ch][8];
             }
             //std::cout << std::endl << "J_sh_:" << std::endl << J_sh_ << std::endl;

             //if (im*nchannels + ch == 0) { // consider simply moving out of ch loop
                 // TODO
             //}
             //blaze::CompressedMatrix<T> rho_w_diag (npix, npix);
             blaze::DynamicMatrix<T> rho_w_diag (npix, npix, 0);
             blaze::diagonal(rho_w_diag) = rho_w[im][ch];
             blaze::CompressedMatrix<T, blaze::columnMajor> rho_w_J_sh_ = rho_w_diag * J_sh_;
             //blaze::DynamicMatrix<T, blaze::columnMajor> rho_w_J_sh_ = rho_w_diag * J_sh_;

             //std::cout << std::endl << "rho_w_J_sh_:" << std::endl << rho_w_J_sh_ << std::endl;  // TODO remove

             size_t cnt = 0;
             //blaze::DynamicVector<size_t> row_vec_temp (rho_w_J_sh_.nonZeros(), 0);
             //blaze::DynamicVector<size_t> col_vec_temp (rho_w_J_sh_.nonZeros(), 0);
             //blaze::DynamicVector<T> val_vec_temp (rho_w_J_sh_.nonZeros(), 0);
             //std::vector<size_t> row_vec_temp = {};
             //std::vector<size_t> col_vec_temp = {};
             //std::vector<T> val_vec_temp = {};
             for (size_t c = 0; c < rho_w_J_sh_.columns(); ++c) {
                 for (typename blaze::CompressedMatrix<T, blaze::columnMajor>::Iterator it=rho_w_J_sh_.begin(c); it!=rho_w_J_sh_.end(c); ++it) {
                 //for (typename blaze::DynamicMatrix<T, blaze::columnMajor>::Iterator it=rho_w_J_sh_.begin(c); it!=rho_w_J_sh_.end(c); ++it) {
                     //row_vec_temp[cnt] = it->index();
                     //col_vec_temp[cnt] = c;
                     //val_vec_temp[cnt] = it->value();
                     row_vec.push_back(it->index() + (3*im + ch)*npix);
                     col_vec.push_back(c);
                     val_vec.push_back(it->value());
                     ++cnt;
                     //std::cout << c << ", " << it->index() << ", " << it->value() << std::endl;  // TODO remove
                 }
             }

             //std::cout << std::endl << "row_vec_temp:" << std::endl << row_vec_temp << std::endl;
             //std::cout << std::endl << "col_vec_temp:" << std::endl << col_vec_temp << std::endl;
             //std::cout << std::endl << "val_vec_temp:" << std::endl << val_vec_temp << std::endl;
             //std::cout << "val_vec_temp size:" << val_vec_temp.size() << std::endl;

             //blaze::subvector(row_vec, idx, row_vec_temp.size()) = row_vec_temp + (3*im + ch)*npix;
             //blaze::subvector(col_vec, idx, row_vec_temp.size()) = col_vec_temp;
             //blaze::subvector(val_vec, idx, row_vec_temp.size()) = val_vec_temp;
             //idx += row_vec_temp.size();
        }
        //cost_cauchy_mat.push_back(cost_im);
    }

    //std::cout << std::endl << "row_vec:" << std::endl << row_vec << std::endl;
    //std::cout << std::endl << "col_vec:" << std::endl << col_vec << std::endl;
    //std::cout << std::endl << "val_vec:" << std::endl << val_vec << std::endl;

    for (size_t el = 0; el < row_vec.size(); el++) {
        J_cauchy(row_vec[el], col_vec[el]) = val_vec[el];
    }

    //std::cout << std::endl << "J_cauchy:" << std::endl << J_cauchy << std::endl;

    //return std::make_tuple(cost_cauchy_mat, cost_aug, J_cauchy, J_aug);
    return std::make_tuple(cost_cauchy, cost_aug, J_cauchy, J_aug);

}







template <typename T>
std::tuple<
        blaze::DynamicVector<T>, // z
        blaze::DynamicVector<T>, // zx
        blaze::DynamicVector<T>, // zy
        blaze::DynamicVector<T>, // dz
        blaze::DynamicMatrix<T>, // N_unnormalized
        blaze::DynamicMatrix<T>, // sh
        //blaze::CompressedMatrix<T>, // J_dz
        blaze::DynamicMatrix<T>, // J_dz
        T // res_z
        >
updateDepth(
        const std::vector<std::vector<blaze::DynamicVector<T>>> & flat_imgs,
        const std::vector<blaze::DynamicVector<T>> & rho,
        const std::vector<std::vector<blaze::DynamicVector<T>>> & s,
        const blaze::DynamicVector<T> & theta,
        const blaze::DynamicVector<T> & z_vector_masked,
        blaze::DynamicVector<T> zx,
        blaze::DynamicVector<T> zy,
        const blaze::DynamicVector<T> & u,
        const std::vector<std::vector<blaze::DynamicVector<T>>> & reweight,
        const std::vector<blaze::DynamicVector<T>> & drho,
        const blaze::DynamicMatrix<T> & K,
        const blaze::DynamicVector<T> & xx,
        const blaze::DynamicVector<T> & yy,
        const blaze::CompressedMatrix<T> & Dx,
        const blaze::CompressedMatrix<T> & Dy,
        //const blaze::DynamicMatrix<T> & Dx,
        //const blaze::DynamicMatrix<T> & Dy,
        size_t maxit = 3,  // option options.LinS.maxit,
        T beta = 5e-4,
        T t = 10,
        size_t maxit_linesearch = 1000,
        float pcg_tol = 1e-10,
        float pcg_maxit = 1000,
        harmo_order sh_order = ho_low
        )
{

    size_t nimages = flat_imgs.size();
    size_t nchannels = flat_imgs[0].size();
    //size_t npix = flat_imgs[0][0].size();

    blaze::DynamicVector<T> z0 = z_vector_masked;
    blaze::DynamicVector<T> z_last;

//    auto gradients = getMaskedGradients(Z, M, K);
//    blaze::DynamicVector<T> z_vector_masked = std::get<0>(gradients);  // Matlab z = z(mask);
//    blaze::DynamicVector<T> zx = std::get<1>(gradients);
//    blaze::DynamicVector<T> zy = std::get<2>(gradients);
//    blaze::DynamicVector<T> xx = std::get<3>(gradients);
//    blaze::DynamicVector<T> yy = std::get<4>(gradients);
//    blaze::CompressedMatrix<T> Dx = std::get<5>(gradients);
//    blaze::CompressedMatrix<T> Dy = std::get<6>(gradients);

    auto normals = getNormalMap<T>(z_vector_masked, zx, zy, K, xx, yy);
    auto N_unnormalized = std::get<2>(normals);
    auto dz = std::get<1>(normals);

    //std::cout << std::endl << "N_unnormalized:" << std::endl << N_unnormalized << std::endl;
    //std::cout << std::endl << "theta:" << std::endl << theta << std::endl;

    auto Jac = calcJacobian<float>(std::get<0>(normals), z_vector_masked, K, xx, yy, Dx, Dy);
    auto J_dz = std::get<1>(Jac);
    auto J_n_un = std::get<0>(Jac);

    //std::cout << std::endl << "J_dz:" << std::endl << J_dz << std::endl;
    //std::cout << std::endl << "J_n_un:" << std::endl << J_n_un << std::endl;

    blaze::DynamicMatrix<T> normals_theta (N_unnormalized.rows(), N_unnormalized.columns());
    for (size_t c = 0; c < N_unnormalized.columns(); ++c) {
        blaze::column(normals_theta, c) = blaze::column(N_unnormalized, c) / theta;
    }

    auto sh = normalsToSh(normals_theta, sh_order);

    //std::cout << std::endl << "sh:" << std::endl << sh << std::endl;

    auto tab_ec = calcEnergyCauchy(flat_imgs, rho, s, sh, theta, drho, dz, u);
    T tab_objective = std::get<1>(tab_ec);

    //std::cout << std::endl << "tab_objective: " << tab_objective << std::endl;   // TODO remove

    blaze::DynamicVector<T> z;
    T res_z;

    for (size_t it = 0; it < maxit; ++it) {

        std::vector<std::vector<blaze::DynamicVector<T>>> reweighted_rho = {};  // fullfilled to pass to calcEnergyPhotometricTerm
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

                //std::cout << std::endl << "rho:" << std::endl << rho[ch] << std::endl;  // TODO remove
                //std::cout << std::endl << "img:" << std::endl << flat_imgs[im][ch] << std::endl;
                //std::cout << std::endl << "weighted_rho:" << std::endl << rho_i_c << std::endl;
                //std::cout << std::endl << "weighted_img:" << std::endl << img_i_c << std::endl;
                //std::cout << std::endl << "sh:" << std::endl << sh << std::endl;
                //std::cout << std::endl << "s[im][ch]:" << std::endl << s[im][ch] << std::endl;
                //std::cout << std::endl << "l:" << std::endl << l << std::endl;
            }
        }

        auto ept = calcEnergyPhotometricTerm(reweighted_rho, reweighted_img, s, theta, dz, N_unnormalized, u, J_n_un, J_dz, beta, sh_order);
        //std::vector<std::vector<blaze::DynamicVector<T>>> cost_cauchy = std::get<0>(ept);
        blaze::DynamicVector<T> cost_cauchy = std::get<0>(ept);
        blaze::DynamicVector<T> cost_aug = std::get<1>(ept);
        //blaze::CompressedMatrix<T> J_cauchy = std::get<2>(ept);
        //blaze::CompressedMatrix<T> J_aug = std::get<3>(ept);
        blaze::DynamicMatrix<T> J_cauchy = std::get<2>(ept);
        blaze::DynamicMatrix<T> J_aug = std::get<3>(ept);

        //std::cout << std::endl << "cost_cauchy:" << std::endl << cost_cauchy << std::endl;
        //std::cout << std::endl << "cost_aug:" << std::endl << cost_aug << std::endl;
        //std::cout << std::endl << "J_cauchy:" << std::endl << J_cauchy << std::endl;
        //std::cout << std::endl << "J_aug:" << std::endl << J_aug << std::endl;

        T t_step = 2/(2 + 1/t);
        size_t count = 1;

        auto F = blaze::trans(J_aug)*J_aug + blaze::trans(J_cauchy) * J_cauchy;
        auto b = -blaze::trans(J_aug)*cost_aug - blaze::trans(J_cauchy) * cost_cauchy;

        //std::cout << std::endl << "F:" << std::endl << F << std::endl;   // TODO remove
        //std::cout << std::endl << "b:" << std::endl << b << std::endl;

        //blaze::DynamicMatrix<T> L;  // TODO debug and enable
        //blaze::DynamicMatrix<T> F_dense = F;
        //blaze::llh(F_dense, L);  // result slightly differs, consider parameters using of Cholesky decomposition

        //std::cout << std::endl << "L:" << std::endl << L << std::endl;

        std::vector<size_t> pcgIts = {};
        auto L = blaze::IdentityMatrix<T>(F.rows());  // TODO replace with good preconditioner
        blaze::DynamicVector<T> z_step = metric::pcg<T>(F, b, L, pcgIts, pcg_tol, pcg_maxit);  // TODO debug, add tolerance and maxit
        //std::cout << "z_step:" << std::endl << z_step << std::endl << std::endl;    // TODO remove

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

            //std::cout << std::endl << std::endl << "---- iter " << it << " | "  << count << std::endl << std::endl;  // TODO remove

            z = z0 + t_step*z_step;

            //depthToNormals
            blaze::DynamicVector<T> zx_ = Dx*z;
            blaze::DynamicVector<T> zy_ = Dy*z;
            auto nm = getNormalMap(z, zx_, zy_, K, xx, yy);
            auto dz = std::get<1>(nm);
            N_unnormalized = std::get<2>(nm);
            //normals_theta  = blaze::DynamicMatrix<T>(N_unnormalized.rows(), N_unnormalized.columns());
            for (size_t c = 0; c < N_unnormalized.columns(); ++c) {
                blaze::column(normals_theta, c) = blaze::column(N_unnormalized, c) / theta;
            }
            auto sh = normalsToSh(normals_theta, sh_order);

            //std::cout << "N_unnormalized:" << std::endl << N_unnormalized << std::endl << std::endl;
            //std::cout << "N_norm_th:" << std::endl << normals_theta << std::endl << std::endl;   // TODO remove
            //std::cout << "z0:" << std::endl << z0 << std::endl << std::endl;
            //std::cout << "z:" << std::endl << z << std::endl << std::endl;
            //std::cout << "dz:" << std::endl << dz << std::endl << std::endl;
            //std::cout << "K:" << std::endl << K << std::endl << std::endl;
            //std::cout << "zx:" << std::endl << zx << std::endl << std::endl;
            //std::cout << "zy:" << std::endl << zy << std::endl << std::endl;
            //std::cout << "Dx:" << std::endl << Dx << std::endl << std::endl;
            //std::cout << "Dy:" << std::endl << Dy << std::endl << std::endl;
            //std::cout << "zx_:" << std::endl << zx_ << std::endl << std::endl;
            //std::cout << "zy_:" << std::endl << zy_ << std::endl << std::endl;
            //std::cout << "xx:" << std::endl << xx << std::endl << std::endl;
            //std::cout << "yy:" << std::endl << yy << std::endl << std::endl;
            //std::cout << "sh:" << std::endl << sh << std::endl << std::endl;

            auto ec = calcEnergyCauchy(flat_imgs, rho, s, sh, theta, drho, dz, u);
            T objective = std::get<1>(ec);

            //std::cout << std::endl << "objective: " << objective << std::endl;  // TODO remove

            ++count;

            if (objective > tab_objective && count < maxit_linesearch) {
                t = t*0.5;
                t_step = 2/(2+1/t);

            } else {

                if (objective > tab_objective) {
                    // TODO disable
                    //std::cout << std::endl << "Not found descent step in z update: step " << t_step << std::endl;
                }
                z_last = z0;
                z0 = z;
                //depthToNormals
                zx = Dx*z0; // updated zx, zy will be returned
                zy = Dy*z0;
                nm = getNormalMap(z0, zx, zy, K, xx, yy);
                dz = std::get<1>(nm);
                N_unnormalized = std::get<2>(nm);
                //normals_theta = blaze::DynamicMatrix<T>(N_unnormalized.rows(), N_unnormalized.columns()); // TODO remove
                for (size_t c = 0; c < N_unnormalized.columns(); ++c) {
                    blaze::column(normals_theta, c) = blaze::column(N_unnormalized, c) / theta;
                }

                //std::cout << "N_norm_th iter:" << std::endl << normals_theta << std::endl << std::endl;   // TODO remove
                //std::cout << "z0 iter:" << std::endl << z0 << std::endl << std::endl;   // TODO remove
                //std::cout << "zx iter:" << std::endl << zx << std::endl << std::endl;   // TODO remove
                //std::cout << "zy iter:" << std::endl << zy << std::endl << std::endl;   // TODO remove
                //std::cout << "dz iter:" << std::endl << dz << std::endl << std::endl;   // TODO remove
                //std::cout << "N_unnormalized iter:" << std::endl << N_unnormalized << std::endl << std::endl;
                //std::cout << "normals_theta iter:" << std::endl << normals_theta << std::endl << std::endl;

                Jac = calcJacobian<float>(std::get<0>(nm), z0, K, xx, yy, Dx, Dy);
                J_dz = std::get<1>(Jac);
                J_n_un = std::get<0>(Jac);  // for next iter

                auto sh = normalsToSh(normals_theta, sh_order);

                //std::cout << "J_dz iter:" << std::endl << J_dz << std::endl << std::endl;
                //std::cout << "J_n_un iter:" << std::endl << J_n_un << std::endl << std::endl;
                //std::cout << "ls sh iter:" << std::endl << sh << std::endl << std::endl;

                break;
            } //  else (objective > tab_objective && count < maxit_linesearch)

            //break; // TODO remove!!
        } // while (true)

        res_z = blaze::norm(1/t_step* F * (z0 - z_last) - b);
        res_z = res_z > std::numeric_limits<T>::epsilon() ? res_z : std::numeric_limits<T>::epsilon();  // drop negative?

        //std::cout << "res_z: " << res_z << std::endl << std::endl;
        //std::cout << "z: " << z << std::endl << std::endl;

        //std::cout << "F:" << std::endl << F << std::endl << std::endl;   // TODO remove
        //std::cout << "b:" << std::endl << b << std::endl << std::endl;   // TODO remove
        //std::cout << "z0 - z_last:" << std::endl << z0 - z_last << std::endl << std::endl;   // TODO remove
        //std::cout << "1/t_step* F * (z0 - z_last) - b:" << std::endl << 1/t_step* F * (z0 - z_last) - b << std::endl << std::endl;   // TODO remove
        //std::cout << "F * (z0 - z_last) - b:" << std::endl << F * (z0 - z_last) - b << std::endl << std::endl;   // TODO remove
        //std::cout << "F * (z0 - z_last):" << std::endl << F * (z0 - z_last) << std::endl << std::endl;   // TODO remove
        //std::cout << "(F * (z0 - z_last)) / t_step:" << std::endl << (F * (z0 - z_last)) / t_step << std::endl << std::endl;   // TODO remove
        //std::cout << "1/t_step:" << std::endl << 1/t_step << std::endl << std::endl;   // TODO remove
        //std::cout << std::endl << "end of outer iter" << std::endl;

    } // for (size_t it = 0; it < maxit; ++it)

    return std::make_tuple(z, zx, zy, dz, N_unnormalized, sh, J_dz, res_z);

}




#endif // _UPS_UPDATE_ALBEDO_HPP

