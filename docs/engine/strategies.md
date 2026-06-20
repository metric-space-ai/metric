# Engine Strategies

Strategies select how an intent is executed.

```text
Intent + Strategy -> Operator + Representation
```

Algorithm names belong at this layer. A user asks to find groups; `k_medoids` or `DBSCAN` is the strategy.

## C++ Strategies

The current promoted C++ strategy objects include:

- `metric::strategies::brute_force`
- `metric::strategies::matrix_cache`
- `metric::strategies::cover_tree`
- `metric::strategies::knn_graph`
- `metric::strategies::k_medoids`
- `metric::strategies::dbscan`
- `metric::strategies::affinity_propagation`
- `metric::strategies::farthest_first`
- `metric::strategies::mgc`
- `metric::strategies::pcfa`

The current C++ roadmap strategy objects include:

- `metric::strategies::som`
- `metric::strategies::koc`
- `metric::strategies::dspcc`

They validate strategy parameters and raise clear `std::invalid_argument`
messages from the engine reduction intent until deterministic mapping
contracts, diagnostics, and CI-backed examples are promoted.

## Python Strategies

The current Python strategy objects include:

- `MDS`
- `ClassicMDS`
- `KMedoids`
- `DBSCAN`
- `FarthestFirst`
- `DistanceProfileCorrelation`

`MDS` and `ClassicMDS` execute the promoted classical-MDS embedding path.
`DiffusionEmbedding`, `PCFA`, `SOM`, `KOC`, `DSPCC`, and `PhateAE` are also importable from
`metric.strategies` as roadmap vocabulary, but Python intent methods raise
`StrategyUnavailableError` for those strategies until deterministic fixtures,
diagnostics, and CI-backed result contracts are promoted.

## Promotion Rule

A strategy should be promoted only when it has:

- deterministic fixtures or documented stochastic controls
- named result objects
- examples that use the intent API
- CI coverage
- documented assumptions and failure modes

This keeps the public engine from becoming a flat algorithm catalog.
