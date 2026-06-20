# Engine Mappings

Mappings produce new metric spaces or derived records from an existing metric space.

```text
MetricSpace -> Mapping -> MappingModel -> MappingResult
```

The first C++ mapping convention follows:

```text
fit(space) -> model
model.transform(space) -> derived MetricSpace
model.inverse_transform(derived_space) -> reconstructed records, if supported
```

## Current C++ Surface

The current engine mapping layer includes:

- `metric::MappingResult`
- deterministic C++ `metric::map`
- C++ `metric::map(space, mapping_adapter)` over promoted mapping adapters
- `metric::mappings::fit`
- `metric::mappings::transform`
- clustered-space mapping from `ClusteringResult`
- LAPACK-backed PCFA mapping
- roadmap KOC mapping factory with explicit unpromoted-contract failures
- PCFA-backed `metric::embed`
- PCFA-backed `metric::reduce`
- DBSCAN-noise-filtered `metric::denoise`
- Python clustered-space mapping from `ClusteringResult`
- deterministic Python `Space.map`
- DBSCAN-noise-filtered Python `Space.denoise`

`MappingResult` stores the derived space plus source-record lineage. Inverse reconstruction is explicit and optional.
The C++ `metric::map(space, metric::mappings::pcfa(components))` form fits a promoted mapping adapter
and transforms the same source space through the semantic map intent. The C++ `metric::embed` intent returns a
PCFA-derived coordinate space with one-to-one lineage and `mapping == "pcfa_embedding"`. The C++ `metric::denoise`
and Python `Space.denoise` intents also return `MappingResult` values, preserving one-to-one lineage for non-noise
records kept after DBSCAN noise filtering.
The C++ `metric::mappings::koc(clusters)` factory is roadmap-only: it validates the requested cluster count and
then raises a clear unpromoted-contract error from `fit(...)`, `transform(...)`, and `metric::map(...)` until the
KOC adapter has deterministic mapping diagnostics and CI-backed examples.

```cpp
auto mapped = metric::map(space, metric::mappings::pcfa(2));
auto coordinates = mapped.space;
```

## Python Status

Python exposes `metric.mappings.make_clustered_space_mapping(...)`, `metric.mappings.fit(...)`, `metric.mappings.transform(...)`, and `metric.mappings.clustered_space(...)` for turning an engine-style `ClusteringResult` into a derived cluster-level `Space`. The derived records are `ClusterRecord` values that keep the cluster label, representative source record, and source members; the derived metric is the source-space distance between cluster representatives. The promoted Python example lives at `python/examples/engine/clustered_space_mapping.py`.

Python also exposes `Space.map(transform=..., metric=...)` and `metric.operators.map_space` for deterministic transforms into derived `Space` objects. The positional compatibility form `Space.map(transform, metric)` remains valid, but no-argument map calls raise `metric.AmbiguousIntentError` and target/strategy mapping forms raise `metric.StrategyUnavailableError` until learned or fitted mapping contracts are promoted. Python also exposes `Space.denoise` and `metric.operators.denoise_space` for DBSCAN-noise filtering into derived `Space` objects. Python also exposes `metric.mappings` as a beta compatibility bridge for installed legacy names; learned or inverse mapping facades should be promoted only when they have named result contracts, examples, and wheel CI coverage.

## Promotion Rule

A mapping becomes part of Core Revival when it:

- starts from a `MetricSpace` / `Space`
- documents what metric is used in the derived space
- stores source-to-target lineage
- has explicit inverse support or explicit unsupported metadata
- is covered by deterministic tests and examples
