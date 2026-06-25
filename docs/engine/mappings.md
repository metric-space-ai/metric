# Mappings

Mappings belong under `mtrc::modify::map` and `mtrc::modify::compose`. A map
creates a derived finite metric space or a reusable mapping artifact from a
source finite metric space.

The source space remains:

```text
records + metric = source finite metric space
```

A mapping may then produce:

- a coordinate result space for visualization or downstream tools
- a reduced or transformed record space
- a derived mapping artifact for out-of-sample records
- reconstruction or inverse-transform support when the implementation can
  provide it
- lineage and preservation diagnostics

diffusion-coordinate geometry, parametric coordinate maps, PCFA, and native parametric solvers
solver components are Level-3 implementation choices inside this mapping layer.
They are not the conceptual center of METRIC.

The parametric diffusion coordinate demonstrator belongs under `mtrc::modify::compose`: source space,
metric-value access, target construction, native parametric solver, mapping
result, artifact, and diagnostics.

From the METRIC viewpoint:

- parametric diffusion coordinates supplies geometry targets for a derived coordinate space.
- A native parametric solver supplies a derived coordinate map and optional reconstruction
  path for coordinate records.
- A native coordinate solver calibrates the coordinate map.
- The source finite metric space remains the object whose geometry is being
  represented.
