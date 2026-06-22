#include <metric/metric/catalog.hpp>
#include <metric/space/index/operators.hpp>
#include <metric/space.hpp>

#include <cassert>
#include <iostream>
#include <string>
#include <vector>

int main()
{
	std::vector<std::string> records = {"cat", "cot", "coat", "dog"};
	const auto space = mtrc::Space::from_records(records, mtrc::Edit<std::string>{});
	const auto selected = mtrc::stats::structural_analysis::representative_indices(records, mtrc::Edit<std::string>{}, 3);
	const auto selected_records = mtrc::stats::structural_analysis::representatives(records, mtrc::Edit<std::string>{}, 3);
	const auto center = mtrc::stats::structural_analysis::medoid_index(records, mtrc::Edit<std::string>{});
	const auto center_record = mtrc::stats::structural_analysis::medoid(records, mtrc::Edit<std::string>{});
	const auto separated = mtrc::stats::structural_analysis::separated_representative_indices(records, mtrc::Edit<std::string>{}, 2);
	const auto separated_records =
		mtrc::stats::structural_analysis::separated_representatives(records, mtrc::Edit<std::string>{}, 2);
	const auto covered = mtrc::stats::structural_analysis::coverage_representative_indices(records, mtrc::Edit<std::string>{}, 1);
	const auto covered_records = mtrc::stats::structural_analysis::coverage_representatives(records, mtrc::Edit<std::string>{}, 1);

	assert((selected == std::vector<std::size_t>{0, 3, 1}));
	assert((selected_records == std::vector<std::string>{"cat", "dog", "cot"}));
	assert(center == 1);
	assert(center_record == "cot");
	assert((separated == std::vector<std::size_t>{0, 3}));
	assert((separated_records == std::vector<std::string>{"cat", "dog"}));
	assert((covered == std::vector<std::size_t>{0, 3}));
	assert((covered_records == std::vector<std::string>{"cat", "dog"}));
	assert(space.distance(selected[0], selected[1]) == 3);

	std::cout << "representatives:";
	for (const auto index : selected) {
		std::cout << " " << records[index];
	}
	std::cout << "\n";
	std::cout << "medoid representative: " << records[center] << "\n";
	std::cout << "separated representatives:";
	for (const auto index : separated) {
		std::cout << " " << records[index];
	}
	std::cout << "\n";
	std::cout << "radius-cover representatives:";
	for (const auto index : covered) {
		std::cout << " " << records[index];
	}
	std::cout << "\n";
	std::cout << "distance(cat, dog) = " << space.distance(selected[0], selected[1]) << "\n";

	return 0;
}
