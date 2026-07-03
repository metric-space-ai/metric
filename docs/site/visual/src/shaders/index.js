/*
 * Native METRIC Visual shader library.
 *
 * The module exposes WebGL1-compatible GLSL strings and source factories for
 * future layers. It intentionally does not create a renderer or compute visual
 * fields in JavaScript; callers provide geometry, scalar values, colors, and
 * optional color-map textures as data.
 */

export const IDENTITY_MAT4 = Object.freeze([
  1, 0, 0, 0,
  0, 1, 0, 0,
  0, 0, 1, 0,
  0, 0, 0, 1,
]);

export const IDENTITY_MAT3 = Object.freeze([
  1, 0, 0,
  0, 1, 0,
  0, 0, 1,
]);

export const ShaderChunk = Object.freeze({
  precision: `
#ifdef GL_ES
precision highp float;
precision highp int;
#endif
`,

  common: `
#ifndef METRIC_COMMON_CHUNK
#define METRIC_COMMON_CHUNK

uniform vec3 u_liftAxis;
uniform float u_selectionLift;
uniform float u_focusLift;

float metricSaturate(float value) {
  return clamp(value, 0.0, 1.0);
}

vec2 metricSaturate2(vec2 value) {
  return clamp(value, vec2(0.0), vec2(1.0));
}

vec3 metricSaturate3(vec3 value) {
  return clamp(value, vec3(0.0), vec3(1.0));
}

vec4 metricSaturate4(vec4 value) {
  return clamp(value, vec4(0.0), vec4(1.0));
}

vec3 metricLiftPosition(vec3 position, float selection, float focus) {
  float lift = metricSaturate(selection) * u_selectionLift + metricSaturate(focus) * u_focusLift;
  return position + normalize(u_liftAxis) * lift;
}

float metricSafeRange(float value) {
  return max(abs(value), 0.000001);
}

#endif
`,

  color: `
#ifndef METRIC_COLOR_CHUNK
#define METRIC_COLOR_CHUNK

uniform vec4 u_baseColor;
uniform float u_alpha;
uniform float u_alphaDiscard;
uniform float u_dataColorMix;
uniform float u_scalarColorMix;
uniform vec2 u_scalarDomain;
uniform vec4 u_scalarLowColor;
uniform vec4 u_scalarHighColor;
uniform vec4 u_missingColor;
uniform vec4 u_selectionColor;
uniform vec4 u_focusColor;
uniform sampler2D u_scalarColorMap;

float metricNormalizeScalar(float value) {
  float span = metricSafeRange(u_scalarDomain.y - u_scalarDomain.x);
  return metricSaturate((value - u_scalarDomain.x) / span);
}

vec4 metricSampleScalarColor(float value) {
  if (value != value) {
    return u_missingColor;
  }
  float t = metricNormalizeScalar(value);
#ifdef METRIC_USE_SCALAR_TEXTURE
  return texture2D(u_scalarColorMap, vec2(t, 0.5));
#else
  return mix(u_scalarLowColor, u_scalarHighColor, t);
#endif
}

vec3 metricLiftColor(vec3 color, float amount) {
  float lift = metricSaturate(amount);
  return mix(color, vec3(1.0), lift);
}

vec4 metricResolveDataColor(vec4 dataColor, float scalarValue, float alphaValue, float selection, float focus) {
  vec4 color = mix(u_baseColor, dataColor, metricSaturate(u_dataColorMix));
  vec4 scalarColor = metricSampleScalarColor(scalarValue);
  color = mix(color, scalarColor, metricSaturate(u_scalarColorMix));

  float selected = metricSaturate(selection);
  float focused = metricSaturate(focus);
  color.rgb = mix(color.rgb, u_selectionColor.rgb, selected * metricSaturate(u_selectionColor.a));
  color.rgb = mix(color.rgb, u_focusColor.rgb, focused * metricSaturate(u_focusColor.a));
  color.rgb = metricLiftColor(color.rgb, selected * 0.08 + focused * 0.05);
  color.a *= u_alpha * metricSaturate(alphaValue);
  return color;
}

#endif
`,

  gamma: `
#ifndef METRIC_GAMMA_CHUNK
#define METRIC_GAMMA_CHUNK

uniform float u_gamma;
uniform float u_exposure;

vec3 metricLinearToSrgb(vec3 color) {
  vec3 low = 12.92 * color;
  vec3 high = 1.055 * pow(max(color, vec3(0.0)), vec3(1.0 / 2.4)) - 0.055;
  return mix(low, high, step(vec3(0.0031308), color));
}

vec3 metricApplyExposure(vec3 color) {
  return vec3(1.0) - exp(-max(color, vec3(0.0)) * max(u_exposure, 0.0));
}

vec4 metricApplyGamma(vec4 color) {
#ifdef METRIC_DISABLE_GAMMA
  return metricSaturate4(color);
#else
  vec3 exposed = metricApplyExposure(color.rgb);
  vec3 corrected = pow(max(exposed, vec3(0.0)), vec3(1.0 / max(u_gamma, 0.0001)));
  return vec4(metricSaturate3(corrected), metricSaturate(color.a));
#endif
}

#endif
`,

  lighting: `
#ifndef METRIC_LIGHTING_CHUNK
#define METRIC_LIGHTING_CHUNK

uniform vec3 u_ambientColor;
uniform float u_ambientStrength;
uniform vec3 u_lightDirection;
uniform vec3 u_lightColor;
uniform float u_diffuseStrength;
uniform vec3 u_specularColor;
uniform float u_specularStrength;
uniform float u_shininess;
uniform vec3 u_cameraPosition;

vec3 metricLightSurface(vec3 baseColor, vec3 normal, vec3 worldPosition) {
  vec3 n = normalize(normal);
  vec3 l = normalize(-u_lightDirection);
  vec3 v = normalize(u_cameraPosition - worldPosition);
  vec3 h = normalize(l + v);

  float diffuse = max(dot(n, l), 0.0);
  float specular = pow(max(dot(n, h), 0.0), max(u_shininess, 1.0));

  vec3 ambientTerm = baseColor * u_ambientColor * u_ambientStrength;
  vec3 diffuseTerm = baseColor * u_lightColor * diffuse * u_diffuseStrength;
  vec3 specularTerm = u_specularColor * specular * u_specularStrength;
  return ambientTerm + diffuseTerm + specularTerm;
}

#endif
`,

  picking: `
#ifndef METRIC_PICKING_CHUNK
#define METRIC_PICKING_CHUNK

vec4 metricEncodePickingId(float id) {
  float value = floor(max(id, 0.0));
  float r = mod(value, 256.0);
  value = floor(value / 256.0);
  float g = mod(value, 256.0);
  value = floor(value / 256.0);
  float b = mod(value, 256.0);
  value = floor(value / 256.0);
  float a = mod(value, 256.0);
  return vec4(r, g, b, a) / 255.0;
}

float metricDecodePickingId(vec4 encoded) {
  vec4 byteValue = floor(encoded * 255.0 + 0.5);
  return byteValue.r + byteValue.g * 256.0 + byteValue.b * 65536.0 + byteValue.a * 16777216.0;
}

#endif
`,

  pointSprite: `
#ifndef METRIC_POINT_SPRITE_CHUNK
#define METRIC_POINT_SPRITE_CHUNK

float metricPointCircleAlpha(vec2 coord, float feather) {
  vec2 p = coord * 2.0 - 1.0;
  float radius = length(p);
  float edge = max(feather, 0.0001);
  return 1.0 - smoothstep(1.0 - edge, 1.0, radius);
}

float metricPointSquareAlpha(vec2 coord, float feather) {
  vec2 edgeDistance = min(coord, 1.0 - coord);
  float edge = max(feather, 0.0001);
  return metricSaturate(min(edgeDistance.x, edgeDistance.y) / edge);
}

float metricPointSpriteAlpha(vec2 coord, float feather) {
#ifdef METRIC_POINT_SHAPE_SQUARE
  return metricPointSquareAlpha(coord, feather);
#else
  return metricPointCircleAlpha(coord, feather);
#endif
}

#endif
`,

  radialFootprint: `
#ifndef METRIC_RADIAL_FOOTPRINT_CHUNK
#define METRIC_RADIAL_FOOTPRINT_CHUNK

float metricRadialFootprintAlpha(vec2 coord, float radius, float feather) {
  vec2 p = coord * 2.0 - 1.0;
  float distanceFromCenter = length(p);
  float r = max(radius, 0.0001);
  float edge = max(feather, 0.0001);
  return 1.0 - smoothstep(r - edge, r, distanceFromCenter);
}

#endif
`,

  edgeAlpha: `
#ifndef METRIC_EDGE_ALPHA_CHUNK
#define METRIC_EDGE_ALPHA_CHUNK

float metricEdgeAlpha(float distanceFromCenter, float thickness, float feather) {
  float halfWidth = max(thickness * 0.5, 0.0001);
  float edge = max(feather, 0.0001);
  float distanceToEdge = abs(distanceFromCenter) - halfWidth;
  return 1.0 - smoothstep(0.0, edge, distanceToEdge);
}

#endif
`,

  surfaceNormals: `
#ifndef METRIC_SURFACE_NORMALS_CHUNK
#define METRIC_SURFACE_NORMALS_CHUNK

vec3 metricSafeNormal(vec3 normal, vec3 fallback) {
  float lengthSquared = dot(normal, normal);
  if (lengthSquared <= 0.000001) {
    return normalize(fallback);
  }
  return normal * inversesqrt(lengthSquared);
}

vec3 metricSurfaceNormalFromTangents(vec3 tangentX, vec3 tangentY) {
  return metricSafeNormal(cross(tangentX, tangentY), vec3(0.0, 1.0, 0.0));
}

vec3 metricTransformNormal(mat3 normalMatrix, vec3 normal) {
  return metricSafeNormal(normalMatrix * normal, vec3(0.0, 1.0, 0.0));
}

#endif
`,
});

export const COMMON_UNIFORMS = Object.freeze({
  u_modelMatrix: IDENTITY_MAT4,
  u_viewMatrix: IDENTITY_MAT4,
  u_projectionMatrix: IDENTITY_MAT4,
  u_viewProjectionMatrix: IDENTITY_MAT4,
  u_normalMatrix: IDENTITY_MAT3,
  u_cameraPosition: [0, 0, 1],
  u_viewport: [1, 1],
  u_liftAxis: [0, 0, 1],
  u_selectionLift: 0,
  u_focusLift: 0,
});

export const COLOR_UNIFORMS = Object.freeze({
  u_baseColor: [0.54, 0.62, 0.7, 1],
  u_alpha: 1,
  u_alphaDiscard: 0.001,
  u_dataColorMix: 1,
  u_scalarColorMix: 0,
  u_scalarDomain: [0, 1],
  u_scalarLowColor: [0.92, 0.96, 1, 1],
  u_scalarHighColor: [0.06, 0.28, 0.58, 1],
  u_missingColor: [0.54, 0.58, 0.64, 0.55],
  u_selectionColor: [1, 0.88, 0.35, 0.65],
  u_focusColor: [0.73, 0.91, 1, 0.55],
  u_scalarColorMap: 0,
  u_gamma: 2.2,
  u_exposure: 1,
});

export const LIGHTING_UNIFORMS = Object.freeze({
  u_ambientColor: [1, 1, 1],
  u_ambientStrength: 0.56,
  u_lightDirection: [-0.35, -0.45, -0.82],
  u_lightColor: [1, 0.98, 0.92],
  u_diffuseStrength: 0.5,
  u_specularColor: [1, 1, 1],
  u_specularStrength: 0.12,
  u_shininess: 28,
});

export const POINT_UNIFORMS = Object.freeze({
  u_pointScale: 1,
  u_minPointSize: 1,
  u_maxPointSize: 96,
  u_sizeAttenuation: 220,
  u_pointFeather: 0.18,
});

export const GLYPH_UNIFORMS = Object.freeze({
  u_billboardRight: [1, 0, 0],
  u_billboardUp: [0, 1, 0],
  u_glyphScale: 1,
  u_glyphFeather: 0.08,
});

export const GROUND_UNIFORMS = Object.freeze({
  u_groundRightAxis: [1, 0, 0],
  u_groundForwardAxis: [0, 1, 0],
  u_groundOffset: 0.001,
  u_footprintRadius: 1,
  u_footprintFeather: 0.35,
});

export const EDGE_UNIFORMS = Object.freeze({
  u_edgeThickness: 1,
  u_edgeFeather: 0.35,
});

export const HEAT_UNIFORMS = Object.freeze({
  u_heatTexture: 0,
  u_heatTextureMix: 1,
});

export const AttributeLayout = Object.freeze({
  point: Object.freeze({
    a_position: { size: 3, semantic: "position" },
    a_color: { size: 4, semantic: "dataColor" },
    a_size: { size: 1, semantic: "size" },
    a_scalar: { size: 1, semantic: "scalar" },
    a_alpha: { size: 1, semantic: "alpha" },
    a_selection: { size: 1, semantic: "selection" },
    a_focus: { size: 1, semantic: "focus" },
    a_pickingId: { size: 1, semantic: "pickingId" },
  }),

  glyph: Object.freeze({
    a_position: { size: 3, semantic: "localPosition" },
    a_instancePosition: { size: 3, semantic: "position" },
    a_glyphScale: { size: 2, semantic: "glyphScale" },
    a_color: { size: 4, semantic: "dataColor" },
    a_scalar: { size: 1, semantic: "scalar" },
    a_alpha: { size: 1, semantic: "alpha" },
    a_selection: { size: 1, semantic: "selection" },
    a_focus: { size: 1, semantic: "focus" },
    a_pickingId: { size: 1, semantic: "pickingId" },
  }),

  box: Object.freeze({
    a_position: { size: 3, semantic: "localPosition" },
    a_normal: { size: 3, semantic: "normal" },
    a_instancePosition: { size: 3, semantic: "position" },
    a_instanceScale: { size: 3, semantic: "scale" },
    a_color: { size: 4, semantic: "dataColor" },
    a_scalar: { size: 1, semantic: "scalar" },
    a_alpha: { size: 1, semantic: "alpha" },
    a_selection: { size: 1, semantic: "selection" },
    a_focus: { size: 1, semantic: "focus" },
    a_pickingId: { size: 1, semantic: "pickingId" },
  }),

  surface: Object.freeze({
    a_position: { size: 3, semantic: "position" },
    a_normal: { size: 3, semantic: "normal" },
    a_color: { size: 4, semantic: "dataColor" },
    a_scalar: { size: 1, semantic: "scalar" },
    a_alpha: { size: 1, semantic: "alpha" },
    a_selection: { size: 1, semantic: "selection" },
    a_focus: { size: 1, semantic: "focus" },
    a_pickingId: { size: 1, semantic: "pickingId" },
  }),

  groundProjection: Object.freeze({
    a_position: { size: 3, semantic: "position" },
    a_corner: { size: 2, semantic: "footprintCorner" },
    a_radius: { size: 1, semantic: "radius" },
    a_color: { size: 4, semantic: "dataColor" },
    a_scalar: { size: 1, semantic: "scalar" },
    a_alpha: { size: 1, semantic: "alpha" },
    a_selection: { size: 1, semantic: "selection" },
    a_focus: { size: 1, semantic: "focus" },
    a_pickingId: { size: 1, semantic: "pickingId" },
  }),

  edge: Object.freeze({
    a_position: { size: 3, semantic: "position" },
    a_edgeDistance: { size: 1, semantic: "edgeDistance" },
    a_color: { size: 4, semantic: "dataColor" },
    a_scalar: { size: 1, semantic: "scalar" },
    a_alpha: { size: 1, semantic: "alpha" },
    a_selection: { size: 1, semantic: "selection" },
    a_focus: { size: 1, semantic: "focus" },
    a_pickingId: { size: 1, semantic: "pickingId" },
  }),

  heatField: Object.freeze({
    a_position: { size: 3, semantic: "position" },
    a_uv: { size: 2, semantic: "uv" },
    a_color: { size: 4, semantic: "dataColor" },
    a_scalar: { size: 1, semantic: "scalar" },
    a_alpha: { size: 1, semantic: "alpha" },
    a_selection: { size: 1, semantic: "selection" },
    a_focus: { size: 1, semantic: "focus" },
    a_pickingId: { size: 1, semantic: "pickingId" },
  }),
});

export const DefaultAttributeLocations = Object.freeze({
  a_position: 0,
  a_normal: 1,
  a_color: 2,
  a_size: 3,
  a_scalar: 4,
  a_alpha: 5,
  a_selection: 6,
  a_focus: 7,
  a_pickingId: 8,
  a_instancePosition: 9,
  a_instanceScale: 10,
  a_glyphScale: 11,
  a_corner: 12,
  a_radius: 13,
  a_edgeDistance: 14,
  a_uv: 15,
});

export function makeShaderSource(options = {}) {
  const version = options.version ? String(options.version).trim() : "";
  const extensions = normalizeStringList(options.extensions);
  const defines = stringifyShaderDefines(options.defines);
  const chunks = normalizeStringList(options.chunks);
  return [
    version,
    ...extensions,
    defines,
    ShaderChunk.precision,
    ...chunks,
  ].filter(Boolean).join("\n\n").trimStart();
}

export function stringifyShaderDefines(defines = {}) {
  const lines = [];
  for (const [name, value] of Object.entries(defines)) {
    if (value === false || value === undefined || value === null) continue;
    lines.push(value === true ? `#define ${name}` : `#define ${name} ${value}`);
  }
  return lines.join("\n");
}

export function mergeShaderDefines(...defineSets) {
  return Object.assign({}, ...defineSets.filter(Boolean));
}

export function mergeUniformDefaults(...uniformSets) {
  const merged = {};
  for (const uniforms of uniformSets) {
    if (!uniforms) continue;
    for (const [name, value] of Object.entries(uniforms)) {
      merged[name] = cloneUniformValue(value);
    }
  }
  return merged;
}

export function createShaderProgramDescriptor(options = {}) {
  return {
    kind: options.kind || "material",
    label: options.label || options.kind || "material",
    primitive: options.primitive || "triangles",
    vertex: options.vertex || "",
    fragment: options.fragment || "",
    attributes: options.attributes || {},
    uniforms: mergeUniformDefaults(options.uniforms),
    defines: { ...(options.defines || {}) },
    attribLocations: options.attribLocations || DefaultAttributeLocations,
    metadata: { ...(options.metadata || {}) },
  };
}

export function createPointShader(options = {}) {
  const defines = mergeShaderDefines({
    METRIC_POINT_SHAPE_SQUARE: options.shape === "square",
    METRIC_SIZE_ATTENUATION: options.sizeAttenuation === true,
    METRIC_USE_SCALAR_TEXTURE: options.scalarTexture === true,
    METRIC_PICKING_PASS: options.picking === true,
    METRIC_DISABLE_GAMMA: options.gamma === false,
  }, options.defines);

  const vertex = makeShaderSource({
    defines,
    chunks: [
      ShaderChunk.common,
      `
attribute vec3 a_position;
attribute vec4 a_color;
attribute float a_size;
attribute float a_scalar;
attribute float a_alpha;
attribute float a_selection;
attribute float a_focus;
attribute float a_pickingId;

uniform mat4 u_modelMatrix;
uniform mat4 u_viewMatrix;
uniform mat4 u_projectionMatrix;
uniform float u_pointScale;
uniform float u_minPointSize;
uniform float u_maxPointSize;
uniform float u_sizeAttenuation;

varying vec4 v_dataColor;
varying float v_scalar;
varying float v_alpha;
varying float v_selection;
varying float v_focus;
varying float v_pickingId;

void main() {
  vec3 liftedPosition = metricLiftPosition(a_position, a_selection, a_focus);
  vec4 worldPosition = u_modelMatrix * vec4(liftedPosition, 1.0);
  vec4 viewPosition = u_viewMatrix * worldPosition;
  gl_Position = u_projectionMatrix * viewPosition;

  float pointSize = a_size * u_pointScale;
#ifdef METRIC_SIZE_ATTENUATION
  pointSize *= u_sizeAttenuation / max(0.001, -viewPosition.z);
#endif
  gl_PointSize = clamp(pointSize, u_minPointSize, u_maxPointSize);

  v_dataColor = a_color;
  v_scalar = a_scalar;
  v_alpha = a_alpha;
  v_selection = a_selection;
  v_focus = a_focus;
  v_pickingId = a_pickingId;
}
`,
    ],
  });

  const fragment = makeShaderSource({
    defines,
    chunks: [
      ShaderChunk.common,
      ShaderChunk.color,
      ShaderChunk.gamma,
      ShaderChunk.picking,
      ShaderChunk.pointSprite,
      `
uniform float u_pointFeather;

varying vec4 v_dataColor;
varying float v_scalar;
varying float v_alpha;
varying float v_selection;
varying float v_focus;
varying float v_pickingId;

void main() {
  float spriteAlpha = metricPointSpriteAlpha(gl_PointCoord, u_pointFeather);
  if (spriteAlpha <= 0.001) {
    discard;
  }

  vec4 color = metricResolveDataColor(v_dataColor, v_scalar, v_alpha, v_selection, v_focus);
  color.a *= spriteAlpha;
  if (color.a <= u_alphaDiscard) {
    discard;
  }

#ifdef METRIC_PICKING_PASS
  gl_FragColor = metricEncodePickingId(v_pickingId);
#else
  gl_FragColor = metricApplyGamma(color);
#endif
}
`,
    ],
  });

  return createShaderProgramDescriptor({
    kind: "point",
    label: "PointMaterial",
    primitive: "points",
    vertex,
    fragment,
    attributes: AttributeLayout.point,
    uniforms: mergeUniformDefaults(COMMON_UNIFORMS, COLOR_UNIFORMS, POINT_UNIFORMS, options.uniforms),
    defines,
    metadata: { shape: options.shape || "circle" },
  });
}

export function createGlyphShader(options = {}) {
  const defines = mergeShaderDefines({
    METRIC_BILLBOARD_GLYPHS: options.billboard !== false,
    METRIC_USE_SCALAR_TEXTURE: options.scalarTexture === true,
    METRIC_PICKING_PASS: options.picking === true,
    METRIC_DISABLE_GAMMA: options.gamma === false,
  }, options.defines);

  const vertex = makeShaderSource({
    defines,
    chunks: [
      ShaderChunk.common,
      `
attribute vec3 a_position;
attribute vec3 a_instancePosition;
attribute vec2 a_glyphScale;
attribute vec4 a_color;
attribute float a_scalar;
attribute float a_alpha;
attribute float a_selection;
attribute float a_focus;
attribute float a_pickingId;

uniform mat4 u_modelMatrix;
uniform mat4 u_viewProjectionMatrix;
uniform vec3 u_billboardRight;
uniform vec3 u_billboardUp;
uniform float u_glyphScale;

varying vec4 v_dataColor;
varying float v_scalar;
varying float v_alpha;
varying float v_selection;
varying float v_focus;
varying float v_pickingId;

void main() {
  vec3 localPosition = vec3(a_position.xy * a_glyphScale * u_glyphScale, a_position.z);
#ifdef METRIC_BILLBOARD_GLYPHS
  vec3 oriented = u_billboardRight * localPosition.x + u_billboardUp * localPosition.y + u_liftAxis * localPosition.z;
#else
  vec3 oriented = localPosition;
#endif
  vec3 anchor = metricLiftPosition(a_instancePosition, a_selection, a_focus);
  vec4 worldPosition = u_modelMatrix * vec4(anchor + oriented, 1.0);
  gl_Position = u_viewProjectionMatrix * worldPosition;

  v_dataColor = a_color;
  v_scalar = a_scalar;
  v_alpha = a_alpha;
  v_selection = a_selection;
  v_focus = a_focus;
  v_pickingId = a_pickingId;
}
`,
    ],
  });

  const fragment = makeShaderSource({
    defines,
    chunks: [
      ShaderChunk.common,
      ShaderChunk.color,
      ShaderChunk.gamma,
      ShaderChunk.picking,
      `
varying vec4 v_dataColor;
varying float v_scalar;
varying float v_alpha;
varying float v_selection;
varying float v_focus;
varying float v_pickingId;

void main() {
  vec4 color = metricResolveDataColor(v_dataColor, v_scalar, v_alpha, v_selection, v_focus);
  if (color.a <= u_alphaDiscard) {
    discard;
  }

#ifdef METRIC_PICKING_PASS
  gl_FragColor = metricEncodePickingId(v_pickingId);
#else
  gl_FragColor = metricApplyGamma(color);
#endif
}
`,
    ],
  });

  return createShaderProgramDescriptor({
    kind: "glyph",
    label: "GlyphMaterial",
    primitive: "triangles",
    vertex,
    fragment,
    attributes: AttributeLayout.glyph,
    uniforms: mergeUniformDefaults(COMMON_UNIFORMS, COLOR_UNIFORMS, GLYPH_UNIFORMS, options.uniforms),
    defines,
  });
}

export function createBoxShader(options = {}) {
  const defines = mergeShaderDefines({
    METRIC_USE_SCALAR_TEXTURE: options.scalarTexture === true,
    METRIC_PICKING_PASS: options.picking === true,
    METRIC_DISABLE_GAMMA: options.gamma === false,
    METRIC_UNLIT: options.lighting === false,
  }, options.defines);

  const vertex = makeShaderSource({
    defines,
    chunks: [
      ShaderChunk.common,
      ShaderChunk.surfaceNormals,
      `
attribute vec3 a_position;
attribute vec3 a_normal;
attribute vec3 a_instancePosition;
attribute vec3 a_instanceScale;
attribute vec4 a_color;
attribute float a_scalar;
attribute float a_alpha;
attribute float a_selection;
attribute float a_focus;
attribute float a_pickingId;

uniform mat4 u_modelMatrix;
uniform mat4 u_viewProjectionMatrix;
uniform mat3 u_normalMatrix;

varying vec4 v_dataColor;
varying float v_scalar;
varying float v_alpha;
varying float v_selection;
varying float v_focus;
varying float v_pickingId;
varying vec3 v_normal;
varying vec3 v_worldPosition;

void main() {
  vec3 localPosition = a_position * max(a_instanceScale, vec3(0.0001));
  vec3 instancePosition = metricLiftPosition(a_instancePosition, a_selection, a_focus);
  vec4 worldPosition = u_modelMatrix * vec4(instancePosition + localPosition, 1.0);
  gl_Position = u_viewProjectionMatrix * worldPosition;

  v_dataColor = a_color;
  v_scalar = a_scalar;
  v_alpha = a_alpha;
  v_selection = a_selection;
  v_focus = a_focus;
  v_pickingId = a_pickingId;
  v_normal = metricTransformNormal(u_normalMatrix, a_normal);
  v_worldPosition = worldPosition.xyz;
}
`,
    ],
  });

  const fragment = makeShaderSource({
    defines,
    chunks: [
      ShaderChunk.common,
      ShaderChunk.color,
      ShaderChunk.gamma,
      ShaderChunk.lighting,
      ShaderChunk.picking,
      `
varying vec4 v_dataColor;
varying float v_scalar;
varying float v_alpha;
varying float v_selection;
varying float v_focus;
varying float v_pickingId;
varying vec3 v_normal;
varying vec3 v_worldPosition;

void main() {
  vec4 color = metricResolveDataColor(v_dataColor, v_scalar, v_alpha, v_selection, v_focus);
  if (color.a <= u_alphaDiscard) {
    discard;
  }

#ifdef METRIC_PICKING_PASS
  gl_FragColor = metricEncodePickingId(v_pickingId);
#else
#ifdef METRIC_UNLIT
  gl_FragColor = metricApplyGamma(color);
#else
  vec3 lit = metricLightSurface(color.rgb, v_normal, v_worldPosition);
  gl_FragColor = metricApplyGamma(vec4(lit, color.a));
#endif
#endif
}
`,
    ],
  });

  return createShaderProgramDescriptor({
    kind: "box",
    label: "BoxMaterial",
    primitive: "triangles",
    vertex,
    fragment,
    attributes: AttributeLayout.box,
    uniforms: mergeUniformDefaults(COMMON_UNIFORMS, COLOR_UNIFORMS, LIGHTING_UNIFORMS, options.uniforms),
    defines,
  });
}

export function createSurfaceShader(options = {}) {
  const defines = mergeShaderDefines({
    METRIC_USE_SCALAR_TEXTURE: options.scalarTexture === true,
    METRIC_PICKING_PASS: options.picking === true,
    METRIC_DISABLE_GAMMA: options.gamma === false,
    METRIC_UNLIT: options.lighting === false,
  }, options.defines);

  const vertex = makeShaderSource({
    defines,
    chunks: [
      ShaderChunk.common,
      ShaderChunk.surfaceNormals,
      `
attribute vec3 a_position;
attribute vec3 a_normal;
attribute vec4 a_color;
attribute float a_scalar;
attribute float a_alpha;
attribute float a_selection;
attribute float a_focus;
attribute float a_pickingId;

uniform mat4 u_modelMatrix;
uniform mat4 u_viewProjectionMatrix;
uniform mat3 u_normalMatrix;

varying vec4 v_dataColor;
varying float v_scalar;
varying float v_alpha;
varying float v_selection;
varying float v_focus;
varying float v_pickingId;
varying vec3 v_normal;
varying vec3 v_worldPosition;

void main() {
  vec3 liftedPosition = metricLiftPosition(a_position, a_selection, a_focus);
  vec4 worldPosition = u_modelMatrix * vec4(liftedPosition, 1.0);
  gl_Position = u_viewProjectionMatrix * worldPosition;

  v_dataColor = a_color;
  v_scalar = a_scalar;
  v_alpha = a_alpha;
  v_selection = a_selection;
  v_focus = a_focus;
  v_pickingId = a_pickingId;
  v_normal = metricTransformNormal(u_normalMatrix, a_normal);
  v_worldPosition = worldPosition.xyz;
}
`,
    ],
  });

  const fragment = makeShaderSource({
    defines,
    chunks: [
      ShaderChunk.common,
      ShaderChunk.color,
      ShaderChunk.gamma,
      ShaderChunk.lighting,
      ShaderChunk.picking,
      `
varying vec4 v_dataColor;
varying float v_scalar;
varying float v_alpha;
varying float v_selection;
varying float v_focus;
varying float v_pickingId;
varying vec3 v_normal;
varying vec3 v_worldPosition;

void main() {
  vec4 color = metricResolveDataColor(v_dataColor, v_scalar, v_alpha, v_selection, v_focus);
  if (color.a <= u_alphaDiscard) {
    discard;
  }

#ifdef METRIC_PICKING_PASS
  gl_FragColor = metricEncodePickingId(v_pickingId);
#else
#ifdef METRIC_UNLIT
  gl_FragColor = metricApplyGamma(color);
#else
  vec3 lit = metricLightSurface(color.rgb, v_normal, v_worldPosition);
  gl_FragColor = metricApplyGamma(vec4(lit, color.a));
#endif
#endif
}
`,
    ],
  });

  return createShaderProgramDescriptor({
    kind: "surface",
    label: "SurfaceMaterial",
    primitive: "triangles",
    vertex,
    fragment,
    attributes: AttributeLayout.surface,
    uniforms: mergeUniformDefaults(COMMON_UNIFORMS, COLOR_UNIFORMS, LIGHTING_UNIFORMS, options.uniforms),
    defines,
  });
}

export function createGroundProjectionShader(options = {}) {
  const defines = mergeShaderDefines({
    METRIC_USE_SCALAR_TEXTURE: options.scalarTexture === true,
    METRIC_PICKING_PASS: options.picking === true,
    METRIC_DISABLE_GAMMA: options.gamma === false,
  }, options.defines);

  const vertex = makeShaderSource({
    defines,
    chunks: [
      ShaderChunk.common,
      `
attribute vec3 a_position;
attribute vec2 a_corner;
attribute float a_radius;
attribute vec4 a_color;
attribute float a_scalar;
attribute float a_alpha;
attribute float a_selection;
attribute float a_focus;
attribute float a_pickingId;

uniform mat4 u_modelMatrix;
uniform mat4 u_viewProjectionMatrix;
uniform vec3 u_groundRightAxis;
uniform vec3 u_groundForwardAxis;
uniform float u_groundOffset;

varying vec2 v_footprintUv;
varying vec4 v_dataColor;
varying float v_scalar;
varying float v_alpha;
varying float v_selection;
varying float v_focus;
varying float v_pickingId;

void main() {
  vec3 center = metricLiftPosition(a_position, a_selection, a_focus);
  vec3 offset = normalize(u_groundRightAxis) * a_corner.x * a_radius
    + normalize(u_groundForwardAxis) * a_corner.y * a_radius
    + normalize(u_liftAxis) * u_groundOffset;
  vec4 worldPosition = u_modelMatrix * vec4(center + offset, 1.0);
  gl_Position = u_viewProjectionMatrix * worldPosition;

  v_footprintUv = a_corner * 0.5 + 0.5;
  v_dataColor = a_color;
  v_scalar = a_scalar;
  v_alpha = a_alpha;
  v_selection = a_selection;
  v_focus = a_focus;
  v_pickingId = a_pickingId;
}
`,
    ],
  });

  const fragment = makeShaderSource({
    defines,
    chunks: [
      ShaderChunk.common,
      ShaderChunk.color,
      ShaderChunk.gamma,
      ShaderChunk.picking,
      ShaderChunk.radialFootprint,
      `
uniform float u_footprintRadius;
uniform float u_footprintFeather;

varying vec2 v_footprintUv;
varying vec4 v_dataColor;
varying float v_scalar;
varying float v_alpha;
varying float v_selection;
varying float v_focus;
varying float v_pickingId;

void main() {
  float footprintAlpha = metricRadialFootprintAlpha(v_footprintUv, u_footprintRadius, u_footprintFeather);
  if (footprintAlpha <= 0.001) {
    discard;
  }

  vec4 color = metricResolveDataColor(v_dataColor, v_scalar, v_alpha, v_selection, v_focus);
  color.a *= footprintAlpha;
  if (color.a <= u_alphaDiscard) {
    discard;
  }

#ifdef METRIC_PICKING_PASS
  gl_FragColor = metricEncodePickingId(v_pickingId);
#else
  gl_FragColor = metricApplyGamma(color);
#endif
}
`,
    ],
  });

  return createShaderProgramDescriptor({
    kind: "ground-projection",
    label: "GroundProjectionMaterial",
    primitive: "triangles",
    vertex,
    fragment,
    attributes: AttributeLayout.groundProjection,
    uniforms: mergeUniformDefaults(COMMON_UNIFORMS, COLOR_UNIFORMS, GROUND_UNIFORMS, options.uniforms),
    defines,
  });
}

export function createEdgeShader(options = {}) {
  const defines = mergeShaderDefines({
    METRIC_USE_SCALAR_TEXTURE: options.scalarTexture === true,
    METRIC_PICKING_PASS: options.picking === true,
    METRIC_DISABLE_GAMMA: options.gamma === false,
  }, options.defines);

  const vertex = makeShaderSource({
    defines,
    chunks: [
      ShaderChunk.common,
      `
attribute vec3 a_position;
attribute float a_edgeDistance;
attribute vec4 a_color;
attribute float a_scalar;
attribute float a_alpha;
attribute float a_selection;
attribute float a_focus;
attribute float a_pickingId;

uniform mat4 u_modelMatrix;
uniform mat4 u_viewProjectionMatrix;

varying float v_edgeDistance;
varying vec4 v_dataColor;
varying float v_scalar;
varying float v_alpha;
varying float v_selection;
varying float v_focus;
varying float v_pickingId;

void main() {
  vec3 liftedPosition = metricLiftPosition(a_position, a_selection, a_focus);
  vec4 worldPosition = u_modelMatrix * vec4(liftedPosition, 1.0);
  gl_Position = u_viewProjectionMatrix * worldPosition;

  v_edgeDistance = a_edgeDistance;
  v_dataColor = a_color;
  v_scalar = a_scalar;
  v_alpha = a_alpha;
  v_selection = a_selection;
  v_focus = a_focus;
  v_pickingId = a_pickingId;
}
`,
    ],
  });

  const fragment = makeShaderSource({
    defines,
    chunks: [
      ShaderChunk.common,
      ShaderChunk.color,
      ShaderChunk.gamma,
      ShaderChunk.picking,
      ShaderChunk.edgeAlpha,
      `
uniform float u_edgeThickness;
uniform float u_edgeFeather;

varying float v_edgeDistance;
varying vec4 v_dataColor;
varying float v_scalar;
varying float v_alpha;
varying float v_selection;
varying float v_focus;
varying float v_pickingId;

void main() {
  float edgeAlpha = metricEdgeAlpha(v_edgeDistance, u_edgeThickness, u_edgeFeather);
  if (edgeAlpha <= 0.001) {
    discard;
  }

  vec4 color = metricResolveDataColor(v_dataColor, v_scalar, v_alpha, v_selection, v_focus);
  color.a *= edgeAlpha;
  if (color.a <= u_alphaDiscard) {
    discard;
  }

#ifdef METRIC_PICKING_PASS
  gl_FragColor = metricEncodePickingId(v_pickingId);
#else
  gl_FragColor = metricApplyGamma(color);
#endif
}
`,
    ],
  });

  return createShaderProgramDescriptor({
    kind: "edge",
    label: "EdgeMaterial",
    primitive: "triangles",
    vertex,
    fragment,
    attributes: AttributeLayout.edge,
    uniforms: mergeUniformDefaults(COMMON_UNIFORMS, COLOR_UNIFORMS, EDGE_UNIFORMS, options.uniforms),
    defines,
  });
}

export function createHeatFieldShader(options = {}) {
  const defines = mergeShaderDefines({
    METRIC_USE_SCALAR_TEXTURE: options.scalarTexture === true,
    METRIC_USE_HEAT_TEXTURE: options.heatTexture === true,
    METRIC_PICKING_PASS: options.picking === true,
    METRIC_DISABLE_GAMMA: options.gamma === false,
  }, options.defines);

  const vertex = makeShaderSource({
    defines,
    chunks: [
      ShaderChunk.common,
      `
attribute vec3 a_position;
attribute vec2 a_uv;
attribute vec4 a_color;
attribute float a_scalar;
attribute float a_alpha;
attribute float a_selection;
attribute float a_focus;
attribute float a_pickingId;

uniform mat4 u_modelMatrix;
uniform mat4 u_viewProjectionMatrix;

varying vec2 v_uv;
varying vec4 v_dataColor;
varying float v_scalar;
varying float v_alpha;
varying float v_selection;
varying float v_focus;
varying float v_pickingId;

void main() {
  vec3 liftedPosition = metricLiftPosition(a_position, a_selection, a_focus);
  vec4 worldPosition = u_modelMatrix * vec4(liftedPosition, 1.0);
  gl_Position = u_viewProjectionMatrix * worldPosition;

  v_uv = a_uv;
  v_dataColor = a_color;
  v_scalar = a_scalar;
  v_alpha = a_alpha;
  v_selection = a_selection;
  v_focus = a_focus;
  v_pickingId = a_pickingId;
}
`,
    ],
  });

  const fragment = makeShaderSource({
    defines,
    chunks: [
      ShaderChunk.common,
      ShaderChunk.color,
      ShaderChunk.gamma,
      ShaderChunk.picking,
      `
uniform sampler2D u_heatTexture;
uniform float u_heatTextureMix;

varying vec2 v_uv;
varying vec4 v_dataColor;
varying float v_scalar;
varying float v_alpha;
varying float v_selection;
varying float v_focus;
varying float v_pickingId;

void main() {
  float scalarValue = v_scalar;
  float textureAlpha = 1.0;
#ifdef METRIC_USE_HEAT_TEXTURE
  vec4 heatSample = texture2D(u_heatTexture, v_uv);
  scalarValue = mix(v_scalar, heatSample.r, metricSaturate(u_heatTextureMix));
  textureAlpha = heatSample.a;
#endif

  vec4 color = metricResolveDataColor(v_dataColor, scalarValue, v_alpha * textureAlpha, v_selection, v_focus);
  if (color.a <= u_alphaDiscard) {
    discard;
  }

#ifdef METRIC_PICKING_PASS
  gl_FragColor = metricEncodePickingId(v_pickingId);
#else
  gl_FragColor = metricApplyGamma(color);
#endif
}
`,
    ],
  });

  return createShaderProgramDescriptor({
    kind: "heat-field",
    label: "HeatFieldMaterial",
    primitive: "triangles",
    vertex,
    fragment,
    attributes: AttributeLayout.heatField,
    uniforms: mergeUniformDefaults(COMMON_UNIFORMS, COLOR_UNIFORMS, HEAT_UNIFORMS, options.uniforms),
    defines,
  });
}

export const ShaderFactory = Object.freeze({
  point: createPointShader,
  glyph: createGlyphShader,
  box: createBoxShader,
  surface: createSurfaceShader,
  groundProjection: createGroundProjectionShader,
  edge: createEdgeShader,
  heatField: createHeatFieldShader,
});

export function createShaderCompileSmokeGL(options = {}) {
  let nextShaderId = 1;
  let nextProgramId = 1;
  const shaders = new Map();
  const programs = new Map();
  const failLabels = new Set(options.failLabels || []);

  const gl = {
    VERTEX_SHADER: 0x8B31,
    FRAGMENT_SHADER: 0x8B30,
    COMPILE_STATUS: 0x8B81,
    LINK_STATUS: 0x8B82,
    ACTIVE_ATTRIBUTES: 0x8B89,
    ACTIVE_UNIFORMS: 0x8B86,
    FLOAT: 0x1406,
    FLOAT_VEC2: 0x8B50,
    FLOAT_VEC3: 0x8B51,
    FLOAT_VEC4: 0x8B52,
    INT: 0x1404,
    BOOL: 0x8B56,
    SAMPLER_2D: 0x8B5E,
    SAMPLER_CUBE: 0x8B60,
    INT_VEC2: 0x8B53,
    INT_VEC3: 0x8B54,
    INT_VEC4: 0x8B55,
    BOOL_VEC2: 0x8B57,
    BOOL_VEC3: 0x8B58,
    BOOL_VEC4: 0x8B59,
    FLOAT_MAT2: 0x8B5A,
    FLOAT_MAT3: 0x8B5B,
    FLOAT_MAT4: 0x8B5C,
    TEXTURE0: 0x84C0,
    TEXTURE_2D: 0x0DE1,
  };

  gl.createShader = (type) => {
    const shader = { id: nextShaderId, type, source: "", compiled: false };
    nextShaderId += 1;
    shaders.set(shader.id, shader);
    return shader;
  };
  gl.shaderSource = (shader, source) => {
    shader.source = source;
  };
  gl.compileShader = (shader) => {
    shader.compiled = !failLabels.has(shader.type === gl.VERTEX_SHADER ? "vertex" : "fragment");
  };
  gl.getShaderParameter = (shader, parameter) => (
    parameter === gl.COMPILE_STATUS ? shader.compiled : null
  );
  gl.getShaderInfoLog = (shader) => (shader.compiled ? "" : "Fake shader compile failure");
  gl.deleteShader = (shader) => {
    if (shader) shaders.delete(shader.id);
  };

  gl.createProgram = () => {
    const program = { id: nextProgramId, shaders: [], linked: false };
    nextProgramId += 1;
    programs.set(program.id, program);
    return program;
  };
  gl.attachShader = (program, shader) => {
    program.shaders.push(shader);
  };
  gl.bindAttribLocation = () => {};
  gl.linkProgram = (program) => {
    program.linked = !failLabels.has("program") && program.shaders.every((shader) => shader.compiled);
  };
  gl.getProgramParameter = (program, parameter) => {
    if (parameter === gl.LINK_STATUS) return program.linked;
    if (parameter === gl.ACTIVE_ATTRIBUTES || parameter === gl.ACTIVE_UNIFORMS) return 0;
    return null;
  };
  gl.getProgramInfoLog = (program) => (program.linked ? "" : "Fake program link failure");
  gl.deleteProgram = (program) => {
    if (program) programs.delete(program.id);
  };
  gl.useProgram = () => {};
  gl.getActiveAttrib = () => null;
  gl.getAttribLocation = () => -1;
  gl.getActiveUniform = () => null;
  gl.getUniformLocation = () => null;
  gl.uniform1f = () => {};
  gl.uniform1i = () => {};
  gl.uniform1fv = () => {};
  gl.uniform2fv = () => {};
  gl.uniform3fv = () => {};
  gl.uniform4fv = () => {};
  gl.uniform2iv = () => {};
  gl.uniform3iv = () => {};
  gl.uniform4iv = () => {};
  gl.uniformMatrix2fv = () => {};
  gl.uniformMatrix3fv = () => {};
  gl.uniformMatrix4fv = () => {};
  gl.activeTexture = () => {};
  gl.bindTexture = () => {};
  gl.__metricSmokeState = { shaders, programs };

  return gl;
}

function normalizeStringList(value) {
  if (!value) return [];
  return Array.isArray(value) ? value.filter(Boolean) : [value];
}

function cloneUniformValue(value) {
  if (Array.isArray(value)) return value.slice();
  if (ArrayBuffer.isView(value)) return new value.constructor(value);
  if (value && typeof value === "object") return { ...value };
  return value;
}
