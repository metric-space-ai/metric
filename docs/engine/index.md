# Framework Components

METRIC is organized around one object:

```text
records + metric = finite metric space
```

Everything else is a way to inspect, store, modify, map, solve for, or expose
that finite metric space.

## Three-Level Documentation Rule

| Level | Role | Examples |
| --- | --- | --- |
| Level 1 | Finite metric-space concepts: records, metric, space, entropy, correlation, sampling, and transformation. | `(X, d)`, record identity, metric law, finite-space property, derived space. |
| Level 2 | Framework components and C++ namespaces that own the work. | `mtrc::record`, `mtrc::metric`, `mtrc::space`, `mtrc::stats`, `mtrc::modify`, `mtrc::solve`, `mtrc::numeric`. |
| Level 3 | Concrete algorithms, estimators, papers, and compatibility adapters. | TWED, Wasserstein/EMD, MGC, entropy estimators, PHATE, autoencoders, native DNN solvers, PCFA, SOM/KOC, MDS, DBSCAN. |

Level 3 names are introduced only inside their Level 2 component and only after
the finite metric-space question is explicit.

## Level-1 Source Homes

| Namespace | Source home | Meaning |
| --- | --- | --- |
| `mtrc::record` | `metric/record/...` | record identity and record adapters |
| `mtrc::space` | `metric/space/...` | finite-space storage, indexes, selection, split, merge |
| `mtrc::metric` | `metric/metric/...` | true metrics, custom metrics, composition, admission |
| `mtrc::stats` | `metric/stats/...` | questions about an existing finite metric space |
| `mtrc::modify` | `metric/modify/...` | derived or modified finite metric spaces |
| `mtrc::solve` | `metric/solve/...` | native solvers used by metric-space components |
| `mtrc::numeric` | `metric/numeric/...` | low-level numeric contracts; METRIC's native numerical core with historical source provenance documented centrally; no external numeric dependency/public boundary |

## Guides

- [Finite-Space Mental Model](mental-model.md)
- [Metric Space](metric-space.md)
- [Space Storage And Indexes](representations.md)
- [Stats And Modify Components](operators.md)
- [Algorithm Choices](strategies.md)
- [Mappings](mappings.md)
- [Finite Metric Dynamics](dynamics.md)
- [Execution Policies](runtime.md)
- [Scale-Safe Performance Plan](scale-performance-plan.md)
- [Migration](migration.md)

## Placement Rule

Concrete algorithms and papers are implementation references. They belong after
the finite metric-space question and the owning `mtrc` component are clear.
