# GRAE10 Metric Engine Example

This example renders `grae10-data.json` through the native `metric-webgl`
runtime. The canonical page is `index.html`.

Contracts:

- Dataset: `metric.visual.grae10.dataset.v1`, `60000` records.
- Source geometry: MNIST train images, normalized as `train_x.reshape(60000, 28 * 28) / 255`.
- PHATE-AE reference: 3D GRAE with `epochs=30`, `n_components=3`, `lr=.0001`, `batch_size=128`, `t='auto'`, `knn=8`.
- Animation target: 2D GRAE with `epochs=30`, `n_components=2`, `lr=.0001`, `batch_size=128`, `t='auto'`, `knn=8`.
- Runtime path: direct `metric-webgl` scene code in `index.html`.
- Required render elements: shader sphere points, bounded floor, coordinate grid,
  ground projection, cluster labels, hover record preview, camera-depth
  `BokehPass`, and a separate post-bokeh label render pass.
- Animation: the scene rotates slowly around its own axis. The timeline holds
  the 2D target projection, morphs quickly to 3D, holds the 3D metric-space
  coordinates, then morphs back.
- Coordinates: y-up, `groundY = 0`, `flatY ~= 0.18`, `PLOT_RADIUS ~= 1.78`,
  `SPACE_HEIGHT ~= 2.68`.
- Camera: oblique y-up perspective with interactive zoom, drag, pause, and reset.

Do not replace this example with a generic point-cloud page. The GRAE10 visual
is the reference for the METRIC miniature data-rendering style.
