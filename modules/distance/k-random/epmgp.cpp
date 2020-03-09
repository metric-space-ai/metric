/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/

// approximation of probability for multidimensional normal distribution bounded by (hyper)rectangle
// https://arxiv.org/pdf/1111.6832.pdf
// based on local_gaussian.m Matlab code

#ifndef _EPMGP_CPP
#define _EPMGP_CPP

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
T erfcx_simple(T x) { // for double on x86_64, inf starts at -26
    return std::exp(x*x)*std::erfc(x);
}


// https://stackoverflow.com/questions/39777360/accurate-computation-of-scaled-complementary-error-function-erfcx
double erfcx_double (double x)
{
    double a, d, e, m, p, q, r, s, t;

    a = fmax (x, 0.0 - x); // NaN preserving absolute value computation

    /* Compute q = (a-4)/(a+4) accurately. [0,INF) -> [-1,1] */
    m = a - 4.0;
    p = a + 4.0;
    r = 1.0 / p;
    q = m * r;
    t = fma (q + 1.0, -4.0, a);
    e = fma (q, -a, t);
    q = fma (r, e, q);

    /* Approximate (1+2*a)*exp(a*a)*erfc(a) as p(q)+1 for q in [-1,1] */
    p =             0x1.edcad78fc8044p-31;  //  8.9820305531190140e-10
    p = fma (p, q,  0x1.b1548f14735d1p-30); //  1.5764464777959401e-09
    p = fma (p, q, -0x1.a1ad2e6c4a7a8p-27); // -1.2155985739342269e-08
    p = fma (p, q, -0x1.1985b48f08574p-26); // -1.6386753783877791e-08
    p = fma (p, q,  0x1.c6a8093ac4f83p-24); //  1.0585794011876720e-07
    p = fma (p, q,  0x1.31c2b2b44b731p-24); //  7.1190423171700940e-08
    p = fma (p, q, -0x1.b87373facb29fp-21); // -8.2040389712752056e-07
    p = fma (p, q,  0x1.3fef1358803b7p-22); //  2.9796165315625938e-07
    p = fma (p, q,  0x1.7eec072bb0be3p-18); //  5.7059822144459833e-06
    p = fma (p, q, -0x1.78a680a741c4ap-17); // -1.1225056665965572e-05
    p = fma (p, q, -0x1.9951f39295cf4p-16); // -2.4397380523258482e-05
    p = fma (p, q,  0x1.3be1255ce180bp-13); //  1.5062307184282616e-04
    p = fma (p, q, -0x1.a1df71176b791p-13); // -1.9925728768782324e-04
    p = fma (p, q, -0x1.8d4aaa0099bc8p-11); // -7.5777369791018515e-04
    p = fma (p, q,  0x1.49c673066c831p-8);  //  5.0319701025945277e-03
    p = fma (p, q, -0x1.0962386ea02b7p-6);  // -1.6197733983519948e-02
    p = fma (p, q,  0x1.3079edf465cc3p-5);  //  3.7167515521269866e-02
    p = fma (p, q, -0x1.0fb06dfedc4ccp-4);  // -6.6330365820039094e-02
    p = fma (p, q,  0x1.7fee004e266dfp-4);  //  9.3732834999538536e-02
    p = fma (p, q, -0x1.9ddb23c3e14d2p-4);  // -1.0103906603588378e-01
    p = fma (p, q,  0x1.16ecefcfa4865p-4);  //  6.8097054254651804e-02
    p = fma (p, q,  0x1.f7f5df66fc349p-7);  //  1.5379652102610957e-02
    p = fma (p, q, -0x1.1df1ad154a27fp-3);  // -1.3962111684056208e-01
    p = fma (p, q,  0x1.dd2c8b74febf6p-3);  //  2.3299511862555250e-01

    /* Divide (1+p) by (1+2*a) ==> exp(a*a)*erfc(a) */
    d = a + 0.5;
    r = 1.0 / d;
    r = r * 0.5;
    q = fma (p, r, r); // q = (p+1)/(1+2*a)
    t = q + q;
    e = (p - q) + fma (t, -a, 1.0); // residual: (p+1)-q*(1+2*a)
    r = fma (e, r, q);

    /* Handle argument of infinity */
    if (a > 0x1.fffffffffffffp1023) r = 0.0;

    /* Handle negative arguments: erfcx(x) = 2*exp(x*x) - erfcx(|x|) */
    if (x < 0.0) {
        s = x * x;
        d = fma (x, x, -s);
        e = exp (s);
        r = e - r;
        r = fma (e, d + d, r);
        r = r + e;
        if (e > 0x1.fffffffffffffp1023) r = e; // avoid creating NaN
    }
    return r;
}



template <typename T>
T erfcx(T x) { // for double, inf starts at -26 on x86_64
    return (T)erfcx_double((double)x);
}




template <typename T>
std::tuple<std::vector<T>, std::vector<T>, std::vector<T>>
truncNormMoments(
        std::vector<T> lowerBIN,
        std::vector<T> upperBIN,
        std::vector<T> muIN,
        std::vector<T> sigmaIN
        )
{
    size_t n = lowerBIN.size();

    assert(upperBIN.size()==n && muIN.size()==n && sigmaIN.size()==n);

    std::vector<T> logZhatOUT (n, 0);
    std::vector<T> muHatOUT (n, 0);
    std::vector<T> sigmaHatOUT (n, 0);

    for (size_t i = 0; i<n; ++i) {

        auto lowerB = lowerBIN[i];
        auto upperB = upperBIN[i];
        auto mu = muIN[i];
        auto sigma = sigmaIN[i];

        assert(lowerB<=upperB);

        T logZhat, meanConst, varConst;

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
                        auto logZhatOtherTail = std::log(0.5) + std::log(erfcx(b)) - b*b;
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
                            //logZhat = 0; // lim[-Inf]logZhat = 0
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


template <typename T>
std::tuple<T, blaze::DynamicVector<T>, blaze::DynamicMatrix<T>>
local_gaussian_axis_aligned_hyperrectangles(
        blaze::DynamicVector<T> m,
        blaze::DynamicMatrix<T> K,
        blaze::DynamicVector<T> lowerB,
        blaze::DynamicVector<T> upperB
        )
{
    size_t n = m.size();
    assert(lowerB.size() == n && upperB.size() == n && K.rows() == n);

    size_t maxSteps = 200;
    T epsConverge = 1e-8;

    blaze::DynamicVector<T> tauSite (K.rows(), 0);
    blaze::DynamicVector<T> nuSite (K.rows(), 0);


    T logZ = 0;
    blaze::DynamicVector<T> mu = (lowerB + upperB) / 2.0;
    blaze::DynamicMatrix<T> sigma = K;
    blaze::DynamicVector<T> KinvM = blaze::evaluate(blaze::inv(K) * m); // TODO test!!
    blaze::DynamicVector<T> muLast (mu.size(), 1);
    muLast = muLast * -inf;
    bool converged = false;
    size_t k = 1;

    // here we only define expressions, no calculations are made
    auto tauCavity = 1/blaze::diagonal(sigma) - tauSite; // TODO evaluate once blaze::diagonal(sigma)
    auto nuCavity =  mu/blaze::diagonal(sigma) - nuSite;
    blaze::DynamicVector<T> sighat (n, 0); // TODO test well
    auto deltatauSite = 1.0/sighat - tauCavity - tauSite; // TODO test
    auto logZhat = blaze::DynamicVector<T>(n, 0);
    blaze::DynamicMatrix<T> L;

    std::vector<T> muInSTL (n, 0);
    std::vector<T> sigmaInSTL (n, 0);
    std::vector<T> lowerbSTL (lowerB.size(), 0);
    std::vector<T> upperbSTL (upperB.size(), 0);

    while (!converged && k < maxSteps) {
        blaze::DynamicVector<T> muInBlaze ( nuCavity * (1/tauCavity) ); // componentwise, TODO check
        blaze::DynamicVector<T> sigmaInBlaze = 1/tauCavity;

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
        blaze::DynamicVector<T> muhat (muhatSTL.size(), 0);
        //blaze::DynamicVector<double> sighat (sighatSTL.size(), 0); // moved outside loop
        assert(logZhat.size() == n && muhat.size() == n && sighat.size() == n); // TODO remove after testing
        for (size_t i = 0; i < n; ++i) {
            logZhat[i] = logZhatSTL[i];
            muhat[i] = muhatSTL[i];
            sighat[i] = sighatSTL[i];
        }

        //auto deltatauSite = 1.0/sighat - tauCavity - tauSite; // definition moved out of loop
        tauSite = blaze::evaluate(tauSite + deltatauSite);
        nuSite = blaze::evaluate(muhat/sighat - nuCavity);

        // here in Matlab code goes 'if any(tauSite)<0' which seems never to be executed

        blaze::DiagonalMatrix<blaze::DynamicMatrix<T>> sSiteHalf (tauSite.size(), 0);
        for (size_t i = 0; i<tauSite.size(); ++i) {
            sSiteHalf(i, i) = std::sqrt(tauSite[i]);
        }
        blaze::IdentityMatrix<T> eye (K.rows());
        blaze::llh(eye + sSiteHalf*K*sSiteHalf, L);
        L = blaze::trans(L); // get lower from upper
        //L = eye + sSiteHalf*K*sSiteHalf; // TODO remove
        //std::cout << "L:\n" << L << "\n";
        //blaze::potrf(L, 'U'); // LAPACK issue
        //std::cout << "L_chol:\n" << L << "\n";
        auto V = blaze::inv(blaze::trans(L)) * (sSiteHalf*K);
        sigma = K - blaze::trans(V)*V;
        mu = sigma*(nuSite + KinvM);

        blaze::DynamicVector<T> diff = muLast - mu;
        //double dotsqr = blaze::evaluate(blaze::trans(diff) * diff);
        if (std::sqrt(blaze::trans(diff) * diff) < epsConverge) // (norm(muLast-mu)) < epsConverge
            converged = true;
        else
            muLast = mu;

        k++;
    }

    T lZ1 = 0;
    blaze::DiagonalMatrix<blaze::DynamicMatrix<T>> tau (n, 0);
    blaze::DiagonalMatrix<blaze::DynamicMatrix<T>> diagTauSite (n, 0);
    blaze::DiagonalMatrix<blaze::DynamicMatrix<T>> diagTauCavity (n, 0);
    //blaze::DynamicMatrix<double> diagTauSite (n, n, 0);
    //blaze::DynamicMatrix<double> diagTauCavity (n, n, 0);

    for (size_t i = 0; i<n; ++i) {
        lZ1 += std::log(1 + tauSite[i]/tauCavity[i])*0.5 - std::log(L(i, i));
        tau(i, i) = 1/(tauCavity[i] + tauSite[i]);
        diagTauSite(i, i) = tauSite[i];
        diagTauCavity(i, i) = tauCavity[i];
    }
    blaze::DynamicVector<T> diffSite (nuSite - tauSite*m);
    T lZ2 = 0.5*(blaze::trans(diffSite)*(sigma-tau)*diffSite);
    auto lZ3 = 0.5*( blaze::trans(nuCavity)*( blaze::inv(diagTauSite + diagTauCavity)*(tauSite*nuCavity/tauCavity - 2*nuSite) ) );
    auto lZ4 = - 0.5*( blaze::trans(tauCavity*m)*( blaze::inv(diagTauSite + diagTauCavity)*(tauSite*m - 2*nuSite) ) );
    logZ = lZ1 + lZ2 + lZ3 + lZ4 + blaze::sum(logZhat);

    return std::make_tuple(logZ, mu, sigma);
}




} // namespace

#endif  // _EPMGP_CPP

