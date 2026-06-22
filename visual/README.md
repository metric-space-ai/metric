# METRIC Visual

Native browser visualization for finite metric spaces, records, relations,
graphs, coordinate states, properties, timelines, mappings, and solver traces.

This library has no build step and no JavaScript dependencies. It uses plain ES
modules, Canvas 2D, raw WebGL, and METRIC-owned GLSL shaders. Native C++
examples export `metric.evidence.v1` today and should converge on
`metric.visual.v1` as the stable import/export format.

Babyplots is used only as a technical reference for high-volume point clouds,
morph animation, turntable camera behavior, and interaction quality. No
Babyplots dependency, adapter, namespace, package structure, or data convention
is exposed here.

Open the process-curve demo through any static file server rooted at the
repository:

```text
visual/examples/process-curve-condition-monitoring/index.html
```

The demo expects:

```text
docs/examples/assets/process-curve-external/evidence.json
```
