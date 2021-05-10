/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2019 Panda Team
*/

#ifndef _METRIC_UTILS_POOR_MANS_QUANTUM_DISTRIBUTION_NORMAL_CPP
#define _METRIC_UTILS_POOR_MANS_QUANTUM_DISTRIBUTION_NORMAL_CPP

#include <cassert>
#include "Normal.hpp"

namespace metric {

inline Normal::Normal()
    : _generator(std::random_device {}())
{
}

/*** random sampling ***/
inline float Normal::rnd()
{
    std::normal_distribution<float> norm_dist(_p1, _p2);
    return norm_dist(_generator);
}

inline float Normal::median() { return _p1; }

inline float Normal::quantil(float p) { return icdf(p); }

inline float Normal::mean() { return _p1; }

inline float Normal::variance() { return _p2; }

/*** pdf ***/

inline float Normal::pdf(const float x)
{
    float z = (x - _p1) / _p2;
    return -0.9189385332f - std::log(_p2) - z * z / 2.f;
}

/*** cdf ***/

inline float Normal::cdf(const float x) { return (0.5f * (1.f + (x - _p1) / (_p2 * 1.41421356237309504880f))); }

/*** icdf ***/
template <typename T>
T erfcinv(T z);

inline float Normal::icdf(const float x) { return _p1 + -1.41421356237309504880f * erfcinv(2 * x) * _p2; }

template <class T>
T polyeval(const std::vector<T>& poly, const T& z)
{
    int n = poly.size();
    T sum = poly[n - 1];
    for (int i = n - 2; i >= 0; --i) {
        sum *= z;
        sum += poly[i];
    }
    return sum;
}

/*** originally by John Maddock 2006 under Boost Software License, Version 1.0, only refactored for use with this lib **/
template <typename T>
T erfinv_imp(const T& p, const T& q)
{
    T result = 0;

    if (p <= 0.5) {
        static const float Y = 0.0891314744949340820313f;
        static const std::vector<T> P{ -0.000508781949658280665617L, -0.00836874819741736770379L,
            0.0334806625409744615033L, -0.0126926147662974029034L, -0.0365637971411762664006L,
            0.0219878681111168899165L, 0.00822687874676915743155L, -0.00538772965071242932965L };
        static const std::vector<T> Q{ 1, -0.970005043303290640362L, -1.56574558234175846809L,
            1.56221558398423026363L, 0.662328840472002992063L, -0.71228902341542847553L, -0.0527396382340099713954L,
            0.0795283687341571680018L, -0.00233393759374190016776L, 0.000886216390456424707504L };

        T g = p * (p + 10);
        T r = polyeval(P, p) / polyeval(Q, p);
        result = g * Y + g * r;
    } else if (q >= 0.25) {
        static const float Y = 2.249481201171875f;
        static const std::vector<T> P = { -0.202433508355938759655L, 0.105264680699391713268L, 8.37050328343119927838L,
            17.6447298408374015486L, -18.8510648058714251895L, -44.6382324441786960818L, 17.445385985570866523L,
            21.1294655448340526258L, -3.67192254707729348546L };
        static const std::vector<T> Q
            = { 1L, 6.24264124854247537712L, 3.9713437953343869095L, -28.6608180499800029974L, -20.1432634680485188801L,
                  48.5609213108739935468L, 10.8268667355460159008L, -22.6436933413139721736L, 1.72114765761200282724L };
        T g = std::sqrt(-2 * std::log(q));
        T xs = q - 0.25;
        T r = polyeval(P, xs) / polyeval(Q, xs);
        result = g / (Y + r);
    } else {
        T x = std::sqrt(-std::log(q));
        if (x < 3) {
            static const float Y = 0.807220458984375f;
            static const std::vector<T> P = { -0.131102781679951906451L, -0.163794047193317060787L,
                0.117030156341995252019L, 0.387079738972604337464L, 0.337785538912035898924L, 0.142869534408157156766L,
                0.0290157910005329060432L, 0.00214558995388805277169L, -0.679465575181126350155e-6L,
                0.285225331782217055858e-7L, -0.681149956853776992068e-9L };
            static const std::vector<T> Q = { 1, 3.46625407242567245975L, 5.38168345707006855425L,
                4.77846592945843778382L, 2.59301921623620271374L, 0.848854343457902036425L, 0.152264338295331783612L,
                0.01105924229346489121L };
            T xs = x - 1.125;
            T R = polyeval(P, xs) / polyeval(Q, xs);
            result = Y * x + R * x;
        } else if (x < 6) {
            static const float Y = 0.93995571136474609375f;
            static const std::vector<T> P
                = { -0.0350353787183177984712L, -0.00222426529213447927281L, 0.0185573306514231072324L,
                      0.00950804701325919603619L, 0.00187123492819559223345L, 0.000157544617424960554631L,
                      0.460469890584317994083e-5L, -0.230404776911882601748e-9L, 0.266339227425782031962e-11L };
            static const std::vector<T> Q
                = { 1L, 1.3653349817554063097L, 0.762059164553623404043L, 0.220091105764131249824L,
                      0.0341589143670947727934L, 0.00263861676657015992959L, 0.764675292302794483503e-4L };
            T xs = x - 3;
            T R = polyeval(P, xs) / polyeval(Q, xs);
            result = Y * x + R * x;
        } else if (x < 18) {
            static const float Y = 0.98362827301025390625f;
            static const std::vector<T> P
                = { -0.0167431005076633737133L, -0.00112951438745580278863L, 0.00105628862152492910091L,
                      0.000209386317487588078668L, 0.149624783758342370182e-4L, 0.449696789927706453732e-6L,
                      0.462596163522878599135e-8L, -0.281128735628831791805e-13L, 0.99055709973310326855e-16L };
            static const std::vector<T> Q
                = { 1L, 0.591429344886417493481L, 0.138151865749083321638L, 0.0160746087093676504695L,
                      0.000964011807005165528527L, 0.275335474764726041141e-4L, 0.282243172016108031869e-6L };
            T xs = x - 6;
            T R = polyeval(P, xs) / polyeval(Q, xs);
            result = Y * x + R * x;
        } else if (x < 44) {
            static const float Y = 0.99714565277099609375f;
            static const std::vector<T> P = { -0.0024978212791898131227L, -0.779190719229053954292e-5L,
                0.254723037413027451751e-4L, 0.162397777342510920873e-5L, 0.396341011304801168516e-7L,
                0.411632831190944208473e-9L, 0.145596286718675035587e-11L, -0.116765012397184275695e-17L };
            static const std::vector<T> Q
                = { 1L, 0.207123112214422517181L, 0.0169410838120975906478L, 0.000690538265622684595676L,
                      0.145007359818232637924e-4L, 0.144437756628144157666e-6L, 0.509761276599778486139e-9L };
            T xs = x - 18;
            T R = polyeval(P, xs) / polyeval(Q, xs);
            result = Y * x + R * x;
        } else {
            static const float Y = 0.99941349029541015625f;
            static const std::vector<T> P = { -0.000539042911019078575891L, -0.28398759004727721098e-6L,
                0.899465114892291446442e-6L, 0.229345859265920864296e-7L, 0.225561444863500149219e-9L,
                0.947846627503022684216e-12L, 0.135880130108924861008e-14L, -0.348890393399948882918e-21L };
            static const std::vector<T> Q
                = { 1L, 0.0845746234001899436914L, 0.00282092984726264681981L, 0.468292921940894236786e-4L,
                      0.399968812193862100054e-6L, 0.161809290887904476097e-8L, 0.231558608310259605225e-11L };
            T xs = x - 44;
            T R = polyeval(P, xs) / polyeval(Q, xs);
            result = Y * x + R * x;
        }
    }
    return result;
}

template <typename T>
T erfcinv(T z)
{
    assert((z >= 0) && (z <= 2));
    // if((z < 0) || (z > 2))
    //   std::cout << "Argument outside range [0,2] in inverse erfc function (got p=%1%)." << std::endl;

    T p, q, s;
    if (z > 1) {
        q = 2 - z;
        p = 1 - q;
        s = -1;
    } else {
        p = 1 - z;
        q = z;
        s = 1;
    }
    return s * erfinv_imp(p, q);
}

template <typename T>
T erfinv(T z)
{
    assert((z >= -1) && (z <= 1));
    //std::cout << "Argument outside range [-1, 1] in inverse erf function (got p=%1%)." << std::endl;

    T p, q, s;
    if (z < 0) {
        p = -z;
        q = 1 - p;
        s = -1;
    } else {
        p = z;
        q = 1 - z;
        s = 1;
    }

    return s * erfinv_imp(p, q);
}

}
#endif
