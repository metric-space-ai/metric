# Execution Policies

Execution policy belongs to `mtrc::space::storage`. It controls how a finite
metric space is accessed for a computation: directly, through a table, through
an index, or through a graph-backed form.

The important rule is that execution policy does not define the metric. It only
chooses how metric values are computed or reused.

Current execution concepts:

- exact or unsupported approximate behavior
- lazy live access or materialized table access
- serial or parallel metric evaluation where the metric is safe for it
- stale-state diagnostics when a space changes after a storage object was built
- representation diagnostics for storage and index objects

Use explicit storage/index objects when the workflow needs reproducibility,
cost accounting, or query-speed control.
