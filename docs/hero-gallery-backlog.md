# Hero Gallery Backlog

This backlog tracks public METRIC hero examples. A gallery item is real only when
it points to checked data, checked native C++ evidence, and a visual generated
from that evidence or from a documented reference asset. Planned items are
placeholders and must not claim results.

## Evidence Contract

Every public gallery item needs:

- **Dataset/source:** real dataset or documented reference asset, including local
  path and license/source notes when available.
- **Native C++ evidence:** checked-in C++ executable or source file that verifies
  counts, invariants, exported values, and failure conditions.
- **Visualization:** visual output generated from checked evidence or a documented
  reference asset. No screenshots or hand-authored result images as evidence.
- **Bindings:** show language bindings only after they exist and route to native
  C++ code. Until then, keep the public hero native C++ only.

## Gallery Items

| Hero | Status | Dataset/source | Native C++ evidence | Visualization |
|---|---|---|---|---|
| MNIST dimension reduction | LIVE | Full MNIST training set, `60,000` records; parametric diffusion coordinate/GRAE10 reference documented in `docs/examples/mnist-grae10-gallery.md`. | `examples/engine/mnist_grae10_integrity.cpp` verifies record count, label match, finite 2D coordinates, and finite 3D coordinates. | `visual/examples/grae10-metric-engine/index.html` is the native METRIC Visual Engine 60k plot. |
| UCR process curves | LIVE | UCR Time Series Anomaly Detection datasets 2021, DOI `10.6084/m9.figshare.26410744.v1`, CC BY 4.0; local slices documented in `docs/examples/process-curve-external-gallery.md`. | `examples/engine/process_curve_external_gallery.cpp` exports query winners, margins, tables, and assertions. | `docs/examples/assets/process-curve-external/hero-overview.svg` plus CSV/summary assets. |
| Mixed records | PLANNED | Needs a real heterogeneous source, or a clearly documented checked fixture if no public source is suitable. | Needs native composed-metric evidence with per-field contributions, neighbors, representatives, outliers, and invariants. | Needs a visual showing native field structure and composed metric behavior from exported C++ evidence. |
| Cross-space dependence | PLANNED | Needs a real paired-space dataset with documented relation between spaces. | Needs native dependence inputs, admitted metrics, baseline construction, and statistic export. | Needs paired-space visual plus dependency field from exported evidence. |
| Condition monitoring | PLANNED | Needs real process runs with operating labels, anomaly windows, or maintenance state. | Needs native entropy/anomaly diagnostics, thresholds, nearest records, and invariant checks. | Needs time-aware metric-space visual from exported evidence. |
| Relation matrix | PLANNED | Needs a real relation-rich record set or documented finite metric dataset with meaningful labels. | Needs native metric table, neighborhood queries, representatives, and metric-law checks. | Needs readable block-ordered matrix and neighborhood visual from exported C++ evidence. |
| Dynamics | PLANNED | Needs real temporal or perturbation data, or a reproducible native process model explicitly documented as such. | Needs native transition inputs, finite-space trajectories, stability diagnostics, and comparisons. | Needs trajectory and perturbation visual from exported evidence. |
| Metric discovery | PLANNED | Needs candidate metrics, record domains, admission criteria, and rejected candidates. | Needs native metric-admission results, rejected-candidate evidence, and validation checks. | Needs interactive metric-selection visual from exported evidence. |

## Known MNIST Gap

The public MNIST hero is accepted as a protected reference visual plus a native
integrity check. The tree does not yet contain a native C++ executable that
re-trains the full `60,000` record parametric diffusion coordinate/GRAE10 workflow and exports the
accepted visual dataset from scratch.

The existing native parametric diffusion coordinate executable proves the C++ path on a balanced MNIST
subset. It must not be presented as a replacement for the accepted 60k reference
until a full-scale exporter exists and is verified.

## Future-Agent Checklist

1. Start from dataset/source and license.
2. Add or extend native C++ evidence before building the visual.
3. Export machine-readable evidence.
4. Render from exported evidence or a documented reference asset.
5. Remove `PLANNED` only after the checked evidence is present.
