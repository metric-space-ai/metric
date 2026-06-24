# METRIC Visual Materials

This directory contains renderer-agnostic material descriptors for the native
WebGL visual canvas. Materials do not create buffers, textures, cameras, render
passes, or visual metrics. They only describe reusable shader programs and the
uniform/attribute contract future layers can bind to.

## Public shape

```js
import {
  PointMaterial,
  MaterialProgram,
  createSurfaceMaterial,
} from "./materials/index.js";

const material = new PointMaterial({
  dataColor: true,
  scalarColorMap: {
    mix: 0.35,
    domain: [0, 1],
    lowColor: "#eef6ff",
    highColor: "#174f83",
  },
  selectionLift: 0.04,
  focusLift: 0.025,
});

const shader = material.createShaderDescriptor();
// shader.vertex, shader.fragment, shader.attributes, shader.uniforms

const program = new MaterialProgram(gl, material);
```

## Built-ins

- `PointMaterial`: point sprites with circular or square masks, alpha feathering,
  scalar color-map hooks, and picking-pass support.
- `GlyphMaterial`: lightweight instanced glyph/billboard material.
- `BoxMaterial`: instanced box mesh material with ambient/diffuse/specular
  lighting defaults for miniature scenes.
- `SurfaceMaterial`: vertex-colored surface material with normal-based lighting.
- `GroundProjectionMaterial`: radial footprint material for projected ground
  shadows, focus hints, or density impressions supplied by data.
- `EdgeMaterial`: expanded-edge strip material with data-driven edge alpha.
- `HeatFieldMaterial`: planar field material driven by scalar attributes or an
  optional heat texture supplied by the caller.

## Common uniforms

All built-ins use the same naming pattern where possible:

- matrices: `u_modelMatrix`, `u_viewMatrix`, `u_projectionMatrix`,
  `u_viewProjectionMatrix`, `u_normalMatrix`
- data color: `u_baseColor`, `u_dataColorMix`
- scalar color: `u_scalarColorMix`, `u_scalarDomain`, `u_scalarLowColor`,
  `u_scalarHighColor`, `u_scalarColorMap`
- alpha/gamma: `u_alpha`, `u_alphaDiscard`, `u_gamma`, `u_exposure`
- interaction: `u_liftAxis`, `u_selectionLift`, `u_focusLift`,
  `u_selectionColor`, `u_focusColor`
- lighting: `u_ambientColor`, `u_ambientStrength`, `u_lightDirection`,
  `u_lightColor`, `u_diffuseStrength`, `u_specularColor`,
  `u_specularStrength`, `u_shininess`, `u_cameraPosition`

The scalar color-map texture hook is enabled with `scalarColorMap.texture: true`
or a shader option that defines `METRIC_USE_SCALAR_TEXTURE`. The material only
exposes the sampler uniform; texture creation and binding stay with the caller.

## Attribute contract

The shader descriptors export `attributes` metadata. Common semantics include:

- `a_position`: vertex or instance position
- `a_color`: data-provided RGBA color
- `a_scalar`: scalar value for color-map sampling
- `a_alpha`: per-item alpha multiplier
- `a_selection`: selection weight in `[0, 1]`
- `a_focus`: focus weight in `[0, 1]`
- `a_pickingId`: numeric id encoded by picking-pass shaders

Mesh-like materials add attributes such as `a_normal`, `a_instancePosition`,
`a_instanceScale`, `a_corner`, `a_edgeDistance`, or `a_uv` as needed.

## Design constraints

- WebGL1 GLSL by default. WebGL2 can be introduced by future callers through
  defines, but no built-in requires WebGL2.
- No runtime dependencies, npm packages, TypeScript, external renderer, source visual references, or the earlier point-cloud renderer.
- No renderer is created here.
- No metric math, entropy, density, or heat computation is performed in
  JavaScript. Field values, scalar buffers, and textures are supplied by layers.
