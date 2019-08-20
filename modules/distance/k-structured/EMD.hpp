/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 Michael Welsch
*/
#ifndef _METRIC_DISTANCE_K_STRUCTURED_EMD_HPP
#define _METRIC_DISTANCE_K_STRUCTURED_EMD_HPP
#include <vector>

namespace metric {

/**
 * @class EMD
 *
 * @brief Earth mover's distance
 *
 */
template <typename V>
struct EMD {
    using value_type = V;
    using distance_type = value_type;

    explicit EMD() {}
    /**
     * @brief Construct a new EMD object with cost matrix
     *
     * @param C_ cost matrix
     */
    explicit EMD(std::vector<std::vector<value_type>>&& C_)
        : C(C_)
        , is_C_initialized(true)
    {
    }

    /**
     * @brief Construct a new EMD object
     *
     * @param rows, cols size of cost matrix
     * @param extra_mass_penalty_
     * @param F_
     */
    EMD(std::size_t rows, std::size_t cols, const value_type& extra_mass_penalty_ = -1,
        std::vector<std::vector<value_type>>* F_ = nullptr)
        : C(default_ground_matrix(rows, cols))
        , extra_mass_penalty(extra_mass_penalty_)
        , F(F_)
        , is_C_initialized(true)
    {
    }

    /**
     * @brief Construct a new EMD object
     *
     * @param C_  cost matrix
     * @param extra_mass_penalty_
     * @param F_
     */
    EMD(const std::vector<std::vector<value_type>>& C_, const value_type& extra_mass_penalty_ = -1,
        std::vector<std::vector<value_type>>* F_ = nullptr)
        : C(C_)
        , extra_mass_penalty(extra_mass_penalty_)
        , F(F_)
        , is_C_initialized(true)
    {
    }

    /**
     * @brief Calculate EMD distance between Pc and Qc
     *
     * @tparam Container
     * @param Pc
     * @param Qc
     * @return
     */
    template <typename Container>
    distance_type operator()(const Container& Pc, const Container& Qc) const;

    EMD(EMD&&) = default;
    EMD(const EMD&) = default;
    EMD& operator=(const EMD&) = default;
    EMD& operator=(EMD&&) = default;

private:
    mutable std::vector<std::vector<value_type>> C;
    value_type extra_mass_penalty = -1;
    std::vector<std::vector<value_type>>* F = nullptr;
    mutable bool is_C_initialized = false;

    std::vector<std::vector<value_type>> default_ground_matrix(std::size_t rows, std::size_t cols) const;
};

}  // namespace metric

#include "EMD.cpp"

#endif  // Header Guard
