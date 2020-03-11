
#include <vector>
#include <iostream>
#include <tuple>
//#include "../../modules/distance/k-random/epmgp.cpp"
#include "../../modules/distance/k-random/entropy.hpp"

#include <limits>



int main() {






    /*

    blaze::DynamicVector<double> x = {1, 2, 3};
    std::cout << "\nmvnpdf at point: " << metric::mvnpdf(x) << "\n\n";

    // */

    /*

    for (double a = -30; a < 530; a += 1) {
        //std::cout << a << " " << std::log(0.5) + std::log(epmgp::erfcx(a)) - a*a << "\n";
        std::cout << a << " " << epmgp::erfcx(a) << " " << epmgp::erfcx_simple(a) << "\n";
    }
    // */

    /*

    //for (int x = 5; x>-30; x -= 1) {
    for (double a = -50; a<50; a += 1) {
//        auto logZhat1 = std::log(1 - std::exp( std::log(0.5) + std::log(epmgp::erfcx_simple(-a)) - a*a ));
//        auto logZhat2 = std::log(0.5) + std::log(epmgp::erfcx_simple(a)) - a*a;
//        auto logZhat1d = std::log(1 - std::exp( std::log(0.5) + std::log(epmgp::erfcx_double(-a)) - a*a ));
//        auto logZhat2d = std::log(0.5) + std::log(epmgp::erfcx_double(a)) - a*a;
//        auto errMinus = epmgp::erfcx(-a);
//        auto errPlus = epmgp::erfcx(a);
//        auto logErrPlus = std::log(errPlus);
//        auto logErrMinus = std::log(errMinus);
//        auto expArg = std::log(0.5) + std::log(epmgp::erfcx(-a)) - a*a;
//        auto expResult = std::exp( std::log(0.5) + std::log(epmgp::erfcx(-a)) - a*a );
        //std::cout << "x: " << x << ", erfcx(x): " << epmgp::erfcx(x) << "\n";

        double logZhat;
        if (a < -26) {
            auto logZhatOtherTail = std::log(0.5) + std::log(epmgp::erfcx(-a)) - a*a;
            logZhat = std::log(1 - std::exp(logZhatOtherTail));
            ////std::cout << "log(erfcx(-a)) " << a << " " << std::log(erfcx(-a)) << "\n"; // TODO remove
            //logZhat = 0; // lim[-Inf](logZhat) = 0
        } else {
            //double long_a = a;
            //logZhat = std::log(0.5) + std::log(epmgp::erfcx_double(long_a)) - long_a*long_a;
            logZhat = std::log(0.5) + std::log(epmgp::erfcx(a)) - a*a;
        }

//        double b = a;
//        if (b > 26) {
//            auto logZhatOtherTail = std::log(0.5) + std::log(epmgp::erfcx(b)) - b*b;
//            logZhat = std::log(1 - std::exp(logZhatOtherTail));
//        }
//        else
//            logZhat = std::log(0.5) + std::log(epmgp::erfcx(-b)) - b*b;
        std::cout <<
                     "x: " << a <<
                     ", logZhat: " << logZhat <<
//                     ", logZhat1: " << logZhat1 <<
//                     ", logZhat2: " << logZhat2 <<
//                     ", logZhat1d: " << logZhat1d <<
//                     ", logZhat2d: " << logZhat2d <<
//                     ", errMinus: " << errMinus <<
//                     ", errPlus: " << errPlus <<
//                     ", logErrMinus: " << logErrMinus <<
//                     ", logErrPlus: " << logErrPlus <<
//                     ", expArg: " << expArg <<
//                     ", expResult: " << expResult <<
//                     ", logArg: " << 1 - std::exp( std::log(0.5) + std::log(epmgp::erfcx(-a)) - a*a ) <<
//                     ", std::erfc(a): " << std::erfc(a) <<
//                     ", std::exp(a*a): " << std::exp(a*a) <<
                     "\n";
    }
    // */

    /*

    //std::vector<double> lower = {-0.5, -0.5};
    //std::vector<double> upper = {0.5, 0.5};
    //std::vector<double> mu = {0, 0};
    //std::vector<double> sigma = {1, 1};

    double inf = std::numeric_limits<double>::infinity();

//    std::vector<double> lower = {-0.5, -0.5, 0.5};
//    std::vector<double> upper = {0.5, 0.5, inf};
//    std::vector<double> mu = {0, 0, 0};
//    std::vector<double> sigma = {1, 0, 1};

//    std::vector<double> lower = {-inf};
//    std::vector<double> upper = {0.5};
//    std::vector<double> mu = {-100};
//    std::vector<double> sigma = {1};

    std::vector<double> lower = {-50};
    std::vector<double> upper = {60};
    std::vector<double> mu = {0};
    std::vector<double> sigma = {1};



    auto result = epmgp::truncNormMoments(lower, upper, mu, sigma);

    auto logZ = std::get<0>(result);
    auto muOUT = std::get<1>(result);
    auto sigmaOUT = std::get<2>(result);

    std::cout << "\n\nlogZ mu sigma\n";
    for (size_t i = 0; i<logZ.size(); ++i) {
        std::cout << logZ[i] << " " << muOUT[i] << " " << sigmaOUT[i] << "\n";
    }
    // */


    //*

//    blaze::DynamicVector<double> m = {0, 0};
//    blaze::DynamicMatrix<double> K = {{1, 0}, {0, 1}};
//    blaze::DynamicVector<double> lowerB = {-0.5, -0.5};
//    blaze::DynamicVector<double> upperB = {0.5, 0.5};

//    blaze::DynamicVector<double> m = {5, -5};
//    blaze::DynamicMatrix<double> K = {{1, 0.3}, {0.3, 1}};
//    blaze::DynamicVector<double> lowerB = {0.5, 1.5};
//    blaze::DynamicVector<double> upperB = {1.5, 2.5};

    blaze::DynamicVector<double> m = {-250, -250};
    blaze::DynamicMatrix<double> K = {{1, 0}, {0, 1}};
    blaze::DynamicVector<double> lowerB = {200, 200};
    blaze::DynamicVector<double> upperB = {30000, 30000};

//    blaze::DynamicVector<double> m = {5, -5, -3};
//    blaze::DynamicMatrix<double> K = {{1, 0.3, 0.7}, {0.3, 1, 0.5}, {0.7, 0.5, 1}};
//    blaze::DynamicVector<double> lowerB = {0.5, 1.5, 0};
//    blaze::DynamicVector<double> upperB = {1.5, 2.5, 3};

    auto result2 = epmgp::local_gaussian_axis_aligned_hyperrectangles(m, K, lowerB, upperB);

    auto logZ2 = std::get<0>(result2);
    auto muOUT2 = std::get<1>(result2);
    auto sigmaOUT2 = std::get<2>(result2);

    std::cout << "\n\logZ2 = " << logZ2 << "\n";
    std::cout << "muOUT2 = \n" << muOUT2 << "\n";
    std::cout << "sigmaOUT2 = \n" << sigmaOUT2 << "\n";

    // */



    // testing entropy_kpN
    //*

    std::cout << "\n\ntesting entropy_kpN\n\n";

    std::vector<std::vector<double>> v = { {5,5}, {2,2}, {3,3}, {5,1} };

    {
        //auto e_f = metric::entropy<void, metric::Chebyshev<double>>();
        auto ekpn = metric::entropy_kpN<void, metric::Chebyshev<double>>();
        auto e = ekpn(v, metric::Chebyshev<double>(), 2, 3);
        std::cout << "using Chebyshev: " << e << std::endl;
    }

    // */



    //*

    std::cout << "\n\nTesting entropy_kpN function on uniformly distributed r. v.s:\n";

    //std::random_device rd;
    //std::mt19937 gen(rd());
    std::mt19937 gen(1);
    std::uniform_real_distribution<double> dis(0.0, 1.0);

    std::vector<std::vector<double>> urv;
    //std::vector<std::vector<double>> urv2;
    //std::vector<std::vector<double>> urv3;
    ///std::vector<std::deque<double>> urv4;

    for (size_t i = 0; i<1000; ++i) {
        //urv.push_back({dis(gen), dis(gen), dis(gen), dis(gen), dis(gen), dis(gen), dis(gen)});
        //urv.push_back({dis(gen), dis(gen), dis(gen)});
        urv.push_back({dis(gen), dis(gen)});
        //urv2.push_back({dis(gen), dis(gen)});
        //urv4.push_back({dis(gen), dis(gen)});
        //urv.push_back({dis(gen)});
    }
    //for (size_t i = 0; i<250; ++i) {
        //urv3.push_back({dis(gen), dis(gen)});
    //}

    auto ekpn_cheb = metric::entropy_kpN<void, metric::Chebyshev<double>>();
    auto ekpn_eucl = metric::entropy_kpN<void, metric::Euclidian<double>>();

    std::cout << "using Chebyshev: "
              << ekpn_cheb(urv, metric::Chebyshev<double>(), 3, 10)
              << ", "
              << ekpn_cheb(urv, metric::Chebyshev<double>(), 7, 20)
              << std::endl;

    //auto e = metric::entropy_kpN<std::vector<double>, metric::Euclidian<double>>(urv, metric::Euclidian<double>(), 3, 10);
    auto e = ekpn_eucl(urv, metric::Euclidian<double>(), 3, 10);
    std::cout << "using Euclidean: " << e << std::endl;





    // */





    return 0;
}
