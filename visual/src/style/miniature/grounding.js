import { normalizePathEvidence } from "../../curves/path-data.js";

/**
 * Creates the reusable miniature ground relation contract.
 *
 * Grounding is visual composition only. It defines how exported positions,
 * projections and scalar fields are staged on a photographic floor. It does
 * not compute metric values or infer new records.
 *
 * @param {object} [options]
 * @returns {object}
 */
export function createMiniatureGroundingPreset(options = {}) {
  const projection = options.projection || {};
  const ground = options.ground || {};
  const contact = options.contact || {};
  return deepFreeze({
    enabled: options.enabled !== false,
    kind: "miniature-grounding",
    plane: options.plane || "xz",
    groundY: finiteNumber(options.groundY, ground.groundY, -0.56),
    ground: {
      gridScale: positiveNumber(ground.gridScale, options.gridScale, 6),
      gridWidth: positiveNumber(ground.gridWidth, 0.014),
      axisWidth: positiveNumber(ground.axisWidth, 0.01),
      alpha: positiveNumber(ground.alpha, options.groundAlpha, 0.42),
      fade: positiveNumber(ground.fade, 0.92),
      gridAlpha: finiteNumber(ground.gridAlpha, 0.28),
      axisAlpha: finiteNumber(ground.axisAlpha, 0.24),
      stageTilt: finiteNumber(ground.stageTilt, -0.12),
      stageScale: point2(ground.stageScale, [0.92, 0.78]),
      sheen: finiteNumber(ground.sheen, 0.06),
      matte: finiteNumber(ground.matte, 0.78),
      contactShade: finiteNumber(ground.contactShade, 0.16),
      horizonFade: finiteNumber(ground.horizonFade, 0.12),
      horizonColor: color3(ground.horizonColor, [0.94, 0.95, 0.91]),
      sheenColor: color3(ground.sheenColor, [1, 0.98, 0.9]),
      ...(ground || {}),
    },
    projection: createMiniatureProjectionStyle(projection),
    contact: {
      enabled: contact.enabled !== false,
      alpha: finiteNumber(contact.alpha, 0.46),
      radiusScale: positiveNumber(contact.radiusScale, 1.28),
      shadowDensity: finiteNumber(contact.shadowDensity, 1.44),
      coreDensity: finiteNumber(contact.coreDensity, 0.5),
      edgeTint: finiteNumber(contact.edgeTint, 0.48),
      shadowDirection: point2(contact.shadowDirection, projection.shadowDirection, [-0.44, 0.9]),
      shadowTail: finiteNumber(contact.shadowTail, projection.shadowTail, 0.62),
      shadowTailStrength: finiteNumber(contact.shadowTailStrength, projection.shadowTailStrength, 0.34),
      contactHardness: finiteNumber(contact.contactHardness, projection.contactHardness, 0.38),
      surfaceLift: finiteNumber(contact.surfaceLift, projection.surfaceLift, 0.018),
      ...(contact || {}),
    },
    metadata: {
      nativeMetricStyle: true,
      role: "miniature-ground-relation",
      ...(options.metadata || {}),
    },
  });
}

/**
 * Creates renderer material hints for semantic ground projections.
 *
 * @param {object} [options]
 * @returns {object}
 */
export function createMiniatureProjectionStyle(options = {}) {
  return deepFreeze({
    kind: "miniature-semantic-shadow",
    lighting: "projection",
    alphaMode: "blend",
    alpha: finiteNumber(options.alpha, 0.52),
    softness: finiteNumber(options.softness, 0.76),
    pointPixelScale: positiveNumber(options.pointPixelScale, 16),
    minPointSize: positiveNumber(options.minPointSize, 2),
    maxPointSize: positiveNumber(options.maxPointSize, 180),
    footprintStretch: positiveNumber(options.footprintStretch, 1.86),
    footprintSkew: finiteNumber(options.footprintSkew, -0.34),
    footprintCore: finiteNumber(options.footprintCore, 0.18),
    footprintFalloff: positiveNumber(options.footprintFalloff, 1.12),
    colorMix: finiteNumber(options.colorMix, 0.66),
    shadowDensity: finiteNumber(options.shadowDensity, 1.22),
    coreDensity: finiteNumber(options.coreDensity, 0.4),
    edgeTint: finiteNumber(options.edgeTint, 0.58),
    neutralShadow: color3(options.neutralShadow, [0.38, 0.42, 0.44]),
    shadowDirection: point2(options.shadowDirection, [-0.44, 0.9]),
    shadowTail: finiteNumber(options.shadowTail, 0.58),
    shadowTailStrength: finiteNumber(options.shadowTailStrength, 0.28),
    contactHardness: finiteNumber(options.contactHardness, 0.32),
    surfaceLift: finiteNumber(options.surfaceLift, 0.02),
    ...(options || {}),
  });
}

/**
 * Creates a semantic contact-shadow descriptor for a spatial data layer.
 *
 * The descriptor is a visual companion to the source layer. It reuses exported
 * renderer channels, projects only the y coordinate onto the miniature ground,
 * and does not compute metric values or alter source data.
 *
 * @param {object} descriptor
 * @param {object} [options]
 * @returns {object|null}
 */
export function createMiniatureContactShadowDescriptor(descriptor, options = {}) {
  if (!descriptor || typeof descriptor !== "object") return null;
  if (descriptor.visible === false) return null;
  const primitive = descriptor.primitive || descriptor.kind;
  const includeSamples = options.includeSamples === true;
  const supported = primitive === "InstancedBoxLayer"
    || primitive === "InstancedGlyphLayer"
    || primitive === "CurveTubeMeshLayer"
    || primitive === "CurveRibbonLayer"
    || primitive === "CurvePolylineLayer"
    || (includeSamples && primitive === "InstancedPointLayer");
  if (!supported) return null;

  const pathContact = createPathContactChannels(descriptor, options);
  const positionChannel = getChannel(descriptor.channels, ["position", "sourcePosition", "targetPosition"]);
  const position = pathContact?.position || projectPositionChannel(positionChannel, options.groundY);
  if (!position) return null;
  const count = channelCount(position, 3);
  if (!count) return null;

  const contact = options.contact || {};
  const projection = options.projection || {};
  const order = Number.isFinite(Number(descriptor.order)) ? Number(descriptor.order) - 1 : -30;
  const isPathContact = Boolean(pathContact);
  const size = pathContact?.size || contactSizeChannel(descriptor.channels || {}, count, contact);
  const color = pathContact?.color || cloneChannel(getChannel(descriptor.channels, ["color", "rgba"]));

  return {
    schema: "metric.visual.layer_descriptor.v1",
    id: `${descriptor.id || descriptor.kind || "layer"}:miniature-contact`,
    kind: "miniature-contact-shadow",
    primitive: "GroundProjectionLayer",
    visible: true,
    order,
    source: {
      ...(descriptor.source || {}),
      contactSourceLayerId: descriptor.id,
      contactSourcePrimitive: primitive,
    },
    channels: {
      position,
      size,
      ...(color ? { color } : {}),
    },
    geometry: {
      mode: isPathContact ? "path-contact-shadow" : "contact-shadow",
      plane: options.plane || "xz",
      groundY: finiteNumber(options.groundY, -0.56),
      projectionSource: isPathContact ? "source-layer-path-evidence" : "source-layer-position",
    },
    material: createMiniatureProjectionStyle({
      alpha: finiteNumber(isPathContact ? contact.pathAlpha : contact.alpha, isPathContact ? 0.2 : projection.alpha, isPathContact ? 0.2 : 0.42),
      softness: finiteNumber(isPathContact ? contact.pathSoftness : contact.softness, projection.softness, isPathContact ? 0.9 : 0.78),
      pointPixelScale: positiveNumber(isPathContact ? contact.pathPointPixelScale : contact.pointPixelScale, isPathContact ? undefined : projection.pointPixelScale, isPathContact ? 5.6 : 18),
      footprintStretch: positiveNumber(isPathContact ? contact.pathFootprintStretch : contact.footprintStretch, isPathContact ? undefined : projection.footprintStretch, isPathContact ? 1.46 : 1.95),
      footprintSkew: finiteNumber(isPathContact ? contact.pathFootprintSkew : contact.footprintSkew, isPathContact ? undefined : projection.footprintSkew, isPathContact ? -0.24 : -0.32),
      footprintCore: finiteNumber(isPathContact ? contact.pathFootprintCore : contact.footprintCore, isPathContact ? undefined : projection.footprintCore, isPathContact ? 0.06 : 0.14),
      footprintFalloff: positiveNumber(isPathContact ? contact.pathFootprintFalloff : contact.footprintFalloff, isPathContact ? undefined : projection.footprintFalloff, isPathContact ? 1.08 : 1.12),
      colorMix: finiteNumber(isPathContact ? contact.pathColorMix : contact.colorMix, isPathContact ? undefined : projection.colorMix, isPathContact ? 0.28 : 0.62),
      shadowDensity: finiteNumber(isPathContact ? contact.pathShadowDensity : contact.shadowDensity, isPathContact ? undefined : projection.shadowDensity, isPathContact ? 0.46 : 1.44),
      coreDensity: finiteNumber(isPathContact ? contact.pathCoreDensity : contact.coreDensity, isPathContact ? undefined : projection.coreDensity, isPathContact ? 0.16 : 0.5),
      edgeTint: finiteNumber(isPathContact ? contact.pathEdgeTint : contact.edgeTint, isPathContact ? undefined : projection.edgeTint, isPathContact ? 0.32 : 0.48),
      neutralShadow: color3(contact.neutralShadow, projection.neutralShadow, [0.38, 0.42, 0.44]),
      shadowDirection: point2(contact.shadowDirection, projection.shadowDirection, [-0.44, 0.9]),
      shadowTail: finiteNumber(isPathContact ? contact.pathShadowTail : contact.shadowTail, isPathContact ? undefined : projection.shadowTail, isPathContact ? 0.44 : 0.62),
      shadowTailStrength: finiteNumber(isPathContact ? contact.pathShadowTailStrength : contact.shadowTailStrength, isPathContact ? undefined : projection.shadowTailStrength, isPathContact ? 0.08 : 0.34),
      contactHardness: finiteNumber(isPathContact ? contact.pathContactHardness : contact.contactHardness, isPathContact ? undefined : projection.contactHardness, isPathContact ? 0.18 : 0.38),
      surfaceLift: finiteNumber(contact.surfaceLift, projection.surfaceLift, 0.018),
      ...(contact.material || {}),
    }),
    animation: { mode: "none" },
    picking: { mode: "none" },
    metadata: {
      nativeMetricStyle: true,
      miniature: true,
      miniatureRole: "contact-shadow",
      sourceLayerId: descriptor.id,
      contactSourcePrimitive: primitive,
      pathContactShadow: isPathContact,
      ...(contact.metadata || {}),
    },
  };
}

/**
 * Creates contact-shadow descriptors for all supported source descriptors.
 *
 * @param {object[]|object} descriptors
 * @param {object} [options]
 * @returns {object[]}
 */
export function createMiniatureContactShadowDescriptors(descriptors, options = {}) {
  const source = Array.isArray(descriptors) ? descriptors : [descriptors];
  return source
    .map((descriptor) => createMiniatureContactShadowDescriptor(descriptor, options))
    .filter(Boolean);
}

/**
 * Converts a stage grounding contract to `createMiniatureGroundDescriptor()`
 * options.
 *
 * @param {object} [grounding]
 * @param {object} [overrides]
 * @returns {object}
 */
export function createMiniatureGroundDescriptorOptions(grounding = {}, overrides = {}) {
  const source = grounding?.kind === "miniature-grounding"
    ? grounding
    : createMiniatureGroundingPreset(grounding);
  return {
    groundY: finiteNumber(overrides.groundY, source.groundY),
    y: finiteNumber(overrides.y, overrides.groundY, source.groundY),
    gridScale: positiveNumber(overrides.gridScale, source.ground?.gridScale, 6),
    material: {
      ...(source.ground || {}),
      ...(overrides.material || {}),
    },
    ...copyWithout(overrides, ["groundY", "y", "gridScale", "material"]),
  };
}

function getChannel(channels = {}, names) {
  const keys = Array.isArray(names) ? names : [names];
  for (const key of keys) {
    if (key && channels[key]) return channels[key];
  }
  for (const channel of Object.values(channels || {})) {
    if (!channel || typeof channel !== "object") continue;
    if (keys.includes(channel.semantic) || keys.includes(channel.channel)) return channel;
  }
  return null;
}

function channelArray(channel) {
  if (!channel) return null;
  if (ArrayBuffer.isView(channel) || Array.isArray(channel)) return channel;
  if (ArrayBuffer.isView(channel.array) || Array.isArray(channel.array)) return channel.array;
  if (ArrayBuffer.isView(channel.data) || Array.isArray(channel.data)) return channel.data;
  return null;
}

function channelItemSize(channel, fallback = 1) {
  const itemSize = Number(channel?.itemSize ?? channel?.size ?? channel?.components);
  return Number.isFinite(itemSize) && itemSize > 0 ? Math.floor(itemSize) : fallback;
}

function channelCount(channel, fallbackItemSize = 1) {
  const count = Number(channel?.count);
  if (Number.isFinite(count) && count >= 0) return Math.floor(count);
  const array = channelArray(channel);
  if (!array) return 0;
  return Math.floor(array.length / channelItemSize(channel, fallbackItemSize));
}

function projectPositionChannel(channel, groundY) {
  const source = channelArray(channel);
  if (!source) return null;
  const itemSize = channelItemSize(channel, 3);
  if (itemSize < 3) return null;
  const count = channelCount(channel, itemSize);
  const projected = new Float32Array(count * 3);
  const y = finiteNumber(groundY, -0.56);
  for (let index = 0; index < count; index += 1) {
    const offset = index * itemSize;
    projected[index * 3] = finiteNumber(source[offset], 0);
    projected[index * 3 + 1] = y;
    projected[index * 3 + 2] = finiteNumber(source[offset + 2], source[offset + 1], 0);
  }
  return {
    semantic: "contact-position",
    itemSize: 3,
    count,
    array: projected,
  };
}

function contactSizeChannel(channels, count, options = {}) {
  const baseSize = positiveNumber(options.size, options.baseSize, 1.35);
  const radiusScale = positiveNumber(options.radiusScale, 1.15);
  const heightScale = finiteNumber(options.heightScale, 0.22);
  const minSize = positiveNumber(options.minSize, 0.65);
  const maxSize = positiveNumber(options.maxSize, 3.4);
  const sizeChannel = getChannel(channels, ["size", "scale", "radius"]);
  const heightChannel = getChannel(channels, ["height"]);
  const sizeArray = channelArray(sizeChannel);
  const heightArray = channelArray(heightChannel);
  const sizeItemSize = channelItemSize(sizeChannel, 1);
  const out = new Float32Array(count);
  for (let index = 0; index < count; index += 1) {
    let sourceSize = 1;
    if (sizeArray) {
      const offset = index * sizeItemSize;
      sourceSize = 0;
      for (let component = 0; component < sizeItemSize; component += 1) {
        sourceSize = Math.max(sourceSize, Math.abs(finiteNumber(sizeArray[offset + component], 0)));
      }
      sourceSize = sourceSize || 1;
    }
    const height = Math.abs(finiteNumber(heightArray?.[index], 0));
    const visualSize = baseSize * (0.78 + Math.min(1.2, sourceSize * radiusScale) * 0.18 + Math.min(1.4, height) * heightScale);
    out[index] = Math.min(maxSize, Math.max(minSize, visualSize));
  }
  return {
    semantic: "contact-shadow-size",
    itemSize: 1,
    count,
    array: out,
  };
}

function cloneChannel(channel) {
  const array = channelArray(channel);
  if (!array) return null;
  const next = ArrayBuffer.isView(array) ? new array.constructor(array) : array.slice();
  return {
    ...channel,
    itemSize: channelItemSize(channel, 4),
    count: channelCount(channel, channelItemSize(channel, 4)),
    array: next,
  };
}

function createPathContactChannels(descriptor, options = {}) {
  const primitive = descriptor?.primitive || descriptor?.kind;
  if (!["CurveTubeMeshLayer", "CurveRibbonLayer", "CurvePolylineLayer"].includes(primitive)) return null;
  const evidence = descriptor?.source?.evidence
    || descriptor?.source?.geometry?.evidence
    || descriptor?.descriptor?.evidence
    || descriptor?.evidence;
  if (!evidence) return null;

  let normalized;
  try {
    normalized = normalizePathEvidence(evidence, {
      defaultWidth: finiteNumber(descriptor?.material?.radius, descriptor?.geometry?.radius, descriptor?.material?.width, 1),
    });
  } catch {
    return null;
  }
  const totalCount = normalized.paths.reduce((sum, path) => sum + Math.max(0, path.count), 0);
  if (!totalCount) return null;

  const position = new Float32Array(totalCount * 3);
  const color = new Float32Array(totalCount * 4);
  const size = new Float32Array(totalCount);
  const groundY = finiteNumber(options.groundY, -0.56);
  const radiusScale = positiveNumber(options.contact?.pathRadiusScale, 0.06);
  const baseSize = positiveNumber(options.contact?.pathBaseSize, 0.42);
  let cursor = 0;

  for (const path of normalized.paths) {
    for (let point = 0; point < path.count; point += 1) {
      const source = point * 3;
      const target = cursor * 3;
      position[target] = finiteNumber(path.points[source], 0);
      position[target + 1] = groundY;
      position[target + 2] = finiteNumber(path.points[source + 2], path.points[source + 1], 0);
      if (path.colors) color.set(path.colors.subarray(point * 4, point * 4 + 4), cursor * 4);
      else color.set([0.2, 0.3, 0.34, 0.78], cursor * 4);
      const width = path.widths ? finiteNumber(path.widths[point], 1) : 1;
      size[cursor] = Math.max(0.12, baseSize + Math.min(3, Math.abs(width)) * radiusScale);
      cursor += 1;
    }
  }

  return {
    position: {
      semantic: "path-contact-position",
      itemSize: 3,
      count: totalCount,
      array: position,
    },
    color: {
      semantic: "path-contact-color",
      itemSize: 4,
      count: totalCount,
      array: color,
    },
    size: {
      semantic: "path-contact-shadow-size",
      itemSize: 1,
      count: totalCount,
      array: size,
    },
  };
}

function copyWithout(input, excluded) {
  const out = {};
  for (const [key, value] of Object.entries(input || {})) {
    if (!excluded.includes(key)) out[key] = value;
  }
  return out;
}

function finiteNumber(...values) {
  for (const value of values) {
    const number = Number(value);
    if (Number.isFinite(number)) return number;
  }
  return 0;
}

function positiveNumber(...values) {
  for (const value of values) {
    const number = Number(value);
    if (Number.isFinite(number) && number > 0) return number;
  }
  return 1;
}

function color3(...values) {
  const fallback = values.pop() || [0, 0, 0];
  for (const value of values) {
    if (!value) continue;
    if (Array.isArray(value) || ArrayBuffer.isView(value)) {
      const divisor = Math.max(value[0] || 0, value[1] || 0, value[2] || 0) > 1 ? 255 : 1;
      return [
        clamp01(finiteNumber(value[0], fallback[0]) / divisor),
        clamp01(finiteNumber(value[1], fallback[1]) / divisor),
        clamp01(finiteNumber(value[2], fallback[2]) / divisor),
      ];
    }
    if (typeof value === "string") {
      const hex = value.trim();
      if (/^#([0-9a-f]{3}|[0-9a-f]{6})$/i.test(hex)) {
        const full = hex.length === 4
          ? `#${hex[1]}${hex[1]}${hex[2]}${hex[2]}${hex[3]}${hex[3]}`
          : hex;
        return [
          parseInt(full.slice(1, 3), 16) / 255,
          parseInt(full.slice(3, 5), 16) / 255,
          parseInt(full.slice(5, 7), 16) / 255,
        ];
      }
    }
    if (typeof value === "object") {
      return [
        finiteNumber(value.r, value.x, fallback[0]),
        finiteNumber(value.g, value.y, fallback[1]),
        finiteNumber(value.b, value.z, fallback[2]),
      ];
    }
  }
  return fallback.slice();
}

function clamp01(value) {
  return Math.max(0, Math.min(1, Number.isFinite(value) ? value : 0));
}

function point2(...values) {
  const fallback = values.pop() || [0, 0];
  for (const value of values) {
    if (!value) continue;
    if (Array.isArray(value) || ArrayBuffer.isView(value)) {
      return [finiteNumber(value[0], fallback[0]), finiteNumber(value[1], fallback[1])];
    }
    if (typeof value === "object") {
      return [finiteNumber(value.x, value[0], fallback[0]), finiteNumber(value.y, value[1], fallback[1])];
    }
  }
  return fallback.slice();
}

function deepFreeze(value) {
  if (!value || typeof value !== "object" || Object.isFrozen(value)) return value;
  Object.freeze(value);
  for (const entry of Object.values(value)) deepFreeze(entry);
  return value;
}
