/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/

#include <vector>
#include <random>

//#include "../../modules/distance.hpp"
#include "../../modules/distance/k-random/VOI.hpp"

#include "../../modules/distance/k-structured/Edit.hpp"



int main() {

    std::cout << "Entropy, Mutual Information and Variation of Information example have started" << std::endl;
    std::cout << std::endl;


    // Entropy

    std::cout << "Entropies:" << std::endl;

    std::vector<std::vector<double>> v = { {5,5}, {2,2}, {3,3}, {5,1} };

    {
        auto e_f = metric::Entropy_simple<void, metric::Chebyshev<double>>(metric::Chebyshev<double>(), 3);
        auto e = e_f(v);
        std::cout << "using Chebyshev: " << e << std::endl;
    }

    auto e_f3 = metric::Entropy_simple<void, metric::P_norm<double>>(metric::P_norm<double>(3), 3);
    auto e_f2 = metric::Entropy_simple<void, metric::P_norm<double>>(metric::P_norm<double>(2), 3);
    auto e_f1 = metric::Entropy_simple<void, metric::P_norm<double>>(metric::P_norm<double>(1), 3);
    {
        auto e = e_f3(v);
        std::cout << "using General Minkowsky, 3: " << e << std::endl;
    }
    {
        auto e = e_f2(v);
        std::cout << "using General Minkowsky, 2: " << e << std::endl;
    }

    {
        auto e_f = metric::Entropy_simple<void, metric::Euclidian<double>>(metric::Euclidian<double>(), 3);
        auto e = e_f(v);
        std::cout << "using Euclidean: " << e << std::endl;
    }

    {
        auto e = e_f1(v);
        std::cout << "using General Minkowsky, 1: " << e << std::endl;
    }

    {
        auto e_f = metric::Entropy_simple<void, metric::Manhatten<double>>(metric::Manhatten<double>(), 3);
        auto e = e_f(v);
        std::cout << "using Manhatten: " << e << std::endl;
    }

    {
        auto e_f = metric::Entropy_simple<std::vector<double>>();
        auto e = e_f(v);
        std::cout << "using Default: " << e << std::endl;
    }

    {
        auto ekpn = metric::Entropy<void, metric::Chebyshev<double>>(metric::Chebyshev<double>(), 2, 3);
        auto e = ekpn(v);
        std::cout << "entropy_kpN, using Chebyshev: " << e << std::endl;
    }

    std::cout << std::endl;



    std::vector<std::string> input = {
            "AAA",
            "HJGJHFG",
            "BBB",
            "AAAA",
            "long long long long long long string"
    };

    auto ee_f = metric::Entropy_simple<void, metric::Edit<void>>(metric::Edit<void>(), 3); // <void> or <char> or anything in Edit does not matter here since Container type is deduced
    auto ee =  ee_f(input);

    std::cout << "using Edit with strings: " << ee << std::endl << std::endl;


    // Mutual Information

    std::vector<std::vector<double>> v1 = {{5,5}, {2,2}, {3,3}, {5,5}};
    std::vector<std::vector<double>> v2 = {{5,5}, {2,2}, {3,3}, {1,1}};

    std::cout << "Mutual Information:" << std::endl;

    std::cout << "MI (version 2) for v1, v1: " << metric::mutualInformation(v1, v1) << std::endl;
    std::cout << "MI (version 1) for v1, v1: " << metric::mutualInformation(v1, v1, 3, metric::Chebyshev<double>(), 1) << std::endl;
    std::cout << "MI (version 2) for v2, v2: " << metric::mutualInformation(v2, v2) << std::endl;
    std::cout << "MI (version 1) for v2, v2: " << metric::mutualInformation(v2, v2, 3, metric::Chebyshev<double>(), 1) << std::endl;
    std::cout << "MI (version 2) for v1, v2: " << metric::mutualInformation(v1, v2) << std::endl;
    std::cout << "MI (version 1) for v1, v2: " << metric::mutualInformation(v1, v2, 3, metric::Chebyshev<double>()) << std::endl;
    std::cout << "MI (version 2) for v2, v1: " << metric::mutualInformation(v2, v1) << std::endl;
    std::cout << "MI (version 1) for v2, v1: " << metric::mutualInformation(v2, v1, 3, metric::Chebyshev<double>(), 1) << std::endl;

	std::cout << std::endl;

		
	// Variation of Information, normalized Variation of Information
	
	std::cout << "Variation of Information, normalized Variation of Information:" << std::endl;
	
    std::cout << "VOI = " << metric::variationOfInformation(v1, v2, 2, 3) << std::endl;
    std::cout << "VOI (Manhatten) = " << metric::variationOfInformation<std::vector<std::vector<double>>, metric::Manhatten<double>>(v1, v2, 2, 3) << std::endl;
    std::cout << "VOI norm = " << metric::variationOfInformation_normalized(v1, v2, 2, 3) << std::endl;

	// functor

    auto f_voi = metric::VOI<long double>(2, 3);
    std::cout << "VOI functor = " << f_voi(v1, v2) << std::endl;

    auto f_voi_norm = metric::VOI_normalized<long double>(2, 3);
    std::cout << "VOI functor norm = " << f_voi_norm(v1, v2) << std::endl;


    std::cout << "\n\nTesting entropy function on uniformly distributed r. v.s:\n";

    //std::random_device rd;
    //std::mt19937 gen(rd());
    std::mt19937 gen(1);
    std::uniform_real_distribution<double> dis(0.0, 1.0);

    std::vector<std::vector<double>> urv;
    std::vector<std::vector<double>> urv2;
    std::vector<std::vector<double>> urv3;
    std::vector<std::deque<double>> urv4;

    for (size_t i = 0; i<1000; ++i) {
        //urv.push_back({dis(gen), dis(gen), dis(gen), dis(gen), dis(gen), dis(gen), dis(gen)});
        //urv.push_back({dis(gen), dis(gen), dis(gen)});
        urv.push_back({dis(gen), dis(gen)});
        urv2.push_back({dis(gen), dis(gen)});
        urv4.push_back({dis(gen), dis(gen)});
        //urv.push_back({dis(gen)});
    }
    for (size_t i = 0; i<250; ++i) {
        urv3.push_back({dis(gen), dis(gen)});
    }

    auto e_f_cheb = metric::Entropy_simple<void, metric::Chebyshev<double>>(metric::Chebyshev<double>(), 3);
    std::cout << "using Chebyshev: "
              << e_f_cheb(urv)
              << ", "
              << e_f_cheb(urv2)
              << std::endl;

    auto e_f_eucl = metric::Entropy_simple<void, metric::Euclidian<double>>(metric::Euclidian<double>(), 3);
    auto e = e_f_eucl(urv);
    std::cout << "using Euclidean: " << e << std::endl;

    auto ekpn_cheb = metric::Entropy<void, metric::Chebyshev<double>>(metric::Chebyshev<double>(), 3, 10);
    auto ekpn_eucl = metric::Entropy<void, metric::Euclidian<double>>(metric::Euclidian<double>(), 3, 10);

    std::cout << "entropy_kpN, using Chebyshev: "
              << ekpn_cheb(urv) << "\n";

    std::cout << "entropy_kpN, using Euclidean: " << ekpn_eucl(urv) << std::endl;


    auto rf_voi = metric::VOI<double>();
    std::cout << "using VOI: " << rf_voi(urv, urv2) << std::endl;

    std::cout << "conv test " << metric::entropy_details::conv_diff_entropy_inv(metric::entropy_details::conv_diff_entropy(-0.118)) << std::endl;




    // testing entropy::estimate

    std::cout << "\n\nstart entropy_avg test\n";
    auto e_avg = e_f_eucl.estimate(urv);
    std::cout << "\nentropy_avg result: " << e_avg << "\n\n";

    auto e_plane = e_f_eucl(urv3);
    std::cout << "default entropy for vector of sample size 250: " << e_plane << "\n\n";


    // testing types

    {
        auto e_f = metric::Entropy_simple<void, metric::Chebyshev<double>>(metric::Chebyshev<double>(), 3);
        std::cout << "\n\ntesting deque type\n\nChebyshev, full: " << e_f(urv4) << std::endl;
        std::cout << "Chebyshev estimation:\n" << e_f.estimate(urv4) << std::endl;
    }


    // measuring the entropy estimation bias
    std::cout << "\n\nmeasuring the entropy estimation bias on random vector\n\n";


    //std::vector<std::deque<double>> urv6 = {{0.1}, {0.3}, {0.5}, {0.7}, {0.9}, {0.51}, {0.5}};
    using ElType = float; // long double;
    std::vector<std::deque<ElType>> urv6 =
        {{0.1, 0.5, 0.7}, {0.3, 0.7, 0.9}, {0.5, 0.9, 0.1}, {0.7, 0.1, 0.3}, {0.9, 0.3, 0.5}, {0.51, 0.91, 0.11}, {0.5, 0.9, 0.1}};
    //std::cout << "\nshort test vector result: " << urv6.size() << " | " << e_f_eucl(urv6) << "\n\n";
    std::cout << "\nshort test vector result: " << urv6.size() << " | " << e_f_cheb(urv6) << "\n\n";


    auto e_f_mink1 = metric::Entropy_simple<void, metric::P_norm<ElType>>(metric::P_norm<ElType>(1), 3);
    auto e_f_mink2 = metric::Entropy_simple<void, metric::P_norm<ElType>>(metric::P_norm<ElType>(2), 3);
    auto e_f_manh = metric::Entropy_simple<void, metric::Manhatten<ElType>>(metric::Manhatten<ElType>(), 3);
    auto e_kpn_cheb = metric::Entropy<void, metric::Chebyshev<ElType>>(metric::Chebyshev<ElType>(), 3, 5);

    std::cout << "short test vector result,  manh: " << urv6.size() << " | " << e_f_manh(urv6) << "\n";
    std::cout << "short test vector result, mink1: " << urv6.size() << " | " << e_f_mink1(urv6) << "\n";
    std::cout << "short test vector result,  eucl: " << urv6.size() << " | " << e_f_eucl(urv6) << "\n";
    std::cout << "short test vector result, mink2: " << urv6.size() << " | " << e_f_mink2(urv6) << "\n";
    std::cout << "short test vector result,  cheb: " << urv6.size() << " | " << e_f_cheb(urv6) << "\n";
    std::cout << "short test vector result,   kpN: " << urv6.size() << " | " << e_kpn_cheb(urv6) << "\n";

    size_t step = 1000;

    std::vector<std::deque<double>> urv5;

    auto e_f_eucl500 = metric::Entropy_simple<void, metric::Euclidian<double>>(metric::Euclidian<double>(), 500);
    std::cout << "\nlength | entropy | kpN entropy\n";
    for (size_t i = 0; i<25; ++i) {
        for (size_t i = 0; i<step; ++i) {
            //urv5.push_back({dis(gen), dis(gen)});
            //urv5.push_back({dis(gen)});
            //urv5.push_back({dis(gen), dis(gen), dis(gen), dis(gen)});
            urv5.push_back({dis(gen), dis(gen), dis(gen)});
        }
        //std::cout << urv5.size() << " | " << e_f_eucl(urv5) << "\n";
        //std::cout << urv5.size() << " | " << e_f_cheb(urv5) << "\n";
        //std::cout << urv5.size() << " | " << e_f_manh(urv5) << "\n";
        //std::cout << urv5.size() << " | " << e_f_mink(urv5, 3, 2.0, metric::P_norm<double>(2)) << "\n";
        std::cout << urv5.size() << " | " << e_f_eucl500(urv5) << " | " << ekpn_cheb(urv5) << "\n";
    }



    return 0;
}
//*/
