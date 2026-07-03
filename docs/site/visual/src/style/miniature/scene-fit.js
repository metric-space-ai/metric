const DEFAULT_POSITION_CHANNELS = Object.freeze([
  "position",
  "targetPosition",
  "sourcePosition",
  "start",
  "end",
]);

/**
 * Computes x/z bounds for ordinary METRIC layer descriptors.
 *
 * The function reads renderer channels only. It does not inspect record payloads
 * and does not infer new metric-space facts.
 *
 * @param {object[]|object} descriptors
 * @param {object} [options]
 * @returns {object|null}
 */
export function computeMiniatureSceneBounds(descriptors, options = {}) {
  const layers = Array.isArray(descriptors) ? descriptors : [descriptors];
  const channelNames = options.channelNames || DEFAULT_POSITION_CHANNELS;
  const axes = normalizeAxes(options.axes);
  let minX = Infinity;
  let maxX = -Infinity;
  let minZ = Infinity;
  let maxZ = -Infinity;
  let minY = Infinity;
  let maxY = -Infinity;
  let count = 0;

  for (const descriptor of layers) {
    const channels = descriptor?.channels || {};
    for (const key of channelNames) {
      const channel = channels[key];
      const array = channel?.array;
      const itemSize = Math.max(1, Math.floor(Number(channel?.itemSize || channel?.size || 0)));
      if (!array || itemSize < 3) continue;
      for (let offset = 0; offset < array.length; offset += itemSize) {
        const x = Number(array[offset + axes.x]);
        const y = Number(array[offset + axes.y]);
        const z = Number(array[offset + axes.z]);
        if (!Number.isFinite(x) || !Number.isFinite(z)) continue;
        minX = Math.min(minX, x);
        maxX = Math.max(maxX, x);
        minZ = Math.min(minZ, z);
        maxZ = Math.max(maxZ, z);
        if (Number.isFinite(y)) {
          minY = Math.min(minY, y);
          maxY = Math.max(maxY, y);
        }
        count += 1;
      }
    }
  }

  if (!count || ![minX, maxX, minZ, maxZ].every(Number.isFinite)) return null;
  if (!Number.isFinite(minY) || !Number.isFinite(maxY)) {
    minY = 0;
    maxY = 0;
  }
  return {
    minX,
    maxX,
    minY,
    maxY,
    minZ,
    maxZ,
    centerX: (minX + maxX) * 0.5,
    centerY: (minY + maxY) * 0.5,
    centerZ: (minZ + maxZ) * 0.5,
    spanX: Math.max(0, maxX - minX),
    spanY: Math.max(0, maxY - minY),
    spanZ: Math.max(0, maxZ - minZ),
    count,
  };
}

/**
 * Fits descriptor coordinates onto a miniature tabletop stage.
 *
 * This is a renderer transform. It keeps descriptor semantics and channel
 * metadata, clones typed arrays, and only maps already-exported coordinates
 * into a staged camera-friendly x/z footprint.
 *
 * @param {object[]|object} descriptors
 * @param {object} [options]
 * @returns {{descriptors: object[], fit: object|null}}
 */
export function fitMiniatureSceneDescriptors(descriptors, options = {}) {
  const layers = (Array.isArray(descriptors) ? descriptors : [descriptors]).filter(Boolean);
  if (options.enabled === false) return { descriptors: layers, fit: null };

  const bounds = options.bounds || computeMiniatureSceneBounds(layers, options);
  if (!bounds) return { descriptors: layers, fit: null };

  const targetSpan = positiveNumber(options.targetSpan, 2);
  const sourceSpan = Math.max(bounds.spanX, bounds.spanZ, 1e-6);
  const scale = positiveNumber(options.scale, targetSpan / sourceSpan);
  const center = normalizeCenter(options.center, bounds);
  const yScale = positiveNumber(options.yScale, 1);
  const yOffset = finiteNumber(options.yOffset, 0);
  const preserveY = options.preserveY !== false;
  const channelNames = options.channelNames || DEFAULT_POSITION_CHANNELS;
  const axes = normalizeAxes(options.axes);
  const sameRoomMorph = normalizeSameRoomMorphOptions(options.sameRoomMorph ?? options.morphRoom, options);
  const fit = {
    bounds,
    center,
    scale,
    targetSpan,
    yScale,
    yOffset,
    preserveY,
    axes,
    channelNames: channelNames.slice(),
    sameRoomMorph,
  };

  return {
    descriptors: layers.map((descriptor) => fitDescriptor(descriptor, fit)),
    fit,
  };
}

/**
 * Creates a ground descriptor option block from a fit result.
 *
 * @param {object|null} fit
 * @param {object|boolean} [ground]
 * @returns {object|boolean}
 */
export function miniatureGroundFromFit(fit, ground = {}) {
  if (ground === false) return false;
  const options = ground && typeof ground === "object" ? ground : {};
  const bounds = fit?.bounds;
  const scale = positiveNumber(fit?.scale, 1);
  const width = bounds ? Math.max(1, bounds.spanX * scale) : positiveNumber(options.width, options.size, 2.6);
  const depth = bounds ? Math.max(1, bounds.spanZ * scale) : positiveNumber(options.depth, options.size, 2.4);
  const padding = positiveNumber(options.padding, 0.48);
  return {
    ...options,
    width: positiveNumber(options.width, width + padding),
    depth: positiveNumber(options.depth, depth + padding),
  };
}

function fitDescriptor(descriptor, fit) {
  const originalChannels = descriptor.channels || {};
  const channels = { ...originalChannels };
  for (const key of fit.channelNames) {
    const channel = channels[key];
    const array = channel?.array;
    const itemSize = Math.max(1, Math.floor(Number(channel?.itemSize || channel?.size || 0)));
    if (!array || itemSize < 3) continue;
    const next = cloneNumericArray(array);
    for (let offset = 0; offset < next.length; offset += itemSize) {
      const xOffset = offset + fit.axes.x;
      const yOffset = offset + fit.axes.y;
      const zOffset = offset + fit.axes.z;
      next[xOffset] = (Number(array[xOffset]) - fit.center.x) * fit.scale;
      next[zOffset] = (Number(array[zOffset]) - fit.center.z) * fit.scale;
      if (!fit.preserveY) {
        next[yOffset] = (Number(array[yOffset]) - fit.center.y) * fit.yScale + fit.yOffset;
      } else if (fit.yScale !== 1 || fit.yOffset !== 0) {
        next[yOffset] = Number(array[yOffset]) * fit.yScale + fit.yOffset;
      }
    }
    channels[key] = { ...channel, array: next };
  }
  const morphRoom = applySameRoomMorph(channels, fit, descriptor, originalChannels);
  return {
    ...descriptor,
    channels,
    metadata: {
      ...(descriptor.metadata || {}),
      miniatureSceneFit: Boolean(fit),
      ...(morphRoom ? { miniatureSameRoomMorph: morphRoom } : {}),
    },
  };
}

function applySameRoomMorph(channels, fit, descriptor, originalChannels) {
  const options = fit.sameRoomMorph || {};
  if (options.enabled === false) return null;
  const targetNames = options.channels || ["position", "targetPosition"];
  const summaries = [];

  for (const key of targetNames) {
    const channel = channels[key];
    if (!channel) continue;
    const originalChannel = originalChannels[key] || channel;
    const originalArray = channelArray(originalChannel);
    const fittedArray = channelArray(channel);
    if (!originalArray || !fittedArray) continue;
    const originalItemSize = channelItemSize(originalChannel, channelItemSize(channel, 3));
    const fittedItemSize = channelItemSize(channel, originalItemSize);
    const count = Math.min(channelCount(originalChannel, originalItemSize), channelCount(channel, fittedItemSize));
    if (!count) continue;

    const originalStats = channelStats(originalChannel, fit.axes);
    const isFlatY = originalStats && originalStats.spanY <= options.flatSpanEpsilon;
    const isFlatZWith2DY = originalStats
      && originalItemSize >= 2
      && originalStats.spanZ <= options.flatSpanEpsilon
      && originalStats.spanY > options.flatSpanEpsilon
      && options.reprojectFlatZ !== false;
    const isMorph = isMorphDescriptor(descriptor, key);
    if (!isMorph && options.applyToAllTargets !== true) continue;
    if (!isFlatY && !isFlatZWith2DY && options.force !== true) continue;

    const next = new Float32Array(count * 3);
    for (let index = 0; index < count; index += 1) {
      const originalOffset = index * originalItemSize;
      const fittedOffset = index * fittedItemSize;
      const targetOffset = index * 3;
      const x = finiteNumber(fittedArray[fittedOffset + fit.axes.x], 0);
      const zFromFitted = finiteNumber(fittedArray[fittedOffset + fit.axes.z], fittedArray[fittedOffset + Math.min(2, fittedItemSize - 1)], 0);
      const zFromOriginalY = (finiteNumber(originalArray[originalOffset + fit.axes.y], 0) - fit.center.z) * fit.scale;
      next[targetOffset] = x;
      next[targetOffset + 1] = options.flatY;
      next[targetOffset + 2] = isFlatZWith2DY ? zFromOriginalY : zFromFitted;
    }
    channels[key] = {
      ...channel,
      array: next,
      itemSize: 3,
      size: 3,
      count,
      metadata: {
        ...(channel.metadata || {}),
        miniatureSameRoomMorph: isFlatZWith2DY ? "reproject-flat-z-to-xz" : "flat-y-target",
        miniatureFlatY: options.flatY,
      },
    };
    summaries.push({
      channel: key,
      flatY: options.flatY,
      mode: isFlatZWith2DY ? "reproject-flat-z-to-xz" : "flat-y-target",
      count,
    });
  }

  return summaries.length
    ? {
      enabled: true,
      groundY: options.groundY,
      flatY: options.flatY,
      channels: summaries,
    }
    : null;
}

function isMorphDescriptor(descriptor, key) {
  const animation = descriptor?.animation || {};
  if (animation.channel === key) return true;
  if (String(animation.mode || "").includes("morph")) return true;
  if (Array.isArray(animation.requiresChannels) && animation.requiresChannels.includes(key)) return true;
  return descriptor?.kind === "morph" || descriptor?.source?.viewKind === "morph";
}

function cloneNumericArray(array) {
  if (ArrayBuffer.isView(array)) return new array.constructor(array);
  return new Float32Array(array.map((value) => Number(value) || 0));
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

function channelStats(channel, axes) {
  const array = channelArray(channel);
  if (!array) return null;
  const itemSize = channelItemSize(channel, 3);
  const count = channelCount(channel, itemSize);
  if (!count) return null;
  let minX = Infinity;
  let maxX = -Infinity;
  let minY = Infinity;
  let maxY = -Infinity;
  let minZ = Infinity;
  let maxZ = -Infinity;
  for (let index = 0; index < count; index += 1) {
    const offset = index * itemSize;
    const x = Number(array[offset + Math.min(axes.x, itemSize - 1)]);
    const y = Number(array[offset + Math.min(axes.y, itemSize - 1)]);
    const z = Number(array[offset + Math.min(axes.z, itemSize - 1)]);
    if (Number.isFinite(x)) {
      minX = Math.min(minX, x);
      maxX = Math.max(maxX, x);
    }
    if (Number.isFinite(y)) {
      minY = Math.min(minY, y);
      maxY = Math.max(maxY, y);
    }
    if (Number.isFinite(z)) {
      minZ = Math.min(minZ, z);
      maxZ = Math.max(maxZ, z);
    }
  }
  if (![minX, maxX, minY, maxY, minZ, maxZ].every(Number.isFinite)) return null;
  return {
    spanX: Math.max(0, maxX - minX),
    spanY: Math.max(0, maxY - minY),
    spanZ: Math.max(0, maxZ - minZ),
  };
}

function normalizeSameRoomMorphOptions(value, fitOptions) {
  if (value === false) return { enabled: false };
  const source = value && typeof value === "object" ? value : {};
  const groundY = finiteNumber(source.groundY, fitOptions.groundY, fitOptions.y, -0.56);
  const lift = finiteNumber(source.lift, source.flatLift, fitOptions.flatLift, 0.09);
  return {
    enabled: true,
    groundY,
    flatY: finiteNumber(source.flatY, source.y, fitOptions.flatY, groundY + lift),
    flatSpanEpsilon: positiveNumber(source.flatSpanEpsilon, fitOptions.flatSpanEpsilon, 1e-5),
    reprojectFlatZ: source.reprojectFlatZ !== false,
    applyToAllTargets: source.applyToAllTargets === true,
    force: source.force === true,
    channels: Array.isArray(source.channels) && source.channels.length
      ? source.channels.slice()
      : ["position", "targetPosition"],
  };
}

function normalizeAxes(options = {}) {
  return {
    x: axisIndex(options.x, 0),
    y: axisIndex(options.y, 1),
    z: axisIndex(options.z, 2),
  };
}

function axisIndex(value, fallback) {
  if (value === "x") return 0;
  if (value === "y") return 1;
  if (value === "z") return 2;
  const number = Number(value);
  return Number.isInteger(number) && number >= 0 ? number : fallback;
}

function normalizeCenter(value, bounds) {
  if (Array.isArray(value) || ArrayBuffer.isView(value)) {
    return {
      x: finiteNumber(value[0], bounds.centerX),
      y: finiteNumber(value[1], bounds.centerY),
      z: finiteNumber(value[2], bounds.centerZ),
    };
  }
  if (value && typeof value === "object") {
    return {
      x: finiteNumber(value.x, bounds.centerX),
      y: finiteNumber(value.y, bounds.centerY),
      z: finiteNumber(value.z, bounds.centerZ),
    };
  }
  return { x: bounds.centerX, y: bounds.centerY, z: bounds.centerZ };
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
