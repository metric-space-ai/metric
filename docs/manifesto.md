# Metric-Space Numerics Manifesto

METRIC is built around a simple claim: the dataset becomes numerical when the user defines a meaningful metric on its records.

That metric may come from a vector norm, but it does not have to. Strings can use edit distance. Time series can use TWED. Histograms can use transport cost. Images can use structural or distributional costs. Industrial observations can combine domain-specific record comparisons. In each case, the geometry comes from the metric, not from forcing the record into coordinates first.

## Why This Is Broader Than Vector Search

Vector search starts from coordinates and a vector distance. That is the right model when the embedding already preserves the relationships that matter. METRIC treats that as one useful specialization rather than the foundation.

Metric-space numerics starts one layer earlier:

1. Choose records in their natural form.
2. Define or select a distance that expresses the cost of comparing those records.
3. Build an implicit or explicit finite metric space.
4. Run operators, mappings, and diagnostics on that space.

This keeps the modeling decision visible. If an embedding is useful, it can be introduced as a mapping between metric spaces. If a sparse graph or search tree is useful, it can be introduced as a representation of the same space. The representation does not replace the metric contract.

## What METRIC Should Make Easy

METRIC should make these workflows ordinary:

- construct finite metric spaces from arbitrary record types
- switch between matrix, graph, and tree representations
- run nearest-neighbor, entropy, correlation, sparsification, and mapping operators with explicit assumptions
- keep vector-space methods available when vectors are the right records
- keep neural approximators and embeddings as optional strategies, not as the conceptual center

The project succeeds when users can explain both the record type and the metric before they choose an operator.

## What METRIC Should Avoid

METRIC should not hide domain geometry behind a default embedding step. It should not present experimental mapping or neural code as stable core capability. It should not make broad algorithm lists more visible than the small, tested path for constructing and using a finite metric space.

The stable public frame is therefore narrower than the historical codebase: compute with metric spaces directly, preserve research breadth behind clear stability labels, and promote new algorithms only when their assumptions, examples, and tests are explicit.
