export function getChannel(channels = {}, names) {
  const keys = Array.isArray(names) ? names : [names];
  for (const key of keys) {
    if (key && channels[key]) return channels[key];
  }
  for (const channel of Object.values(channels)) {
    if (!channel || typeof channel !== "object") continue;
    if (keys.includes(channel.channel) || keys.includes(channel.semantic)) return channel;
  }
  return null;
}

export function getChannelArray(channel) {
  if (!channel) return null;
  if (ArrayBuffer.isView(channel)) return channel;
  if (Array.isArray(channel)) return channel;
  if (ArrayBuffer.isView(channel.array) || Array.isArray(channel.array)) return channel.array;
  if (ArrayBuffer.isView(channel.data) || Array.isArray(channel.data)) return channel.data;
  return null;
}

export function getChannelItemSize(channel, fallback = 1) {
  const itemSize = Number(channel?.itemSize ?? channel?.size ?? channel?.components);
  return Number.isFinite(itemSize) && itemSize > 0 ? Math.floor(itemSize) : fallback;
}

export function getChannelCount(channel, fallbackItemSize = 1) {
  if (!channel) return 0;
  const count = Number(channel.count);
  if (Number.isFinite(count) && count >= 0) return Math.floor(count);
  const array = getChannelArray(channel);
  if (!array || typeof array.length !== "number") return 0;
  return Math.floor(array.length / getChannelItemSize(channel, fallbackItemSize));
}

export function inferInstanceCount(channels = {}, preferred = []) {
  for (const name of preferred) {
    const count = getChannelCount(getChannel(channels, name));
    if (count > 0) return count;
  }
  for (const channel of Object.values(channels)) {
    const count = getChannelCount(channel);
    if (count > 0) return count;
  }
  return 0;
}

export function channelToFloat32(channel, itemSize, count, fallback = 0) {
  const out = new Float32Array(Math.max(0, count) * itemSize);
  fillWithFallback(out, itemSize, fallback);
  const source = getChannelArray(channel);
  if (!source) return out;
  const sourceItemSize = getChannelItemSize(channel, itemSize);
  const sourceCount = Math.min(count, getChannelCount(channel, sourceItemSize));
  for (let index = 0; index < sourceCount; index += 1) {
    const sourceOffset = index * sourceItemSize;
    const targetOffset = index * itemSize;
    for (let component = 0; component < itemSize; component += 1) {
      const fallbackValue = Array.isArray(fallback) || ArrayBuffer.isView(fallback)
        ? fallback[Math.min(component, fallback.length - 1)]
        : fallback;
      const value = Number(source[sourceOffset + Math.min(component, sourceItemSize - 1)]);
      out[targetOffset + component] = Number.isFinite(value) ? value : fallbackValue;
    }
  }
  return out;
}

export function scalarChannel(channels, names, count, fallback = 0) {
  return channelToFloat32(getChannel(channels, names), 1, count, fallback);
}

export function combineScalarChannels(channels, names, count, fallback = 0) {
  const out = new Float32Array(count);
  out.fill(fallback);
  for (const name of names) {
    const channel = getChannel(channels, name);
    if (!channel) continue;
    const next = channelToFloat32(channel, 1, count, 0);
    for (let index = 0; index < count; index += 1) {
      out[index] = Math.max(out[index], next[index]);
    }
  }
  return out;
}

export function positionChannel(channels, names, count) {
  return channelToFloat32(getChannel(channels, names), 3, count, [0, 0, 0]);
}

export function colorChannel(channels, count, options = {}) {
  const defaultColor = options.defaultColor || [0.32, 0.48, 0.68, 1];
  const color = getChannel(channels, options.names || ["color", "rgba"]);
  const alpha = getChannel(channels, options.alphaNames || ["alpha"]);
  const scalar = getChannel(channels, options.scalarNames || ["scalar", "value"]);
  const category = getChannel(channels, options.categoryNames || ["category", "group"]);
  const out = new Float32Array(count * 4);

  if (color) {
    const colorArray = getChannelArray(color);
    const itemSize = getChannelItemSize(color, 4);
    for (let index = 0; index < count; index += 1) {
      const sourceOffset = index * itemSize;
      const targetOffset = index * 4;
      out[targetOffset] = finiteColor(colorArray?.[sourceOffset], defaultColor[0]);
      out[targetOffset + 1] = finiteColor(colorArray?.[sourceOffset + 1], defaultColor[1]);
      out[targetOffset + 2] = finiteColor(colorArray?.[sourceOffset + 2], defaultColor[2]);
      out[targetOffset + 3] = itemSize >= 4 ? finiteColor(colorArray?.[sourceOffset + 3], defaultColor[3]) : defaultColor[3];
    }
  } else if (scalar) {
    const values = channelToFloat32(scalar, 1, count, 0);
    const domain = numericDomain(scalar.domain, values);
    for (let index = 0; index < count; index += 1) {
      const c = scalarColor(values[index], domain);
      out.set(c, index * 4);
    }
  } else if (category) {
    const values = channelToFloat32(category, 1, count, 0);
    for (let index = 0; index < count; index += 1) {
      out.set(categoryColor(values[index]), index * 4);
    }
  } else {
    for (let index = 0; index < count; index += 1) {
      out.set(defaultColor, index * 4);
    }
  }

  if (alpha) {
    const alphaValues = channelToFloat32(alpha, 1, count, 1);
    for (let index = 0; index < count; index += 1) {
      out[index * 4 + 3] *= clamp01(alphaValues[index]);
    }
  }
  return out;
}

export function computeBoundsFromChannel(channel) {
  const array = getChannelArray(channel);
  if (!array) return null;
  const itemSize = getChannelItemSize(channel, 3);
  const count = getChannelCount(channel, itemSize);
  if (!count) return null;
  const min = [Infinity, Infinity, Infinity];
  const max = [-Infinity, -Infinity, -Infinity];
  for (let index = 0; index < count; index += 1) {
    const offset = index * itemSize;
    for (let axis = 0; axis < 3; axis += 1) {
      const value = Number(array[offset + Math.min(axis, itemSize - 1)]);
      if (!Number.isFinite(value)) continue;
      min[axis] = Math.min(min[axis], value);
      max[axis] = Math.max(max[axis], value);
    }
  }
  if (!Number.isFinite(min[0])) return null;
  return { min, max };
}

export function numericDomain(domain, values) {
  if (Array.isArray(domain) && domain.length >= 2) {
    const min = Number(domain[0]);
    const max = Number(domain[1]);
    if (Number.isFinite(min) && Number.isFinite(max) && min !== max) return [min, max];
  } else if (domain && typeof domain === "object") {
    const min = Number(domain.min);
    const max = Number(domain.max);
    if (Number.isFinite(min) && Number.isFinite(max) && min !== max) return [min, max];
  }
  let min = Infinity;
  let max = -Infinity;
  for (const value of values || []) {
    const number = Number(value);
    if (!Number.isFinite(number)) continue;
    min = Math.min(min, number);
    max = Math.max(max, number);
  }
  if (!Number.isFinite(min) || !Number.isFinite(max)) return [0, 1];
  return min === max ? [min - 1, max + 1] : [min, max];
}

export function scalarColor(value, domain = [0, 1]) {
  const t = clamp01((Number(value) - domain[0]) / Math.max(domain[1] - domain[0], 0.000001));
  const a = [0.12, 0.22, 0.36, 1];
  const b = [0.12, 0.58, 0.54, 1];
  const c = [0.94, 0.62, 0.28, 1];
  return t < 0.5 ? mixColor(a, b, t * 2) : mixColor(b, c, (t - 0.5) * 2);
}

export function categoryColor(value) {
  const palette = [
    [0.16, 0.37, 0.60, 1],
    [0.82, 0.29, 0.36, 1],
    [0.16, 0.62, 0.56, 1],
    [0.89, 0.52, 0.22, 1],
    [0.43, 0.35, 0.48, 1],
    [0.34, 0.46, 0.56, 1],
  ];
  const index = Math.abs(Math.floor(Number(value) || 0)) % palette.length;
  return palette[index].slice();
}

export function clamp01(value) {
  return Math.max(0, Math.min(1, Number.isFinite(Number(value)) ? Number(value) : 0));
}

function fillWithFallback(array, itemSize, fallback) {
  if (Array.isArray(fallback) || ArrayBuffer.isView(fallback)) {
    for (let index = 0; index < array.length; index += itemSize) {
      for (let component = 0; component < itemSize; component += 1) {
        array[index + component] = fallback[Math.min(component, fallback.length - 1)] ?? 0;
      }
    }
    return;
  }
  array.fill(fallback);
}

function finiteColor(value, fallback) {
  const number = Number(value);
  return Number.isFinite(number) ? clamp01(number) : fallback;
}

function mixColor(a, b, t) {
  return [
    a[0] + (b[0] - a[0]) * t,
    a[1] + (b[1] - a[1]) * t,
    a[2] + (b[2] - a[2]) * t,
    a[3] + (b[3] - a[3]) * t,
  ];
}

