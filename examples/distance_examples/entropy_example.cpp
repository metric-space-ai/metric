/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/

#include <vector>
#include "../../modules/distance.hpp"



int main() {
	
	std::cout << "Entropy, Mutual Information and Variation of Information example have started" << std::endl;
	std::cout << "" << std::endl;
	   

    // Entropy

    std::cout << "Entropies:" << std::endl;

	std::vector<std::vector<double>> v = { {5,5}, {2,2}, {3,3}, {5,1} };

    auto e = entropy(v, 3, 2.0, metric::Chebyshev<double>());
    std::cout << "using Chebyshev: " << e << std::endl;

    e = entropy(v, 3, 2.0, metric::P_norm<double>(3));
    std::cout << "using General Minkowsky, 3: " << e << std::endl;

    e = entropy(v, 3, 2.0, metric::P_norm<double>(2));
    std::cout << "using General Minkowsky, 2: " << e << std::endl;

    e = entropy(v, 3, 2.0, metric::Euclidian<double>());
    std::cout << "using Euclidean: " << e << std::endl;

    e = entropy_kl(v, 3, 2.0, metric::Euclidian<double>());
    std::cout << "using Euclidean Kozachenko-Leonenko: " << e << std::endl;

    e = entropy(v, 3, 2.0, metric::P_norm<double>(1));
    std::cout << "using General Minkowsky, 1: " << e << std::endl;

    e = entropy(v, 3, 2.0, metric::Manhatten<double>());
    std::cout << "using Manhatten: " << e << std::endl;

    e = metric::entropy(v);
    std::cout << "using Default: " << e << std::endl;

    std::cout << std::endl;



    std::vector<std::string> input = {
            "AAA",
            "HJGJHFG",
            "BBB",
            "AAAA",
            "long long long long long long string"
    };

    auto ee =  metric::entropy(input, 3, 2.0, metric::Edit<void>()); // <void> or <char> or anything does not matter here since Container type is deduced

    std::cout << "using Edit with strings: " << ee << std::endl << std::endl;
    //*/


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
	std::cout << "VOI (Manhatten) = " << metric::variationOfInformation<std::vector<std::vector<double>>, metric::Manhatten<double>>(v1, v2) << std::endl;
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


    return 0;
}
