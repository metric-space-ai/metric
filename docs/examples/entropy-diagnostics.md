# Entropy Diagnostics

Metric-space entropy is an expert diagnostic on a finite metric space. It estimates local freedom in the dataset from distances, so the metric remains part of the diagnostic instead of being hidden behind a vector embedding.

The same operator works across record families when each family has an explicit metric:

- numeric records with Chebyshev distance
- string records with edit distance
- structured records with a domain metric

The promoted C++ example [metric_space_entropy.cpp](../../examples/core/metric_space_entropy.cpp) runs in the core CI path and asserts regression values for numeric and well-posed string records.

## Current C++ Shape

```cpp
#include <metric/correlation/entropy.hpp>
#include <metric/metric/catalog.hpp>

#include <string>
#include <vector>

std::vector<std::string> records = {
    "AAA",
    "BBB",
    "ABA",
    "ABB",
    "BAA",
    "BAB",
};

mtrc::Entropy<void, mtrc::Edit<char>> entropy(mtrc::Edit<char>(), 3, 2);
double value = entropy(records);
```

## Estimator Assumptions And Edge Cases

`mtrc::Entropy` is a kpN local-Gaussian, Kozachenko-Leonenko-style estimator of the
**differential** entropy of a finite metric space ([hal-01272527](https://hal.inria.fr/hal-01272527/document)). Read the result with these assumptions in mind:

- **Metric is authoritative.** The estimate is computed from metric values only; the default record family uses Chebyshev distance.
- **Parameters `k` and `p`.** `k` is the nearest-neighbor count; `p` is the local approximation order (number of realizations used to fit each local Gaussian). For small spaces both are **silently clamped** (`p` to at least 3 and at most `n - 1`, `k` to at least 2 and below `p`). The constructor arguments `(metric, 3, 2)` above therefore run with an effective `p = 3`. The reported `neighbor_count`/`approximation_order` in `EntropyResult` are the *requested* values, which may differ from the effective ones.
- **Minimum size.** At least 4 records are required. For fewer records (including an empty space) the estimate is the sentinel `std::nan("estimation failed")`; check `std::isnan(result.value)` rather than the `exact` flag to detect failure.
- **Equal record depth.** This estimator requires equal-length records because the local Gaussian fit uses one record depth for the whole finite space. Ragged strings or ragged numeric records return the NaN sentinel through the promoted wrapper instead of being silently truncated or reinterpreted.
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
  `k` and `p` are clamped to fit the space, so the neighbor count that actually defined each
  `eps` can be smaller than the reported `k`. Using `digamma(k)` rather than
  `digamma(k_effective)` means two requests that clamp to the *same* geometry (e.g.
  `(k=3, p=2)` and `(k=7, p=70)` on four records both run `k_effective=2, p_effective=3`)
  report **different** entropies that differ only by `digamma(k_requested)`. Correcting this
  collapses those fixtures, so it is deferred with the others.

The production regime (large `n`, no clamping) is unaffected by any of these choices.

The restored Python core wheel does not promote entropy bindings yet. Entropy belongs in the Expert API because interpretation depends on metric assumptions, neighborhood size, sampling density, and scale.
