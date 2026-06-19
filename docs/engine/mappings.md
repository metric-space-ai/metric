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
- `metric::mappings::fit`
- `metric::mappings::transform`
- clustered-space mapping from `ClusteringResult`
- LAPACK-backed PCFA mapping
- PCFA-backed `metric::reduce`

`MappingResult` stores the derived space plus source-record lineage. Inverse reconstruction is explicit and optional.

## Python Status

Python exposes `metric.mappings` as a beta compatibility bridge for installed legacy names. A Python engine mapping facade should be promoted only when it has named result contracts, examples, and wheel CI coverage.

## Promotion Rule

A mapping becomes part of Core Revival when it:

- starts from a `MetricSpace` / `Space`
- documents what metric is used in the derived space
- stores source-to-target lineage
- has explicit inverse support or explicit unsupported metadata
- is covered by deterministic tests and examples
