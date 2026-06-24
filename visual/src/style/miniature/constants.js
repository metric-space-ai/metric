export const MINIATURE_STYLE_ID = "metric.visual.style.miniature.photographic";

export const MINIATURE_STYLE_SCHEMA = "metric.visual.miniature_style.v1";

export const MINIATURE_STYLE_PROFILE_SCHEMA = "metric.visual.miniature_style_profile.v1";

export const TROIS_MINIATURE_SOURCE_AUDIT = Object.freeze({
  demos: Object.freeze([
    "/tmp/metric-troisjs-github-io/src/components/demos/Demo1.vue",
    "/tmp/metric-troisjs-github-io/src/components/demos/Demo2.vue",
    "/tmp/metric-troisjs-github-io/src/components/demos/Demo3.vue",
    "/tmp/metric-troisjs-github-io/src/components/demos/Demo4.vue",
    "/tmp/metric-troisjs-github-io/src/components/demos/Demo5.vue",
    "/tmp/metric-troisjs-github-io/src/components/demos/Demo6.vue",
    "/tmp/metric-troisjs-github-io/src/components/demos/Demo7.vue",
    "/tmp/metric-troisjs-github-io/src/components/misc/ShadowsExample.vue",
    "/tmp/metric-troisjs-github-io/src/components/misc/LightExample.vue",
    "/tmp/metric-troisjs-github-io/src/components/materials/SubSurface.vue",
    "/tmp/metric-troisjs-github-io/src/components/materials/Matcap.vue",
    "/tmp/metric-troisjs-github-io/src/components/materials/Textures.vue",
    "/tmp/metric-trois-site/assets/Demo1.ea0cb3fb.js",
    "/tmp/metric-trois-site/assets/Demo2.836d891c.js",
    "/tmp/metric-trois-site/assets/Demo3.e1be2da1.js",
    "/tmp/metric-trois-site/assets/Demo4.ef5a6b30.js",
    "/tmp/metric-trois-site/assets/Demo5.ee55b5a1.js",
    "/tmp/metric-trois-site/assets/Demo6.7620ed17.js",
    "/tmp/metric-trois-site/assets/Demo7.e98226ba.js",
    "/tmp/metric-trois-site/assets/ShadowsExample.b12b538f.js",
    "/tmp/metric-trois-site/assets/LightExample.80edbe9a.js",
    "/tmp/metric-trois-site/assets/SubSurface.1f1d6931.js",
    "/tmp/metric-trois-site/assets/Matcap.6ac6e810.js",
    "/tmp/metric-trois-site/assets/Textures.53c6bcc6.js",
  ]),
  effects: Object.freeze([
    "BokehPass(focus, aperture, maxblur)",
    "FXAAPass(resolution)",
    "FilmPass(noiseIntensity, scanlinesIntensity, scanlinesCount, grayscale)",
    "HalftonePass(shape, radius, rotateR, rotateG, rotateB, scatter)",
    "SMAAPass(width, height)",
    "SSAOPass(options)",
    "TiltShiftPass(blurRadius, gradientRadius, start, end)",
    "UnrealBloomPass(strength, radius, threshold)",
    "ZoomBlurPass(center, strength)",
  ]),
  materials: Object.freeze([
    "BasicMaterial",
    "LambertMaterial",
    "PhongMaterial",
    "PhysicalMaterial",
    "PointsMaterial",
    "StandardMaterial",
    "ToonMaterial",
    "MatcapMaterial",
    "ShaderMaterial",
    "SubSurfaceMaterial",
  ]),
  extractedIngredients: Object.freeze([
    "white or black photographic stage backgrounds",
    "long-lens and high-oblique cameras aimed at a centered subject",
    "orbit damping and gentle auto-rotation as composition, not data motion",
    "ambient fill plus colored point or spot rigs",
    "Phong-like vertex/data color lighting",
    "Standard/Physical glossy materials with metalness and roughness controls",
    "matcap and shader-material hooks for non-photoreal accents",
    "screen-space antialiasing through FXAA/SMAA",
    "selective bloom for luminous accents",
    "tilt-shift two-axis blur with pointer-driven horizontal focus line",
    "bokeh aperture/maxblur depth-of-field controls",
    "film grain, halftone texture, zoom blur, and SSAO as optional photographic treatments",
    "simplex-like shader displacement/noise as surface texture, not data recomputation",
    "pointer-coupled focus/motion parameters",
    "grounded floor, semantic colored shadows, and isometric projection footprints",
  ]),
});

export const MINIATURE_DESCRIPTOR_TAG = "miniature-photographic";

export const MINIATURE_LAYER_ROLES = Object.freeze({
  sample: "miniature-sample",
  mesh: "miniature-mesh",
  ground: "miniature-ground-plane",
  projection: "miniature-ground-projection",
  surface: "miniature-surface",
  edge: "miniature-relation-edge",
  field: "miniature-field",
  glyph: "miniature-glyph",
  image: "miniature-image",
});
