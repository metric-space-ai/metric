// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <cassert>
#include <type_traits>
#include <vector>

#include "metric/engine.hpp"

struct AbsoluteDistance {
	auto operator()(int lhs, int rhs) const -> int
	{
		const auto difference = lhs - rhs;
		return difference < 0 ? -difference : difference;
	}
};

int main()
{
	auto space = metric::make_space(std::vector<int>{0, 1, 10, 11, 30}, AbsoluteDistance{});

	const auto denoised = metric::denoise(space, metric::strategies::dbscan(2.0, 2));
	using Result = decltype(denoised);
	static_assert(std::is_same<typename Result::space_type::record_type, int>::value);

	assert(denoised.mapping == "density_denoise");
	assert(denoised.strategy == "dbscan_noise_filter");
	assert(denoised.representation == "metric_space");
	assert(!denoised.inverse_supported);
	assert(denoised.source_record_count == space.size());
	assert(denoised.space.size() == 4);
	assert(denoised.space.record(denoised.space.id(0)) == 0);
	assert(denoised.space.record(denoised.space.id(3)) == 11);
	assert(denoised.source_records.size() == denoised.space.size());
	assert(denoised.source_records[0][0] == space.id(0));
	assert(denoised.source_records[3][0] == space.id(3));
	assert(denoised.representative_records[3] == space.id(3));

	const auto direct = metric::denoise(space, 2.0, 2);
	assert(direct.space.size() == denoised.space.size());
	assert(direct.source_record_count == denoised.source_record_count);

	const auto dense = metric::denoise(space, metric::strategies::dbscan(100.0, 2));
	assert(dense.space.size() == space.size());
	assert(dense.source_records.size() == space.size());

	auto sparse = metric::make_space(std::vector<int>{0, 10}, AbsoluteDistance{});
	const auto empty = metric::denoise(sparse, metric::strategies::dbscan(1.0, 2));
	assert(empty.space.empty());
	assert(empty.source_records.empty());
	assert(empty.representative_records.empty());
	assert(empty.source_record_count == sparse.size());

	return 0;
}
