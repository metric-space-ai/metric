# Engine Demo Expected Outputs

The flagship C++ engine demos are CI-safe smoke fixtures under
`examples/engine/`. These snippets record the diagnostic output shape produced
by the promoted examples. Numeric values are deterministic for the current core
fixtures; future algorithm changes should update these snippets with the
corresponding test and fixture changes.

## String Edit Space

Command:

```bash
build/core/examples/engine/engine_strings_edit_space
```

Expected output:

```text
nearest string = metrics at 1
string groups = 2
```

## Process Curves

Command:

```bash
build/core/examples/engine/engine_process_curves_space
```

Expected output:

```text
nearest process curve = baseline at 1
process curve graph edges = 2
```

## Vector Space As A Special Case

Command:

```bash
build/core/examples/engine/engine_vector_space_special_case
```

Expected output:

```text
record_kind = aligned_vector
metric_law = metric
vector dimension metadata = 2
available representations = metric_space,matrix_cache,cover_tree_index,knn_graph_index
nearest vector = alpha-0 at 0.141421
indexed nearest = alpha-0 via cover_tree_index
runtime policy = exact_lazy_serial via metric_space
matrix cache reuse = matrix_cache,matrix_cache
vector groups = 3
vector outlier = outlier score 10.9567
mapped vector dimensions = 2
```

## Histogram Transport Space

Command:

```bash
build/core/examples/engine/engine_histogram_transport_space
```

Expected output:

```text
nearest histograms = 1, 3
histogram runtime representation = matrix_cache
```

## Mixed Structured Records

Command:

```bash
build/core/examples/engine/engine_mixed_structured_records
```

Expected output:

```text
nearest mixed records = warmup-drift, warmup-alert
warmup distance = 0.216838
message contribution = 0
mixed record groups = 2 via matrix_cache
mixed record outlier = spike-stop
runtime policy = exact_materialized_serial via matrix_cache
```

## Cross-Space Dependency

Command:

```bash
build/core/examples/engine/engine_cross_space_mgc
```

Expected output:

```text
cross-space MGC = 1
```

## Learnable Structure Map

Command:

```bash
build/core/examples/engine/engine_phate_autoencoder_map
```

Expected output:

```text
source record_kind = aligned_vector
source metric_law = metric
source fixture size = 5
source codec = vector_record_codec
source nearest neighbors = 2 via matrix_cache
source groups = 2 via matrix_cache
source dbscan noise = 0 via matrix_cache
runtime policy = exact_materialized_serial via knn_graph_index
diffusion steps = 2
diffusion kernel scale = 1
diffusion graph recall = 1
training epochs = 160
PHATE-AE bottleneck loss: 0.27826 -> 0.179383
PHATE-AE reconstruction MSE: 2.0274
out-of-sample support = parametric_space_transform
out-of-sample records = 2
deterministic seed = 23
```
