
#include <vector>
#include <random>
#include <iostream>

#include "../../modules/correlation/entropy.hpp"



void print_entropies(size_t step, size_t dim, size_t k) {

    std::cout << "\n----------------";
    std::cout << "step: " << step << "\n";
    std::cout << "dimensions: " << dim << "\n";
    std::cout << "k: " << k << "\n";
    std::cout << "p: " << k*2 << ", " << k*3 << ", " << k*4 << ", " << k*10 << "\n";


    auto e_eucl = metric::entropy<void, metric::Euclidian<double>>();
//    auto e_cheb = metric::entropy<void, metric::Chebyshev<double>>();
    auto e_kpn_cheb = metric::entropy_kpN<void, metric::Chebyshev<double>>();


    std::mt19937 gen(1);
    std::uniform_real_distribution<double> dis(0.0, 1.0);

    std::vector<std::deque<double>> v;

    std::cout << "\nlength | entropy | kpN entropy (k*2) | kpN entropy (k*3) | kpN entropy (k*4) | kpN entropy (k*10) \n";
    for (size_t i = 0; i<25; ++i) {

        for (size_t j = 0; j<step; ++j) {
            std::deque<double> row;
            for (size_t j_d = 0; j_d<dim; ++j_d) {
                row.push_back(dis(gen));
            }
            v.push_back(row);
        }

        std::cout
                << v.size() << " | "
                << e_eucl(v, k) << " | "
                << e_kpn_cheb(v, metric::Chebyshev<double>(), k, k*2) << " | "
                << e_kpn_cheb(v, metric::Chebyshev<double>(), k, k*3) << " | "
                << e_kpn_cheb(v, metric::Chebyshev<double>(), k, k*4) << " | "
                << e_kpn_cheb(v, metric::Chebyshev<double>(), k, k*10) << " | "

                << "\n";
    }

}




int main() {

    size_t step = 1000;

    std::vector<size_t> dim_values = {1, 2, 3, 4, 5, 7, 10, 25};
    std::vector<size_t> k_values = {3, 5, 10, 15, 20, 30, 50};

    for (size_t i_k = 0; i_k < k_values.size(); ++i_k)
        for (size_t i_dim = 0; i_dim < dim_values.size(); ++i_dim)
            print_entropies(step, dim_values[i_dim], k_values[i_k]);

    return 0;
}
