# Mappings

Mappings belong under `mtrc::modify::map` and `mtrc::modify::compose`. A map
creates a derived finite metric space or a reusable fitted transform from a
source finite metric space.

The source space remains:

```text
records + metric = source finite metric space
```

A mapping may then produce:

- a coordinate result space for visualization or downstream tools
- a reduced or transformed record space
- a fitted model for out-of-sample records
- reconstruction or inverse-transform support when the implementation can
  provide it
- lineage and preservation diagnostics

PHATE-style geometry, autoencoders, PCFA, SOM/KOC-style maps, and native DNN
solvers are Level-3 implementation choices inside this mapping layer. They are
not the conceptual center of METRIC.

The PHATE-AE demonstrator belongs under `mtrc::modify::compose`: source space,
metric-value access, target construction, native parametric solver, mapping
result, artifact, and diagnostics.

From the METRIC viewpoint:

- PHATE supplies geometry targets for a derived coordinate space.
- An autoencoder supplies a fitted parametric map and optional reconstruction
  path for coordinate records.
- A native DNN is the solver component that trains the fitted map.
- The source finite metric space remains the object whose geometry is being
  represented.
