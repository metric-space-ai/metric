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
string groups = 4
string benchmark metric winner = alerts at 1
string benchmark vector baseline winner = alter at 0
string benchmark baseline mismatch = yes
string benchmark records = 12
string benchmark queries = 4
string benchmark metric correct = 4/4
string benchmark vector mismatches = 4/4
string benchmark average metric margin = 2
string benchmark dense evaluations = 144
```

## Process Curves

Command:

```bash
build/core/examples/engine/engine_process_curves_space
```

Expected output:

```text
process hero metric winner = late_ramp at 2
process hero vector baseline winner = flat_hold at 3
process hero baseline mismatch = yes
process hero metric margin = 3
process hero groups = 2
process hero outliers = 2
process hero dense evaluations = 25
process curve graph edges = 4
process benchmark records = 10
process benchmark queries = 4
process benchmark metric correct = 4/4
process benchmark vector mismatches = 4/4
process benchmark average metric margin = 3
process benchmark dense evaluations = 100
process benchmark graph edges = 24
process gallery records = 15
process gallery queries = 6
process gallery metric correct = 6/6
process gallery vector mismatches = 6/6
process gallery average metric margin = 2.33333
process gallery dense evaluations = 225
process gallery graph edges = 60
```

## External Process Curve Gallery

Command:

```bash
build/core/examples/engine/engine_process_curve_external_gallery
```

Expected output:

```text
process external source = UCR_Time_Series_Anomaly_Detection_2021
process external license = CC BY 4.0
process external domains = 2
process external records = 48
process external queries = 16
process external power_demand records = 24
process external power_demand queries = 8
process external power_demand metric correct = 8/8
process external power_demand vector mismatches = 8/8
process external power_demand average metric margin = 370.005
process external power_demand first query = downsampled_power_demand_044_anomaly_start_18485
process external power_demand first metric winner = power_demand_044_anomaly_start_18485 at 324
process external power_demand first vector winner = power_demand_045_pre_anomaly_23152 at 592.967
process external power_demand dense evaluations = 576
process external internal_bleeding records = 24
process external internal_bleeding queries = 8
process external internal_bleeding metric correct = 8/8
process external internal_bleeding vector mismatches = 8/8
process external internal_bleeding average metric margin = 345.164
process external internal_bleeding first query = downsampled_internal_bleeding_026_anomaly_start_5684
process external internal_bleeding first metric winner = internal_bleeding_026_anomaly_start_5684 at 324
process external internal_bleeding first vector winner = internal_bleeding_026_pre_anomaly_5479 at 180.819
process external internal_bleeding dense evaluations = 576
process external dense evaluations = 1152
```

## Process Curve PHATE Map

Command:

```bash
build/core/examples/engine/engine_process_curve_phate_map
```

Expected output:

```text
process PHATE records = 10
process PHATE queries = 4
process PHATE source metric = aligned_curve_distance
process PHATE feature codec = process_curve_feature_codec
process PHATE pipeline components = 11
process PHATE pipeline codec = feature_record_codec
process PHATE distance provider = matrix_cache_distance_provider
process PHATE affinity kernel = exponential_affinity_kernel
process PHATE diffusion operator = lazy_row_normalized_diffusion_operator
process PHATE geometry target evaluations = 100
process PHATE metric nearest family = late_ramp
process PHATE vector baseline family = flat
process PHATE latent nearest family = late_ramp
process PHATE metric correct = 4/4
process PHATE vector misses = 4/4
process PHATE latent correct = 4/4
process PHATE query = query_delayed_ramp:metric=late_ramp,vector=flat,latent=late_ramp
process PHATE query = query_delayed_ramp_long:metric=late_ramp,vector=flat,latent=late_ramp
process PHATE query = query_delayed_ramp_short:metric=late_ramp,vector=normal,latent=late_ramp
process PHATE query = query_delayed_ramp_deep_delay:metric=late_ramp,vector=flat,latent=late_ramp
process PHATE bottleneck loss = 0.6213 -> 0.160793
process PHATE out-of-sample support = parametric_space_transform
process PHATE out-of-sample anchor recall = 0.5
process PHATE mapped-anchor source rank = 1.75
process PHATE max mapped-anchor source rank = 3
process PHATE original-metric distance penalty = 0.25
process PHATE max original-metric distance penalty = 1
process PHATE inverse support = none_feature_codec
process PHATE deterministic seed = 29
```

## Process Curve PHATE Gallery

Command:

```bash
build/core/examples/engine/engine_process_curve_phate_gallery
```

Expected output:

```text
process PHATE gallery records = 15
process PHATE gallery queries = 6
process PHATE gallery source metric = aligned_curve_distance
process PHATE gallery feature codec = process_curve_gallery_feature_codec
process PHATE gallery pipeline components = 11
process PHATE gallery distance provider = matrix_cache_distance_provider
process PHATE gallery affinity kernel = exponential_affinity_kernel
process PHATE gallery diffusion operator = lazy_row_normalized_diffusion_operator
process PHATE gallery target evaluations = 225
process PHATE gallery metric nearest family = late_ramp
process PHATE gallery vector baseline family = flat
process PHATE gallery latent nearest family = late_ramp
process PHATE gallery metric correct = 6/6
process PHATE gallery vector misses = 6/6
process PHATE gallery latent correct = 6/6
process PHATE gallery average metric margin = 2.16667
process PHATE gallery query = gallery_delay_A:metric=late_ramp,vector=flat,latent=late_ramp
process PHATE gallery query = gallery_delay_B:metric=late_ramp,vector=flat,latent=late_ramp
process PHATE gallery query = gallery_delay_C:metric=late_ramp,vector=normal,latent=late_ramp
process PHATE gallery query = gallery_delay_D:metric=late_ramp,vector=normal,latent=late_ramp
process PHATE gallery query = gallery_delay_E:metric=late_ramp,vector=flat,latent=late_ramp
process PHATE gallery query = gallery_delay_F:metric=late_ramp,vector=flat,latent=late_ramp
process PHATE gallery neighbor preservation = 0.4
process PHATE gallery OOS anchor recall = 0.833333
process PHATE gallery first-anchor match rate = 0.833333
process PHATE gallery mapped-anchor source rank = 1.33333
process PHATE gallery max mapped-anchor source rank = 3
process PHATE gallery original-metric distance penalty = 0.166667
process PHATE gallery max original-metric distance penalty = 1
process PHATE gallery bottleneck loss = 0.46694 -> 0.0860003
process PHATE gallery inverse support = none_feature_codec
process PHATE gallery deterministic seed = 41
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
nearest histograms = shifted_mass_0, vector_decoy_6
histogram runtime representation = matrix_cache
histogram benchmark metric winner = shifted_mass_0 at 1
histogram benchmark vector baseline winner = vector_decoy_0 at 0.707107
histogram benchmark baseline mismatch = yes
histogram benchmark records = 8
histogram benchmark queries = 4
histogram benchmark metric correct = 4/4
histogram benchmark vector mismatches = 4/4
histogram benchmark average metric margin = 3.25
histogram benchmark dense evaluations = 64
```

## Distribution Image Recoding

Command:

```bash
build/core/examples/engine/engine_distribution_image_recoding
```

Expected output:

```text
distribution hero metric winner = vertical_shift_left at 1
distribution hero vector baseline winner = horizontal_at_vertical_query at 0.666667
distribution hero baseline mismatch = yes
distribution hero metric margin = 0.333333
distribution hero groups = 2
distribution hero dense evaluations = 64
distribution benchmark records = 8
distribution benchmark queries = 4
distribution benchmark metric correct = 4/4
distribution benchmark vector mismatches = 4/4
distribution benchmark average metric margin = 0.416667
distribution benchmark graph edges = 2
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
mixed benchmark records = 8
mixed benchmark queries = 4
mixed benchmark metric correct = 4/4
mixed benchmark vector mismatches = 4/4
mixed benchmark average metric margin = 0.358734
mixed benchmark query evaluations = 32
```

## Cross-Space Dependency

Command:

```bash
build/core/examples/engine/engine_cross_space_mgc
```

Expected output:

```text
cross-space MGC = 1
cross-space paired records = 12
cross-space raw-vector pairing correct = 3/12
cross-space raw-vector mismatches = 9/12
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
pipeline = native_phate_autoencoder_pipeline
pipeline components = 11
pipeline codec = vector_record_codec
pipeline distance provider = matrix_cache_distance_provider
pipeline affinity kernel = exponential_affinity_kernel
pipeline diffusion operator = lazy_row_normalized_diffusion_operator
diffusion steps = 2
diffusion dense evaluations = 25
diffusion kernel scale = 1
diffusion graph recall = 1
training epochs = 160
mapping = native_phate_autoencoder
strategy = native_dnn_phate_ae
PHATE-AE bottleneck loss: 0.248425 -> 0.160057
PHATE-AE neighbor preservation = 0.8
PHATE-AE reconstruction MSE: 2.04967
vector baseline out-of-sample support = nearest_anchor_only
vector baseline inverse support = no_decoder
vector baseline first anchor = 0
out-of-sample support = parametric_space_transform
inverse support = decoder_inverse_transform
out-of-sample records = 2
out-of-sample anchor recall = 1
out-of-sample first-anchor match = 1
out-of-sample mapped-anchor rank = 1
deterministic seed = 23
```

## PHATE Pipeline Builder

Command:

```bash
build/core/examples/engine/engine_phate_pipeline_builder
```

Expected output:

```text
phate pipeline builder plan = native_phate_autoencoder_pipeline
phate pipeline builder components = 11
phate pipeline builder replacement points = 10
phate pipeline default executable = yes
phate pipeline matrix-cache executable = yes
phate pipeline metadata replacement executable = no
phate pipeline builder codec = vector_record_codec
phate pipeline builder distance provider = matrix_cache_distance_provider
phate pipeline builder affinity kernel = exponential_affinity_kernel
phate pipeline builder diffusion operator = lazy_row_normalized_diffusion_operator
```
