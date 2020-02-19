/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/

// approximation of probability for multidimensional normal distribution bounded by (hyper)rectangle
// https://arxiv.org/pdf/1111.6832.pdf
// based on local_gaussian.m Matlab code

#ifndef _METRIC_DISTANCE_K_RANDOM_VOI_CPP
#define _METRIC_DISTANCE_K_RANDOM_VOI_CPP

#include <vector>
#include <tuple>
#include <cmath>
#include <limits>
#include <cassert>

#include "../../../3rdparty/blaze/Blaze.h"

#include <iostream> // TODO remove



namespace epmgp {


double inf = std::numeric_limits<double>::infinity();


template <typename T>
int sgn(T val) { // sign for arbitrary type
    return (T(0) < val) - (val < T(0));
}


template <typename T>
T erfcx(T x) {
    return std::exp(x*x)*std::erfc(x);
}


// TODO make template
std::tuple<std::vector<double>, std::vector<double>, std::vector<double>>
truncNormMoments(
        std::vector<double> lowerBIN,
        std::vector<double> upperBIN,
        std::vector<double> muIN,
        std::vector<double> sigmaIN
        )
{
    size_t n = lowerBIN.size();

    assert(upperBIN.size()==n && muIN.size()==n && sigmaIN.size()==n);

    std::vector<double> logZhatOUT (n, 0);
    std::vector<double> muHatOUT (n, 0);
    std::vector<double> sigmaHatOUT (n, 0);

    //double inf = std::numeric_limits<double>::infinity();

    for (size_t i = 0; i<n; ++i) {

        auto lowerB = lowerBIN[i];
        auto upperB = upperBIN[i];
        auto mu = muIN[i];
        auto sigma = sigmaIN[i];

        assert(lowerB<=upperB);

        double logZhat, meanConst, varConst;

        auto a = (lowerB - mu)/(std::sqrt(2*sigma));
        auto b = (upperB - mu)/(std::sqrt(2*sigma));

        if (std::isinf(a) && std::isinf(b)) {
            if (sgn(a)==sgn(b)) {
                logZhatOUT[i] = -inf;
                muHatOUT[i] = a;
                sigmaHatOUT[i] = 0;
                continue;
            } else {
                logZhatOUT[i] = 0;
                muHatOUT[i] = mu;
                sigmaHatOUT[i] = sigma;
                continue;
            }
        } else {
            if (a > b) {
                logZhatOUT[i] = -inf;
                muHatOUT[i] = mu;
                sigmaHatOUT[i] = 0;
                continue;
            } else {
                if (a == -inf) {
                    if (b > 26) {
                        auto logZhatOtherTail = std::log(0.5) + std::log(erfcx(b)) - b*b; // erfcx def= exp(z*z)*erfc(z)
                        logZhat = std::log(1 - std::exp(logZhatOtherTail));
                    }
                    else
                        logZhat = std::log(0.5) + std::log(erfcx(-b)) - b*b;
                    meanConst = -2/erfcx(-b);
                    varConst = -2/erfcx(-b)*(upperB + mu);
                } else {
                    if (b == inf) {
                        if (a < -26) {
                            auto logZhatOtherTail = std::log(0.5) + std::log(erfcx(-a)) - a*a;
                            logZhat = std::log(1 - std::exp(logZhatOtherTail));
                        } else {
                            logZhat = std::log(0.5) + std::log(erfcx(a)) - a*a;
                        }
                        meanConst = 2/erfcx(a);
                        varConst = 2/erfcx(a)*(lowerB + mu);
                    } else {
                        if (sgn(a)==sgn(b)) {
                            auto abs_a = std::abs(a);
                            auto abs_b = std::abs(b);
                            auto maxab = (abs_a < abs_b) ? abs_b : abs_a;
                            auto minab = (abs_a < abs_b) ? abs_a : abs_b;
                            logZhat = std::log(0.5) - minab*minab + std::log( std::abs( std::exp(-(maxab*maxab - minab*minab))*erfcx(maxab) - erfcx(minab) ) );
                            meanConst = 2*sgn(a)*( 1/((erfcx(std::abs(a)) - std::exp(a*a - b*b)*erfcx(std::abs(b)))) - 1/((std::exp(b*b - a*a)*erfcx(std::abs(a)) - erfcx(std::abs(b)))) );
                            varConst =  2*sgn(a)*( (lowerB + mu)/((erfcx(std::abs(a)) - std::exp(a*a - b*b)*erfcx(std::abs(b)))) - (upperB + mu)/((std::exp(b*b - a*a)*erfcx(std::abs(a)) - erfcx(std::abs(b)))) );
                        } else {
                            if (std::abs(b) >= std::abs(a)) {
                                if (a >= -26) {
                                    logZhat = std::log(0.5) - a*a + std::log( erfcx(a) - std::exp(-(b*b - a*a))*erfcx(b) );
                                    meanConst = 2*(1/((erfcx(a) - std::exp(a*a - b*b)*erfcx(b))) - 1/((std::exp(b*b - a*a)*erfcx(a) - erfcx(b))));
                                    varConst = 2*((lowerB + mu)/((erfcx(a) - std::exp(a*a - b*b)*erfcx(b))) - (upperB + mu)/((std::exp(b*b - a*a)*erfcx(a) - erfcx(b))));
                                } else {
                                    logZhat = std::log(0.5) + std::log( 2 - std::exp(-(b*b))*erfcx(b) - std::exp(-(a*a))*erfcx(-a) );
                                    meanConst = 2*( 1/((erfcx(a) - std::exp(a*a - b*b)*erfcx(b))) - 1/(std::exp(b*b)*2 - erfcx(b)) );
                                    varConst = 2*( (lowerB + mu)/((erfcx(a) - std::exp(a*a - b*b)*erfcx(b))) - (upperB + mu)/(std::exp(b*b)*2 - erfcx(b)) );
                                }
                            } else {
                                if (b <= 26) {
                                    logZhat = std::log(0.5) - b*b + std::log( erfcx(-b) - std::exp(-(a*a - b*b))*erfcx(-a));
                                    meanConst = -2*( 1/((erfcx(-a) - std::exp(a*a - b*b)*erfcx(-b))) - 1/((std::exp(b*b - a*a)*erfcx(-a) - erfcx(-b))) );
                                    varConst = -2*( (lowerB + mu)/((erfcx(-a) - std::exp(a*a - b*b)*erfcx(-b))) - (upperB + mu)/((std::exp(b*b - a*a)*erfcx(-a) - erfcx(-b))) );
                                } else {
                                    logZhat = std::log(0.5) + std::log( 2 - std::exp(-(a*a))*erfcx(-a) - std::exp(-(b*b))*erfcx(b) );
                                    meanConst = -2*( 1/(erfcx(-a) - std::exp(a*a)*2) - 1/(std::exp(b*b - a*a)*erfcx(-a) - erfcx(-b)) );
                                    varConst = -2*( (lowerB + mu)/(erfcx(-a) - std::exp(a*a)*2) - (upperB + mu)/(std::exp(b*b - a*a)*erfcx(-a) - erfcx(-b)) );
                                }
                            }
                        }
                    }
                }
            }
        }

        auto muHat = mu + meanConst*std::sqrt(sigma/(2*M_PI));
        auto sigmaHat = sigma + varConst*std::sqrt(sigma/(2*M_PI)) + mu*mu - muHat*muHat;

        logZhatOUT[i] = logZhat;
        muHatOUT[i] = muHat;
        sigmaHatOUT[i] = sigmaHat;

    }

    return std::make_tuple(logZhatOUT, muHatOUT, sigmaHatOUT);
}


// TODO make template
std::tuple<double, blaze::DynamicVector<double>, blaze::DynamicMatrix<double>>
local_gaussian_axis_aligned_hyperrectangles(
        blaze::DynamicVector<double> m,
        blaze::DynamicMatrix<double> K,
        blaze::DynamicVector<double> lowerB,
        blaze::DynamicVector<double> upperB
        )
{
    size_t n = m.size();
    assert(lowerB.size() == n && upperB.size() == n && K.rows() == n);

    size_t maxSteps = 200;
    double epsConverge = 1e-8;

    blaze::DynamicVector<double> tauSite (K.rows(), 0);
    blaze::DynamicVector<double> nuSite (K.rows(), 0);


    double logZ = 0;
    blaze::DynamicVector<double> mu = (lowerB + upperB) / 2.0;
    blaze::DynamicMatrix<double> sigma = K;
    blaze::DynamicVector<double> KinvM = blaze::evaluate(blaze::inv(K) * m); // TODO test!!
    blaze::DynamicVector<double> muLast (mu.size(), 1);
    muLast = muLast * -inf;
    bool converged = false;
    size_t k = 1;

    //blaze::DynamicVector<double> dsigma = blaze::diagonal(sigma);

    // here we only define expressions, no calculations are made
    //auto tauCavity = 1/dsigma - tauSite; // TODO evaluate once blaze::diagonal(sigma)
    //auto nuCavity =  mu/dsigma - nuSite;
    auto tauCavity = 1/blaze::diagonal(sigma) - tauSite; // TODO evaluate once blaze::diagonal(sigma)
    auto nuCavity =  mu/blaze::diagonal(sigma) - nuSite;
    //std::vector<double> sighat;
    //auto deltatauSite = 1/sighat - tauCavity - tauSite;
    blaze::DynamicVector<double> logZhat;
    blaze::DynamicMatrix<double> L;

    while (!converged && k < maxSteps) {

        //blaze::DynamicVector<double> tauCavity = 1/blaze::diagonal(sigma) - tauSite; // TODO evaluate once blaze::diagonal(sigma)
        //blaze::DynamicVector<double> nuCavity =  mu/blaze::diagonal(sigma) - nuSite;

        //blaze::DynamicVector<double> nuCavityValue = evaluate(nuCavity);
        blaze::DynamicVector<double> muInBlaze ( nuCavity * (1/tauCavity) ); // componentwise, TODO check
        blaze::DynamicVector<double> sigmaInBlaze = 1/tauCavity;

        std::vector<double> muInSTL (muInBlaze.size(), 0);
        std::vector<double> sigmaInSTL (sigmaInBlaze.size(), 0);
        std::vector<double> lowerbSTL (lowerB.size(), 0);
        std::vector<double> upperbSTL (upperB.size(), 0);
        assert(muInSTL.size() == n && sigmaInSTL.size() == n); // TODO remove after testing
        for (size_t i = 0; i < n; ++i) {
            muInSTL[i] = muInBlaze[i];
            sigmaInSTL[i] = sigmaInBlaze[i];
            lowerbSTL[i] = lowerB[i];
            upperbSTL[i] = upperB[i];
        }

        auto hat = truncNormMoments(lowerbSTL, upperbSTL, muInSTL, sigmaInSTL);

        auto logZhatSTL = std::get<0>(hat);
        auto muhatSTL = std::get<1>(hat);
        auto sighatSTL = std::get<2>(hat);
        logZhat = blaze::DynamicVector<double>(logZhatSTL.size(), 0);
        blaze::DynamicVector<double> muhat (muhatSTL.size(), 0);
        blaze::DynamicVector<double> sighat (sighatSTL.size(), 0);
        assert(logZhat.size() == n && muhat.size() == n && sighat.size() == n); // TODO remove after testing
        for (size_t i = 0; i < n; ++i) {
            logZhat[i] = logZhatSTL[i];
            muhat[i] = muhatSTL[i];
            sighat[i] = sighatSTL[i];
        }

        auto deltatauSite = 1/sighat - tauCavity - tauSite; // TODO move definition out of loop
        tauSite = blaze::evaluate(tauSite + deltatauSite);
        nuSite = blaze::evaluate(muhat/sighat - nuCavity);

        // here in Matlab code goes 'if any(tauSite)<0' which seems never to be executed

        blaze::DiagonalMatrix<blaze::DynamicMatrix<double>> sSiteHalf (tauSite.size(), 0);
        for (size_t i = 0; i<tauSite.size(); ++i) {
            sSiteHalf(i, i) = std::sqrt(tauSite[i]);
        }
        blaze::IdentityMatrix<double> eye (K.rows());
        L = eye + sSiteHalf*K*sSiteHalf; // TODO debug
        //std::cout << "L:\n" << L << "\n";
        blaze::potrf(L, 'U');
//        L = chol(eye(size(K)) + SsiteHalf*K*SsiteHalf);
        auto V = blaze::inv(blaze::trans(L)) * (sSiteHalf*K);
//        V = L'\(SsiteHalf*K);
        sigma = K - blaze::trans(V)*V;
        mu = sigma*(nuSite + KinvM);

        blaze::DynamicVector<double> diff = muLast - mu;
        //double dotsqr = blaze::evaluate(blaze::trans(diff) * diff);
        if (std::sqrt(blaze::trans(diff) * diff) < epsConverge) // (norm(muLast-mu)) < epsConverge
            converged = true;
        else
            muLast = mu;

        k++;
    }


    // TODO
//    lZ1 = 0.5*sum(log( 1 + tauSite./tauCavity)) - sum(log(diag(L)));
    double lZ1 = 0;
    blaze::DiagonalMatrix<blaze::DynamicMatrix<double>> tau (n, 0);
    blaze::DiagonalMatrix<blaze::DynamicMatrix<double>> diagTauSite (n, 0);
    blaze::DiagonalMatrix<blaze::DynamicMatrix<double>> diagTauCavity (n, 0);

    for (size_t i = 0; i<n; ++i) {
        lZ1 += std::log(1 + tauSite[i]/tauCavity[i])*0.5 - std::log(L(i, i));
        tau(i, i) = 1/(tauCavity[i] + tauSite[i]);
        diagTauSite(i, i) = tauSite[i];
        diagTauCavity(i, i) = tauCavity[i];
    }
//    lZ2 = 0.5*(nuSite - tauSite.*m)'*(sigma - diag(1./(tauCavity + tauSite)))*(nuSite - tauSite.*m);
    blaze::DynamicVector<double> diffSite (nuSite - tauSite*m);
    double lZ2 = 0.5*(blaze::trans(diffSite)*tau*diffSite);
//    lZ3 = 0.5*nuCavity_t*((diag(tauSite) + diag(tauCavity))\(tauSite.*nuCavity./tauCavity - 2*nuSite));
    auto lZ3 = 0.5*blaze::trans(nuCavity)*( blaze::trans(diagTauSite + diagTauCavity)*(tauSite*nuCavity/tauCavity - 2*nuSite) );
//    lZ4 = - 0.5*(tauCavity.*m)'*((diag(tauSite) + diag(tauCavity))\(tauSite.*m - 2*nuSite));
    auto lZ4 = - 0.5*blaze::trans(tauCavity*m)*( blaze::trans(diagTauSite + diagTauCavity)*(tauSite*m - 2*nuSite) );
//    logZ = lZ1 + lZ2 + lZ3 + lZ4 + sum(logZhat);
    logZ = lZ1 + lZ2 + lZ3 + lZ4 + blaze::sum(logZhat);

    return std::make_tuple(logZ, mu, sigma);
}




} // namespace

#endif
