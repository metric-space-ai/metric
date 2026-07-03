import { createMiniatureStagePreset } from "./presets.js";

/**
 * Creates MetricVisualRuntime options from the miniature stage contract.
 *
 * The function does not instantiate the runtime and does not import renderer
 * classes. It is the reusable bridge from the photographic style layer to any
 * METRIC visual surface: camera, clear color, controls, focus/postprocess and
 * layer factory wiring are derived from the same stage object.
 *
 * @param {object} [options]
 * @param {HTMLCanvasElement} [options.canvas]
 * @param {object} [options.stage]
 * @param {object} [options.stageOptions]
 * @param {Function|object} [options.layerFactory]
 * @returns {{stage: object, runtimeOptions: object}}
 */
export function createMiniatureRuntimeOptions(options = {}) {
  const stage = options.stage || createMiniatureStagePreset(options.stageOptions || {});
  const camera = {
    ...(stage.camera || {}),
    near: finiteNumber(options.near, options.camera?.near, stage.camera?.near, 0.02),
    far: finiteNumber(options.far, options.camera?.far, stage.camera?.far, 100),
    ...(options.camera || {}),
  };
  const controls = normalizeBooleanOptions(options.controls, {
    enabled: true,
    rotateSpeed: 0.0055,
    focusAxis: stage.focus?.axis,
    focusRadius: stage.focus?.radius,
    updateFocusOnMove: stage.focus?.pointer?.enabled !== false && stage.interaction?.pointerFocus !== false,
    preventDefault: true,
  });
  const renderer = {
    autoResize: true,
    pointer: true,
    ...(options.renderer || {}),
  };
  const layers = {
    factory: options.layerFactory || options.layers?.factory,
    autoLoadFactory: false,
    ...(options.layers || {}),
  };

  return {
    stage,
    runtimeOptions: {
      canvas: options.canvas,
      gl: options.gl,
      pixelRatioCap: finiteNumber(options.pixelRatioCap, 2),
      background: options.background || stage.scene?.clearColor,
      document: options.document,
      scene: options.scene,
      camera,
      controls,
      focusLine: {
        ...(stage.focus || {}),
        ...(options.focusLine || {}),
      },
      postprocess: options.postprocess || stage.postprocess,
      hoverFocus: options.hoverFocus || stage.hoverFocus,
      renderer,
      layers,
    },
  };
}

function normalizeBooleanOptions(value, defaults) {
  if (value === false) return { ...defaults, enabled: false };
  if (value === true || value === undefined || value === null) return { ...defaults };
  if (typeof value === "object") return { ...defaults, ...value };
  return { ...defaults };
}

function finiteNumber(...values) {
  for (const value of values) {
    const number = Number(value);
    if (Number.isFinite(number)) return number;
  }
  return 0;
}
