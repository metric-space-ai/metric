METRIC Visual is native METRIC code.

The raw WebGL point-cloud, morph, and animation direction was informed by
reviewing Babyplots:

  https://github.com/derpylz/babyplots

Babyplots is licensed under the Apache License, Version 2.0. METRIC does not
depend on Babyplots and does not expose Babyplots APIs. If future work ports
substantial Babyplots source into this library, the port must keep the Apache
2.0 license notice required by that source.

The native tilt-shift postprocess implementation is adapted from:

  https://github.com/evanw/glfx.js

glfx.js is licensed under the MIT License, Copyright (C) 2011 by Evan Wallace.

The native postprocess pipeline was also informed by the Trois tilt-shift
wrapper and shader organization:

  https://github.com/troisjs/trois

Trois is licensed under the MIT License, Copyright (c) 2021 troisjs. METRIC
does not depend on Trois, Three.js, or their runtime APIs.

The METRIC `metric-webgl` runtime contains source derived from Three.js r127 for
WebGL scene rendering and postprocessing:

  https://github.com/mrdoob/three.js

Three.js is licensed under the MIT License, Copyright (c) 2010-2021 three.js
authors. The derived source lives under `visual/src/runtime/metric-webgl/` and
keeps its license at `visual/src/runtime/metric-webgl/LICENSE`.
