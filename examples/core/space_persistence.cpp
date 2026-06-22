// Save and reload a finite metric space without turning records into vectors.
//
// The artifact stores records, stable RecordIds, metric identity, the space version,
// and optional materialized pair values. Loading supplies the metric explicitly, so
// arbitrary metric callables remain native C++ code instead of serialized code.

#include <cassert>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <metric/workflow.hpp>

int main()
{
	auto space = mtrc::space::build_checked(
		std::vector<std::string>{"normal pump cycle", "bearing wear", "seal failure"}, mtrc::Edit<char>{});

	const auto normal = space.id(0);
	space.erase(space.id(1));
	const auto fixed = space.insert("maintenance done");

	std::stringstream artifact_stream;
	mtrc::space::persistence::save(artifact_stream, space);

	auto loaded = mtrc::space::persistence::load<std::string>(artifact_stream, mtrc::Edit<char>{});
	assert(loaded.space.version() == space.version());
	assert(loaded.space.id(0) == normal);
	assert(loaded.space.id(2) == fixed);
	assert(loaded.space.distance(normal, fixed) == space.distance(normal, fixed));

	std::cout << "saved records: " << loaded.artifact.record_count() << "\n";
	std::cout << "saved pair values: " << loaded.artifact.pair_count() << "\n";
	std::cout << "distance(normal, fixed): " << loaded.space.distance(normal, fixed) << "\n";
	return 0;
}
