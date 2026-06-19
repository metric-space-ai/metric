# Engine Strategies

Strategies select how an intent is executed.

```text
Intent + Strategy -> Operator + Representation
```

Algorithm names belong at this layer. A user asks to find groups; `k_medoids` or `DBSCAN` is the strategy.

## C++ Strategies

The current C++ strategy objects include:

- `metric::strategies::brute_force`
- `metric::strategies::matrix_cache`
- `metric::strategies::cover_tree`
- `metric::strategies::knn_graph`
- `metric::strategies::k_medoids`
- `metric::strategies::dbscan`
- `metric::strategies::farthest_first`
- `metric::strategies::mgc`
- `metric::strategies::pcfa`

## Python Strategies

The current Python strategy objects include:

- `KMedoids`
- `DBSCAN`
- `FarthestFirst`
- `DistanceProfileCorrelation`

## Promotion Rule

A strategy should be promoted only when it has:

- deterministic fixtures or documented stochastic controls
- named result objects
- examples that use the intent API
- CI coverage
- documented assumptions and failure modes

This keeps the public engine from becoming a flat algorithm catalog.
