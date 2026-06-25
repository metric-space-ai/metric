# Entropy Diagnostics

Entropy is an expert diagnostic on an embedded coordinate finite metric space.
The current kpN estimator uses metric neighborhoods and then fits local Gaussian
coordinate volumes, so it is not a generic source-records-plus-source-metric
operator.

The framework path is explicit:

```text
source records + source metric
  -> source finite metric space
  -> map/embed to a coordinate space with an admitted coordinate-neighborhood metric
  -> stats::properties::entropy(mapped_space_or_mapping_result)
```

The lower-level `mtrc::Entropy` estimator remains available for regression and
expert use, but it is not the promoted pipeline shortcut for arbitrary source
metrics. The promoted stats wrapper should start from a coordinate space or a
`MappingResult`.

## Current C++ Shape

```cpp
#include <metric/core/metric_space.hpp>
#include <metric/metric/catalog.hpp>
#include <metric/modify/map/map.hpp>
#include <metric/stats/properties/entropy.hpp>

#include <vector>

std::vector<int> source_records = {0, 1, 2, 3};
auto source_metric = [](int lhs, int rhs) {
    return static_cast<double>(lhs > rhs ? lhs - rhs : rhs - lhs);
};
auto source_space = mtrc::make_space(source_records, source_metric);

auto mapped = mtrc::map(
    source_space,
    [](int value) {
        return std::vector<double>{
            static_cast<double>(value),
            static_cast<double>(value * value),
        };
    },
    mtrc::Euclidean<double>{});

auto result = mtrc::stats::properties::entropy(mapped, 3, 2);
```

## Estimator Assumptions And Edge Cases

`mtrc::Entropy` is a kpN local-Gaussian, Kozachenko-Leonenko-style estimator of the
**differential** entropy of a finite metric space ([hal-01272527](https://hal.inria.fr/hal-01272527/document)). Read the result with these assumptions in mind:

- **Coordinate space is authoritative.** The estimate uses the derived space's
  metric for neighborhoods and the derived records' coordinates for local
  Gaussian volumes.
- **Parameters `k` and `p`.** `k` is the nearest-neighbor count; `p` is the local approximation order (number of realizations used to calibrate each local Gaussian). For small spaces both are **silently clamped** (`p` to at least 3 and at most `n - 1`, `k` to at least 2 and below `p`). The constructor arguments `(metric, 3, 2)` above therefore run with an effective `p = 3`. The reported `neighbor_count`/`approximation_order` in `EntropyResult` are the *requested* values, which may differ from the effective ones.
- **Minimum size.** At least 4 records are required. For fewer records (including an empty space) the estimate is the sentinel `std::nan("estimation failed")`; check `std::isnan(result.value)` rather than the `exact` flag to detect failure.
- **Equal record depth.** This estimator requires equal-length coordinate
  records because the local Gaussian calibration uses one record depth for the whole
  finite space. Ragged coordinate records return the NaN sentinel through the
  promoted wrapper instead of being silently truncated or reinterpreted.
- **No-information spaces.** The same `std::nan("estimation failed")` sentinel is returned when **no** point yields a valid local Gaussian — for example a zero-diameter space whose records are all identical, where every neighborhood collapses. There is then no local geometry to estimate from.
- **Units and sign.** The value is a differential entropy in nats and **can be negative**. With `exponentiated = true` (`mtrc::Entropy(metric, k, p, true)`), values below 1 are remapped onto a strictly positive, order-preserving scale.
- **Invariances.** The estimate is translation-invariant. It is not guaranteed invariant to record ordering when neighbor distances tie.

These behaviors are pinned by `tests/core_smoke/metric_entropy_properties_smoke.cpp` and `tests/core_smoke/metric_entropy_estimator_smoke.cpp`.

## Estimator Bias — Reference Decision

The kpN estimate is `digamma(n) - digamma(k) + (1/n) * Σ_valid (logG_i - log g_i)`, where `n`
is the record count, the sum runs over the points that produced a valid local Gaussian, and
`k` is the requested neighbor count.

A deliberate reference decision governs the small-space regime: when *no* point yields a
valid local Gaussian, the estimator returns NaN; see "No-information spaces" above. Every
**normalization** question is left **pinned** to the promoted regression values under a
single consistent standard — do not alter valid-input fixtures without sign-off plus
regeneration against the
[hal-01272527](https://hal.inria.fr/hal-01272527/document) reference, for which there is no
in-repo oracle. Three coupled, debatable normalization choices are therefore left untouched:

- **The mean divides by `n`.** Because the sum only includes points with a valid local
  Gaussian, dividing by `n` implicitly assigns 0 to rejected points (whose true contribution
  diverges). Dividing by the count of valid points is an equally ad-hoc alternative and would
  itself be inconsistent unless `digamma(n)` also changed. For every non-degenerate space all
  points contribute, so the distinction is moot there.
- **`digamma(n)` uses the full record count `n`.**
- **`digamma(k)` keeps the *requested* `k`, not the clamped effective `k`.** For small spaces
  `k` and `p` are clamped to match the space, so the neighbor count that actually defined each
  `eps` can be smaller than the reported `k`. Using `digamma(k)` rather than
  `digamma(k_effective)` means two requests that clamp to the *same* geometry (e.g.
  `(k=3, p=2)` and `(k=7, p=70)` on four records both run `k_effective=2, p_effective=3`)
  report **different** entropies that differ only by `digamma(k_requested)`. Correcting this
  collapses those fixtures, so it is deferred with the others.

The production regime (large `n`, no clamping) is unaffected by any of these choices.

The restored Python core wheel does not promote pipeline entropy bindings yet.
The Python correlation entropy adapter is an expert estimator surface,
not the framework pipeline API. Interpretation depends on the embedding,
coordinate metric, neighborhood size, sampling density, and scale.
