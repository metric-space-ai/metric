#include <cassert>
#include <string>
#include <type_traits>
#include <vector>

#include "metric/compat/aliases.hpp"
#include "metric/compat/legacy_space_adapters.hpp"
#include "metric/metric/catalog.hpp"

int main()
{
	std::vector<std::string> records = {"metric", "metrics", "matrix"};

	using legacy_matrix = mtrc::compat::LegacyMatrixSpace<std::string, mtrc::Edit<char>>;
	static_assert(std::is_same<legacy_matrix, mtrc::Matrix<std::string, mtrc::Edit<char>>>::value);

	mtrc::compat::LegacyFiniteSpace<std::string, mtrc::Edit<char>> legacy_space(records, mtrc::Edit<char>{});
	const auto engine_space = mtrc::compat::to_metric_space(legacy_space, mtrc::Edit<char>{});

	static_assert(mtrc::MetricSpaceLike_v<decltype(engine_space)>);
	assert(engine_space.size() == legacy_space.size());
	assert(engine_space.record(mtrc::compat::record_id_from_legacy_index(1)) == legacy_space[1]);
	assert(engine_space.distance(engine_space.id(0), engine_space.id(1)) == legacy_space.distance(0, 1));
	assert(mtrc::compat::legacy_index(engine_space.id(2)) == 2);

	legacy_matrix matrix_space(records, mtrc::Edit<char>{});
	const auto engine_from_matrix = mtrc::compat::to_metric_space(matrix_space, mtrc::Edit<char>{});
	assert(engine_from_matrix.size() == matrix_space.size());
	assert(engine_from_matrix.record(engine_from_matrix.id(2)) == "matrix");
	assert(engine_from_matrix.distance(engine_from_matrix.id(0), engine_from_matrix.id(1)) == matrix_space(0, 1));

	return 0;
}
