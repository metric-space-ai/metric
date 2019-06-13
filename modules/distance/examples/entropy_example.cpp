#include <vector>
#include <random>
#include <cmath>
#include "distance/details/k-related/entropy.hpp"
#include "distance/details/k-related/Standards.hpp"
#include <iostream>

#include "distance/details/k-related/chebyshev.hpp"

//template<typename T>
//struct Chebyshev {
//    T operator()(const std::vector<T> & lhs, const std::vector<T> & rhs) const {
//        T res = 0;
//        for(std::size_t i = 0; i < lhs.size(); i++) {
//            auto m = std::abs(lhs[i]- rhs[i]);
//            if(m > res)
//                res = m;
//        }
//        return res;
//    }
//};


int main() {
    std::vector<std::vector<double>> v = {{1,1}, {2,2}, {3,3}, {5,5}};
    auto e = entropy<double,metric::distance::Euclidian<double>>(v);
    std::cout << e << std::endl;
    std::vector<std::vector<double>> v1 = {{5,5}, {2,2}, {3,3}, {5,5}};
    std::vector<std::vector<double>> v2 = {{5,5}, {2,2}, {3,3}, {5,5}};
    auto pe = pluginEstimator(v1);
    for(auto & d : pe.first) {
        std::cout << d << ", ";
    }
    std::cout << std::endl;
    auto e1 = mutualInformation<double, metric::distance::Chebyshev<double>>(v1,v2);
    std::cout<< e1 << std::endl;

    std::cout << mutualInformation<double>(v1, v2) << std::endl;
    return 0;
}
