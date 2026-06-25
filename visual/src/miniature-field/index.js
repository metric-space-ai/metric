/*
 * Internal diagnostic renderer exports.
 *
 * This namespace is intentionally not re-exported by visual/src/index.js. Public
 * METRIC Visual callers should enter through createMetricVisual() and semantic
 * show*() commands so rendering stays on MetricVisualRuntime.
 */
export {
  DeterministicNoise3D,
  InstancedBoxField,
  createInstancedBoxField,
} from "./instanced-box-field.js";
