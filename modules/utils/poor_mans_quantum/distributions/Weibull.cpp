/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2019 Panda Team
*/

#ifndef _METRIC_UTILS_POOR_MANS_QUANTUM_DISTRIBUTIONS_WEIBULL_CPP
#define _METRIC_UTILS_POOR_MANS_QUANTUM_DISTRIBUTIONS_WEIBULL_CPP

#include <vector>
#include <random>
#include <cmath>

namespace metric {

inline Weibull::Weibull()
    : _generator(std::random_device {}())
{
}

/*** random sampling ***/
inline float Weibull::rnd()
{
    std::weibull_distribution<float> weibull_dist(_p1, _p2);
    return weibull_dist(_generator);
}

template <typename T>
T gamma(T z);

inline float Weibull::mean() { return 1.f / _p1 * gamma(1.f + 1.f / _p2); }

inline float Weibull::quantil(float p) { return icdf(p); }

/*** pdf ***/
inline float Weibull::pdf(const float x)
{
    const float z = x / _p1;
    const float w = std::exp(-(std::pow(z, _p2)));
    const float y = std::pow(z, (_p2 - 1)) * w * _p2 / _p1;

    if (w == 0)
        return 0;
    else
        return y;
}

/*** cdf ***/
inline float Weibull::cdf(const float x)
{
    const float z = -std::pow(x / _p1, _p2);

    float p;

    if (std::abs(z) < 1e-5f) {
	    p = z + 0.5f * z * z;
    } else {
	    p = -(std::exp(z) - 1.0f);
    }

    return p;
}

/*** icdf ***/
inline float Weibull::icdf(const float x) { return _p1 * std::pow(-std::log(1 - x), 1 / _p2); }

template <typename T>
T ln_gamma(T z)
{
    const T lct[9 + 1]
        = { 0.9999999999998099322768470047347, 676.520368121885098567009190444019, -1259.13921672240287047156078755283,
              771.3234287776530788486528258894, -176.61502916214059906584551354, 12.507343278686904814458936853,
              -0.13857109526572011689554707, 9.984369578019570859563e-6, 1.50563273514931155834e-7 };
    T sum;
    T base;
    const T g_pi = 3.14159265358979323846;
    const T ln_sqrt_2_pi = 0.91893853320467274178;
    if (z < 0.5) {
        return std::log(g_pi / std::sin(g_pi * z)) - ln_gamma(1.0 - z);
    }
    z = z - 1.0;
    base = z + 7.5;
    sum = 0;

    for (int i = 9; i >= 1; i--) {
        sum += lct[i] / (z + ((T)i));
    }
    sum += lct[0];

    return ((ln_sqrt_2_pi + std::log(sum)) - base) + std::log(base) * (z + 0.5);
}

template <typename T>
T gamma(T z)
{
    return std::exp(ln_gamma(z));
}

static float weibull_scale_likelihood(float sigma, std::vector<float>& x, std::vector<float>& w, float xbar, size_t size)
{
    float v;
    std::vector<float> wLocal(size);
    float sumxw;
    float sumw;
    for (size_t i = 0; i < size; i++) {
        wLocal[i] = w[i] * std::exp(x[i] / sigma);
    }
    sumxw = 0;
    sumw = 0;
    for (size_t i = 0; i < size; i++) {
        sumxw += (wLocal[i] * x[i]);
        sumw += wLocal[i];
    }
    v = (sigma + xbar - sumxw / sumw);

    return v;
}

/* based on dfzero from fortan, it finxs the zero in the given search bands, and stops if it is within tolerance. */
static void wdfzero(float* sigmahat, float* likelihood_value, float* err, float* search_bands, float tol,
                    std::vector<float>& x0, std::vector<float>& frequency, float meanUncensored, size_t size)
{
    float exitflag;
    float a, b, c = 0.0, d = 0.0, e = 0.0, m, p, q, r, s;
    float fa, fb, fc;
    float fval;
    float tolerance;
    exitflag = 1;
    *err = exitflag;
    a = search_bands[0];
    b = search_bands[1];
    fa = weibull_scale_likelihood(a, x0, frequency, meanUncensored, size);
    fb = weibull_scale_likelihood(b, x0, frequency, meanUncensored, size);
    if (fa == 0) {
        b = a;
        *sigmahat = b;
        fval = fa;
        *likelihood_value = fval;
        return;
    } else if (fb == 0) {
        fval = fb;
        *likelihood_value = fval;
        *sigmahat = b;
        return;
    } else  // if ((fa > 0) == (fb > 0))
    {
        assert(fa * fb <= 0);
        //std::cout << "ERROR: wdfzero says function values at the interval endpoints must differ in sign\n";
    }
    fc = fb;
    /*Main loop, exit from middle of the loop */
    while (fb != 0) {
        /* Insure that b is the best result so far, a is the previous */
        /* value of b, and that c is  on the opposite size of the zero from b. */
        if ((fb > 0) == (fc > 0)) {
            c = a;
            fc = fa;
            d = b - a;
            e = d;
        }

        float absFC;
        float absFB;
        absFC = std::abs(fc);
        absFB = std::abs(fb);
        if (absFC < absFB) {
            a = b;
            b = c;
            c = a;
            fa = fb;
            fb = fc;
            fc = fa;
        }

        /*set up for test of Convergence, is the interval small enough? */
        m = 0.5f * (c - b);

        float absB, absM, absFA, absE;  //, absFB
        absB = std::abs(b);
        absM = std::abs(m);
        absFA = std::abs(fa);
        absFB = std::abs(fb);
        absE = std::abs(e);

        tolerance = 2.0f * tol * ((absB > 1.0f) ? absB : 1.0f);
        if ((absM <= tolerance) | (fb == 0.0f)) {
	        break;
        }

        /*Choose bisection or interpolation */
        if ((absE < tolerance) | (absFA <= absFB)) {
            /*Bisection */
            d = m;
            e = m;
        } else {
            /*Interpolation */
            s = fb / fa;
            if (a == c) {
                /*Linear interpolation */
                p = 2.0f * m * s;
                q = 1.0f - s;
            } else {
                /*Inverse quadratic interpolation */
                q = fa / fc;
                r = fb / fc;
                p = s * (2.0f * m * q * (q - r) - (b - a) * (r - 1.0f));
                q = (q - 1.0f) * (r - 1.0f) * (s - 1.0f);
            }
            if (p > 0)
                q = -1.0f * q;
            else
                p = -1.0f * p;
        }

        float tempTolerance = tolerance * q;
        float absToleranceQ;
        float absEQ;
        float tempEQ = (0.5f * e * q);
        absToleranceQ = std::abs(tempTolerance);
        absEQ = std::abs(tempEQ);
        /*Is interpolated point acceptable */
        if ((2.0f * p < 3.0f * m * q - absToleranceQ) & (p < absEQ)) {
            e = d;
            d = p / q;
        } else {
            d = m;
            e = m;
        }

        /*Interpolation */
        /*Next point */
        a = b;
        fa = fb;
        if (std::abs(d) > tolerance)
            b = b + d;
        else if (b > c)
            b = b - tolerance;
        else
            b = b + tolerance;
        fb = weibull_scale_likelihood(b, x0, frequency, meanUncensored, size);
    } /*Main loop (While) */
    fval = weibull_scale_likelihood(b, x0, frequency, meanUncensored, size);
    *likelihood_value = fval;
    *sigmahat = b;
    return;
}

inline std::tuple<float, float> weibull_fit(const std::vector<float>& inputData)
{
    size_t size = inputData.size();
    std::vector<float> data(size);
    float alpha = 10;

    float PI = 3.141592653589793238462f;
    float tol = 1e-6f; /* this impacts the non-linear estimation..
                       if your problem is highly unstable (small scale) this might be made larger
                       but we never recommend anything greater than 10e-5.
                       Also if larger it will converge faster, so if yo can live with lower accuracy, you can change it */
    float n;
    float nuncensored = 0;
    float ncensored = 0;
    std::vector<float> censoring(size);
    std::vector<float> frequency(size);
    std::vector<float> var(size);
    std::vector<float> x0(size);

    /*set frequency to all 1.0's */
    /*and censoring to 0.0's */
    for (size_t i = 0; i < size; i++) {
        frequency[i] = 1.0;
        censoring[i] = 0.0;
    }

    for (size_t i = 0; i < size; i++) {
        data[i] = std::log(inputData[i]);
    }

    float mySum;
    mySum = 0;
    for (size_t i = 0; i < size; i++) {
        mySum += frequency[i];
    }
    n = mySum;

    {
        mySum = 0;
        for (size_t i = 0; i < size; i++) {
            mySum += (frequency[i] * censoring[i]);
        }
        ncensored = mySum;
        nuncensored = n - ncensored;
    }

    /* declar local for max/range computation */

    float maxVal, minVal;
    float range, maxx;
    float tempVal;
    maxVal = -1000000000;
    minVal = 1000000000;
    for (size_t i = 0; i < size; i++) {
        tempVal = data[i];
        if (tempVal < minVal)
            minVal = tempVal;
        if (tempVal > maxVal)
            maxVal = tempVal;
    }
    range = maxVal - minVal;
    maxx = maxVal;
    /*Shift x to max(x) == 0, min(x) = -1 to make likelihood eqn more stable. */

    float mean, myStd;
    float sigmahat;
    float meanUncensored;
    float upper, lower;
    float search_band[2];
    for (size_t i = 0; i < size; i++) {
        x0[i] = (data[i] - maxx) / range;
    }
    mean = 0;
    myStd = 0;
    for (size_t i = 0; i < size; i++) {
        mean += x0[i];
    }
    mean /= n;
    for (size_t i = 0; i < size; i++) {
        var[i] = x0[i] - mean;
    }
    for (size_t i = 0; i < size; i++) {
        myStd += var[i] * var[i];
    }
    myStd /= (n - 1);
    myStd = std::sqrt(myStd);
    sigmahat = (std::sqrt(6.0f) * myStd) / PI;
    meanUncensored = 0;
    for (size_t i = 0; i < size; i++) {
        meanUncensored += (frequency[i] * x0[i]) / n;
    }
    if ((tempVal = weibull_scale_likelihood(sigmahat, x0, frequency, meanUncensored, size)) > 0) {
        upper = sigmahat;
        lower = 0.5f * upper;
        while ((tempVal = weibull_scale_likelihood(lower, x0, frequency, meanUncensored, size)) > 0) {
            upper = lower;
            lower = 0.5f * upper;
        }
    } else {
        lower = sigmahat;
        upper = 2.0f * lower;
        while ((tempVal = weibull_scale_likelihood(upper, x0, frequency, meanUncensored, size)) < 0) {
            lower = upper;
            upper = 2.0f * lower;
            /* check for overflow, no finite root */
        }
    }

    search_band[0] = lower;
    search_band[1] = upper;
    /* ... Next we  go find the root (zero) of the likelihood eqn which  wil be the MLE for sigma. */
    /* then  the MLE for mu has an explicit formula from that.  */

    float err;
    float likelihood_value;
    wdfzero(&sigmahat, &likelihood_value, &err, search_band, tol, x0, frequency, meanUncensored, size);

    float muHat;
    float sumfrequency;
    muHat = 0;
    sumfrequency = 0;
    for (size_t i = 0; i < size; i++) {
        tempVal = std::exp(x0[i] / sigmahat);
        sumfrequency += (frequency[i] * tempVal);
    }
    sumfrequency = sumfrequency / nuncensored;
    muHat = sigmahat * std::log(sumfrequency);

    /*Those were parameter estimates for the shifted, scaled data, now */
    /*transform the parameters back to the original location and scale. */
    return { std::exp((range * muHat) + maxx), 1 / (range * sigmahat) };
}

}  // end namespace metric
#endif  // header guard
