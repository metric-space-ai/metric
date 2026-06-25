# `metric.visual.v1` — Evidence Schema Reference

This is the canonical field reference for the only hard boundary between METRIC
C++ and METRIC Visual: the evidence document. C++ computes algorithms and
exports a `metric.visual.v1` document; JavaScript validates, indexes and renders
it. JavaScript never recomputes a METRIC algorithm.

The authoritative validator is [`visual/src/data/schema.js`](../../visual/src/data/schema.js).
Fixtures that exercise every shape live in
[`visual/examples/fixtures/`](../../visual/examples/fixtures/) and are generated
deterministically by
[`visual/tools/build-fixtures.mjs`](../../visual/tools/build-fixtures.mjs).

## Top-level shape

```json
{
  "schema": "metric.visual.v1",
  "provenance": {},
  "datasets": [],
  "records": [],
  "relations": [],
  "spaces": [],
  "properties": [],
  "graphs": [],
  "coordinates": [],
  "timelines": [],
  "events": [],
  "views": [],
  "diagnostics": []
}
```

`schema` must equal `metric.visual.v1`. Every listed key must be an array
(`validateVisualDocument` can downgrade missing optional arrays to warnings via
`{ allowMissingOptionalArrays: true }`, but `datasets` and `records` are always
required).

## Collections and required fields

The validator enforces these required fields and cross-references. IDs must be
unique within each collection; every reference must resolve to an existing ID.

### `datasets`
`id`, `title`, `description`, `source`, `license`. Groups records and evidence
into a named domain.

### `records`
`id`, `dataset_id`, `record_type`, `label`, `payload`.
`dataset_id` must reference a dataset. `payload` carries the original record for
preview; supported families: `string`, `vector`, `time_series`, `histogram`,
`image` (reference), `composed`, and opaque external references. The preview
resolver in [`record-preview.js`](../../visual/src/interaction/record-preview.js)
renders text, numbers, arrays and sparkline series.

### `relations`
`id`, `dataset_id`, `name`, `relation_type`, `value_type`, `record_ids`,
`storage`, `values`. Every entry of `record_ids` must reference a record.
- `relation_type`: `metric`, `baseline_metric`, `distance_like`, `similarity`,
  `transition`, `assignment`, `correlation`, `loss`, `law_check`, `custom`.
- `storage`: `dense_matrix`, `symmetric_dense_matrix`, `sparse_edge_list`,
  `block_matrix`, `external_binary_table`.
- `values`: pair entries. Accepted endpoint keys include
  `{ row_id, column_id, value }`, `{ source, target, value }`,
  `{ from, to, value }`, `{ i, j, value }` and `[source, target, value]`.
- For dense relation storage, `values` must match the exported `record_ids`
  shape: either an `N x N` array or a flat array of length `N * N`.
- Object-form relation values must reference existing records, and those
  endpoints must also be present in the relation's own `record_ids` list.
- For `relation_type: "metric"`, also export law diagnostics (e.g. under
  `metadata.law_check`). The visual library displays them; it does not prove
  them.

### `spaces`
`id`, `dataset_id`, `record_ids`, `primary_relation_id`, `space_type`,
`metadata`. `primary_relation_id` must reference a relation.
- `space_type`: `finite_metric_space`, `baseline_vector_space`,
  `derived_coordinate_space`, `graph_space`, `paired_space`, `custom_space`.

### `properties`
`id`, `target_type`, `value_type`, `values` (and usually `dataset_id`; its
absence is a warning).
- `target_type`: `record`, `pair`, `relation`, `space`, `graph_edge`,
  `timeline_step`.
- `value_type`: `scalar`, `categorical`, `vector`, `tensor`, `boolean`, `rank`,
  `distribution`.
- For `target_type: "record"`, each `values` entry is `{ record_id, value }`
  and `record_id` must reference a record.
- For `target_type: "pair"`, each value entry must reference existing row and
  column records. `relation_id` is optional because some pair properties are
  relation-independent, but when present it must resolve to an exported
  relation.
Examples: entropy, density, local volume, intrinsic dimension, outlier score,
cluster id, representative assignment, reconstruction error, mapping residual.

### `graphs`
`id`, `dataset_id`, `node_record_ids`, `edge_relation_id`, `graph_type`,
`edges`. `edge_relation_id` references a relation; every node id references a
record. Edge endpoints must reference existing records, must be listed in
`node_record_ids`, and must also be compatible with the edge relation's
`record_ids` when that relation is exported. `graph_type`: k-nearest, radius,
transition, diffusion, solver, sparsified, component, custom.

### `coordinates`
`id`, `dataset_id`, `space_id`, `name`, `dimension`, `record_positions`.
`space_id` references a space; `dimension` is a positive number; each
`record_positions` entry is `{ record_id, position: [...] }`, `record_id`
must reference a record, and `position` must contain numeric entries with at
least `dimension` values. Coordinate states are derived display states
(landmark 2D/3D, PHATE, AE latent, PCFA, SOM/KOC grid, graph layout), not the
metric space itself.

### `timelines`
`id`, `dataset_id`, `name`, `steps`. Each step may carry `coordinate_id`,
`property_id`, `relation_id` and/or `graph_id`; when present they must resolve.
Timelines describe state changes over existing records (2D→3D morph, diffusion
/ reverse diffusion steps, Redif inverse-dynamics before/after, training
epochs, solver iterations). They reference exported states; they do not ask
JavaScript to recompute them.

### `views`, `events`, `diagnostics`
Optional. `views` are producer hints for an initial workspace (e.g.
`{ kind: "metric-space", spaceId, coordinateId, propertyId }` or
`{ kind: "relation-matrix", relationId }`). The library may ignore them.

## Loading a document

```js
import { VisualSpace, validateVisualDocument } from "../../visual/src/data/index.js";

const result = validateVisualDocument(doc);      // { ok, errors, warnings }
const space = VisualSpace.fromDocument(doc);      // validates + indexes + normalizes coordinates
space.getRecord("rec-00");
space.relationValue("sensor-metric", "rec-00", "rec-01");
space.coordinateStatesForSpace("sensor-space");
```

Views consume the same document:

```js
import { MetricSpaceView, RelationMatrixView } from "../../visual/src/views/index.js";

const view = MetricSpaceView.fromVisualSpace(doc, { spaceId: "sensor-space", coordinateId: "landmark-3d" });
view.toLayerDescriptors(); // renderable layer descriptors for MetricVisualRuntime
```

## C++ export contract

C++ should emit a `metric.visual.v1` document directly — no JavaScript tooling
in the path. The minimum useful export for a finite metric space is one dataset,
the records (with preview payloads), one `metric` relation (sparse edge list is
the cheapest), one space, at least one 3D coordinate state, and the per-record
properties already computed (entropy/density/outlier/cluster). Sketch:

```cpp
// Pseudocode for an mtrc::visual writer. Values are already computed by METRIC.
writer.begin("metric.visual.v1");
writer.dataset({ .id="sensor", .title=..., .source=..., .license=... });
for (auto& r : records)
  writer.record({ .id=r.id, .dataset_id="sensor", .record_type="time_series_window",
                  .label=r.label, .payload=r.window /* original samples + features */ });
writer.relation({ .id="sensor-metric", .dataset_id="sensor", .relation_type="metric",
                  .value_type="scalar", .record_ids=ids, .storage="sparse_edge_list",
                  .values=knn_edges /* {row_id,column_id,value} */,
                  .metadata={ .law_check={ .triangle="checked", .symmetry="..." } } });
writer.space({ .id="sensor-space", .primary_relation_id="sensor-metric",
               .space_type="finite_metric_space", .record_ids=ids });
writer.coordinates({ .id="landmark-3d", .space_id="sensor-space", .dimension=3,
                     .record_positions=landmark3 });
writer.property({ .id="entropy", .target_type="record", .value_type="scalar",
                  .values=entropy_per_record });
writer.end();
```

Acceptance: the exported document loads directly through `VisualSpace` and the
views with no adapter, and `validateVisualDocument` returns `{ ok: true }`.

## Validation guarantees (negative cases)

The validator rejects, with actionable `path`/`code`/`message` errors:
- a missing record `id` (`required_field`),
- a relation/space/graph/coordinate reference to an unknown record
  (`record_ref`), relation (`relation_ref`), space (`space_ref`) or coordinate
  (`coordinate_ref`),
- a dense relation whose `values` shape does not match `record_ids`
  (`relation_dense_shape`),
- a relation value endpoint that is not listed in the relation's own
  `record_ids` (`relation_record_ref`),
- a pair property whose row/column endpoint cannot be resolved (`record_ref`)
  or whose optional relation id cannot be resolved (`relation_ref`),
- a graph edge endpoint outside `node_record_ids` (`graph_node_ref`) or outside
  the edge relation's record set (`graph_relation_record_ref`),
- a timeline step `graph_id` that cannot be resolved (`graph_ref`),
- duplicate IDs within a collection (`duplicate_id`),
- a non-positive coordinate `dimension` (`coordinate_dimension`),
- a coordinate position with too few numeric entries
  (`coordinate_position_dimension`, `coordinate_position_value`).

See [`visual/tools/check-schema-fixtures.mjs`](../../visual/tools/check-schema-fixtures.mjs)
for the valid/invalid fixture smoke test.
