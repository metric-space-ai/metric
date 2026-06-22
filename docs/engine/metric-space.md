# Metric Space

`mtrc::MetricSpace<Record, Metric>` owns records plus the selected metric. It
assigns stable `mtrc::RecordId` values and answers pairwise metric queries.

```cpp
#include <metric/metric/catalog.hpp>
#include <metric/engine.hpp>

#include <string>
#include <vector>

std::vector<std::string> records = {"cat", "cot", "coat", "dog"};
auto space = mtrc::make_space(records, mtrc::Edit<char>{});

auto lhs = space.id(0);
auto rhs = space.id(1);
auto value = space.distance(lhs, rhs);
```

Records can be inserted, replaced, and erased. These mutations update the space
version so storage and index objects can detect stale state.

The metric remains authoritative. A graph, table, tree, or map is an execution
or derived form over the same source space.

## User-Facing Space API

`mtrc::make_space` is the minimal constructor. The user-facing helpers under
`mtrc::space` add a coherent workflow on top of it without exposing the low-level
storage classes. They reuse the existing `mtrc::space::storage` and
`mtrc::stats::search` components and add no new algorithms.

### Safer construction

`mtrc::space::SpaceBuilder<Record, Metric>` is a staged builder with opt-in
validation. It returns a plain `MetricSpace` or throws `mtrc::MetricInputError`.

```cpp
auto space = mtrc::space::SpaceBuilder<std::string, mtrc::Edit<char>>(mtrc::Edit<char>{})
                 .add_all(records)
                 .require_non_empty()      // refuse an empty space
                 .require_true_metric()    // metric_traits<Metric>::law == metric_law::metric
                 .require_unique_records() // needs an equality-comparable record type
                 .named("strings")         // optional metadata, surfaced by build_described()
                 .build();
```

`mtrc::space::build(records, metric)` and `mtrc::space::build_checked(records, metric)`
are one-call factories; `build_checked` applies the non-empty and true-metric gates.

### Record-set management

`mtrc::space::records` owns batch record-set mutation and ID/position validation.
It contains no metric math. Every batch operation returns a `mutation_report` with
the affected RecordIds and the old/new size and version.

```cpp
namespace records = mtrc::space::records;
auto inserted = records::insert(space, new_records);       // appends; returns new ids
auto replaced = records::replace(space, {{id, record}});   // validates ids first
auto erased   = records::erase(space, {id_a, id_b});       // lenient; reports missing
records::erase_strict(space, ids);                          // throws if any id missing
```

RecordIds are stable: an id keeps referring to the same record across insertion
and erasure of other records, even though its position changes.

### Pairwise values

`mtrc::space::distances` is the representation-neutral pair-value surface. It works
on the space directly (always fresh) or on a provider from `mtrc::space::storage`.

```cpp
namespace distances = mtrc::space::distances;
auto d   = distances::value(space, id_a, id_b);   // one pair
auto row = distances::row(space, id_a);           // distances from one record
auto all = distances::pairs(space);               // every unordered pair {i, j}, i < j

auto table = distances::materialize(space);       // eager DistanceTable provider
distances::checked_value(table, id_a, id_b);      // throws if the provider is stale
```

### Queries

`mtrc::space::query` is a thin facade over `mtrc::stats::search`:

```cpp
namespace query = mtrc::space::query;
auto n  = query::nearest(space, id);              // single nearest (throws if none)
auto kn = query::k_nearest(space, query_record, 5);
auto rn = query::within(space, id, radius);
auto kt = query::k_nearest(space, query_record, 5, mtrc::stats::search::cover_tree{});
```

### Sub-spaces, merge, and lineage

`select_subspace`, `split`, `split_if`, and `merge` (in `metric/space/partition.hpp`)
carve and combine spaces while preserving the parent RecordId of every record. The
derived spaces are independent copies with their own fresh RecordIds, so mutating a
parent never corrupts a previously created sub-space.

The lineage helpers in `metric/space/lineage.hpp` (in namespace `mtrc::space`, and
re-exported into `mtrc`) make that lineage easy to inspect and safe to trust:

```cpp
auto sub    = mtrc::select_subspace(parent, ids);
auto parent_id = mtrc::parent_record_id(sub, sub.space.id(0)); // sub id -> parent id
auto merged = mtrc::merge_checked(a, b);                        // refuses incompatible metrics
auto origin = mtrc::merge_origin(merged, merged.space.id(0));   // merged id -> (input, source id)
```

If a sub-space or merged space is itself structurally mutated after creation, its
lineage vector no longer lines up with its records; the lineage lookups detect this
and throw `mtrc::StaleRepresentationError` instead of returning a wrong source id.
