#include <metric/distance.hpp>
#include <metric/operators.hpp>
#include <metric/space.hpp>

#include <cassert>
#include <iostream>
#include <string>
#include <vector>

int main()
{
	std::vector<std::string> records = {"cat", "cot", "coat", "dog"};
	const auto space = metric::Space::from_records(records, metric::Edit<std::string>{});
	const auto selected = metric::operators::representative_indices(records, metric::Edit<std::string>{}, 3);
	const auto selected_records = metric::operators::representatives(records, metric::Edit<std::string>{}, 3);
	const auto covered = metric::operators::coverage_representative_indices(records, metric::Edit<std::string>{}, 1);
	const auto covered_records = metric::operators::coverage_representatives(records, metric::Edit<std::string>{}, 1);

	assert((selected == std::vector<std::size_t>{0, 3, 1}));
	assert((selected_records == std::vector<std::string>{"cat", "dog", "cot"}));
	assert((covered == std::vector<std::size_t>{0, 3}));
	assert((covered_records == std::vector<std::string>{"cat", "dog"}));
	assert(space.distance(selected[0], selected[1]) == 3);

	std::cout << "representatives:";
	for (const auto index : selected) {
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
