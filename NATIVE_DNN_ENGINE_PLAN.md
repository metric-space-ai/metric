# Native DNN Engine Plan

## Goal

METRIC should be usable as an engine for finite metric spaces. The existing
`metric::dnn` code should become a native backend for learned mappings, with
autoencoders as the first supported model family.

The immediate target is a native PHATE-AE port:

- no hard PyTorch dependency
- reconstruction loss on decoder output
- bottleneck geometry loss on encoder output
- stable sample IDs through shuffled mini-batches
- pluggable codecs, losses, trainer diagnostics, and serialization
- a backend boundary that can later host an optional Torch backend

This is an integration plan, not a rewrite plan. Existing DNN classes remain
usable and are wrapped or extended where needed.

## Current Native DNN Inventory

The repository already contains a small feed-forward DNN stack:

- `metric/utils/dnn.hpp`
  - umbrella include for the native DNN module
- `metric/utils/dnn/Network.h`
  - owns a vector of `Layer<Scalar>` instances
  - owns one terminal `Output<Scalar>` loss
  - owns one `Optimizer<Scalar>`
  - supports JSON construction, cereal parameter save/load, `fit`, and
    `predict`
  - currently has private `forward`, `backprop`, and `update` hooks
- `metric/utils/dnn/Layer.h`
  - virtual layer interface: `forward`, `output`, `backprop`,
    `backprop_data`, `update`, parameter serialization
- `metric/utils/dnn/Layer/FullyConnected.h`
  - dense layer with activations
  - batches are represented as rows of observations
- `metric/utils/dnn/Output/RegressionMSE.h`
  - terminal MSE output
  - computes derivative with respect to the final network output
- `metric/utils/dnn/Optimizer/RMSProp.h`
  - currently the most complete optimizer implementation
- `metric/utils/dnn/Optimizer/SGD.h`
  - present, but it does not yet implement the full current
    `Optimizer<Scalar>` virtual interface
- `metric/mapping/autoencoder.hpp`
  - derives from `dnn::Network<Scalar>`
  - converts flat vectors to Blaze matrices
  - trains with `fit(trainData, trainData, ...)`
  - assumes an even number of layers and splits encoder/decoder at
    `num_layers() / 2`
  - provides `encode`, `decode`, and `predict`

Important constraint: `Network::fit` currently supports exactly one terminal
`Output` object. PHATE-AE needs multiple loss terms, including one anchored at
the bottleneck layer. The native port therefore needs a trainer/loss layer on
top of the existing network primitives.

## Design Principles

1. Keep native DNN first.
   PHATE-AE must train with Blaze and `metric::dnn` only.

2. Make sample identity explicit.
   Mini-batch row indexes are transient. Losses that use precomputed PHATE
   targets must address samples by stable IDs.

3. Separate data conversion from model training.
   Autoencoder data conversion should move behind codecs. The trainer should
   see typed batches, IDs, matrices, and metadata.

4. Do not couple PHATE-AE to a single loss formula.
   Bottleneck supervision should support coordinate MSE first and pairwise
   geometry losses next.

5. Extend `Network` minimally.
   Add training hooks and metadata rather than replacing the layer and
   optimizer implementations.

6. Keep backend choice behind interfaces.
   A future Torch backend must be optional and must not leak into public native
   PHATE-AE headers.

## Target Architecture

```text
MetricSpace<Record, Metric>
        |
        v
RecordCodec<Record, Scalar>
        |
        v
EncodedDataset<Scalar>
  - sample IDs
  - feature matrix
  - optional target views
        |
        v
AutoencoderModel<NativeDnnBackend>
  - metric::dnn::Network<Scalar>
  - encoder/bottleneck metadata
  - decoder metadata
        |
        v
NativeDnnTrainer
  - batch sampler
  - forward cache
  - composite loss
  - optimizer update
  - diagnostics
        |
        v
FittedMapping
  - encode records to latent metric space
  - decode latent vectors when supported
  - serialize model, codec, losses, and metadata
```

## Core Types To Add

### Stable IDs

Use a stable ID type shared with the finite metric space engine plan.

```cpp
struct SampleId {
    std::uint64_t value;
};
```

Training batches should never assume that matrix row `i` is the global sample
`i`. A batch carries both IDs and row data.

```cpp
template <class Scalar>
struct DnnBatch {
    std::vector<SampleId> ids;
    blaze::DynamicMatrix<Scalar> x;
};
```

For supervised bottleneck targets:

```cpp
template <class Scalar>
struct BottleneckTargetTable {
    std::unordered_map<SampleId, std::vector<Scalar>> coordinates;
};
```

For pairwise geometry targets:

```cpp
template <class Scalar>
struct PairwiseGeometryTarget {
    SampleId a;
    SampleId b;
    Scalar target_distance;
    Scalar weight;
};
```

### Codecs

Autoencoder currently converts `std::vector<InputDataType>` to a dense Blaze
matrix and uses one `normValue`. Replace that ad hoc conversion with codecs.

```cpp
template <class Record, class Scalar>
class RecordCodec {
public:
    using matrix_type = blaze::DynamicMatrix<Scalar>;

    virtual ~RecordCodec() = default;
    virtual std::size_t feature_count() const = 0;
    virtual matrix_type encode_batch(span<const Record> records) const = 0;
    virtual std::vector<Record> decode_batch(const matrix_type& matrix) const = 0;
    virtual nlohmann::json to_json() const = 0;
};
```

Initial codecs:

- `FlatVectorCodec`
  - replaces current `Autoencoder::convertData`
  - supports normalization and denormalization
  - stores feature count and norm value
- `BlazeMatrixCodec`
  - wraps already-materialized dense numeric data
- `MetricSpaceFeatureCodec`
  - consumes a finite metric space representation when records need to be
    materialized as feature vectors

The codec contract should also store:

- scalar type
- feature shape
- normalization policy
- missing-value policy, if needed later
- whether inverse decoding is supported

### Encoded Dataset

```cpp
template <class Scalar>
class EncodedDataset {
public:
    using matrix_type = blaze::DynamicMatrix<Scalar>;

    std::size_t size() const;
    const std::vector<SampleId>& ids() const;
    const matrix_type& features() const;
    std::uint64_t source_space_version() const;
};
```

The dataset is the bridge between finite metric spaces and native DNN training.
It must preserve the order of rows and the ID assigned to every row.

### Autoencoder Model Metadata

The current autoencoder infers the bottleneck as `num_layers() / 2`. Keep that
as a default, but store it explicitly.

```cpp
struct AutoencoderTopology {
    std::size_t input_layer = 0;
    std::size_t bottleneck_layer;
    std::size_t output_layer;
    std::vector<std::size_t> encoder_layers;
    std::vector<std::size_t> decoder_layers;
};
```

The native model wrapper should own or reference:

- `metric::dnn::Network<Scalar>`
- topology metadata
- codec metadata
- backend name: `native_dnn`
- training/loss metadata after fitting

## Required Changes To `metric::dnn::Network`

The composite trainer needs controlled access to forward activations, anchored
gradients, and optimizer updates. The smallest useful extension is to expose
training hooks without changing existing `fit` behavior.

Add a protected or public low-level API:

```cpp
template <class Scalar>
class Network {
public:
    using Matrix = blaze::DynamicMatrix<Scalar>;

    void forward_all(const Matrix& input, std::vector<Matrix>* activations);
    void backprop_from_layer_gradients(
        const Matrix& input,
        const std::vector<Matrix>& output_gradients_by_layer);
    void apply_optimizer();
};
```

The trainer can then compute all loss gradients externally and ask the network
to perform one backward pass. Existing `fit(x, y, ...)` can remain implemented
through the current terminal `Output` path.

Alternative if the public API should stay smaller:

- make a `NetworkTrainerAccess` friend class
- keep raw hooks private to `metric::dnn`
- expose only the higher-level trainer API to users

Also standardize comments and tests around batch orientation. The
implementation uses rows as observations.

## Composite Objective Model

Current `Output<Scalar>` represents one terminal loss. Add a new trainer-level
loss interface that can be anchored at any layer output.

```cpp
enum class LossAnchorKind {
    final_output,
    layer_output
};

struct LossAnchor {
    LossAnchorKind kind;
    std::size_t layer_index;
};

template <class Scalar>
struct LossEvaluation {
    Scalar value = 0;
    blaze::DynamicMatrix<Scalar> gradient;
};

template <class Scalar>
class LossTerm {
public:
    virtual ~LossTerm() = default;
    virtual std::string name() const = 0;
    virtual LossAnchor anchor() const = 0;
    virtual LossEvaluation<Scalar> evaluate(
        const DnnBatch<Scalar>& batch,
        const std::vector<blaze::DynamicMatrix<Scalar>>& activations) = 0;
    virtual nlohmann::json to_json() const = 0;
};
```

The composite objective sums weighted loss terms.

```text
L_total =
    w_reconstruction * L_reconstruction(x_hat, x)
  + w_bottleneck    * L_bottleneck(z, target_geometry)
  + w_regularizer   * L_regularizer(parameters)
```

The trainer must accumulate gradients by anchor layer before one backward pass.
For a feed-forward chain:

1. Run forward pass and store layer outputs.
2. Evaluate every loss term.
3. Add each term gradient to `gradients_by_layer[anchor.layer_index]`.
4. Backpropagate from the final layer to the first layer.
5. At each layer, add any local anchored gradient to the upstream gradient.
6. Update parameters once.

This avoids running separate backward passes for every loss term and keeps all
terms consistent with the same forward activations.

## Initial Loss Terms

### ReconstructionMSELoss

Native replacement for the current autoencoder objective.

- anchor: final layer
- target: input matrix `x`
- value: mean squared reconstruction error
- gradient: derivative with respect to final output
- can internally reuse the math from `RegressionMSE`

### BottleneckCoordinateMSELoss

First PHATE-AE bottleneck objective.

- anchor: bottleneck layer
- target: coordinates keyed by `SampleId`
- value: MSE between bottleneck activation `z_i` and target coordinate `y_i`
- gradient: derivative with respect to `z_i`

This supports the common PHATE-AE pattern where PHATE coordinates supervise the
autoencoder latent space.

### BottleneckPairwiseDistanceLoss

Second-stage geometry objective for native metric-space training.

- anchor: bottleneck layer
- target: weighted sample pairs keyed by `SampleId`
- value: weighted stress between latent distances and target distances
- gradient: derivative with respect to both latent rows in each pair

This allows supervision from a finite metric space relation rather than only
from coordinate targets.

### BottleneckAffinityLoss

Optional later PHATE-specific objective.

- anchor: bottleneck layer
- target: affinity or transition probabilities
- value: KL or cross-entropy style divergence
- useful only after the native probability/affinity representation is stable

Do not block the initial PHATE-AE port on this term.

## Bottleneck Supervision For PHATE-AE

PHATE-AE training should be assembled in two steps.

1. Build geometry targets from the finite metric space.
   - input: `MetricSpace<Record, Metric>`
   - native PHATE or PHATE-like operator computes either:
     - coordinates per `SampleId`
     - pairwise distances per `SampleId` pair
     - affinities per `SampleId` pair
   - targets are stored outside the batch matrix and addressed by stable ID

2. Train the native autoencoder with a composite objective.
   - reconstruction term sees `x` and decoded `x_hat`
   - bottleneck term sees `z` and geometry targets for the same batch IDs
   - sampler guarantees that every mini-batch can resolve target rows by ID

Initial default:

```text
loss =
    1.0 * ReconstructionMSELoss(final_output)
  + lambda_geometry * BottleneckCoordinateMSELoss(bottleneck_layer)
```

Later:

```text
loss =
    1.0 * ReconstructionMSELoss(final_output)
  + lambda_stress * BottleneckPairwiseDistanceLoss(bottleneck_layer)
```

Use coordinate MSE first because it has simpler batching and gradient behavior.
Add pairwise stress once the ID-aware sampler can request pair-complete or
neighbor-rich batches.

## Trainer API

Add a native trainer that is separate from the legacy `Network::fit`.

```cpp
template <class Scalar>
struct TrainingSpec {
    std::size_t epochs = 100;
    std::size_t batch_size = 32;
    std::uint64_t seed = 123;
    bool shuffle = true;
    Scalar gradient_clip_norm = 0;
    Scalar early_stop_min_delta = 0;
    std::size_t early_stop_patience = 0;
};

template <class Scalar>
class NativeDnnTrainer {
public:
    TrainingReport fit(
        AutoencoderModel<Scalar>& model,
        const EncodedDataset<Scalar>& dataset,
        const CompositeLoss<Scalar>& objective,
        const TrainingSpec<Scalar>& spec);
};
```

Responsibilities:

- create shuffled mini-batches while preserving `SampleId`
- run one forward pass per batch
- evaluate all loss terms
- accumulate anchored gradients
- run one backward pass per batch
- call optimizer update once per batch
- collect diagnostics
- expose callbacks compatible with current `Callback<Scalar>` where practical

The trainer should not own the input records. It trains on an encoded dataset
that can be rebuilt from a metric space and codec.

## Mapping API

Expose the fitted autoencoder as a mapping between finite metric spaces.

```cpp
template <class Record, class Metric, class Scalar>
class NativeAutoencoderMapping {
public:
    void fit(const MetricSpace<Record, Metric>& space);
    MetricSpace<std::vector<Scalar>, EuclideanMetric> transform(
        const MetricSpace<Record, Metric>& space) const;
    std::vector<Record> inverse_transform(
        span<const std::vector<Scalar>> latent) const;
};
```

For PHATE-AE:

- `fit` builds codec features
- geometry targets come from the source finite metric space
- `transform` returns bottleneck vectors as records
- the latent metric defaults to Euclidean distance
- `inverse_transform` is available only when the codec supports decoding

## Diagnostics

The trainer should produce a structured report instead of only printing.

```cpp
struct TrainingReport {
    std::vector<EpochReport> epochs;
    bool stopped_early = false;
    std::string stop_reason;
};
```

Per epoch:

- total loss
- reconstruction loss
- bottleneck loss
- optional regularization loss
- reconstruction RMSE
- bottleneck coordinate RMSE or pairwise stress
- pairwise distance correlation on a held-out ID sample
- neighbor preservation or trustworthiness for latent space
- gradient norm per layer
- parameter norm per layer
- NaN/Inf counters
- batch timing
- sample coverage

Diagnostics matter because composite objectives can look healthy in total while
one term collapses. The report must preserve per-term values.

## Serialization

Keep compatibility with `Network::save` for raw native DNN models, but add a
higher-level artifact for engine-backed mappings.

Artifact contents:

- format version
- backend name: `native_dnn`
- native network JSON
- native network parameters
- optimizer configuration
- autoencoder topology
- codec type and codec metadata
- loss specifications and weights
- training spec
- training diagnostics summary
- scalar type
- source feature count
- optional source metric-space version or checksum

Do not serialize full training data by default. Store sample IDs and target
metadata only when explicitly requested for reproducibility.

Suggested layout:

```text
NativeAutoencoderArtifact
  manifest.json
  network.cereal
  diagnostics.json
```

The existing single-file cereal path can remain supported for low-level
`metric::dnn::Network`.

## JSON And Registry Support

Extend JSON construction with registries instead of hard-coded `if` chains over
every future type.

Initial registries:

- layer registry
- activation registry
- optimizer registry
- loss registry
- codec registry

This can start small. The immediate requirement is loss and codec
serialization for PHATE-AE artifacts.

## Optimizer Plan

Short term:

- use `RMSProp<Scalar>` as the default native optimizer
- add tests that verify it updates dense layer weights through the trainer
- ensure optimizer state is reset at the beginning of fitting

Before exposing optimizer selection broadly:

- make `SGD<Scalar>` implement the complete `Optimizer<Scalar>` interface
- either update or remove stale AdaGrad code from public includes
- serialize all optimizer state needed for resume training, or document that
  artifacts serialize configuration but not in-progress optimizer moments

Future optional optimizers:

- Adam
- AdamW
- learning-rate schedules
- weight decay decoupled from gradients

## Backend Boundary

Define a backend interface around model construction, training, inference, and
serialization.

```cpp
enum class DnnBackendKind {
    native_dnn,
    torch_optional
};
```

Native backend is mandatory:

- implemented with `metric::dnn`
- compiled by default
- used by PHATE-AE tests

Torch backend is optional future work:

- compiled only behind a CMake option
- no public hard dependency in core headers
- no PHATE-AE feature should require it
- same high-level codec, loss, sample ID, and mapping contracts

The purpose of a future Torch backend is acceleration and interoperability, not
correctness or feature ownership.

## Implementation Phases

### Phase 0: Baseline And Tests

- Add focused tests for current `Network`, `RegressionMSE`, `RMSProp`, and
  `Autoencoder` behavior.
- Record the row-as-observation batch convention in tests.
- Verify save/load round trip for a small dense autoencoder.
- Confirm which optimizers satisfy the current virtual interface.

### Phase 1: Native DNN Training Hooks

- Add forward activation capture to `Network`.
- Add anchored-gradient backprop support.
- Add public or friend-mediated optimizer update access.
- Keep existing `fit` behavior unchanged.
- Add gradient tests for a one-layer and multi-layer dense network.

### Phase 2: IDs, Dataset, And Codecs

- Add `SampleId` and `DnnBatch`.
- Add `EncodedDataset`.
- Add `FlatVectorCodec` as a direct replacement for current autoencoder data
  conversion.
- Update autoencoder wrappers to use codecs internally.
- Add tests showing that shuffled batches preserve sample IDs.

### Phase 3: Composite Loss

- Add `LossTerm` and `CompositeLoss`.
- Add `ReconstructionMSELoss`.
- Add `BottleneckCoordinateMSELoss`.
- Add per-term diagnostics.
- Train a dense autoencoder with reconstruction-only loss and match legacy
  behavior within tolerance.
- Train the same model with an added bottleneck coordinate target.

### Phase 4: Engine Autoencoder Mapping

- Add `AutoencoderModel` metadata wrapper.
- Add `NativeDnnTrainer`.
- Add `NativeAutoencoderMapping`.
- Return bottleneck vectors as a finite metric space with Euclidean metric.
- Support inverse transform when the codec is invertible.

### Phase 5: PHATE-AE Native Port

- Build PHATE geometry targets keyed by source `SampleId`.
- Start with `BottleneckCoordinateMSELoss`.
- Add training examples and tests on small deterministic finite metric spaces.
- Validate:
  - reconstruction improves
  - bottleneck target error decreases
  - latent neighbor preservation is reasonable
  - no PyTorch dependency is needed

### Phase 6: Serialization And Python Surface

- Add high-level autoencoder artifact serialization.
- Add loss and codec JSON specs.
- Bind the stable API to Python only after the native C++ API is stable.
- Python should expose engine concepts, not raw template mechanics.

### Phase 7: Optional Torch Backend

- Add backend selection only after the native PHATE-AE path is complete.
- Keep Torch behind an optional CMake target.
- Reuse the same `TrainingSpec`, codec metadata, loss specs, and mapping API.

## Non-Goals

- Do not introduce a general autograd engine in the first native port.
- Do not require PyTorch for PHATE-AE.
- Do not replace existing `metric::dnn::Network` construction APIs.
- Do not make autoencoder codecs responsible for metric-space algorithms.
- Do not serialize full training data by default.

## Open Decisions

1. Whether `Network` low-level hooks should be public or restricted to a
   friend trainer class.
2. Whether bottleneck layers should be addressed by index only or by optional
   stable layer names.
3. Whether pairwise bottleneck losses should use within-batch pairs only or a
   sampler that constructs neighbor-complete batches.
4. Whether optimizer moment state must be serialized for exact resume support.
5. Which finite metric-space representation is the first source for PHATE
   geometry targets.

## Acceptance Criteria

The native DNN engine work is ready for PHATE-AE when:

- a dense autoencoder can be trained through `NativeDnnTrainer`
- the same trainer supports reconstruction-only and composite objectives
- bottleneck supervision is keyed by stable sample IDs
- shuffling mini-batches does not break target lookup
- a fitted model can encode records into a latent finite metric space
- diagnostics report total loss and per-term loss
- model artifacts include network, topology, codec, loss, and backend metadata
- the full native PHATE-AE example builds and runs without PyTorch

