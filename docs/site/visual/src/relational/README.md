# Relation-Space Visuals

Native dependency-free helpers for visualizing exported finite metric-space relation evidence.

The module treats pair values as the source of truth. It normalizes record ids and encodes already-exported relation values for matrix and graph visuals; it does not recompute metric algorithms, embeddings, shortest paths, clustering, or nearest-neighbor indexes from raw records.

## Small Self-Test

Paste this into a browser console or any ES module environment that can import `visual/src/relational/index.js`:

```js
import {
  buildDenseRelationMatrix,
  buildRelationMatrixTextureData,
  buildSparseNeighborhoodGraph,
  checkSymmetricRelations,
  createRelationMatrixLayerDescriptor,
} from "./visual/src/relational/index.js";

const evidence = {
  recordIds: ["a", "b", "c"],
  pairs: [
    { source: "a", target: "b", value: 0.2 },
    { source: "b", target: "a", value: 0.2 },
    { source: "a", target: "c", value: 0.7 },
    { source: "c", target: "a", value: 0.7 },
    { source: "b", target: "c", value: 0.4 },
    { source: "c", target: "b", value: 0.4 },
  ],
};

const matrix = buildDenseRelationMatrix(evidence);
console.assert(matrix.size === 3);
console.assert(Math.abs(matrix.values[0 * 3 + 1] - 0.2) < 1e-6);
console.assert(checkSymmetricRelations(evidence).isSymmetric);

const texture = buildRelationMatrixTextureData(evidence, { scale: "quantile", palette: "metric" });
console.assert(texture.data.length === 3 * 3 * 4);

const graph = buildSparseNeighborhoodGraph(evidence, { mode: "topK", topK: 1 });
console.assert(graph.edges.length === 2);

const descriptor = createRelationMatrixLayerDescriptor(texture, { rect: [0.04, 0.08, 0.42, 0.42] });
console.assert(descriptor.primitive === "RelationMatrixLayer");
```

## Input Shape

Preferred evidence shape:

```js
const evidence = {
  recordIds: ["record-1", "record-2"],
  pairs: [
    { source: "record-1", target: "record-2", value: 0.31 },
    { source: "record-2", target: "record-1", value: 0.31 },
  ],
};
```

Accepted endpoint aliases are `source/target`, `sourceId/targetId`, `from/to`, `a/b`, and `i/j` when `recordIds` are supplied. Accepted value keys default to `value`, then `distance`, `relation`, `weight`, and `score`.
