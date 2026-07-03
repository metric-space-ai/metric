/*
 * Material descriptors for the native METRIC Visual WebGL pipeline.
 *
 * Materials are declarative and renderer-agnostic. They expose shader sources,
 * attributes, uniform defaults, and optional compile settings. They do not own
 * buffers, textures, cameras, render passes, or data transforms.
 */

import { ShaderProgram } from "../engine/shader.js";
import {
  createBoxShader,
  createEdgeShader,
  createGlyphShader,
  createGroundProjectionShader,
  createHeatFieldShader,
  createPointShader,
  createSurfaceShader,
  mergeShaderDefines,
  mergeUniformDefaults,
} from "../shaders/index.js";

export const MaterialKind = Object.freeze({
  point: "point",
  glyph: "glyph",
  box: "box",
  surface: "surface",
  groundProjection: "ground-projection",
  edge: "edge",
  heatField: "heat-field",
});

export const AlphaMode = Object.freeze({
  opaque: "opaque",
  blend: "blend",
  additive: "additive",
  premultiplied: "premultiplied",
});

export const BuiltInMaterialShaders = Object.freeze({
  [MaterialKind.point]: createPointShader,
  [MaterialKind.glyph]: createGlyphShader,
  [MaterialKind.box]: createBoxShader,
  [MaterialKind.surface]: createSurfaceShader,
  [MaterialKind.groundProjection]: createGroundProjectionShader,
  [MaterialKind.edge]: createEdgeShader,
  [MaterialKind.heatField]: createHeatFieldShader,
});

const DEFAULT_MATERIAL_OPTIONS = Object.freeze({
  dataColor: true,
  alpha: 1,
  gamma: 2.2,
  alphaMode: AlphaMode.opaque,
  depthTest: true,
  depthWrite: true,
  cullFace: false,
  selectionLift: 0,
  focusLift: 0,
});

const MINIATURE_BASE_COLORS = Object.freeze({
  point: [0.38, 0.54, 0.72, 1],
  glyph: [0.44, 0.5, 0.62, 1],
  box: [0.5, 0.62, 0.72, 1],
  surface: [0.48, 0.6, 0.66, 1],
  groundProjection: [0.32, 0.48, 0.6, 0.55],
  edge: [0.18, 0.25, 0.32, 0.78],
  heatField: [0.52, 0.62, 0.7, 0.9],
});

export class MaterialDescriptor {
  constructor(options = {}) {
    const kind = options.kind || MaterialKind.point;
    const shaderFactory = options.shaderFactory || BuiltInMaterialShaders[kind];
    if (typeof shaderFactory !== "function") {
      throw new Error(`Unknown material kind: ${kind}`);
    }

    const alpha = numberOr(options.alpha, DEFAULT_MATERIAL_OPTIONS.alpha);
    const alphaMode = options.alphaMode || (alpha < 1 ? AlphaMode.blend : DEFAULT_MATERIAL_OPTIONS.alphaMode);

    this.kind = kind;
    this.name = options.name || materialNameFromKind(kind);
    this.label = options.label || this.name;
    this.shaderFactory = shaderFactory;
    this.shaderOptions = { ...(options.shaderOptions || {}) };
    this.dataColor = normalizeDataColorOptions(options.dataColor, kind);
    this.scalarColorMap = normalizeScalarColorMapOptions(options.scalarColorMap || options.scalarMap);
    this.lighting = normalizeLightingOptions(options);
    this.alpha = alpha;
    this.alphaMode = alphaMode;
    this.gamma = options.gamma === false ? false : numberOr(options.gamma, DEFAULT_MATERIAL_OPTIONS.gamma);
    this.selection = normalizeInteractionOptions(options.selection, {
      color: options.selectionColor,
      lift: options.selectionLift,
    });
    this.focus = normalizeInteractionOptions(options.focus, {
      color: options.focusColor,
      lift: options.focusLift,
    });
    this.depthTest = options.depthTest ?? DEFAULT_MATERIAL_OPTIONS.depthTest;
    this.depthWrite = options.depthWrite ?? (alphaMode === AlphaMode.opaque);
    this.cullFace = options.cullFace ?? DEFAULT_MATERIAL_OPTIONS.cullFace;
    this.blend = normalizeBlendOptions(options.blend, alphaMode);
    this.defines = { ...(options.defines || {}) };
    this.uniforms = mergeUniformDefaults(options.uniforms || {});
    this.metadata = { ...(options.metadata || {}) };
  }

  createShaderDescriptor(options = {}) {
    const shaderOptions = this.createShaderOptions(options);
    const shader = this.shaderFactory(shaderOptions);
    return {
      ...shader,
      label: options.label || shader.label || this.label,
      uniforms: mergeUniformDefaults(shader.uniforms, this.createUniformDefaults(), options.uniforms),
      defines: mergeShaderDefines(shader.defines, this.defines, options.defines),
      metadata: {
        ...(shader.metadata || {}),
        material: this.toJSON(),
        ...(options.metadata || {}),
      },
    };
  }

  createShaderOptions(options = {}) {
    return {
      ...this.shaderOptions,
      scalarTexture: this.scalarColorMap.texture === true || this.scalarColorMap.sampler === true,
      picking: options.picking === true,
      gamma: this.gamma !== false,
      lighting: this.lighting.enabled,
      defines: mergeShaderDefines(this.defines, options.defines),
      uniforms: mergeUniformDefaults(this.createUniformDefaults(), options.uniforms),
      ...options.shaderOptions,
    };
  }

  createUniformDefaults() {
    return mergeUniformDefaults(
      this.dataColor.uniforms,
      this.scalarColorMap.uniforms,
      this.lighting.uniforms,
      {
        u_alpha: this.alpha,
        u_gamma: this.gamma === false ? 1 : this.gamma,
        u_selectionLift: this.selection.lift,
        u_focusLift: this.focus.lift,
        u_selectionColor: this.selection.color,
        u_focusColor: this.focus.color,
      },
      this.uniforms,
    );
  }

  createProgram(gl, options = {}) {
    return new MaterialProgram(gl, this, options);
  }

  with(overrides = {}) {
    return new MaterialDescriptor({
      ...this.toOptions(),
      ...overrides,
      shaderOptions: {
        ...this.shaderOptions,
        ...(overrides.shaderOptions || {}),
      },
      uniforms: mergeUniformDefaults(this.uniforms, overrides.uniforms),
      defines: mergeShaderDefines(this.defines, overrides.defines),
      metadata: {
        ...this.metadata,
        ...(overrides.metadata || {}),
      },
    });
  }

  toOptions() {
    return {
      kind: this.kind,
      name: this.name,
      label: this.label,
      shaderFactory: this.shaderFactory,
      shaderOptions: { ...this.shaderOptions },
      dataColor: { ...this.dataColor.options },
      scalarColorMap: { ...this.scalarColorMap.options },
      ambient: { ...this.lighting.ambient },
      diffuse: { ...this.lighting.diffuse },
      specular: { ...this.lighting.specular },
      lighting: this.lighting.enabled,
      alpha: this.alpha,
      alphaMode: this.alphaMode,
      gamma: this.gamma,
      selection: { color: this.selection.color.slice(), lift: this.selection.lift },
      focus: { color: this.focus.color.slice(), lift: this.focus.lift },
      depthTest: this.depthTest,
      depthWrite: this.depthWrite,
      cullFace: this.cullFace,
      blend: { ...this.blend },
      defines: { ...this.defines },
      uniforms: mergeUniformDefaults(this.uniforms),
      metadata: { ...this.metadata },
    };
  }

  toJSON() {
    return {
      kind: this.kind,
      name: this.name,
      label: this.label,
      dataColor: { ...this.dataColor.options },
      scalarColorMap: { ...this.scalarColorMap.options },
      lighting: {
        enabled: this.lighting.enabled,
        ambient: { ...this.lighting.ambient },
        diffuse: { ...this.lighting.diffuse },
        specular: { ...this.lighting.specular },
      },
      alpha: this.alpha,
      alphaMode: this.alphaMode,
      gamma: this.gamma,
      selection: { color: this.selection.color.slice(), lift: this.selection.lift },
      focus: { color: this.focus.color.slice(), lift: this.focus.lift },
      depthTest: this.depthTest,
      depthWrite: this.depthWrite,
      cullFace: this.cullFace,
      blend: { ...this.blend },
      defines: { ...this.defines },
      uniforms: mergeUniformDefaults(this.uniforms),
      metadata: { ...this.metadata },
    };
  }
}

export class MaterialProgram {
  constructor(gl, material, options = {}) {
    if (!gl) {
      throw new Error("MaterialProgram requires a WebGL-like context.");
    }

    this.gl = gl;
    this.material = material instanceof MaterialDescriptor ? material : new MaterialDescriptor(material);
    this.shader = this.material.createShaderDescriptor(options);
    this.program = new ShaderProgram(gl, {
      label: this.shader.label,
      vertex: this.shader.vertex,
      fragment: this.shader.fragment,
      attribLocations: options.attribLocations || this.shader.attribLocations,
      defines: options.compileDefines,
    });
    this.uniforms = mergeUniformDefaults(this.shader.uniforms);
    this.disposed = false;

    if (options.applyUniforms !== false) {
      this.setUniforms(this.uniforms);
    }
  }

  use() {
    this.program.use();
    return this;
  }

  setUniform(name, value, explicitType) {
    this.program.setUniform(name, value, explicitType);
    this.uniforms[name] = cloneUniformValue(value);
    return this;
  }

  setUniforms(uniforms = {}) {
    for (const [name, value] of Object.entries(uniforms)) {
      this.setUniform(name, value);
    }
    return this;
  }

  bindTexture(name, texture, unit = 0, target = this.gl.TEXTURE_2D) {
    this.program.bindTexture(name, texture, unit, target);
    this.uniforms[name] = unit;
    return this;
  }

  getAttribLocation(name) {
    return this.program.getAttribLocation(name);
  }

  getUniformLocation(name) {
    return this.program.getUniformLocation(name);
  }

  dispose() {
    if (this.disposed) return;
    this.program.dispose();
    this.disposed = true;
  }
}

export class PointMaterial extends MaterialDescriptor {
  constructor(options = {}) {
    super(createBuiltInOptions(MaterialKind.point, {
      alphaMode: AlphaMode.blend,
      shaderOptions: {
        shape: options.shape || "circle",
        sizeAttenuation: options.sizeAttenuation === true,
      },
      uniforms: {
        u_baseColor: MINIATURE_BASE_COLORS.point,
        u_pointFeather: numberOr(options.feather, 0.18),
      },
    }, options));
  }
}

export class GlyphMaterial extends MaterialDescriptor {
  constructor(options = {}) {
    super(createBuiltInOptions(MaterialKind.glyph, {
      alphaMode: AlphaMode.blend,
      shaderOptions: {
        billboard: options.billboard !== false,
      },
      uniforms: {
        u_baseColor: MINIATURE_BASE_COLORS.glyph,
        u_glyphScale: numberOr(options.glyphScale, 1),
      },
    }, options));
  }
}

export class BoxMaterial extends MaterialDescriptor {
  constructor(options = {}) {
    super(createBuiltInOptions(MaterialKind.box, {
      alphaMode: AlphaMode.opaque,
      lighting: options.lighting ?? true,
      uniforms: {
        u_baseColor: MINIATURE_BASE_COLORS.box,
        u_ambientStrength: 0.62,
        u_diffuseStrength: 0.38,
        u_specularStrength: 0.1,
      },
    }, options));
  }
}

export class SurfaceMaterial extends MaterialDescriptor {
  constructor(options = {}) {
    super(createBuiltInOptions(MaterialKind.surface, {
      alphaMode: numberOr(options.alpha, 1) < 1 ? AlphaMode.blend : AlphaMode.opaque,
      lighting: options.lighting ?? true,
      uniforms: {
        u_baseColor: MINIATURE_BASE_COLORS.surface,
        u_ambientStrength: 0.54,
        u_diffuseStrength: 0.44,
        u_specularStrength: 0.08,
      },
    }, options));
  }
}

export class GroundProjectionMaterial extends MaterialDescriptor {
  constructor(options = {}) {
    super(createBuiltInOptions(MaterialKind.groundProjection, {
      alpha: numberOr(options.alpha, 0.55),
      alphaMode: AlphaMode.blend,
      depthWrite: false,
      uniforms: {
        u_baseColor: MINIATURE_BASE_COLORS.groundProjection,
        u_alpha: numberOr(options.alpha, 0.55),
        u_footprintFeather: numberOr(options.feather, 0.35),
      },
    }, options));
  }
}

export class EdgeMaterial extends MaterialDescriptor {
  constructor(options = {}) {
    super(createBuiltInOptions(MaterialKind.edge, {
      alpha: numberOr(options.alpha, 0.78),
      alphaMode: AlphaMode.blend,
      depthWrite: false,
      uniforms: {
        u_baseColor: MINIATURE_BASE_COLORS.edge,
        u_alpha: numberOr(options.alpha, 0.78),
        u_edgeThickness: numberOr(options.thickness, 1),
        u_edgeFeather: numberOr(options.feather, 0.35),
      },
    }, options));
  }
}

export class HeatFieldMaterial extends MaterialDescriptor {
  constructor(options = {}) {
    super(createBuiltInOptions(MaterialKind.heatField, {
      alpha: numberOr(options.alpha, 0.9),
      alphaMode: AlphaMode.blend,
      depthWrite: false,
      shaderOptions: {
        heatTexture: options.heatTexture === true,
      },
      scalarColorMap: {
        mix: options.scalarMix ?? 1,
        domain: options.domain || [0, 1],
        lowColor: options.lowColor || [0.92, 0.97, 1, 0.25],
        highColor: options.highColor || [0.98, 0.37, 0.18, 0.95],
        texture: options.scalarTexture === true,
      },
      uniforms: {
        u_baseColor: MINIATURE_BASE_COLORS.heatField,
        u_alpha: numberOr(options.alpha, 0.9),
        u_heatTextureMix: numberOr(options.heatTextureMix, 1),
      },
    }, options));
  }
}

export const BuiltInMaterials = Object.freeze({
  PointMaterial,
  GlyphMaterial,
  BoxMaterial,
  SurfaceMaterial,
  GroundProjectionMaterial,
  EdgeMaterial,
  HeatFieldMaterial,
});

export function createMaterialDescriptor(options = {}) {
  return options instanceof MaterialDescriptor ? options : new MaterialDescriptor(options);
}

export function createMaterialProgram(gl, material, options = {}) {
  return createMaterialDescriptor(material).createProgram(gl, options);
}

export function createPointMaterial(options = {}) {
  return new PointMaterial(options);
}

export function createGlyphMaterial(options = {}) {
  return new GlyphMaterial(options);
}

export function createBoxMaterial(options = {}) {
  return new BoxMaterial(options);
}

export function createSurfaceMaterial(options = {}) {
  return new SurfaceMaterial(options);
}

export function createGroundProjectionMaterial(options = {}) {
  return new GroundProjectionMaterial(options);
}

export function createEdgeMaterial(options = {}) {
  return new EdgeMaterial(options);
}

export function createHeatFieldMaterial(options = {}) {
  return new HeatFieldMaterial(options);
}

function createBuiltInOptions(kind, defaults, options) {
  return {
    ...DEFAULT_MATERIAL_OPTIONS,
    ...defaults,
    ...options,
    kind,
    name: options.name || defaults.name || materialNameFromKind(kind),
    shaderOptions: {
      ...(defaults.shaderOptions || {}),
      ...(options.shaderOptions || {}),
    },
    scalarColorMap: {
      ...(defaults.scalarColorMap || {}),
      ...(options.scalarColorMap || options.scalarMap || {}),
    },
    uniforms: mergeUniformDefaults(defaults.uniforms, options.uniforms),
    defines: mergeShaderDefines(defaults.defines, options.defines),
    metadata: {
      ...(defaults.metadata || {}),
      ...(options.metadata || {}),
    },
  };
}

function normalizeDataColorOptions(input, kind) {
  const base = MINIATURE_BASE_COLORS[kind] || MINIATURE_BASE_COLORS.point;
  if (input === false) {
    return {
      options: { enabled: false, color: base.slice(), mix: 0 },
      uniforms: {
        u_baseColor: base,
        u_dataColorMix: 0,
      },
    };
  }
  if (Array.isArray(input) || typeof input === "string") {
    const color = parseColor(input, base);
    return {
      options: { enabled: true, color, mix: 0 },
      uniforms: {
        u_baseColor: color,
        u_dataColorMix: 0,
      },
    };
  }

  const options = input && typeof input === "object" ? input : {};
  const color = parseColor(options.color || options.baseColor, base);
  const mix = numberOr(options.mix, options.enabled === false ? 0 : 1);
  return {
    options: {
      enabled: options.enabled !== false,
      color,
      mix,
    },
    uniforms: {
      u_baseColor: color,
      u_dataColorMix: mix,
    },
  };
}

function normalizeScalarColorMapOptions(input = {}) {
  const options = input && typeof input === "object" ? input : {};
  const lowColor = parseColor(options.lowColor || options.minColor, [0.92, 0.96, 1, 1]);
  const highColor = parseColor(options.highColor || options.maxColor, [0.06, 0.28, 0.58, 1]);
  const domain = normalizeDomain(options.domain);
  const mix = numberOr(options.mix, options.enabled ? 1 : 0);
  return {
    options: {
      enabled: options.enabled === true || mix > 0 || options.texture === true || options.sampler === true,
      texture: options.texture === true,
      sampler: options.sampler === true,
      domain,
      lowColor,
      highColor,
      mix,
    },
    texture: options.texture === true,
    sampler: options.sampler === true,
    uniforms: {
      u_scalarColorMix: mix,
      u_scalarDomain: domain,
      u_scalarLowColor: lowColor,
      u_scalarHighColor: highColor,
      u_scalarColorMap: numberOr(options.textureUnit, 0),
    },
  };
}

function normalizeLightingOptions(options) {
  const enabled = options.lighting !== false;
  const ambient = normalizeLightTerm(options.ambient, {
    color: [1, 1, 1],
    strength: 0.56,
  });
  const diffuse = normalizeLightTerm(options.diffuse, {
    color: options.lightColor || [1, 0.98, 0.92],
    strength: 0.5,
  });
  const specular = normalizeLightTerm(options.specular, {
    color: [1, 1, 1],
    strength: 0.12,
    shininess: 28,
  });

  return {
    enabled,
    ambient,
    diffuse,
    specular,
    uniforms: {
      u_ambientColor: ambient.color,
      u_ambientStrength: ambient.strength,
      u_lightColor: diffuse.color,
      u_diffuseStrength: diffuse.strength,
      u_specularColor: specular.color,
      u_specularStrength: specular.strength,
      u_shininess: numberOr(specular.shininess, 28),
      ...(options.lightDirection ? { u_lightDirection: options.lightDirection } : {}),
      ...(options.cameraPosition ? { u_cameraPosition: options.cameraPosition } : {}),
    },
  };
}

function normalizeLightTerm(input, fallback) {
  if (input === false || input === 0) {
    return { ...fallback, strength: 0 };
  }
  if (typeof input === "number") {
    return { ...fallback, strength: input };
  }
  if (Array.isArray(input) || typeof input === "string") {
    return { ...fallback, color: parseRgb(input, fallback.color) };
  }
  const options = input && typeof input === "object" ? input : {};
  return {
    color: parseRgb(options.color, fallback.color),
    strength: numberOr(options.strength ?? options.intensity, fallback.strength),
    shininess: numberOr(options.shininess, fallback.shininess),
  };
}

function normalizeInteractionOptions(input, fallback = {}) {
  const options = input && typeof input === "object" ? input : {};
  return {
    color: parseColor(options.color || fallback.color, fallback.color || [1, 1, 1, 0]),
    lift: numberOr(options.lift ?? fallback.lift, 0),
  };
}

function normalizeBlendOptions(input, alphaMode) {
  if (input && typeof input === "object") {
    return { ...input };
  }
  return {
    enabled: alphaMode !== AlphaMode.opaque,
    mode: alphaMode,
  };
}

function normalizeDomain(input) {
  if (!Array.isArray(input) || input.length < 2) return [0, 1];
  const min = Number(input[0]);
  const max = Number(input[1]);
  if (Number.isFinite(min) && Number.isFinite(max) && min !== max) return [min, max];
  if (Number.isFinite(min)) return [min - 1, min + 1];
  return [0, 1];
}

function materialNameFromKind(kind) {
  return `${String(kind).replace(/(^|-)([a-z])/g, (_match, _dash, letter) => letter.toUpperCase())}Material`;
}

function numberOr(value, fallback) {
  const number = Number(value);
  return Number.isFinite(number) ? number : fallback;
}

function parseColor(input, fallback = [0, 0, 0, 1]) {
  if (Array.isArray(input)) {
    return [
      normalizeChannel(input[0], fallback[0]),
      normalizeChannel(input[1], fallback[1]),
      normalizeChannel(input[2], fallback[2]),
      normalizeChannel(input[3], fallback[3] ?? 1),
    ];
  }
  if (input && typeof input === "object") {
    return [
      normalizeChannel(input.r, fallback[0]),
      normalizeChannel(input.g, fallback[1]),
      normalizeChannel(input.b, fallback[2]),
      normalizeChannel(input.a, fallback[3] ?? 1),
    ];
  }
  if (typeof input === "string") {
    return parseHexColor(input, fallback);
  }
  return fallback.slice();
}

function parseRgb(input, fallback = [1, 1, 1]) {
  return parseColor(input, [...fallback, 1]).slice(0, 3);
}

function parseHexColor(input, fallback) {
  const value = input.trim().replace(/^#/, "");
  if (![3, 4, 6, 8].includes(value.length) || /[^0-9a-f]/i.test(value)) {
    return fallback.slice();
  }
  const expanded = value.length <= 4
    ? value.split("").map((character) => character + character).join("")
    : value;
  const channels = [
    parseInt(expanded.slice(0, 2), 16) / 255,
    parseInt(expanded.slice(2, 4), 16) / 255,
    parseInt(expanded.slice(4, 6), 16) / 255,
    expanded.length >= 8 ? parseInt(expanded.slice(6, 8), 16) / 255 : fallback[3] ?? 1,
  ];
  return channels;
}

function normalizeChannel(value, fallback = 0) {
  const number = Number(value);
  if (!Number.isFinite(number)) return fallback;
  if (number > 1) return Math.max(0, Math.min(255, number)) / 255;
  return Math.max(0, Math.min(1, number));
}

function cloneUniformValue(value) {
  if (Array.isArray(value)) return value.slice();
  if (ArrayBuffer.isView(value)) return new value.constructor(value);
  if (value && typeof value === "object") return { ...value };
  return value;
}
