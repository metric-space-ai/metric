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
process workflow metric winner = late_ramp at 2
process workflow vector comparison winner = flat_hold at 3
process workflow baseline mismatch = yes
process workflow metric margin = 3
process workflow groups = 2
process workflow outliers = 2
process workflow dense evaluations = 25
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

## Process Curve parametric diffusion coordinates Map

Command:

```bash
build/core/examples/engine/engine_process_curve_diffusion_coordinate_map
```

Expected output:

```text
process diffusion coordinate records = 10
process diffusion coordinate queries = 4
process diffusion coordinate source metric = aligned_curve_distance
process diffusion coordinate record-coordinate codec = process_curve_coordinate_codec
process diffusion-coordinate pipeline components = 11
process diffusion-coordinate pipeline codec = record_coordinate_codec
process diffusion coordinate distance provider = distance_table_pairwise_distances
process diffusion coordinate affinity kernel = exponential_affinity_kernel
process diffusion-coordinate operator = lazy_row_normalized_diffusion_operator
process diffusion-coordinate target target evaluations = 100
process diffusion coordinate metric nearest family = late_ramp
process diffusion coordinate vector baseline family = flat
process diffusion coordinate latent nearest family = late_ramp
process diffusion coordinate metric correct = 4/4
process diffusion coordinate vector misses = 4/4
process diffusion coordinate latent correct = 4/4
process diffusion coordinate query = query_delayed_ramp:metric=late_ramp,vector=flat,latent=late_ramp
process diffusion coordinate query = query_delayed_ramp_long:metric=late_ramp,vector=flat,latent=late_ramp
process diffusion coordinate query = query_delayed_ramp_short:metric=late_ramp,vector=normal,latent=late_ramp
process diffusion coordinate query = query_delayed_ramp_deep_delay:metric=late_ramp,vector=flat,latent=late_ramp
process diffusion coordinate target error = 0.6213 -> 0.160793
process diffusion coordinate out-of-sample support = parametric_space_transform
process diffusion coordinate out-of-sample anchor recall = 0.5
process parametric diffusion coordinate mapped-anchor source rank = 1.75
process diffusion coordinate max mapped-anchor source rank = 3
process diffusion coordinate original-metric distance penalty = 0.25
process diffusion coordinate max original-metric distance penalty = 1
process diffusion coordinate inverse support = none_record_coordinate_codec
process diffusion coordinate deterministic seed = 29
```

## Process Curve parametric diffusion coordinates Gallery

Command:

```bash
build/core/examples/engine/engine_process_curve_diffusion_coordinate_gallery
```

Expected output:

```text
process diffusion-coordinate gallery records = 15
process diffusion-coordinate gallery queries = 6
process diffusion-coordinate gallery source metric = aligned_curve_distance
process diffusion-coordinate gallery record-coordinate codec = process_curve_gallery_coordinate_codec
process diffusion-coordinate gallery pipeline components = 11
process diffusion-coordinate gallery distance provider = distance_table_pairwise_distances
process diffusion-coordinate gallery affinity kernel = exponential_affinity_kernel
process diffusion-coordinate gallery diffusion operator = lazy_row_normalized_diffusion_operator
process diffusion-coordinate gallery target evaluations = 225
process diffusion-coordinate gallery metric nearest family = late_ramp
process diffusion-coordinate gallery vector baseline family = flat
process diffusion-coordinate gallery latent nearest family = late_ramp
process diffusion-coordinate gallery metric correct = 6/6
process diffusion-coordinate gallery vector misses = 6/6
process diffusion-coordinate gallery latent correct = 6/6
process diffusion-coordinate gallery average metric margin = 2.16667
process diffusion-coordinate gallery query = gallery_delay_A:metric=late_ramp,vector=flat,latent=late_ramp
process diffusion-coordinate gallery query = gallery_delay_B:metric=late_ramp,vector=flat,latent=late_ramp
process diffusion-coordinate gallery query = gallery_delay_C:metric=late_ramp,vector=normal,latent=late_ramp
process diffusion-coordinate gallery query = gallery_delay_D:metric=late_ramp,vector=normal,latent=late_ramp
process diffusion-coordinate gallery query = gallery_delay_E:metric=late_ramp,vector=flat,latent=late_ramp
process diffusion-coordinate gallery query = gallery_delay_F:metric=late_ramp,vector=flat,latent=late_ramp
process diffusion-coordinate gallery neighbor preservation = 0.4
process diffusion-coordinate gallery OOS anchor recall = 0.833333
process diffusion-coordinate gallery first-anchor match rate = 0.833333
process diffusion-coordinate gallery mapped-anchor source rank = 1.33333
process diffusion-coordinate gallery max mapped-anchor source rank = 3
process diffusion-coordinate gallery original-metric distance penalty = 0.166667
process diffusion-coordinate gallery max original-metric distance penalty = 1
process diffusion-coordinate gallery target error = 0.46694 -> 0.0860003
process diffusion-coordinate gallery inverse support = none_record_coordinate_codec
process diffusion-coordinate gallery deterministic seed = 41
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
available representations = metric_space,distance_table,cover_tree_index,knn_graph_index
nearest vector = alpha-0 at 0.141421
indexed nearest = alpha-0 via cover_tree_index
runtime policy = exact_lazy_serial via metric_space
distance table reuse = distance_table,distance_table
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
histogram runtime representation = distance_table
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
distribution workflow metric winner = vertical_shift_left at 1
distribution workflow vector comparison winner = horizontal_at_vertical_query at 0.666667
distribution workflow baseline mismatch = yes
distribution workflow metric margin = 0.333333
distribution workflow groups = 2
distribution workflow dense evaluations = 64
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
mixed record groups = 2 via distance_table
mixed record outlier = spike-stop
runtime policy = exact_materialized_serial via distance_table
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

## Cross-Space Dependency (Hero)

Permutation-test dependence between event-log text (edit distance) and process
curves (TWED), with naive-vectorisation baselines. See
[Cross-Space Dependency (Hero)](cross-space-dependency.md) for the full interpretation.

Command:

```bash
build/core/examples/engine/engine_cross_space_dependency
```

Expected output:

```text
cross-space records = 48
cross-space left space = event_logs/edit_distance
cross-space right space = process_curves/twed
coupled compare() statistic = 0.947979
coupled MGC statistic = 0.947979
coupled MGC null mean = 0.002297
coupled MGC null sd = 0.030674
coupled MGC standardized effect = 30.830194
coupled MGC permutation p-value = 0.005000
coupled MGC permutations = 199
coupled MGC decision (alpha=0.050000) = dependent
coupled baseline scalar pearson r = 0.079196
coupled baseline scalar pearson p-value = 0.618500
coupled baseline scalar decision (alpha=0.050000) = independent
coupled baseline forced-vector MGC = 0.510452
decoupled MGC statistic = -0.007853
decoupled MGC null mean = 0.002456
decoupled MGC null sd = 0.033055
decoupled MGC standardized effect = -0.311873
decoupled MGC permutation p-value = 0.535000
decoupled MGC permutations = 199
decoupled MGC decision (alpha=0.050000) = independent
decoupled baseline scalar pearson r = 0.056721
decoupled baseline scalar pearson p-value = 0.732000
decoupled baseline scalar decision (alpha=0.050000) = independent
decoupled baseline forced-vector MGC = -0.003169
permuted MGC statistic = -0.018790
permuted MGC null mean = -0.003803
permuted MGC null sd = 0.023432
permuted MGC standardized effect = -0.639573
permuted MGC permutation p-value = 0.720000
permuted MGC permutations = 199
permuted MGC decision (alpha=0.050000) = independent
cross-space verdict = metric_detects_dependence_baseline_misses_it
```

## Learnable Structure Map

Command:

```bash
build/core/examples/engine/engine_parametric_diffusion_coordinate_map
```

Expected output:

```text
source record_kind = aligned_vector
source metric_law = metric
source fixture size = 5
source codec = vector_record_codec
source nearest neighbors = 2 via distance_table
source groups = 2 via distance_table
source dbscan density-unassigned = 0 via distance_table
runtime policy = exact_materialized_serial via knn_graph_index
pipeline = parametric_diffusion_coordinate_pipeline
pipeline components = 11
pipeline codec = vector_record_codec
pipeline distance provider = distance_table_pairwise_distances
pipeline affinity kernel = exponential_affinity_kernel
pipeline diffusion operator = lazy_row_normalized_diffusion_operator
diffusion steps = 2
diffusion dense evaluations = 25
diffusion kernel scale = 1
diffusion graph recall = 1
calibration steps = 160
mapping = parametric_diffusion_coordinates
strategy = native_metric_diffusion_coordinate_solver
parametric diffusion coordinate target error: 0.248425 -> 0.160057
parametric diffusion coordinate neighbor preservation = 0.8
parametric diffusion coordinate reconstruction MSE: 2.04967
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

## parametric diffusion coordinates Pipeline Builder

Command:

```bash
build/core/examples/engine/engine_parametric_diffusion_coordinate_pipeline_builder
```

Expected output:

```text
diffusion coordinate pipeline builder plan = parametric_diffusion_coordinate_pipeline
diffusion coordinate pipeline builder components = 11
diffusion coordinate pipeline builder replacement points = 10
diffusion coordinate pipeline default executable = yes
diffusion coordinate pipeline distance-table executable = yes
diffusion coordinate pipeline metadata replacement executable = no
diffusion coordinate pipeline builder codec = vector_record_codec
diffusion coordinate pipeline builder distance provider = distance_table_pairwise_distances
diffusion coordinate pipeline builder affinity kernel = exponential_affinity_kernel
diffusion coordinate pipeline builder diffusion operator = lazy_row_normalized_diffusion_operator
```
