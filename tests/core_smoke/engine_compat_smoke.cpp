#include <cassert>
#include <string>
#include <type_traits>
#include <vector>

#include "metric/compat/aliases.hpp"
#include "metric/compat/legacy_space_adapters.hpp"
#include "metric/distance.hpp"

int main()
{
	std::vector<std::string> records = {"metric", "metrics", "matrix"};

	using legacy_matrix = metric::compat::LegacyMatrixSpace<std::string, metric::Edit<char>>;
	static_assert(std::is_same<legacy_matrix, metric::Matrix<std::string, metric::Edit<char>>>::value);

	metric::compat::LegacyFiniteSpace<std::string, metric::Edit<char>> legacy_space(records, metric::Edit<char>{});
	const auto engine_space = metric::compat::to_metric_space(legacy_space, metric::Edit<char>{});

	static_assert(metric::MetricSpaceLike_v<decltype(engine_space)>);
	assert(engine_space.size() == legacy_space.size());
	assert(engine_space.record(metric::compat::record_id_from_legacy_index(1)) == legacy_space[1]);
	assert(engine_space.distance(engine_space.id(0), engine_space.id(1)) == legacy_space.distance(0, 1));
	assert(metric::compat::legacy_index(engine_space.id(2)) == 2);

	legacy_matrix matrix_space(records, metric::Edit<char>{});
	const auto engine_from_matrix = metric::compat::to_metric_space(matrix_space, metric::Edit<char>{});
	assert(engine_from_matrix.size() == matrix_space.size());
	assert(engine_from_matrix.record(engine_from_matrix.id(2)) == "matrix");
	assert(engine_from_matrix.distance(engine_from_matrix.id(0), engine_from_matrix.id(1)) == matrix_space(0, 1));

	return 0;
}
