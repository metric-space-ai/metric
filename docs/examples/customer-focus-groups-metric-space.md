# Customer Focus Groups As A Finite Metric Space

This example models customer rows as records in a finite metric space instead
of flattening them into a generic vector table first.

Source:

- [customer_focus_groups_metric_space.py](../../python/examples/engine/customer_focus_groups_metric_space.py)

## Why This Is A Finite Metric-Space Example

The records contain mixed fields:

- categorical region and plan
- numeric monthly revenue and seat count
- product sets
- event/journey sequences

A vector-first workflow has to choose encodings, scales, missing-value policy,
and centroid semantics before it can run a standard clustering method. That
often produces centers that are numeric artifacts rather than customers. In this
example, the metric is the primary object: each field contributes a real finite
record distance, and the focus-group representatives are medoids, i.e. actual
source customers.

## Workflow

```text
customer table -> composed customer metric -> finite metric space
               -> k-medoids compression -> real reference personas
               -> radius/equalized panels -> coverage-oriented panels
```

The example uses `Space.compress(strategy=KMedoids(groups=3))` to produce three
weighted metric-measure representatives. The result keeps:

- source customer IDs for the medoid personas
- assignment of every customer to a persona
- per-persona multiplicity and normalized weight
- `metric_status` and `validity` text

It also splits the space by `plan` and derives one medoid persona per subspace,
showing the focus-group workflow directly over the finite metric space.

## Expected Output

```text
metric personas = ['globex', 'stark', 'northwind']
persona weights = [0.4, 0.3, 0.3]
split personas = ['globex', 'northwind', 'stark']
radius coverage panel = ['acme', 'initech', 'northwind', 'wonka', 'stark', 'wayne']
uniform-density panel = ['acme', 'initech', 'northwind', 'wonka', 'stark', 'wayne']
vector-first centers are synthetic = [(1.605, 0.73, 2.0), (0.097, 0.025, 0.0), (0.427, 0.18, 1.0)]
```

The last line is the important contrast: the vector baseline produces synthetic
numeric centers. The metric-space route produces real customer records that can
be inspected, named, weighted, and traced back to source rows.
