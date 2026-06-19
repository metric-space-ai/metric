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
- `metric::mappings::fit`
- `metric::mappings::transform`
- clustered-space mapping from `ClusteringResult`
- LAPACK-backed PCFA mapping
- PCFA-backed `metric::embed`
- PCFA-backed `metric::reduce`
- DBSCAN-noise-filtered `metric::denoise`
- deterministic Python `Space.map`
- DBSCAN-noise-filtered Python `Space.denoise`

`MappingResult` stores the derived space plus source-record lineage. Inverse reconstruction is explicit and optional. The C++ `metric::embed` intent returns a PCFA-derived coordinate space with one-to-one lineage and `mapping == "pcfa_embedding"`. The C++ `metric::denoise` and Python `Space.denoise` intents also return `MappingResult` values, preserving one-to-one lineage for non-noise records kept after DBSCAN noise filtering.

## Python Status

Python exposes `Space.map(transform=..., metric=...)` and `metric.operators.map_space` for deterministic transforms into derived `Space` objects. The positional compatibility form `Space.map(transform, metric)` remains valid, but no-argument map calls raise `metric.AmbiguousIntentError` and target/strategy mapping forms raise `metric.StrategyUnavailableError` until learned or fitted mapping contracts are promoted. Python also exposes `Space.denoise` and `metric.operators.denoise_space` for DBSCAN-noise filtering into derived `Space` objects. Python also exposes `metric.mappings` as a beta compatibility bridge for installed legacy names; learned or inverse mapping facades should be promoted only when they have named result contracts, examples, and wheel CI coverage.

## Promotion Rule

A mapping becomes part of Core Revival when it:

- starts from a `MetricSpace` / `Space`
- documents what metric is used in the derived space
- stores source-to-target lineage
- has explicit inverse support or explicit unsupported metadata
- is covered by deterministic tests and examples
