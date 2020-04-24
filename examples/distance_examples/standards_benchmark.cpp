#include "../../modules/utils.hpp"

#include <vector>
#include <iostream>
#include <chrono>
#include "../../3rdparty/blaze/Blaze.h"
//#include "../../3rdparty/Eigen/Dense"
//#if __has_include(<armadillo>)
//    #include <armadillo>
//    #define ARMA_EXISTS
//#endif // linker parameter: -larmadillo


namespace metric {

template <typename RT>
class EuclideanIterTest {

public:

    using RecordType = RT;
    using ValueType = contained_value_t<RecordType>;


    DistanceType operator()(const RT & a, const RT & b) const {
        if (a.size() != b.size()) {
            DistanceType dnan = std::nan("Input container sizes do not match");
            return dnan;
        }
        DistanceType sum = 0;
        for (auto it1 = a.begin(), it2 = b.begin(); it1 != a.end() && it2 != b.end(); ++it1, ++it2) {
            sum += (*it1 - *it2) * (*it1 - *it2);
        }
        return std::sqrt(sum);
    }

};

} // namespace metric

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

    using V = double;
    using R = blaze::DynamicVector<V>; //
    using R2 = std::vector<V>;

    R stlv0(sample_size, 0);
    R stlv1(sample_size, 0);
    R2 blazev0(sample_size, 0);
    R2 blazev1(sample_size, 0);

    double lower_bound = 0;
    double upper_bound = 1;
    std::uniform_real_distribution<V> unif(lower_bound, upper_bound);
    std::default_random_engine re;
//    for (V& v : stlv0) {
//        v = unif(re);
//    }
//    for (V& v : stlv1) {
//        v = unif(re);
//    }
    V rv;
    for (size_t i = 0; i<sample_size; ++i) {
        rv = unif(re);
        stlv0[i] = rv;
        blazev0[i] = rv;
        rv =  unif(re);
        stlv1[i] = rv;
        blazev1[i] = rv;
    }

    auto m = metric::EuclideanIterTest<R>();
    auto m2 = metric::EuclideanIterTest<R2>();

    std::cout << "  STL: time = " << timeIt(m, 10, stlv0, stlv1) << " ns" << std::endl;
    std::cout << "Blaze: time = " << timeIt(m2, 10, blazev0, blazev1) << " ns" << std::endl;


    return 0;
}
