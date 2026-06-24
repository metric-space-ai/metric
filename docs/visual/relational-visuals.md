# Relational Visuals

`visual/src/relational/` provides native relation-space visualization helpers for finite metric-space evidence. The subsystem is dependency-free JavaScript and WebGL-oriented.

The builders consume exported pair relation values. They do not recompute metrics, embeddings, clustering, graph topology, or nearest-neighbor indexes from raw records.

Core capabilities:

- Dense matrix construction with `buildDenseRelationMatrix()`.
- RGBA8 WebGL texture payloads with `buildRelationMatrixTextureData()`.
- Sparse edge descriptors with `buildSparseNeighborhoodGraph()`.
- Min/max and quantile scaling helpers.
- Symmetric/asymmetric diagnostics for visualization decisions.
- Layer descriptor factories for matrix, graph edges, and selected record focus.
- Isolated `RelationMatrixLayer` with embedded shader strings.

Integration is documented in `visual/src/relational/INTEGRATION.md`.

