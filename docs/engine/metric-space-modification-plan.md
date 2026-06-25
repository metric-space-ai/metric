# Metric-Space Modification Roadmap

This roadmap defines how METRIC should grow from individual `modify`
components into a unified framework for changing finite metric spaces as
mathematical objects in their own right.

The scope is deliberately narrower than a general machine-learning toolbox. A
strategy belongs here only if it is meaningful for an arbitrary finite metric
space and can be implemented from metric calls, pairwise distances, or a
distance matrix. Euclidean coordinates, vector spaces, kernels, manifolds, and
generative record constructors are special cases or adapter layers. They may be
useful, but they are not the organizing principle of this roadmap.

The unifying object is a finite metric-measure space:

```text
(X, d, mu)
```

`X` is the record set, `d` is the admitted metric, and `mu` is a probability or
weight measure over records. Current APIs mostly operate on `(X, d)` and carry
lineage in result objects. Future modification APIs should make `mu` explicit
whenever reduction, thinning, compression, or statistical representativeness is
part of the contract.

## Goals

The core distinction is between two different and often opposing objectives.

| Objective | Meaning | Typical output |
| --- | --- | --- |
| Distribution-preserving reduction | Keep the empirical measure statistically representative; dense regions remain denser in the reduced result. | weighted sample, metric-stratified sample, finite-space coreset |
| Uniform-density reduction | Intentionally flatten metric sampling density; dense regions are thinned more aggressively. | separated sample, radius cover, maximal metric net |

This distinction should be visible in public naming, result metadata, examples,
and diagnostics. A user should never have to infer whether a reduced space is
representing the original population distribution or a uniform metric cover.

## Application Patterns

Finite metric spaces are not only an abstract target; they are the practical
object that remains after domain records and a domain distance have been made
explicit. The usual pipeline is:

```text
mixed records + composed metric -> finite metric space -> groups, representatives, subspaces, sketches
```

This is different from a conventional vector-first workflow. A vector workflow
often has to flatten heterogeneous data into one numeric table, choose arbitrary
encodings and scales, impute missing values, and then use Euclidean tools whose
centers may not be valid records. That can be useful when the domain is already
Euclidean, but it is a lossy reduction when the records contain categories,
sets, strings, histograms, process curves, event sequences, or domain-specific
penalties. METRIC should let users encode the actual notion of similarity first
and then run finite-space operations over the resulting distances.

| Scenario | Vector-first limitation | Finite metric-space workflow |
| --- | --- | --- |
| Customer focus groups | One-hot encoding, numeric scaling, and k-means centroids can produce segments whose "center" is not a real customer and whose distance depends on preprocessing choices. | Model each customer row as a mixed record, compose numeric, categorical, set, text, and time-behavior costs, build `(customers, d)`, find density groups or k-medoids, and use medoids/coverage representatives as real reference personas. |
| Segment comparison | Comparing averages of encoded columns can hide that two segments have similar totals but different internal structure. | Split the finite space by region, plan, cohort, campaign, or time window; compare subspaces by distance distributions, representative drift, density profiles, and metric-measure summaries. |
| Representative panels | Random samples preserve population frequency but may over-sample dense duplicate behavior; uniform samples may erase important prevalence. | Choose explicitly between distribution-preserving thinning, weighted compression, or uniform-density metric nets, with multiplicities and validity reported. |
| Anomaly review | Coordinate outlier scores can be dominated by whichever record fields were easiest to vectorize. | Use the composed metric to score kNN distance, inspect density-unassigned records, inspect local volume, and keep lineage back to original records. |

The intended advantage is precision of metric modeling, not rejection of vector methods.
Vector methods remain valid when the records are genuinely vector records and
Euclidean geometry is the desired metric. For mixed finite records, the metric
space is the primary object and vector embeddings are optional views or
approximations.

## Current Baseline

`mtrc::modify` already has the right high-level component boundaries:

- `represent`: selects an unmodified subset of source records without assigning
  non-selected records.
- `reduce::compress`: selects representatives and assigns every source record to
  its nearest representative; the promoted route is mainly farthest-first
  coverage.
- `resample::density_filter`: removes DBSCAN-unassigned records while preserving the source
  metric over retained records.
- `map`: transforms records into derived spaces. Coordinate maps are supported
  as a special case, but they are not the core abstraction for finite-space
  modification.
- `expand`: reserves the component boundary for domain-supported interpolation
  or record generation.
- `dynamics`: evolves a finite space through diffusion or other finite-state
  dynamics.

The baseline is metric-first: representative and compression paths keep source
records unmodified, so the metric law over the retained subset is inherited from
the source metric. That should remain the default for stable reduction
operators.

## Target Taxonomy

Future public APIs should be organized by the modification intent, not by paper
or algorithm name.

| Intent | Contract | Stable first strategies | Research candidates |
| --- | --- | --- | --- |
| `represent` | Select source records only; no collapse or assignment. | farthest-first, medoid, separated, radius coverage | low-doubling hierarchical nets |
| `compress` | Select source representatives, assign all source records, and report weights/multiplicity. | k-center/coverage, radius coverage, k-medoids | streaming metric coresets, metric-measure quantization |
| `thin` | Remove records under a statistical or metric sampling policy. | preserve-distribution sample, metric-stratified sample, separated sample | distance-matrix balanced sampling |
| `equalize` | Change local sampling density toward a target density. | kNN-density thinning, local-volume thinning | adaptive separated nets, density-ratio correction |
| `sketch` | Preserve a chosen finite-space observable rather than records themselves. | distance-distribution sketch, kNN-recall sketch | graph spanners, spectral sparsifiers |
| `quotient` | Collapse source groups into derived representative records. | k-medoids cluster quotient | density hierarchy quotient, task-conditioned quotient |
| `expand` | Generate or interpolate records where the domain supplies a valid constructor. | no stable generic finite-metric route | domain-specific only, outside this core roadmap |
| `map` | Produce a derived record space with an explicit target metric. | deterministic metric-preserving transforms when available | finite-metric embeddings as analysis aids, not default reduction |

Stable operators in this roadmap must work on arbitrary finite metric spaces.
If a strategy requires coordinates, a positive-semidefinite kernel, or a record
generator, it belongs in a separate adapter or experimental `map` / `expand`
document, not in the finite-metric-space modification core.

## Result Metadata Contract

All modifying operators should report enough metadata to audit what happened to
the source space.

Required for subset or compression operators:

- source lineage for every derived record
- representative source `RecordId`s
- assignment map when source records are collapsed or summarized
- per-representative multiplicity and normalized weight
- coverage radius and average assignment distance when assignments exist
- `metric_status` for the derived metric
- `validity` text that states whether the result is a source-metric subset,
  metric-measure summary, density correction, or experimental sketch

Optional diagnostics, promoted per strategy:

- kNN recall against the source space
- distance-distribution drift
- local-density or local-volume drift
- graph stretch or connectivity drift
- objective value, such as k-center max error or k-median average error
- randomness seed and inclusion probabilities for statistical sampling

Weights should be first-class for compression. For a representative `r_i`, its
integer multiplicity is the number of source records assigned to `r_i`; its
normalized mass is `multiplicity_i / source_record_count`. More advanced
weighted operators may replace integer multiplicity with explicit probability
mass, but the result must state which convention is used.

## Paper Backbone

These references define the first research map for finite metric spaces. A paper
is relevant here only when its objective and implementation can be expressed
with distances between finitely many records.

| Area | Reference | Relevance to METRIC |
| --- | --- | --- |
| k-center / farthest-first | Gonzalez, "Clustering to Minimize the Maximum Intercluster Distance" (1985) | Mathematical basis for coverage-style representative selection and bounded max assignment distance. |
| k-medoids / k-median | Kaufman and Rousseeuw, [Clustering by Means of Medoids](https://wis.kuleuven.be/stat/robust/papers/publications-1987/kaufmanrousseeuw-clusteringbymedoids-l1norm-1987.pdf); Arya et al., [Local Search Heuristics for k-Median and Facility Location Problems](https://theory.stanford.edu/~kamesh/lsearch.pdf) | Average-assignment compression using actual source records as centers; works from a dissimilarity or distance matrix. |
| Metric nets | Har-Peled and Mendel, [Fast Construction of Nets in Low Dimensional Metrics, and Their Applications](https://arxiv.org/abs/cs/0409057) | Hierarchical nets, covers, and low-doubling finite metrics; useful for scalable metric-only representatives. |
| Density grouping | Ester et al., [DBSCAN](https://cdn.aaai.org/KDD/1996/KDD96-037.pdf); HDBSCAN as research-only | Density-reachable groups, density-unassigned records, and future density-normalization diagnostics over arbitrary distance functions. |
| Graph preservation | Spielman and Teng, [Spectral Sparsification of Graphs](https://arxiv.org/abs/0808.4134); Bhore et al., [Online Spanners in Metric Spaces](https://drops.dagstuhl.de/entities/document/10.4230/LIPIcs.ESA.2022.18) | Preserve graph Laplacian behavior or shortest-path stretch after sparse graph construction. |
| Metric-measure comparison | Memoli, [Gromov-Wasserstein Distances](https://media.adelaide.edu.au/acvt/Publications/2011/2011-Gromov%E2%80%93Wasserstein%20Distances%20and%20the%20Metric%20Approach%20to%20Object%20Matching.pdf) | Compares metric-measure spaces and gives a target language for reduction drift. |

Not-core for this roadmap:

- DPPs, kernel herding, MMD, and Stein-point methods unless the API first
  exposes a metric-only formulation. Requiring a PSD kernel is a different
  mathematical object.
- k-means, PCA-family methods, neural coordinate solvers, and continuous
  Poisson-disk algorithms as default modification routes. They may be useful
  for vector records, but vector records are only one special case.
- Domain interpolation, imputation, and oversampling without an explicit record
  constructor and validity contract.

## Implementation Roadmap

### Phase 1: Roadmap And Contracts

- Publish this document as the shared mathematical and API direction.
- Keep v1 honest: the roadmap names the framework and first stable operators; it
  does not imply every research algorithm is implemented.
- Add cross-links from the engine docs and supported-surface docs when code work
  starts.
- Define promotion rules for each strategy before adding public APIs.

### Phase 2: First Stable API Additions

- Extend compression results with representative multiplicity and normalized
  weights derived from assignments.
- Rename or alias the current farthest-first compression objective as
  `coverage` or `k_center` while preserving existing
  `compress(space, count)` behavior.
- Add radius-bounded compression: the user supplies a coverage radius and the
  implementation chooses as many representatives as needed.
- Add k-medoids compression: minimize average assignment distance rather than
  max assignment distance.
- Add distribution-preserving thinning: start with deterministic regular,
  metric-stratified, or assignment-weighted sampling; keep randomized sampling
  opt-in and seeded.

Current implementation status:

- Implemented: compression multiplicities and normalized weights in C++ and
  Python result objects.
- Implemented: `coverage` / `k_center` aliases for the promoted count-based
  coverage objective.
- Implemented: `radius_coverage` compression where the user supplies a metric
  radius and the implementation chooses the representative count.
- Implemented: `k_medoids` compression where actual source medoids minimize
  average assignment distance.
- Implemented: distribution-preserving thinning via deterministic regular
  source-order sampling (`thin` / `distribution_sample` with
  `preserve_distribution`). The result carries retained source IDs,
  one-to-one lineage, normalized sample weights, metric status, and validity;
  it intentionally leaves the full-source assignment map empty because removed
  records are not collapsed onto representatives by this contract.

### Phase 3: Uniformization And Density Diagnostics

- Add uniform-density thinning using existing separated/radius-cover primitives
  as the metric-only first implementation. Treat this as maximal net
  construction over the finite metric space, not as a continuous Euclidean
  Poisson-disk sampler.
- Add local-density diagnostics based on kNN radius or local volume.
- Report before/after density drift, coverage radius, and representative weights.
- Document when a result preserves empirical distribution versus when it
  intentionally changes density.

Current implementation status:

- Implemented: uniform-density thinning via maximal metric radius nets
  (`uniform_density` / `UniformDensity`) over source records.
- Implemented in C++ and Python: coverage radius, average assignment distance,
  before/after average nearest-neighbor distance, local-density drift,
  before/after average local-volume count and density, and local-volume drift
  diagnostics on uniform-density thinning results.
- Implemented in C++ and Python: top-level assignment summaries for
  `uniform_density` / `equalize`, including source-to-representative
  assignments, nearest-representative distances, representative multiplicities,
  normalized weights, coverage radius, and average assignment distance.
- Implemented: `equalize` as a promoted density-normalization intent over the
  same metric-only maximal radius-net construction.
- Implemented: Python fixture coverage for density drift across edit-distance,
  histogram-transport, and mixed structured-record metrics, proving the
  promoted uniform-density/equalize diagnostics do not require vector records.
- Pending: adaptive equalization beyond a single global radius remains
  research-only until its target density model and promotion fixtures are
  stronger.

### Phase 4: Research Track

- Prototype graph-spanner, spectral-sparsifier, hierarchical-net, and
  Gromov-Wasserstein diagnostics behind experimental docs or examples.
- Keep coordinate-only and kernel-only algorithms out of this roadmap's stable
  surface; document them under `map` only when they are needed for vector-space
  special cases.
- Promote only after deterministic fixtures prove assumptions, expected values,
  and failure modes.

Current implementation status:

- Documented: experimental finite-metric-space research track in
  [Metric-Space Research Track](metric-space-research-track.md), including
  hierarchical nets, adaptive equalization, distance-distribution sketches,
  kNN-recall sketches, graph spanners, spectral graph sparsifiers,
  metric-measure drift, and density hierarchy quotients.
- Prototyped: Python research-only distance-distribution sketch and drift
  diagnostic under `metric.experimental`, with line and edit-distance fixtures.
- Prototyped: Python research-only kNN-recall sketch under
  `metric.experimental`, with line and edit-distance fixtures and explicit
  source-index lineage.
- Prototyped: Python research-only greedy metric graph spanner under
  `metric.experimental`, with line and edit-distance fixtures and stretch
  diagnostics.
- Prototyped: Python research-only hierarchical metric net under
  `metric.experimental`, with line and edit-distance fixtures, source-index
  lineage, per-level assignments, multiplicities, and parent links.
- Prototyped: Python research-only adaptive density equalization under
  `metric.experimental`, with line and edit-distance fixtures, sparse-first
  local-volume priority, assignments, multiplicities, and density diagnostics.
- Prototyped: Python research-only metric-measure drift diagnostic under
  `metric.experimental`, comparing normalized weights and weighted pair-distance
  observables without claiming to solve full Gromov-Wasserstein matching.
- Prototyped: Python research-only density hierarchy quotient under
  `metric.experimental`, with radius-component groups, source medoids,
  multiplicities, parent links, and within-group diagnostics.
- Remaining research gap: spectral sparsification stays documentation-only until
  graph construction assumptions are explicit; none of the research prototypes
  should be promoted before stronger deterministic fixtures and cost limits.
- Next promotion candidates: hierarchical metric nets and metric-measure drift
  diagnostics. They are the first research routes to harden because both are
  native to finite metric-measure spaces and do not require coordinates,
  kernels, centroids, or generated records.

## Promotion Criteria

A strategy can move from research to promoted API only when it has:

- a concrete user problem and record type
- a finite-metric assumption, stated as true metric, pseudo-metric, or distance
- deterministic fixtures with exact expected results or stable contract checks
- complexity notes for exact and approximate routes
- result metadata with lineage, metric status, validity, and diagnostics
- C++ smoke coverage and, where relevant, Python binding coverage
- documentation that explains when the strategy should not be used

Research-only algorithms must not silently appear as default routes. A user can
opt into them, but defaults should remain simple, deterministic, and based only
on finite metric-space data.

## Scale-Safe Operator Plan

Finite metric-space modification is governed by distance-evaluation cost. Stable
operators must plan around metric calls, not vector operations.

Every promoted modifying operator should expose or internally consume:

- a distance-evaluation estimate before exact all-pairs work
- a memory estimate before dense distance-matrix materialization
- an exactness flag and representation name in the result
- a clear refusal or downgrade reason when a budget is exceeded
- reusable distance providers when several objectives are computed together

Operator-specific requirements:

- `compress` / `k_medoids`: preflight the assignment phase and any dense
  grouping provider. Large defaults may use sampled candidates only when the
  result marks `exact=False` and reports sampled-assignment diagnostics.
- `radius_coverage` / `uniform_density` / `equalize`: radius-net construction
  should be able to run against live, cached, blocked, or chunked providers.
  Exact global coverage must not be claimed if chunking only evaluates local
  chunk pairs and representative refinements.
- `sketch`: exact distance-distribution or graph-spanner prototypes must report
  pair count, distance evaluations, and whether a dense matrix was materialized.
  Sampled sketches need seed/policy and quality diagnostics before promotion.
- `metric_measure_drift`: exact pair-observable diagnostics are `O(n^2 + m^2)`;
  promotion requires preflight and a sampled approximation route for large
  finite metric-measure spaces.

Implementation should reuse the existing scale primitives from
[Scale-Safe Performance Plan](scale-performance-plan.md): live distance
providers, symmetric/blocked distance tables, chunked views, landmark providers,
sample plans, and execution-context reuse. A vector-only acceleration is an
adapter optimization, not a substitute for finite-metric planning.

## Test Plan

Add deterministic C++ smoke fixtures before promoting new APIs:

- Line metric: known farthest-first, radius coverage, k-center, and k-medoids
  representatives.
- Dense cluster plus sparse tail: distinguishes distribution-preserving thinning
  from uniform-density thinning.
- String edit distance: proves compression does not require vector centroids.
- Histogram transport/Wasserstein: proves behavior over non-Euclidean records.
- Mixed structured records: exercises composed metrics and representative
  lineage.

Current fixture status:

- Implemented: line-metric compression fixtures for farthest-first, coverage,
  k-center, radius coverage, k-medoids, assignments, multiplicities, and
  normalized weights.
- Implemented: duplicated dense cluster plus sparse-tail thinning fixtures in
  Python and C++ smoke source, proving `PreserveDistribution` keeps empirical
  order while `UniformDensity` / `equalize` collapse duplicate metric balls and
  retain sparse-tail coverage.
- Implemented: string edit-distance and histogram-transport fixtures for
  compression, uniform-density thinning, equalization, and research sketches.
- Implemented: mixed structured-record fixtures for composed metrics and
  uniform-density/equalize lineage.
- Implemented: the `metric_modify_components` CTest label is wired as its own
  time-bounded `core-cpp.yml` CI step and passes locally for the promoted
  modification contracts. Research candidates still need C++ smoke coverage if
  they move out of `metric.experimental`.

Every promoted compression/thinning test should validate:

- assignments cover every source record when collapse is part of the operator
- representative IDs are valid source records
- multiplicities sum to the source record count
- normalized weights sum to `1.0` within numeric tolerance
- coverage radius and average assignment distance are stable
- subset-based derived spaces preserve the source metric law
- validity text distinguishes metric subset, weighted metric-measure summary,
  density correction, and finite-space sketch

Examples should explain which objective to choose:

- preserve source density
- flatten source density
- minimize maximum assignment error
- minimize average assignment error
- preserve neighbor behavior
- preserve distance distribution or graph structure

## Compatibility Notes

Existing `represent`, `compress`, `density_filter`, and `map` APIs should remain valid.
New strategy names should be additive or aliases unless a major-version migration
explicitly permits breaking changes.

The stable surface should continue to favor metric-only algorithms. Vector-only,
kernel-only, or domain-generator methods belong under `map`, `expand`, or
experimental strategy docs and must never be presented as the general solution
for finite metric spaces.
