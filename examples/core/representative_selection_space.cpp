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

	assert((selected == std::vector<std::size_t>{0, 3, 1}));
	assert((selected_records == std::vector<std::string>{"cat", "dog", "cot"}));
	assert(space.distance(selected[0], selected[1]) == 3);

	std::cout << "representatives:";
	for (const auto index : selected) {
		std::cout << " " << records[index];
	}
	std::cout << "\n";
	std::cout << "distance(cat, dog) = " << space.distance(selected[0], selected[1]) << "\n";

	return 0;
}
