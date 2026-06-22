// End-to-end user workflow over a finite metric space with the mtrc::space user API:
// build a space safely, manage its record set in batches, read pairwise values, run neighbour
// queries, carve out a sub-space with lineage, and recover a stale cache after a mutation.
//
// The metric here is the edit distance over short strings, so records are plain strings and never get
// turned into vectors first.

#include <cassert>
#include <iostream>
#include <string>
#include <vector>

#include <metric/metric/catalog.hpp>

#include <metric/core/metric_space.hpp>
#include <metric/space/builder.hpp>
#include <metric/space/cache.hpp>
#include <metric/space/distances.hpp>
#include <metric/space/lineage.hpp>
#include <metric/space/query.hpp>
#include <metric/space/records.hpp>
#include <metric/space/storage/distance_table.hpp>

int main()
{
	namespace records = mtrc::space::records;
	namespace distances = mtrc::space::distances;
	namespace query = mtrc::space::query;
	namespace cache = mtrc::space::cache;

	// 1. Build a space safely. The builder validates only what we opt into.
	auto space = mtrc::space::SpaceBuilder<std::string, mtrc::Edit<char>>(mtrc::Edit<char>{})
					 .add_all(std::vector<std::string>{"cat", "cot", "coat", "dog"})
					 .require_non_empty()
					 .require_true_metric()
					 .build();
	assert(space.size() == 4);
	const auto cat = space.id(0);

	// 2. Manage the record set in batches. RecordIds are stable across mutation.
	const auto inserted = records::insert(space, std::vector<std::string>{"cart", "dot"});
	assert(inserted.count() == 2 && inserted.new_size == 6);
	assert(space.record(cat) == "cat"); // identity preserved

	// 3. Read pairwise values without knowing the storage form.
	std::cout << "d(cat, coat) = " << distances::value(space, cat, space.id(2)) << "\n";
	const auto row = distances::row(space, cat);
	assert(row.size() == space.size());

	// 4. Query the space by record and by id.
	const auto nearest = query::nearest(space, std::string("cut"));
	std::cout << "nearest to \"cut\": \"" << space.record(nearest.id) << "\" at distance " << nearest.distance
			  << "\n";
	const auto neighbours = query::k_nearest(space, cat, 2);
	assert(neighbours.size() == 2);
	const auto close = query::within(space, cat, 1);
	assert(!close.empty());

	// 5. Carve out a sub-space and keep lineage back to the parent records.
	auto sub = mtrc::select_subspace(space, std::vector<mtrc::RecordId>{cat, space.id(2)});
	assert(mtrc::parent_record_id(sub, sub.space.id(0)) == cat);
	assert(sub.space.record(sub.space.id(0)) == space.record(cat));

	// 6. Build a materialized cache, then detect and recover staleness after a mutation.
	mtrc::space::storage::DistanceTable<decltype(space)> table(space);
	assert(!cache::is_stale(table));
	records::erase(space, std::vector<mtrc::RecordId>{space.id(5)});
	assert(cache::is_stale(table)); // the mutation invalidated the cache
	assert(cache::refresh(table, space)); // rebuilt to the current version
	assert(!cache::is_stale(table) && table.record_count() == space.size());

	std::cout << "space user workflow ok\n";
	return 0;
}
