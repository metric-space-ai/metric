#include <vector>
#include <iostream>
#include <chrono>

#include "../../3rdparty/blaze/Blaze.h"
#include "../../3rdparty/Eigen/Dense"
#if __has_include(<armadillo>)
    #include <armadillo>
    #define ARMA_EXISTS
#endif // linker parameter: -larmadillo

#include "../../modules/distance.hpp"



template<typename F, typename... Args>
auto timeIt(F func, size_t N, Args&&... args) {
    using namespace std::chrono;
    double total = 0;
    for (size_t i = 0; i < N; ++i) {
        const auto t1 = steady_clock::now();
        auto result = func(std::forward<Args>(args)...);
        const auto t2 = steady_clock::now();
        total += duration_cast<nanoseconds>(t2 - t1).count();
    }
    return total / N / 1000;
}


int main()
{

    int sample_size = 1000;

    std::vector<double> stlv0(sample_size, 0);
    std::vector<double> stlv1(sample_size, 0);
    double lower_bound = 0;
    double upper_bound = 1;
    std::uniform_real_distribution<double> unif(lower_bound, upper_bound);
    std::default_random_engine re;
    for (double& v : stlv0) {
        v = unif(re);
    }
    for (double& v : stlv1) {
        v = unif(re);
    }

    auto m = metric::Euclidean<double>();

    std::cout << "Time = " << timeIt(m, 10, stlv0, stlv1) << " ns" << std::endl;

    return 0;
}
