/* PANDA presents
██████╗  ██████╗  ██████╗ ██████╗     ███╗   ███╗ █████╗ ███╗   ██╗███████╗     ██████╗ ██╗   ██╗ █████╗ ███╗   ██╗████████╗██╗   ██╗███╗   ███╗                                    
██╔══██╗██╔═══██╗██╔═══██╗██╔══██╗    ████╗ ████║██╔══██╗████╗  ██║██╔════╝    ██╔═══██╗██║   ██║██╔══██╗████╗  ██║╚══██╔══╝██║   ██║████╗ ████║                                    
██████╔╝██║   ██║██║   ██║██████╔╝    ██╔████╔██║███████║██╔██╗ ██║███████╗    ██║   ██║██║   ██║███████║██╔██╗ ██║   ██║   ██║   ██║██╔████╔██║                                    
██╔═══╝ ██║   ██║██║   ██║██╔══██╗    ██║╚██╔╝██║██╔══██║██║╚██╗██║╚════██║    ██║▄▄ ██║██║   ██║██╔══██║██║╚██╗██║   ██║   ██║   ██║██║╚██╔╝██║                                    
██║     ╚██████╔╝╚██████╔╝██║  ██║    ██║ ╚═╝ ██║██║  ██║██║ ╚████║███████║    ╚██████╔╝╚██████╔╝██║  ██║██║ ╚████║   ██║   ╚██████╔╝██║ ╚═╝ ██║                                    
╚═╝      ╚═════╝  ╚═════╝ ╚═╝  ╚═╝    ╚═╝     ╚═╝╚═╝  ╚═╝╚═╝  ╚═══╝╚══════╝     ╚══▀▀═╝  ╚═════╝ ╚═╝  ╚═╝╚═╝  ╚═══╝   ╚═╝    ╚═════╝ ╚═╝     ╚═╝                                    
                                                                                                                        Licensed under MPL 2.0.                                     
                                                                                                                        Michael Welsch (c) 2019.   

a library for computing with random distributed variables

a random distributed variable is just like a single value but with uncertainty around it.

you can do most operation with them like with real numbers 
and a few special ones, that do not make sense for real numbers.

This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Michael Welsch
*/

#ifndef _METRIC_UTILS_POOR_MANS_QUANTUM_HPP
#define _METRIC_UTILS_POOR_MANS_QUANTUM_HPP

#ifndef RV_SAMPLES
#define RV_SAMPLES 10000  // precision [1000 .. 1000000]
#endif

#ifndef RV_ERROR
#define RV_ERROR 0.05  // statistical error level alpha [0.001 .. 0.2]
#endif

#include "poor_mans_quantum/distributions/Discrete.hpp"
namespace metric {
template <typename Distribution = Discrete<float>, typename T = float>
class PMQ {
public:
    /**
     * @brief Construct a new PMQ object
     * 
     * @param par1 
     * @param par2 
     * @param n 
     * @param d 
     */
    PMQ(T par1 = 1.0, T par2 = 0.0, size_t n = 1000, Distribution d = Distribution());
	
	// Updated by Stepan Mamontov 26.02.2020
    /**
     * @brief Construct a new PMQ object
     * 
     * @param data 
     * @param d 
     */
    PMQ(std::vector<T> data, Distribution d = Distribution());
	
	// Updated by Stepan Mamontov 26.02.2020
    /**
     * @brief Construct a new PMQ object
     * 
     * @param data 
     * @param d 
     */
    //PMQ(std::vector<float> data, Distribution d = Distribution());

    /**
     * @brief 
     * 
     * @return 
     */
    T rnd();

    /**
     * @brief 
     * 
     * @return 
     */
    T median();
    
    /**
     * @brief 
     * 
     * @param p 
     * @return
     */
    T quantil(T p);

    /**
     * @brief 
     * 
     * @return 
     */
    T mean();
    
    /**
     * @brief 
     * 
     * @return
     */
    T variance();

    /**
     * @brief 
     * 
     * @param x 
     * @return 
     */
    T pdf(T x);

    /**
     * @brief 
     * 
     * @param x 
     * @return 
     */
    T cdf(T x);

    /**
     * @brief 
     * 
     * @param x 
     * @return 
     */
    T icdf(T x);

    /**
     * @brief 
     * 
     * @return size_t 
     */
    size_t size();

    /**
     * @brief 
     * 
     * @tparam mT 
     * @param rv 
     * @return 
     */
    template <typename mT>
    mT believe(mT rv);

    /**
     * @brief 
     * 
     * @param confidencelevel 
     * @return
     */
    std::tuple<PMQ<Discrete<float>>, PMQ<Discrete<float>>> confidence(const T& confidencelevel = 1 - RV_ERROR) const;

    /**
     * @brief 
     * 
     * @tparam mT1 
     * @tparam mT2 
     * @param set_0_left 
     * @param set_0_right 
     * @param confidencelevel 
     * @return
     */
    template <typename mT1, typename mT2>
    std::tuple<PMQ<Discrete<float>>, PMQ<Discrete<float>>> merged_confidence(
        const mT1& set_0_left, const mT2& set_0_right, const T confidencelevel = 1 - RV_ERROR) const;

    /**
     * @brief 
     * 
     * @tparam mT1 
     * @tparam mT2 
     * @param set_left 
     * @param set_right 
     * @return
     */
    template <typename mT1, typename mT2>
    float in_confidence(const mT1& set_left, const mT2& set_right) const;

    /**
     * @brief 
     * 
     * @tparam mT1 
     * @tparam mT2 
     * @param set_left 
     * @param set_right 
     * @return 
     */
    template <typename mT1, typename mT2>
    float out_confidence(const mT1& set_left, const mT2& set_right) const;

    /**
     * @brief 
     * 
     * @return
     */
    float is_normal_distributed();

    /**
     * @brief 
     * 
     * @return
     */
    float is_weibull_distributed();


private:
    Distribution _dist;
    std::mt19937_64 _generator;
};

}
#include "poor_mans_quantum/poor_mans_quantum.cpp"

#endif  // headerguard
