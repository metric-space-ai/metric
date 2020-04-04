
#include <vector>
#include <random>
#include <iostream>

#include "../../modules/correlation/entropy.hpp"
//#include "../../modules/distance/k-random/VOI.hpp"

template <typename T>
void print_entropies(size_t step, size_t dim, size_t k) {

    std::cout << "\n----------------";
    std::cout << "step: " << step << "\n";
    std::cout << "dimensions: " << dim << "\n";
    std::cout << "k: " << k << "\n";
    std::cout << "p: " << k*2 << ", " << k*3 << ", " << k*4 << ", " << k*10 << "\n";




    std::mt19937 gen(1);
    std::uniform_real_distribution<T> dis(0.0, 1.0);

//    using Cnt = std::deque<T>;
    using Cnt = std::vector<T>;

    std::vector<Cnt> v;
    //std::vector<Cnt> v2;

    std::cout << "\nlength | entropy | kpN entropy (k*2) | kpN entropy (k*3) | kpN entropy (k*4) | kpN entropy (k*10) \n";
    for (size_t i = 0; i<10; ++i) {

        for (size_t j = 0; j<step; ++j) {
            Cnt row;//, row2;
            for (size_t j_d = 0; j_d<dim; ++j_d) {
                row.push_back(dis(gen));
                //row2.push_back(dis(gen));
            }
            v.push_back(row);
            //v2.push_back(row2);
        }


        auto e_kpn_cheb2 = metric::Entropy<void, metric::Euclidian<T>>(metric::Euclidian<T>(), k, k+2);
        auto e_kpn_cheb3 = metric::Entropy<void, metric::Euclidian<T>>(metric::Euclidian<T>(), k, k+3);
        auto e_kpn_cheb4 = metric::Entropy<void, metric::Euclidian<T>>(metric::Euclidian<T>(), k, k+4);
        auto e_kpn_cheb10 = metric::Entropy<void, metric::Euclidian<T>>(metric::Euclidian<T>(), k, k+10);
        auto e_eucl = metric::Entropy_simple<void, metric::Chebyshev<T>>(metric::Chebyshev<T>(), k);

        std::cout
                << v.size() << " | "
                << e_eucl(v) << " | "
                << e_kpn_cheb2(v) << " | "
                << e_kpn_cheb3(v) << " | "
                << e_kpn_cheb4(v) << " | "
                << e_kpn_cheb10(v) << " | "
//                << metric::variationOfInformation(v, v2)
//                << metric::variationOfInformation(v, v)

                << "\n";
    }

}



//*

int main() {

    size_t step = 1000;

    std::vector<size_t> dim_values = {1, 2, 5, 25}; // {1, 2, 3, 4, 5, 7, 10, 25};
    std::vector<size_t> k_values =  {3, 30}; // {3, 5, 10, 15, 20, 30}; //, 50};

    for (size_t i_k = 0; i_k < k_values.size(); ++i_k)
        for (size_t i_dim = 0; i_dim < dim_values.size(); ++i_dim)
            print_entropies<long double>(step, dim_values[i_dim], k_values[i_k]);

    return 0;
}

// */





