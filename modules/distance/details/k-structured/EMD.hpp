/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 Michael Welsch
*/
#ifndef _METRIC_DISTANCE_DETAILS_K_STRUCTURED_EMD_HPP
#define _METRIC_DISTANCE_DETAILS_K_STRUCTURED_EMD_HPP
#include <vector>

namespace metric
{

namespace distance
{

    template <typename V>
    struct EMD{
        using value_type = V;
        using distance_type = value_type;

        mutable std::vector< std::vector< value_type > > C;
        value_type extra_mass_penalty = -1;
        std::vector< std::vector< value_type > > * F = nullptr;
        mutable bool is_C_initialized = false;

        explicit EMD()  {}
        explicit EMD(std::vector<std::vector<value_type>> && C_): C(C_), is_C_initialized(true) {}
        EMD(std::size_t rows, std::size_t cols,
            const value_type & extra_mass_penalty_ = -1,
            std::vector<std::vector<value_type>> *F_ = nullptr):
            C(default_ground_matrix(rows, cols)), extra_mass_penalty(extra_mass_penalty_), F(F_),  is_C_initialized(true) {}
        EMD(const std::vector<std::vector<value_type>> & C_,
            const value_type & extra_mass_penalty_ = -1, std::vector<std::vector<value_type>> *F_ = nullptr):
            C(C_), extra_mass_penalty(extra_mass_penalty_), F(F_),  is_C_initialized(true) {}

        template<typename Container>
        distance_type  operator()(const Container &Pc, const Container &Qc) const;

        std::vector<std::vector<value_type>> default_ground_matrix(std::size_t rows, std::size_t cols) const ;

        EMD(EMD &&) = default;
        EMD(const EMD &) = default;
        EMD & operator = (const EMD &) = default;
        EMD & operator = (EMD &&) = default;
    };

//#ifndef _METRIC_DISTANCE_HPP
/*** structural similartiy (for images) ***/
// template <typename Container>
// struct EMD{
//     typename Container::value_type
//     operator()(const Container &Pc,
//                const Container &Qc,
//                //const std::vector<std::vector<typename Container::value_type>> &C,
//                //typename Container::value_type maxC = std::numeric_limits<typename Container::value_type>::min(), // disabled my Max F
//                typename Container::value_type extra_mass_penalty = -1,
//                std::vector<std::vector<typename Container::value_type>> *F = NULL) const;
//     typename Container::value_type
//     operator()(const Container &Pc,
//                const Container &Qc,
//                const std::vector<std::vector<typename Container::value_type>> &C,
//                //typename Container::value_type maxC = std::numeric_limits<typename Container::value_type>::min(), // disabled my Max F
//                typename Container::value_type extra_mass_penalty = -1,
//                std::vector<std::vector<typename Container::value_type>> *F = NULL) const;
//     typedef Container value_type; // added by Max F 2018-12-04
// };
//#endif
}

}

#include "EMD.cpp"


#endif // Header Guard
