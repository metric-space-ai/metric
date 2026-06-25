// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// Runnable companion to examples/TUTORIAL.md, on the current mtrc:: surface.
// Build: c++ -std=c++17 -I. examples/tutorial.cpp -framework Accelerate   (macOS)
//        c++ -std=c++17 -I. examples/tutorial.cpp -llapack                (Linux)

#include <metric/quickstart.hpp>

#include <iostream>
#include <string>
#include <vector>

// A finite metric space over strings, compared by edit distance.
static void strings_example()
{
	std::vector<std::string> words = {"metric", "metrics", "matrix", "tree"};
	auto records = mtrc::record::import_records(words);
	auto space = mtrc::space::build_checked(records, mtrc::Edit<char>{});

	std::cout << mtrc::describe_structure(space) << "\n";
	std::cout << mtrc::find_neighbors(space, std::string("metricks"), 2) << "\n";
}

// A coordinate vector space with construction-time validation, then coordinate-safe properties.
static void vectors_example()
{
	std::vector<std::vector<double>> records = {{0.0, 0.0}, {1.0, 0.0}, {0.0, 1.0}, {1.0, 1.0}};

	auto space = mtrc::space::space_builder<std::vector<double>>(mtrc::Euclidean<double>{})
					 .add_all(records)
					 .require_non_empty()
					 .require_uniform_dimension()
					 .require_finite()
					 .build();

	std::cout << "intrinsic dimension: " << mtrc::intrinsic_dimension(space) << "\n";
	auto e = mtrc::stats::properties::entropy(space);
	if (e.succeeded()) {
		std::cout << e << "\n";
	}
	std::cout << mtrc::find_groups(space, mtrc::k_medoids_options(2)) << "\n";
}

int main()
{
	strings_example();
	vectors_example();
	return 0;
}
