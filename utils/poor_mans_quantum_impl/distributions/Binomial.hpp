#ifndef _PMQ_BINOMIAL_HPP
#define _PMQ_BINOMIAL_HPP
/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Copyright (c) 2019 Michael Welsch
*/

#include "../PMQ.hpp"
#include <iostream>

class Binomial
{
  private:
    std::mt19937_64 _generator;
  public:
    float _p1;
    float _p2;
    std::vector<float> _data;
    std::vector<float> _prob;

    Binomial();

    /*** sample a random value ***/
    float rnd();

    // float mean();
    // float quantil(float p);

    // float pdf(const float x);
    float cdf(const int x);
    // float icdf(const float x);
};

 
//  Binomial::Binomial()
//     : _generator(std::random_device{}())
// {
// }

// /*** random sampling ***/
// float  Binomial::rnd()
// {
//     std::binomial_distribution<float> Binomial_dist(_p1, _p2);
//     return Binomial_dist(_generator);
// }

// // float  Binomial::mean()
// // {
// //     return float(1)/_p1*gamma(float(1)+float(1)/_p2);
// // }

 
// // float  Binomial::quantil(float p)
// // {
// //     return icdf(p);
// // }

// // /*** pdf ***/
// // float  Binomial::pdf(const float x)
// // {
// // float z = x / _p1;
// // float w = std::exp(-(std::pow(z,_p2)));
// // float y = std::pow(z,(_p2-1)) * w * _p2 / _p1;

// // if (w==0) return 0;
// // else return y;

// // }


// // /*** icdf ***/
// // float  Binomial::icdf(const float x)
// // {
// //    return _p1 * std::pow(-std::log(1-x),1/_p2);
// // }

// template <typename T>
// T ln__gamma(T z)
//   {
//   const T lct[9+1] = {
//      0.9999999999998099322768470047347,
//     676.520368121885098567009190444019,
//    -1259.13921672240287047156078755283,
//     771.3234287776530788486528258894,
//     -176.61502916214059906584551354,
//      12.507343278686904814458936853,
//     -0.13857109526572011689554707,
//     9.984369578019570859563e-6,
//     1.50563273514931155834e-7
//   };
//     T sum;
//     T base;
//     const T g_pi = 3.14159265358979323846;
//     const T ln_sqrt_2_pi = 0.91893853320467274178;
//     if (z < 0.5) {
//       return std::log(g_pi / std::sin(g_pi * z)) - ln__gamma(1.0 - z);
//     }
//     z = z - 1.0;
//     base = z + 7.5;
//     sum = 0;

//     for(int i=9; i>=1; i--) {
//       sum += lct[i] / (z + ((T) i));
//     }
//     sum += lct[0];
//     return ((ln_sqrt_2_pi + std::log(sum)) - base) + std::log(base)*(z+0.5);
//   }

// template <typename T>
// T betacf(T a, T b, T x) {

//   int max_it =100;
//   T tiny =1.0e-30;
//   T eps = 3.0e-7;
// 	T a_plus_b = a + b;
// 	T a_plus_one = a + 1.0;
// 	T a_minus_one = a - 1.0;
// 	T dj = - a_plus_b * x / a_plus_one;
// 	T Dj = 1.0 + dj;
// 	if (std::abs(Dj) < tiny) {
// 		Dj = tiny;
// 	}
// 	Dj = 1.0 / Dj;
// 	T Cj = 1.0;

// 	T fj = Dj;
// 	int m, m_mult_2;
// 	T deltdj;

// 	for (m = 1; m <= max_it; m++) {
// 		m_mult_2 = 2 * m;

// 		dj = m * (b - m) * x / ((a_minus_one + m_mult_2) * (a + m_mult_2));

// 		Dj = 1.0 + dj * Dj;
// 		if (std::abs(Dj) < tiny) { Dj = tiny; }
// 		Dj = 1.0 / Dj;

// 		Cj = 1.0 + dj / Cj;
// 		if (std::abs(Cj) < tiny) { Cj = tiny; }
// 		fj *= Dj * Cj;
// 		dj = -(a + m) * (a_plus_b + m) * x / ((a + m_mult_2) * (a_plus_one + m_mult_2));
// 		Dj = 1.0 + dj * Dj;
// 		if (std::abs(Dj) < tiny) { Dj = tiny; }
// 		Dj = 1.0 / Dj;

// 		// compute Cj according to its definition
// 		Cj = 1.0 + dj / Cj;
// 		if (std::abs(Cj) < tiny) { Cj = tiny; }

// 		deltdj = Dj * Cj;
// 		fj *= deltdj;

// 		if (std::abs(deltdj - 1.0) < eps) { break; }
// 	}
// 	return fj;
// }

// template <typename T>
// T betai(T a, T b, T x) {

// 	T betacf(T a, T b, T x);
// 	T ln__gamma(T x);
// 	T first_term;

// 	if (x < 0.0 || x > 1.0) {
// 		std::cout	<< "x parameter = " << x << " (p parameter of binomial) is out of [0, 1].";
// 	}
// 	if (x == 0.0 || x == 1.0) {
// 		first_term = 0.0;
// 	}
// 	else {
// 		T log_complete_beta = ln__gamma(a) + ln__gamma(b) - ln__gamma(a + b);
// 		first_term = std::exp( a * log(x) + b * log(1.0 - x) - log_complete_beta);
// 	}
// 	if (x < (a + 1.0) / (a + b + 2.0)) {
// 		return first_term * betacf(a, b, x) / a;
// 	}
// 	else {
// 		return 1.0 - first_term * betacf(b, a , 1.0 - x) / b;
// 	}
// }

// /*** cdf ***/
// //param n total number of trials
// //param k number of successful trials
// //param p success probability of single trial
// float Binomial::cdf(int kk) {
// 	float k = float(kk);
//   float n = float(_data.size());
//   float p = _p1;
// 	return betai(k, n-k + 1, p);
// }


#include "Binomial.cpp"
#endif // header guard