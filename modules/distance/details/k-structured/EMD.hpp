#ifndef _METRIC_DISTANCE_EMD_HPP
#define _METRIC_DISTANCE_EMD_HPP

namespace metric
{

namespace distance
{

    template <typename V>
    struct EMD{
        using value_type = V;
        using distance_type = value_type;

        std::vector< std::vector< value_type > > C;
        value_type extra_mass_penalty = -1;
        std::vector< std::vector< value_type > > * F;

        explicit EMD(): EMD() {}
        explicit EMD(std::vector<std::vector<value_type>> && C_): C(C_) {}
        EMD(std::size_t rows, std::size_t cols,
            const value_type & extra_mass_penalty_ = -1,
            std::vector<std::vector<value_type>> *F_ = nullptr):
            C(default_ground_matrix(rows, cols)), extra_mass_penalty(extra_mass_penalty_), F(F_) {}
        EMD(const std::vector<std::vector<value_type>> & C_,
            const value_type & extra_mass_penalty_ = -1, std::vector<std::vector<value_type>> *F_ = nullptr):
            C(C_), extra_mass_penalty(extra_mass_penalty_), F(F_) {}

        template<typename Container>
        distance_type  operator()(const Container &Pc, const Container &Qc) const;

        std::vector<std::vector<value_type>> default_ground_matrix(std::size_t rows, std::size_t cols);
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
