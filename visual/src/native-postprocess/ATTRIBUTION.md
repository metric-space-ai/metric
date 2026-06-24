# Native Postprocess Attribution

This directory is dependency-free METRIC Visual code. It does not import THREE,
source visual references, local tilt-shift filter, npm packages, or browser build tooling.

The tilt-shift blur algorithm in `tilt-shift.js` is ported from:

- local tilt-shift filter Tilt Shift filter by Evan Wallace
  https://github.com/evanw/local tilt-shift filter
  MIT License, Copyright (C) 2011 by Evan Wallace
- local tilt-shift shader and pass
  wrapper
  https://github.com/troisjs/trois
  MIT License, Copyright (c) 2021 troisjs
- metric focus-depth study example `src/components/demos/Demo5.vue` from the local
  `/tmp/metric-trois-site` reference

The native implementation keeps the glfx/source visual references mechanics: two shader passes,
61 samples from `-30.0` through `30.0`, randomized lookup offset, blur radius
from pixel distance to the focus line divided by `gradientRadius`, pass 1 along
the focus-line direction, and pass 2 orthogonal to it. The WebGL wrapper is
dependency-free and does not import external renderer APIs.

The Demo5 semantics are preserved by accepting `gradientRadius` as a pixel
radius such as `height / 3` and by using a bottom-origin pointer Y for horizontal
focus lines equivalent to `{ x: 0, y: tiltY }` through `{ x: 100, y: tiltY }`.
