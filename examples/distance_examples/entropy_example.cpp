/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/

#include <vector>
#include "../../modules/distance.hpp"

#include <random>

/*

int main() {
	
    std::cout << "Entropy, Mutual Information and Variation of Information example have started" << std::endl;
    std::cout << "" << std::endl;
	   

    // Entropy

    std::cout << "Entropies:" << std::endl;

    std::vector<std::vector<double>> v = { {5,5}, {2,2}, {3,3}, {5,1} };

    auto e = entropy_fn(v, 3, 2.0, metric::Chebyshev<double>());
    std::cout << "using Chebyshev: " << e << std::endl;

    e = entropy_fn(v, 3, 2.0, metric::P_norm<double>(3));
    std::cout << "using General Minkowsky, 3: " << e << std::endl;

    e = entropy_fn(v, 3, 2.0, metric::P_norm<double>(2));
    std::cout << "using General Minkowsky, 2: " << e << std::endl;

    e = entropy_fn(v, 3, 2.0, metric::Euclidian<double>());
    std::cout << "using Euclidean: " << e << std::endl;

    e = entropy_kl(v, 3, 2.0, metric::Euclidian<double>());
    std::cout << "using Euclidean Kozachenko-Leonenko: " << e << std::endl;

    e = entropy_fn(v, 3, 2.0, metric::P_norm<double>(1));
    std::cout << "using General Minkowsky, 1: " << e << std::endl;

    e = entropy_fn(v, 3, 2.0, metric::Manhatten<double>());
    std::cout << "using Manhatten: " << e << std::endl;

    e = metric::entropy_fn(v);
    std::cout << "using Default: " << e << std::endl;

    std::cout << std::endl;



    std::vector<std::string> input = {
            "AAA",
            "HJGJHFG",
            "BBB",
            "AAAA",
            "long long long long long long string"
    };

    auto ee =  metric::entropy_fn(input, 3, 2.0, metric::Edit<void>()); // <void> or <char> or anything does not matter here since Container type is deduced

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
	
    std::cout << "VOI = " << metric::variationOfInformation(v1, v2) << std::endl;
    std::cout << "VOI (Manhatten) = " << metric::variationOfInformation<double, metric::Manhatten<double>>(v1, v2) << std::endl;
    std::cout << "VOI norm = " << metric::variationOfInformation_normalized(v1, v2) << std::endl;

    // functor
    auto f_voi = metric::VOI<long double>();
    std::cout << "VOI functor = " << f_voi(v1, v2) << std::endl;

    auto f_voi_norm = metric::VOI_normalized<long double>();
    std::cout << "VOI functor norm = " << f_voi_norm(v1, v2) << std::endl;
	
    // KL functor
    auto f_voi_kl = metric::VOI_kl<long double>();
    std::cout << "KL VOI functor = " << f_voi_kl(v1, v2) << std::endl;

    auto f_voi_norm_kl = metric::VOI_normalized_kl<long double>();
    std::cout << "KL VOI functor norm = " << f_voi_norm_kl(v1, v2) << std::endl;


    std::cout << "\n\nTesting entropy function on uniformly distributed r. v.s:\n";

    //std::random_device rd;
    //std::mt19937 gen(rd());
    std::mt19937 gen(1);
    std::uniform_real_distribution<double> dis(0.0, 1.0);

    std::vector<std::vector<double>> urv;
    std::vector<std::vector<double>> urv2;
    std::vector<std::vector<double>> urv3;

    for (size_t i = 0; i<1000; ++i) {
        //urv.push_back({dis(gen), dis(gen), dis(gen), dis(gen), dis(gen), dis(gen), dis(gen)});
        //urv.push_back({dis(gen), dis(gen), dis(gen)});
        urv.push_back({dis(gen), dis(gen)});
        urv2.push_back({dis(gen), dis(gen)});
        //urv.push_back({dis(gen)});
    }
    for (size_t i = 0; i<250; ++i) {
        urv3.push_back({dis(gen), dis(gen)});
    }

    std::cout << "using Chebyshev: "
              << entropy_fn(urv, 3, 2.0, metric::Chebyshev<double>())
              << ", "
              << entropy_fn(urv2, 3, 2.0, metric::Chebyshev<double>())
              << std::endl;

    e = entropy_fn(urv, 3, 2.0, metric::Euclidian<double>());
    std::cout << "using Euclidean: " << e << std::endl;

    e = entropy_kl(urv, 3, 2.0, metric::Euclidian<double>());
    std::cout << "using Euclidean Kozachenko-Leonenko: " << e << std::endl;

    auto rf_voi = metric::VOI<double>();
    std::cout << "using VOI: " << rf_voi(urv, urv2) << std::endl;

    std::cout << "conv test " << metric::conv_diff_entropy_inv(metric::conv_diff_entropy(-0.118)) << std::endl;


//    // testing entropy_avg

//    std::cout << "\n\nstart entropy_avg test\n";
//    auto e_avg = metric::entropy_avg(urv);
//    std::cout << "\nentropy_avg result: " << e_avg << "\n\n";

//    e = entropy_fn(urv3, 3, 2.0, metric::Euclidian<double>());
//    std::cout << "default entropy for vector of sample size 250: " << e << "\n\n";


    // testing entropy functor

    auto e_f = metric::entropy<std::vector<double>>();

    return 0;
}
//*/

//*

int main() {

    std::cout << "Entropy, Mutual Information and Variation of Information example have started" << std::endl;
    std::cout << "" << std::endl;


    // Entropy

    std::cout << "Entropies:" << std::endl;

    std::vector<std::vector<double>> v = { {5,5}, {2,2}, {3,3}, {5,1} };

    {
        auto e_f = metric::entropy<void, metric::Chebyshev<double>>();
        auto e = e_f(v, 3, 2.0);
        std::cout << "using Chebyshev: " << e << std::endl;
    }

    auto e_f = metric::entropy<void, metric::P_norm<double>>();
    {
        auto e = e_f(v, 3, 2.0, metric::P_norm<double>(3));
        std::cout << "using General Minkowsky, 3: " << e << std::endl;
    }
    {
        auto e = e_f(v, 3, 2.0, metric::P_norm<double>(2));
        std::cout << "using General Minkowsky, 2: " << e << std::endl;
    }

    {
        auto e_f = metric::entropy<void, metric::Euclidian<double>>();
        auto e = e_f(v, 3, 2.0, metric::Euclidian<double>());
        std::cout << "using Euclidean: " << e << std::endl;
    }

    {
        auto e = entropy_kl(v, 3, 2.0, metric::Euclidian<double>());
        std::cout << "using Euclidean Kozachenko-Leonenko: " << e << std::endl;
    }

    {
        auto e = e_f(v, 3, 2.0, metric::P_norm<double>(1));
        std::cout << "using General Minkowsky, 1: " << e << std::endl;
    }

    {
        auto e_f = metric::entropy<void, metric::Manhatten<double>>();
        auto e = e_f(v, 3, 2.0);
        std::cout << "using Manhatten: " << e << std::endl;
    }

    {
        auto e_f = metric::entropy<std::vector<double>>();
        auto e = e_f(v);
        std::cout << "using Default: " << e << std::endl;
    }

    std::cout << std::endl;



    std::vector<std::string> input = {
            "AAA",
            "HJGJHFG",
            "BBB",
            "AAAA",
            "long long long long long long string"
    };

    auto ee_f = metric::entropy<void, metric::Edit<void>>(); // <void> or <char> or anything in Edit does not matter here since Container type is deduced
    auto ee =  ee_f(input, 3, 2.0);

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

    std::cout << "VOI = " << metric::variationOfInformation(v1, v2) << std::endl;
    std::cout << "VOI (Manhatten) = " << metric::variationOfInformation<double, metric::Manhatten<double>>(v1, v2) << std::endl;
    std::cout << "VOI norm = " << metric::variationOfInformation_normalized(v1, v2) << std::endl;

    // functor
    auto f_voi = metric::VOI<long double>();
    std::cout << "VOI functor = " << f_voi(v1, v2) << std::endl;

    auto f_voi_norm = metric::VOI_normalized<long double>();
    std::cout << "VOI functor norm = " << f_voi_norm(v1, v2) << std::endl;

    // KL functor
    auto f_voi_kl = metric::VOI_kl<long double>();
    std::cout << "KL VOI functor = " << f_voi_kl(v1, v2) << std::endl;

    auto f_voi_norm_kl = metric::VOI_normalized_kl<long double>();
    std::cout << "KL VOI functor norm = " << f_voi_norm_kl(v1, v2) << std::endl;


    // done till here

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

    auto e_f_cheb = metric::entropy<void, metric::Chebyshev<double>>();
    std::cout << "using Chebyshev: "
              << e_f_cheb(urv, 3, 2.0)
              << ", "
              << e_f_cheb(urv2, 3, 2.0)
              << std::endl;

    auto e_f_eucl = metric::entropy<void, metric::Euclidian<double>>();
    auto e = e_f_eucl(urv, 3, 2.0);
    std::cout << "using Euclidean: " << e << std::endl;

    e = entropy_kl(urv, 3, 2.0, metric::Euclidian<double>());
    std::cout << "using Euclidean Kozachenko-Leonenko: " << e << std::endl;

    auto rf_voi = metric::VOI<double>();
    std::cout << "using VOI: " << rf_voi(urv, urv2) << std::endl;

    std::cout << "conv test " << metric::conv_diff_entropy_inv(metric::conv_diff_entropy(-0.118)) << std::endl;




    // testing entropy::estimate

    std::cout << "\n\nstart entropy_avg test\n";
    auto e_avg = e_f_eucl.estimate(urv);
    std::cout << "\nentropy_avg result: " << e_avg << "\n\n";

    auto e_plane = e_f_eucl(urv3);
    std::cout << "default entropy for vector of sample size 250: " << e_plane << "\n\n";


    // testing types

    {
        auto e_f = metric::entropy<void, metric::Chebyshev<double>>();
        std::cout << "\n\ntesting deque type\n\nChebyshev, full: " << e_f(urv4, 3, 2.0) << std::endl;
        std::cout << "Chebyshevm estimation:\n" << e_f.estimate(urv4) << std::endl;
    }


    // measuring the entropy estimation bias
    std::cout << "\n\nmeasuring the entropy estimation bias on random vector\n\n";


    //std::vector<std::deque<double>> urv6 = {{0.1}, {0.3}, {0.5}, {0.7}, {0.9}, {0.51}, {0.5}};
    std::vector<std::deque<double>> urv6 =
        {{0.1, 0.5, 0.7}, {0.3, 0.7, 0.9}, {0.5, 0.9, 0.1}, {0.7, 0.1, 0.3}, {0.9, 0.3, 0.5}, {0.51, 0.91, 0.11}, {0.5, 0.9, 0.1}};
    //std::cout << "\nshort test vector result: " << urv6.size() << " | " << e_f_eucl(urv6) << "\n\n";
    std::cout << "\nshort test vector result: " << urv6.size() << " | " << e_f_cheb(urv6) << "\n\n";


    size_t step = 1000;

    std::vector<std::deque<double>> urv5;

    for (size_t i = 0; i<step; ++i) {
        //urv5.push_back({dis(gen), dis(gen)});
        //urv5.push_back({dis(gen)});
        //urv5.push_back({dis(gen), dis(gen), dis(gen), dis(gen)});
        urv5.push_back({dis(gen), dis(gen), dis(gen)});
    }


    std::cout << "length | entropy\n";
    for (size_t i = 0; i<25; ++i) {
        //std::cout << urv5.size() << " | " << e_f_eucl(urv5) << "\n";
        std::cout << urv5.size() << " | " << e_f_cheb(urv5) << "\n";
        for (size_t i = 0; i<step; ++i) {
            //urv5.push_back({dis(gen), dis(gen)});
            //urv5.push_back({dis(gen)});
            //urv5.push_back({dis(gen), dis(gen), dis(gen), dis(gen)});
            urv5.push_back({dis(gen), dis(gen), dis(gen)});
        }
    }



    return 0;
}
//*/
