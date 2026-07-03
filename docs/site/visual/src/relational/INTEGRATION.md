# Relation Visual Integration Hooks

This directory is intentionally isolated. No shared registries or package indexes were edited.

## Exports Needed From `visual/src/index.js`

Add a namespace export when the visual package is ready to expose the subsystem:

```js
export * as relational from "./relational/index.js";
```

or named exports:

```js
export * from "./relational/index.js";
```

## Layer Registry Hook Needed

To instantiate matrix descriptors through the existing layer factory, import and register the layer:

```js
import { RelationMatrixLayer } from "../relational/RelationMatrixLayer.js";
```

Add to the default registry map:

```js
["RelationMatrixLayer", RelationMatrixLayer],
```

The graph edge descriptor factory already targets the existing `RelationEdgeLayer` primitive.

## Descriptor Factories

The integration-facing factories are:

```js
import {
  createRelationMatrixLayerDescriptor,
  createRelationGraphEdgeLayerDescriptor,
  createSelectedRecordFocusDescriptor,
} from "./relational/index.js";
```

`createRelationMatrixLayerDescriptor()` returns a descriptor for the new `RelationMatrixLayer`.

`createRelationGraphEdgeLayerDescriptor()` returns a descriptor for the existing `RelationEdgeLayer`.

`createSelectedRecordFocusDescriptor()` returns focus metadata for row/column and graph-neighborhood highlighting. It is a descriptor payload, not yet tied to a renderer-specific layer.

