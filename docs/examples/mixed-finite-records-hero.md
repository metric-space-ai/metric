# Mixed Finite Metric Records (Hero)

**Question answered:** *Can one heterogeneous record — text, a histogram/image
signal, a time series, and numeric vitals together — be compared directly, with a
real admitted metric per field, so that nearest-neighbour search, clustering,
representatives, outliers and cross-space dependence all see structure that a naive
flattened-vector baseline throws away?*

This is METRIC's composite finite-metric workflow. A single record carries four
genuinely different fields, and each field is scored by a **real, admitted METRIC
true metric** (`metric_law::metric`) — never a hand-rolled stand-in and never a
length-normalised pseudo-distance:

| Field      | Meaning                          | Native metric                    | `record_kind`      |
|------------|----------------------------------|----------------------------------|--------------------|
| `code`     | operator status / log token      | `mtrc::Edit<char>` (Levenshtein) | `sequence`         |
| `spectrum` | unit-mass intensity histogram    | `mtrc::Wasserstein<double>`      | `structured`       |
| `curve`    | variable-length process curve    | `mtrc::TWED<double>`             | `sequence`         |
| `vitals`   | fixed-dimension sensor readout   | `mtrc::Euclidean_standardized`   | `aligned_vector`   |

The composite distance is a **conic (strictly-positive-weighted) sum** of those four
true metrics over the product domain:

```text
D(x, y) = w_code     * Edit(x.code,     y.code)
        + w_spectrum  * W1 (x.spectrum, y.spectrum)
        + w_curve     * TWED(x.curve,    y.curve)
        + w_vitals    * Estd(x.vitals,   y.vitals)
```

A non-negative-weighted sum of metrics is itself a metric, so with **strictly
positive, constant** weights `D` is a true metric: non-negative, symmetric,
triangle-respecting, and zero exactly when every field is equal (identity of
indiscernibles needs every weight `> 0`). The weights are fixed numbers baked into
the metric instance — **not** per-pair normalisers; dividing edit distance by the
longer string, for instance, would break the triangle inequality, so that trap is
deliberately avoided. This is why the composite honestly declares
`metric_law::metric`, and why the cover-tree index (which rejects non-metrics)
accepts it.

Sources:

- application — [mixed_finite_records.cpp](../../examples/engine/mixed_finite_records.cpp)
- composite metric + baseline — [mixed_finite_records.hpp](../../examples/engine/mixed_finite_records.hpp)
- deterministic fixture — [mixed_finite_records_fixture.hpp](../../examples/engine/mixed_finite_records_fixture.hpp)
- metric-admission proof / core-gate test — [metric_mixed_records_smoke.cpp](../../tests/core_smoke/metric_mixed_records_smoke.cpp)

Command:

```bash
build/core/examples/engine/engine_mixed_finite_records
```

## The fleet and the naive baseline

The fixture is a deterministic condition-monitoring fleet: four fault families
(`valveflow`, `bearingrun`, `pumphead`, `filterbed`) × five severity levels, plus
one injected novel fault. Two latent axes drive every record:

- **family** — carried by the text code, the curve *phase*, and a 500×-scale vital;
- **severity** — carried by the spectrum *peak position*, the curve *amplitude*, and
  a small-scale vital. Severity is the held-out outcome the cross-space test targets.

The baseline is "what you reach for without METRIC": flatten every field into one
numeric vector (a 27-bin character histogram for the text, the raw spectrum bins, a
zero-padded fixed-width curve, the raw vitals) and run ordinary Euclidean
nearest-neighbour search. The projection is faithful but structurally blind.

## Expected output

```text
=== Mixed Finite Metric Records ===
fleet records = 20
composite metric law = metric
composite cache key = hero::MixedRecordMetric:bins=16:w_code=0.600000:w_spectrum=1.000000:w_curve=0.500000:w_vitals=1.000000:vitals=mtrc::Euclidean_standardized:n=4:2.1213203435596424:559.01699437494744:0.70178344238091017:0.60827625302982191:curve=mtrc::TWED:penalty=0.000000:elastic=1.000000:zero_padded=0
cover-tree accepted composite metric = yes (records = 20)

--- channel audit (weighted contributions) ---
within-family  (sev0 vs sev4): code=0.0000 spectrum=6.0606 curve=6.2000 vitals=4.2241 total=16.4847
across-family  (fam0 vs fam1): code=5.4000 spectrum=0.0000 curve=3.0000 vitals=1.1898 total=9.5898

--- search ---
brute representation = metric_space, cover-tree representation = cover_tree_index
brute nearest = valveflow sev2.0000 at 0.0000
cover-tree nearest = valveflow sev2.0000 at 0.0000

--- per-channel probes (composite vs naive flatten-and-L2 baseline) ---
channel code(text):
    composite picks text_target (target=text_target OK)
    flat picks      text_decoy (decoy=text_decoy -> baseline lost it)
    composite d(query,target)=0.6000  d(query,decoy)=3.6000
    why: Edit distance keeps character order; the histogram collapses anagrams to zero.
channel spectrum(histogram):
    composite picks spectrum_target (target=spectrum_target OK)
    flat picks      spectrum_decoy (decoy=spectrum_decoy -> baseline lost it)
    composite d(query,target)=0.7576  d(query,decoy)=3.2262
    why: Wasserstein measures transport cost; bin-wise L2 rewards accidental bin overlap.
channel curve(time series):
    composite picks curve_target (target=curve_target OK)
    flat picks      curve_decoy (decoy=curve_decoy -> baseline lost it)
    composite d(query,target)=2.5000  d(query,decoy)=2.7000
    why: TWED edits the elastic length change cheaply; zero padding misaligns the tail, while a uniform baseline drift looks small to positional L2.
probes: composite correct = 3/3, flat wrong = 3/3

--- representatives (k=4) ---
strategy = farthest_first, coverage radius = 16.4847, avg nearest = 6.4810
    rep 0 = valveflow sev0.0000
    rep 1 = pumphead sev4.0000
    rep 2 = bearingrun sev3.0000
    rep 3 = filterbed sev2.0000

--- clustering (k-medoids, k=4) ---
composite: algorithm = kmedoids, clusters = 4, family purity = 1.0000
flat:      clusters = 4, family purity = 1.0000
(note: family is redundantly encoded -- distinct code letter-multisets, the curve phase, and a
 family vital -- so the flat baseline also separates families; the composite's edge is in the
 per-channel probes and the severity MGC below, not in family purity.)

--- distance diagnostics ---
fleet nearest-neighbour distance: min=3.3032 median=3.3032 max(excl anomaly)=3.3032
anomaly nearest-neighbour distance = 39.1314

--- outliers / structure (DBSCAN) ---
strategy = dbscan_noise, representation = distance_table, flagged = 1 of 21
    outlier index 20 (injected anomaly)

--- cross-space MGC vs latent severity ---
algorithm = mgc
composite-space MGC       = 0.4069
standardized-flat MGC     = 0.1313 (fair baseline)
raw-flat MGC              = -0.1263 (un-standardized)
composite advantage       = 0.2756

all invariants hold
```

## What METRIC finds that the vector baseline loses

Each probe holds three fields fixed and varies one, so a single field metric is on
trial. The probes use the **literal naive baseline** — flatten every field and run
plain Euclidean (z-scoring a probability histogram would itself be unusual). In every
case the composite metric picks the structurally correct record and that baseline
picks a decoy it cannot tell apart:

| Channel              | Composite (real metric)                        | Naive flatten-and-L2 blind spot                            |
|----------------------|------------------------------------------------|------------------------------------------------------------|
| `code` (text)        | Edit keeps character **order**                  | a 27-bin character histogram collapses anagrams to distance 0 |
| `spectrum` (signal)  | Wasserstein measures **transport cost**         | bin-wise L2 rewards accidental bin overlap, ignores ground distance |
| `curve` (time series)| TWED edits an **elastic length** change cheaply | zero-padding to a fixed width shoves the tail out of alignment |

The cross-space test makes the same point quantitatively, and against a deliberately
**fair** baseline. **Multiscale Graph Correlation (MGC)** — a dependence statistic in
`[-1, 1]`, never a distance — is run between each geometry and the latent severity
outcome via the public `mtrc::compare()` intent. Because the composite already
standardises its vitals field, the headline baseline is the **per-column z-scored**
flat vector (the same scale correction), so the comparison is not won by a raw-scale
mismatch:

- composite-space MGC vs severity = **0.407**
- standardized-flat MGC vs severity = **0.131** (fair baseline)
- raw-flat MGC vs severity = **−0.126** (un-standardized, for reference)

Even after scale-correcting the baseline, the composite's geometry tracks the latent
severity outcome far better (advantage **0.276**): its severity signal lives on a
moving spectrum peak (saturated under bin-wise L2), a family-dependent curve phase
(swamps positional L2), and a small-scale vital (recovered only by standardisation).
Standardisation alone closes part of the gap (`−0.126 → 0.131`) but cannot recover the
transport and elastic structure, which is what the real per-field metrics provide.

Clustering is **not** where the gap shows: family is redundantly encoded (distinct
code letter-multisets, the curve phase, and a family vital), so k-medoids reaches
family purity `1.0` on *both* the composite metric and the flat baseline. The
composite's edge is in the per-channel probes and the cross-space severity MGC, not in
family purity — the doc states this rather than overclaiming a clustering win.

DBSCAN over the composite metric isolates the injected novel fault as the **only**
outlier (its nearest neighbour is `39.1` away, against a fleet nearest-neighbour
distance of `3.3`), and farthest-first selects one representative per fault family.

## Why this is a true metric, not a marketing claim

The companion core-gate test `metric_mixed_records_smoke` is the proof that the
composite is admitted, not asserted:

- `static_assert`s on the trait (`metric_law::metric`, `record_kind::structured`,
  thread-safe);
- the full metric axiom battery (non-negativity, symmetry, identity of
  indiscernibles, triangle inequality) over a hand-built finite domain that varies
  all four fields, **plus** a deterministic randomized property search over thousands
  of random records;
- the admission gate — strictly positive finite weights are mandatory (a zero or
  negative weight is rejected, because it would demote the composite to a
  pseudometric), and every per-field guard (equal-mass spectra, non-empty finite
  curves) propagates through the composite;
- metric-only routing — the cover-tree index accepts the composite, which it would
  refuse for any `metric_law::distance` callable.

## Target Python shape (aspirational)

The composite-metric API below is **planned, not yet implemented**: today's Python
package is adapter-only and `metrics.Composite` does not exist, so this block is
pseudocode showing the intended shape, not runnable code. The C++ application is the
only runnable reference. (`Space.compare()` and the structural operators are
native-only and raise `StrategyUnavailableError` in the pip-installed wheel.)

```python
# Aspirational API — not yet available in the metric Python package.
from metric import Space, metrics

# one composite metric over four heterogeneous fields, each a real metric
record_metric = metrics.Composite(
    code=metrics.Edit(weight=0.6),
    spectrum=metrics.Wasserstein(weight=1.0),
    curve=metrics.TWED(weight=0.5),
    vitals=metrics.EuclideanStandardized(weight=1.0),
)

fleet = Space(records, metric=record_metric)
fleet.neighbors(query, count=3)               # cover-tree routed (admitted metric)
fleet.representatives(count=4)                # farthest-first archetypes
fleet.groups(count=4)                         # k-medoids fault families
fleet.outliers(strategy="dbscan", ...)        # DBSCAN novelty
fleet.compare(outcome_space, strategy="mgc")  # cross-space dependence vs severity
```

The C++ application is the reference: every claim above is checked in the runnable
example and re-proved, axiom by axiom, in the core smoke test.
