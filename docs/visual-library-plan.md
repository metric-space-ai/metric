# METRIC Visual Library Plan

This file is the short entry point for the METRIC visualization library. The
complete plan lives in:

```text
docs/visual/visual-engine-masterplan.md
```

The library is a standalone browser-side visualization engine for finite metric
spaces, records, relations, graph structures, coordinate states, properties,
timelines, mappings, and solver traces.

It is not an auxiliary developer tool, not a wrapper around C++ examples, and
not an adapter to another plotting library. The contract to METRIC algorithms is
an import and export format:

```text
metric.visual.v1
```

C++ computes all algorithmic results. The visual library imports those results
and renders them with plain JavaScript, Canvas 2D, raw WebGL, and METRIC-owned
GLSL shaders. There is no npm dependency, no TypeScript requirement, no bundler,
and no external renderer.

Babyplots is only a technical reference for high-volume point clouds, morph
animation, turntable camera behavior, and interaction quality. Any useful ideas
or code must be assimilated into native METRIC visual code with license
obligations preserved and without Babyplots public APIs, namespaces, data
contracts, package structure, or dependencies.
