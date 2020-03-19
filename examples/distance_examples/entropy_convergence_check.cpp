
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
    for (size_t i = 0; i<20; ++i) {

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



//*

int main() {

    size_t step = 1000;

    std::vector<size_t> dim_values = {1, 2, 5, 25}; // {1, 2, 3, 4, 5, 7, 10, 25};
    std::vector<size_t> k_values =  {30}; // {3, 5, 10, 15, 20, 30}; //, 50};

    for (size_t i_k = 0; i_k < k_values.size(); ++i_k)
        for (size_t i_dim = 0; i_dim < dim_values.size(); ++i_dim)
            print_entropies(step, dim_values[i_dim], k_values[i_k]);

    return 0;
}

// */



// code for debug
/*
int main() {


//    blaze::DynamicVector<double> m = {0.0739138};
//    blaze::DynamicMatrix<double> K = {{3.07597e-05}};
//    blaze::DynamicVector<double> lb = {0.073792};
//    blaze::DynamicVector<double> ub = {0.073793};

//    auto res = epmgp::local_gaussian_axis_aligned_hyperrectangles(m, K, lb, ub);

//    std::cout << "res:\n" <<  std::get<0>(res) << "\n\n" <<  std::get<1>(res) << "\n\n"  <<  std::get<2>(res) << "\n\n";

//    return 0;

//    std::vector<double> lowerbSTL = {0.07379201115614169};
//    std::vector<double> upperbSTL = {0.07379300905047671};
//    std::vector<double> muInSTL = {-0.002534391667597378};
//    std::vector<double> sigmaInSTL = {-1.054703844320736e-6};

//    auto res = epmgp::truncNormMoments(lowerbSTL, upperbSTL, muInSTL, sigmaInSTL);

//    return 0;



    size_t len = 7000;
    size_t dim = 1;
    size_t k = 3;


    auto e_kpn_cheb = metric::entropy_kpN<void, metric::Chebyshev<double>>();

    std::mt19937 gen(1);
    std::uniform_real_distribution<double> dis(0.0, 1.0);

    std::vector<std::deque<double>> v;

    for (size_t j = 0; j<len; ++j) {
        std::deque<double> row;
        for (size_t j_d = 0; j_d<dim; ++j_d) {
            row.push_back(dis(gen));
        }
        v.push_back(row);
    }

    std::cout
            << v.size() << " | "
            << e_kpn_cheb(v, metric::Chebyshev<double>(), k, k*10)
            << "\n";

    return 0;
}
// */



