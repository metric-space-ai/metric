import { VisualRenderer, VisualScene } from "../engine/index.js";
import { createMiniaturePerspectiveCamera } from "../camera/index.js";
import {
  buildMetricPairPreview,
  buildMetricRecordPreview,
  createFocusLineState,
  createCameraControls,
} from "../interaction/index.js";
import { VisualSpace } from "../data/index.js";
import {
  PickingPass,
  PickingRegistry,
  PickResult,
  createPickRequest,
  pickNearestProjectedPoint,
  pickResultFromNumericId,
  readPickIdFromFramebuffer,
} from "../picking/index.js";
import { createRelationMatrixPicker } from "../relational/index.js";
import {
  getChannel,
  getChannelArray,
  getChannelCount,
  getChannelItemSize,
} from "../layers/channels.js";
import {
  FxaaPass,
  NativePostProcessPipeline,
  TiltShiftPass,
} from "../native-postprocess/index.js";
import { MetricPostFxStack } from "../postfx/index.js";
import { buildLinkedSelectionPresentation } from "../selection/index.js";

const RUNTIME_RENDER_PASS = "Scene(RenderPass)";
const RUNTIME_SCREEN_READABLE_PASS = "ScreenReadableOverlayPass";
const RUNTIME_COMPATIBILITY_PASS_ORDER = Object.freeze(["FxaaPass", "TiltShiftPass"]);

export const DEFAULT_RUNTIME_OPTIONS = Object.freeze({
  pixelRatioCap: 2,
  background: Object.freeze([0.02, 0.025, 0.03, 1]),
  document: Object.freeze({
    validate: true,
  }),
  renderer: Object.freeze({
    autoResize: true,
    autoStart: false,
    pointer: true,
    updateStyle: false,
  }),
  camera: Object.freeze({
    mode: "miniaturePerspective",
  }),
    focusLine: Object.freeze({
      radiusRatio: 1 / 3,
      focusBandRatio: 0,
      blurCurve: 1,
    }),
    postprocess: Object.freeze({
      enabled: true,
      tiltShift: true,
      blurRadius: 10,
      gradientRadius: 100,
      focusBand: 0,
      blurCurve: 1,
      fxaa: false,
      postFx: false,
  }),
  controls: Object.freeze({
    enabled: true,
    mode: "miniaturePerspective",
    preventDefault: true,
    updateFocusOnMove: true,
  }),
  hoverFocus: Object.freeze({
    enabled: false,
    thresholdPx: 42,
    throttleMs: 34,
    maxCandidates: 12000,
    clearOnLeave: false,
    clearOnMiss: false,
    focusWhileDragging: false,
    positionChannels: Object.freeze(["position", "targetPosition", "sourcePosition"]),
    recordChannel: "recordId",
  }),
  inspection: Object.freeze({
    enabled: true,
    hover: true,
    click: true,
    gpu: true,
    relationMatrix: true,
    graph: true,
    cpuFallback: true,
    thresholdPx: 34,
    gpuRadiusPx: 6,
    edgeThresholdPx: 7,
    clearOnMiss: false,
    selectOnClick: true,
    maxCandidates: 20000,
    recordChannel: "recordId",
    positionChannels: Object.freeze(["position", "targetPosition", "sourcePosition"]),
  }),
  layers: Object.freeze({
    autoLoadFactory: true,
    disposeOnRebuild: true,
    warnOnMissingFactory: true,
  }),
});

/**
 * @typedef {Object} MetricVisualRuntimeOptions
 * @property {HTMLCanvasElement} [canvas]
 * @property {WebGLRenderingContext} [gl]
 * @property {number} [pixelRatioCap=2]
 * @property {number[]} [background]
 * @property {Object} [document]
 * @property {boolean} [document.validate=true]
 * @property {Object} [renderer]
 * @property {boolean} [renderer.autoResize=true]
 * @property {boolean} [renderer.autoStart=false]
 * @property {boolean} [renderer.pointer=true]
 * @property {boolean} [renderer.updateStyle=false]
 * @property {Object} [camera]
 * @property {Object} [focusLine]
 * @property {Object|boolean} [postprocess]
 * @property {boolean} [postprocess.enabled=true]
 * @property {boolean} [postprocess.tiltShift=true]
 * @property {number} [postprocess.blurRadius=10]
 * @property {number} [postprocess.gradientRadius=100]
 * @property {boolean} [postprocess.fxaa=false]
 * @property {Object|boolean} [controls]
 * @property {boolean} [controls.enabled=true]
 * @property {string} [controls.mode="miniaturePerspective"]
 * @property {boolean} [controls.preventDefault=true]
 * @property {boolean} [controls.updateFocusOnMove=true]
 * @property {Object} [layers]
 * @property {Function|Object} [layers.factory]
 * @property {boolean} [layers.autoLoadFactory=true]
 * @property {boolean} [layers.disposeOnRebuild=true]
 * @property {boolean} [layers.warnOnMissingFactory=true]
 */

export class RuntimePostProcessPipeline {
  constructor(gl, options = {}) {
    this.gl = gl;
    this.options = options;
    this.enabled = options.enabled !== false;
    const postFxOptions = options.postFx ? normalizePostFxOptions(options.postFx) : null;
    this.focusTarget = normalizeCameraFocusTarget(options.focusTarget || options.cameraFocus || options.focus || null);
    this.lastFocusDistance = null;
    const wantsCameraDepthTexture = postFxOptions?.cameraDof?.enabled === true
      || options.sceneDepthTexture === true
      || options.cameraDepthTexture === true
      || options.depthTexture === true;
    this.native = this.enabled
      ? new NativePostProcessPipeline(gl, {
        ...options,
        sceneDepthTexture: wantsCameraDepthTexture,
      })
      : null;
    this.focusState = null;
    this.tiltShiftPass = null;
    this.fxaaPass = null;
    this.postFxStack = null;

    // METRIC photographic pass order is RenderPass -> FXAA -> TiltShift.
    // METRIC photographic passes are appended afterwards by MetricPostFxStack.
    if (this.native && options.fxaa === true) {
      this.fxaaPass = this.native.addPass(new FxaaPass(gl, {
        enabled: options.fxaa !== false,
        strength: options.fxaaStrength,
        edgeThreshold: options.edgeThreshold,
        edgeThresholdMin: options.edgeThresholdMin,
      }));
    }

    const tiltShiftOptions = options.tiltShift && typeof options.tiltShift === "object"
      ? options.tiltShift
      : {};
    const tiltShiftEnabled = options.tiltShift !== false && tiltShiftOptions.enabled !== false;
    if (this.native && tiltShiftEnabled) {
      this.tiltShiftPass = this.native.addPass(new TiltShiftPass(gl, {
        enabled: true,
        blurRadius: tiltShiftOptions.blurRadius ?? options.blurRadius,
        gradientRadius: tiltShiftOptions.gradientRadius ?? options.gradientRadius,
        focusBand: tiltShiftOptions.focusBand ?? options.focusBand,
        blurCurve: tiltShiftOptions.blurCurve ?? options.blurCurve,
        start: tiltShiftOptions.start ?? options.start,
        end: tiltShiftOptions.end ?? options.end,
      }));
    }

    if (this.native && postFxOptions) {
      this.postFxStack = this.native.addPass(new MetricPostFxStack(gl, postFxOptions));
    }
  }

  get length() {
    if (!this.enabled || !this.native) return 0;
    return this.native.passes.filter((pass) => pass && pass.enabled !== false).length;
  }

  setSize(width, height) {
    if (this.native) this.native.setSize(width, height);
    if (this.focusState) this.setFocusState(this.focusState);
    return this;
  }

  setFocusState(focusState) {
    this.focusState = focusState;
    if (this.tiltShiftPass && typeof this.tiltShiftPass.setFocusState === "function") {
      this.tiltShiftPass.setFocusState(focusState);
    }
    if (this.postFxStack && typeof this.postFxStack.setFocusState === "function") {
      this.postFxStack.setFocusState(focusState);
    }
    return this;
  }

  setFocusTarget(target) {
    this.focusTarget = normalizeCameraFocusTarget(target);
    if (!this.focusTarget) this.lastFocusDistance = null;
    return this;
  }

  clearFocusTarget() {
    this.focusTarget = null;
    this.lastFocusDistance = null;
    return this;
  }

  setCameraFocusPoint(position) {
    return this.setFocusTarget({ type: "point", position });
  }

  setCameraFocusDistance(focusDistance) {
    return this.setFocusTarget({ type: "distance", focusDistance });
  }

  setEnabled(enabled) {
    this.enabled = Boolean(enabled);
    return this;
  }

  getState() {
    const nativePasses = this.native?.passes?.map((pass) => pass?.constructor?.name).filter(Boolean) || [];
    const enabledNativePasses = this.native?.passes
      ?.filter((pass) => pass?.enabled !== false)
      .map((pass) => pass?.constructor?.name)
      .filter(Boolean) || [];
    const compatibilityPasses = enabledNativePasses.filter((name) => RUNTIME_COMPATIBILITY_PASS_ORDER.includes(name));
    const postFxOrder = this.postFxStack?.order?.slice?.() || [];
    const postFxEnabledOrder = this.postFxStack?.enabled !== false
      ? postFxOrder.filter((name) => this.postFxStack?.[name]?.enabled !== false)
      : [];
    const screenReadableOverlay = hasRenderPhaseLayers(this.lastContext?.scene, "screen-readable-overlay");
    return {
      enabled: this.enabled,
      length: this.length,
      nativePasses,
      enabledNativePasses,
      pipelineOrder: [
        RUNTIME_RENDER_PASS,
        ...compatibilityPasses,
        ...postFxEnabledOrder.map((name) => `postFx.${name}`),
        ...(screenReadableOverlay ? [RUNTIME_SCREEN_READABLE_PASS] : []),
      ],
      pipelinePhases: {
        scene: RUNTIME_RENDER_PASS,
        compatibility: compatibilityPasses,
        postFx: postFxEnabledOrder,
        screenReadableOverlay: screenReadableOverlay ? RUNTIME_SCREEN_READABLE_PASS : null,
      },
      sceneDepthTexture: Boolean(this.native?.sceneTarget?.depthTexture),
      sceneDepthEncoding: this.native?.sceneTarget?.depthEncoding || null,
      focusTarget: this.focusTarget ? clonePlainObject(this.focusTarget) : null,
      focusDistance: this.lastFocusDistance,
      tiltShift: {
        enabled: this.tiltShiftPass?.enabled ?? false,
        blurRadius: this.tiltShiftPass?.blurRadius ?? null,
        gradientRadius: this.tiltShiftPass?.gradientRadius ?? null,
      },
      postFx: this.postFxStack ? {
        enabled: this.postFxStack.enabled !== false,
        order: postFxOrder,
        enabledOrder: postFxEnabledOrder,
        passes: this.postFxStack.passes().map((pass) => pass?.constructor?.name).filter(Boolean),
        enabledPasses: this.postFxStack.enabledPasses().map((pass) => pass?.constructor?.name).filter(Boolean),
        cameraDof: this.postFxStack.cameraDof ? {
          enabled: this.postFxStack.cameraDof.enabled !== false,
          model: this.postFxStack.cameraDof.dof?.model ?? null,
          focusModel: this.postFxStack.cameraDof.dof?.focusModel ?? null,
          cocModel: this.postFxStack.cameraDof.dof?.cocModel ?? null,
          depthMode: this.postFxStack.cameraDof.dof?.depthMode ?? null,
          requiresDepthTexture: this.postFxStack.cameraDof.dof?.requiresDepthTexture ?? null,
          usesCameraNearFar: this.postFxStack.cameraDof.dof?.usesCameraNearFar ?? null,
          focusDistance: this.postFxStack.cameraDof.dof?.focusDistance ?? null,
          aperture: this.postFxStack.cameraDof.dof?.aperture ?? null,
          focalLength: this.postFxStack.cameraDof.dof?.focalLength ?? null,
          fStop: this.postFxStack.cameraDof.dof?.fStop ?? null,
          sensorScale: this.postFxStack.cameraDof.dof?.sensorScale ?? null,
          maxBlur: this.postFxStack.cameraDof.dof?.maxBlur ?? null,
          focalRange: this.postFxStack.cameraDof.dof?.focalRange ?? null,
          depthNear: this.postFxStack.cameraDof.dof?.depthNear ?? null,
          depthFar: this.postFxStack.cameraDof.dof?.depthFar ?? null,
          cameraNear: this.postFxStack.cameraDof.dof?.cameraNear ?? null,
          cameraFar: this.postFxStack.cameraDof.dof?.cameraFar ?? null,
        } : null,
      } : null,
    };
  }

  render(context) {
    this.lastContext = context;
    if (!this.enabled || this.length === 0) {
      context.renderer.renderScene(null);
      renderScreenReadableOverlay(context);
      return null;
    }

    if (this.focusState) this.setFocusState(this.focusState);
    const focusDistance = resolveCameraFocusDistance(this.focusTarget, context.camera || context.renderer?.camera || null);
    if (focusDistance != null) {
      this.lastFocusDistance = focusDistance;
      this.postFxStack?.cameraDof?.setFocusDistance?.(focusDistance);
    }
    if (this.postFxStack && typeof this.postFxStack.setTime === "function") {
      const elapsedMs = context?.renderer?.loop?.time?.elapsedMs ?? performanceNow();
      this.postFxStack.setTime(elapsedMs * 0.001);
    }

    const width = context.size.drawingBufferWidth;
    const height = context.size.drawingBufferHeight;
    this.native.renderScene((sceneContext) => {
      context.renderer.renderScene(sceneContext.target || null, { phase: "scene" });
    }, {
      width,
      height,
      target: null,
      focusState: this.focusState,
      camera: context.camera || context.renderer?.camera || null,
    });
    renderScreenReadableOverlay(context);
    return null;
  }

  dispose() {
    if (this.native) this.native.dispose();
  }
}

function renderScreenReadableOverlay(context) {
  if (!hasRenderPhaseLayers(context?.renderer?.scene || context?.scene, "screen-readable-overlay")) return;
  if (typeof context.renderer?.renderScenePhase === "function") {
    context.renderer.renderScenePhase("screen-readable-overlay", null, {
      clear: false,
      clearDepth: false,
    });
    return;
  }
  context.renderer?.renderScene?.(null, {
    phase: "screen-readable-overlay",
    clear: false,
    clearDepth: false,
  });
}

function hasRenderPhaseLayers(scene, phase) {
  if (!scene || !Array.isArray(scene.children)) return false;
  return scene.children.some((child) => {
    if (!child || child.enabled === false || child.visible === false) return false;
    return resolveLayerRenderPhase(child) === phase;
  });
}

function resolveLayerRenderPhase(layer) {
  return layer.renderPhase
    || layer.descriptor?.renderPhase
    || layer.descriptor?.metadata?.renderPhase
    || layer.descriptor?.metadata?.postprocessGroup
    || layer.descriptor?.material?.renderPhase
    || layer.descriptor?.material?.postprocessGroup
    || "scene";
}

export class MetricVisualRuntime {
  constructor(options = {}) {
    this.options = mergeRuntimeOptions(options);
    this.events = new Map();
    this.warnings = [];
    this.document = null;
    this.visualSpace = null;
    this.viewDescriptors = [];
    this.layerDescriptors = [];
    this.layers = [];
    this.layerFactory = this.options.layerFactory || this.options.layers.factory || null;
    this.layerFactoryPromise = null;
    this.stageFocusOptions = null;
    this.focusTarget = normalizeCameraFocusTarget(this.options.focusTarget || this.options.cameraFocus || null);
    this.hoverFocusOptions = normalizeHoverFocusOptions(this.options.hoverFocus);
    this.hoverFocusState = {
      lastAt: 0,
      target: null,
    };
    this.cameraFocusState = {
      focusTarget: this.focusTarget ? clonePlainObject(this.focusTarget) : null,
      focusDistance: null,
    };
    this.unsubscribeHoverFocus = null;
    this.selection = {
      recordId: null,
      pair: null,
      record: null,
      preview: null,
      presentation: null,
      source: null,
      pickSource: null,
    };
    this.inspectionOptions = normalizeInspectionOptions(this.options.inspection ?? this.options.picking);
    this.pickingRegistry = new PickingRegistry();
    this.pickingPass = null;
    this.pickingTarget = null;
    this.pickingIndex = createEmptyPickingIndex();
    this.inspectionState = createEmptyInspectionState();
    this.unsubscribeInspection = null;
    this.disposed = false;

    const background = normalizeColor(this.options.background, DEFAULT_RUNTIME_OPTIONS.background);
    this.scene = this.options.scene || new VisualScene({ backgroundColor: background });
    this.cameraBundle = createMiniaturePerspectiveCamera(this.options.camera);
    this.camera = this.options.cameraObject || this.cameraBundle.camera;
    this.cameraState = this.options.cameraState || this.cameraBundle.state;

    this.renderer = new VisualRenderer({
      ...this.options.renderer,
      canvas: this.options.canvas,
      gl: this.options.gl,
      scene: this.scene,
      camera: this.camera,
      clearColor: background,
      maxPixelRatio: this.options.pixelRatioCap,
      autoStart: false,
      autoResize: this.options.renderer.autoResize,
      pointer: this.options.renderer.pointer,
      updateStyle: this.options.renderer.updateStyle,
    });

    this.canvas = this.renderer.canvas;
    this.focusLine = createFocusLineState(this.createFocusLineOptions());
    this.postprocess = this.createPostProcessPipeline();
    this.syncFocusTarget();
    this.renderer.setPipeline(this.postprocess);

    this.controls = this.createControls();
    this.attachHoverFocus();
    this.attachInspection();
    this.unsubscribeRendererResize = this.renderer.on("resize", ({ size }) => {
      this.handleResize(size);
    });
    this.unsubscribeRendererAfterRender = this.renderer.on("afterRender", (context) => {
      this.handleAfterRender(context);
    });
    this.handleResize(this.renderer.size);

    this.layerState = this.createLayerState("empty");
    if (this.options.layers.autoLoadFactory && !this.layerFactory) {
      this.layerFactoryPromise = this.loadDefaultLayerFactory();
    }

    if (this.options.renderer.autoStart) {
      this.start();
    }
  }

  on(type, listener) {
    if (!this.events.has(type)) this.events.set(type, new Set());
    this.events.get(type).add(listener);
    return () => this.off(type, listener);
  }

  once(type, listener) {
    const off = this.on(type, (payload) => {
      off();
      listener(payload);
    });
    return off;
  }

  off(type, listener) {
    const listeners = this.events.get(type);
    if (listeners) listeners.delete(listener);
    return this;
  }

  emit(type, payload) {
    const listeners = this.events.get(type);
    if (!listeners) return this;
    for (const listener of listeners) {
      listener(payload);
    }
    return this;
  }

  mount(container, options = {}) {
    this.renderer.mount(container, options);
    this.resize();
    return this;
  }

  unmount() {
    this.renderer.unmount();
    return this;
  }

  setDocument(document, options = {}) {
    const visualSpace = VisualSpace.fromDocument(document, {
      validate: options.validate ?? this.options.document.validate,
      validation: options.validation,
      coordinates: options.coordinates,
    });
    this.document = visualSpace.document;
    this.visualSpace = visualSpace;
    this.scene.userData.document = this.document;
    this.scene.userData.visualSpace = this.visualSpace;
    this.emit("documentchange", {
      runtime: this,
      document: this.document,
      visualSpace: this.visualSpace,
    });
    this.emit("visualspacechange", {
      runtime: this,
      visualSpace: this.visualSpace,
    });

    if (options.views !== false) {
      this.setViewDescriptors(options.views || this.document.views || [], { source: "document" });
    } else {
      this.rebuildLayers();
      this.requestRender();
    }
    return this;
  }

  setVisualSpace(visualSpace, options = {}) {
    if (!visualSpace || typeof visualSpace !== "object" || !visualSpace.document) {
      throw new Error("setVisualSpace() requires a VisualSpace-like object with a document.");
    }
    this.visualSpace = visualSpace;
    this.document = visualSpace.document;
    this.scene.userData.document = this.document;
    this.scene.userData.visualSpace = this.visualSpace;
    this.emit("visualspacechange", {
      runtime: this,
      visualSpace: this.visualSpace,
    });

    if (options.views !== false) {
      this.setViewDescriptors(options.views || this.document.views || [], { source: "visualSpace" });
    } else {
      this.rebuildLayers();
      this.requestRender();
    }
    return this;
  }

  setViewDescriptors(views = [], options = {}) {
    this.viewDescriptors = toArray(views);
    this.layerDescriptors = this.layerDescriptorsFromViews(this.viewDescriptors);
    this.emit("viewdescriptorschange", {
      runtime: this,
      views: this.viewDescriptors,
      source: options.source || null,
    });
    this.emit("layerdescriptorschange", {
      runtime: this,
      descriptors: this.layerDescriptors,
      source: "viewDescriptors",
    });
    this.rebuildLayers();
    this.requestRender();
    return this;
  }

  setLayerDescriptors(descriptors = [], options = {}) {
    this.layerDescriptors = toArray(descriptors);
    this.emit("layerdescriptorschange", {
      runtime: this,
      descriptors: this.layerDescriptors,
      source: options.source || null,
    });
    this.rebuildLayers();
    this.requestRender();
    return this;
  }

  addLayerDescriptor(descriptor) {
    this.layerDescriptors.push(descriptor);
    this.emit("layerdescriptorschange", {
      runtime: this,
      descriptors: this.layerDescriptors,
      source: "addLayerDescriptor",
    });
    this.rebuildLayers();
    this.requestRender();
    return this;
  }

  clearLayers(options = {}) {
    this.clearLayerInstances({
      dispose: options.dispose ?? this.options.layers.disposeOnRebuild,
    });
    if (options.descriptors !== false) {
      this.layerDescriptors = [];
    }
    this.layerState = this.createLayerState("empty");
    this.emit("layerschange", {
      runtime: this,
      layers: this.layers,
      descriptors: this.layerDescriptors,
      state: this.layerState,
    });
    this.requestRender();
    return this;
  }

  setLayerFactory(factory, options = {}) {
    this.layerFactory = factory;
    this.layerState = this.createLayerState(factory ? "factory-ready" : "missing-factory");
    this.emit("layerfactorychange", {
      runtime: this,
      factory,
    });
    if (options.rebuild !== false) {
      return this.rebuildLayers();
    }
    return this;
  }

  async loadDefaultLayerFactory(options = {}) {
    if (this.layerFactory) return this.layerFactory;
    try {
      const module = await import("../layers/index.js");
      const factory = module.createLayerFromDescriptor || module.default;
      if (!factory) {
        this.warn("missing-layer-factory-export", "visual/src/layers/index.js did not export createLayerFromDescriptor.");
        return null;
      }
      this.setLayerFactory(factory, { rebuild: options.rebuild !== false });
      return factory;
    } catch (error) {
      this.warn("missing-layer-factory-module", "No layer factory is available; runtime is rendering with no layers.", {
        error,
      });
      return null;
    }
  }

  rebuildLayers() {
    this.clearLayerInstances({ dispose: this.options.layers.disposeOnRebuild });

    if (!this.layerDescriptors.length) {
      this.layerState = this.createLayerState("empty");
      this.emitLayersChange();
      return this;
    }

    if (!this.layerFactory) {
      this.layerState = this.createLayerState("missing-factory", {
        warning: "Layer descriptors are present, but no layerFactory is available.",
      });
      if (this.options.layers.warnOnMissingFactory) {
        this.warn("missing-layer-factory", this.layerState.warning);
      }
      this.emitLayersChange();
      return this;
    }

    const pending = [];
    const errors = [];
    for (const descriptor of this.layerDescriptors) {
      try {
        const result = this.createLayerFromDescriptor(descriptor);
        if (isPromiseLike(result)) {
          pending.push(result.then(
            (layer) => this.addLayerResult(layer, descriptor),
            (error) => {
              const entry = { descriptor, error };
              errors.push(entry);
              this.warn("layer-create-failed", "A layer descriptor could not be materialized.", entry);
            },
          ));
        } else {
          this.addLayerResult(result, descriptor);
        }
      } catch (error) {
        errors.push({ descriptor, error });
      }
    }

    if (pending.length) {
      this.layerState = this.createLayerState("loading", { errors });
      Promise.all(pending).then(() => {
        this.layerState = this.createLayerState(errors.length ? "partial" : "ready", { errors });
        this.emitLayersChange();
        this.requestRender();
      });
    } else {
      this.layerState = this.createLayerState(errors.length ? "partial" : "ready", { errors });
    }

    if (errors.length) {
      for (const entry of errors) {
        this.warn("layer-create-failed", "A layer descriptor could not be materialized.", entry);
      }
    }

    this.emitLayersChange();
    return this;
  }

  selectRecord(recordId, options = {}) {
    const focusTarget = options.focus === false
      ? null
      : focusTargetForRecord(this, recordId, options);
    const record = this.visualSpace?.getRecord ? this.visualSpace.getRecord(recordId) : null;
    const preview = buildRuntimeRecordPreview(this, recordId);
    const selection = {
      recordId,
      pair: null,
      record,
      preview,
      source: options.source || null,
      pickSource: options.pickSource || options.pickingSource || null,
      focusTarget,
    };
    selection.presentation = buildRuntimeSelectionPresentation(this, selection);
    this.selection = selection;
    if (focusTarget) {
      this.setFocusTarget(focusTarget, {
        source: options.source || "selectRecord",
        recordId,
        emit: false,
        render: false,
      });
    }
    this.applySelectionToLayers();
    this.emit("selectionchange", {
      runtime: this,
      selection: this.selection,
    });
    this.requestRender();
    return this;
  }

  selectPair(pair, options = {}) {
    const normalized = normalizeSelectionPair(pair);
    const preview = buildRuntimePairPreview(this, normalized);
    const selection = {
      recordId: null,
      pair: normalized,
      record: null,
      preview,
      source: options.source || null,
      pickSource: options.pickSource || options.pickingSource || normalized.pickSource || null,
      focusTarget: null,
    };
    selection.presentation = buildRuntimeSelectionPresentation(this, selection);
    this.selection = selection;
    this.applySelectionToLayers();
    this.emit("selectionchange", {
      runtime: this,
      selection: this.selection,
    });
    this.requestRender();
    return this;
  }

  clearSelection(options = {}) {
    if (options.focus !== false && this.focusTarget?.source === "record-selection") {
      this.clearFocusTarget({
        source: options.source || "clearSelection",
        emit: false,
        render: false,
      });
    }
    this.selection = {
      recordId: null,
      pair: null,
      record: null,
      preview: null,
      presentation: null,
      source: options.source || null,
      pickSource: null,
    };
    this.applySelectionToLayers();
    this.emit("selectionchange", {
      runtime: this,
      selection: this.selection,
    });
    this.requestRender();
    return this;
  }

  start() {
    this.renderer.start();
    this.emit("start", { runtime: this });
    return this;
  }

  stop() {
    this.renderer.stop();
    this.emit("stop", { runtime: this });
    return this;
  }

  renderOnce(time) {
    if (time === undefined) {
      this.renderer.loop.renderOnce();
    } else {
      this.renderer.loop.renderOnce(time);
    }
    return this;
  }

  requestRender() {
    this.renderer.requestRender();
    return this;
  }

  resize(width, height, options = {}) {
    if (width == null || height == null) {
      this.renderer.resizeToDisplaySize();
    } else {
      this.renderer.setSize(width, height, options);
    }
    return this;
  }

  setFocusLine(focusLine) {
    this.focusLine = focusLine;
    if (this.controls && typeof this.controls.setFocusLine === "function") {
      this.controls.setFocusLine(focusLine);
    }
    this.syncFocusLine();
    this.requestRender();
    return this;
  }

  setFocusOptions(options = {}, detail = {}) {
    if (!options || typeof options !== "object") return this;
    const size = this.renderer.size || {};
    const resolved = resolveRuntimeFocusLine(
      options,
      size.drawingBufferWidth || size.width || 1,
      size.drawingBufferHeight || size.height || 1,
      this.focusLine,
    );
    this.focusLine.setLine(resolved.start, resolved.end, resolved.radius, {
      axis: resolved.axis,
      focusBand: resolved.focusBand,
      blurCurve: resolved.blurCurve,
    });
    if (detail.persist !== false) {
      this.stageFocusOptions = clonePlainObject(options);
      this.options.focusLine = {
        ...this.options.focusLine,
        ...focusOptionsForRuntime(options, resolved),
      };
    }
    if (this.controls && typeof this.controls.setFocusOptions === "function") {
      this.controls.setFocusOptions({
        axis: resolved.axis,
        radius: resolved.radius,
        updateFocusOnMove: options.pointer?.enabled !== false,
      });
    }
    this.syncFocusLine();
    this.emit("focuschange", {
      runtime: this,
      focusLine: this.focusLine,
      detail: { source: detail.source || "setFocusOptions" },
    });
    this.requestRender();
    return this;
  }

  setHoverFocusOptions(options = {}, detail = {}) {
    this.hoverFocusOptions = normalizeHoverFocusOptions(options, this.hoverFocusOptions);
    this.detachHoverFocus();
    this.attachHoverFocus();
    if (detail.emit !== false) {
      this.emit("hoverfocusoptionschange", {
        runtime: this,
        options: clonePlainObject(this.hoverFocusOptions),
        detail: { source: detail.source || "setHoverFocusOptions" },
      });
    }
    return this;
  }

  setInspectionOptions(options = {}, detail = {}) {
    this.inspectionOptions = normalizeInspectionOptions(options, this.inspectionOptions);
    this.detachInspection();
    this.attachInspection();
    if (detail.emit !== false) {
      this.emit("inspectionoptionschange", {
        runtime: this,
        options: clonePlainObject(this.inspectionOptions),
        detail: { source: detail.source || "setInspectionOptions" },
      });
    }
    return this;
  }

  pickAt(input = {}, options = {}) {
    const request = createRuntimePickRequest(this, input, {
      ...this.inspectionOptions,
      ...options,
    });
    const pickOptions = { ...this.inspectionOptions, ...options };
    this.refreshPickingIndex();

    const gpuResult = pickOptions.gpu !== false ? this.pickGpu(request, pickOptions) : null;
    if (gpuResult?.hit) return gpuResult;

    const matrixResult = pickOptions.relationMatrix !== false ? this.pickRelationMatrix(request, pickOptions) : null;
    if (matrixResult?.hit) return matrixResult;

    const graphResult = pickOptions.graph !== false ? this.pickRelationGraph(request, pickOptions) : null;
    if (graphResult?.hit) return graphResult;

    const cpuResult = pickOptions.cpuFallback !== false ? this.pickCpuRecord(request, pickOptions) : null;
    return cpuResult?.hit ? cpuResult : PickResult.none(request, { source: "none" });
  }

  inspectAt(input = {}, options = {}) {
    const result = this.pickAt(input, options);
    const detail = inspectionDetailFromPickResult(result, options, this);
    if (options.select === true || (options.mode === "click" && this.inspectionOptions.selectOnClick !== false)) {
      if (result?.hit) this.applyPickSelection(result, { source: options.source || "inspection" });
      else if (options.clearOnMiss ?? this.inspectionOptions.clearOnMiss) this.clearSelection({ source: options.source || "inspection-miss" });
      this.inspectionState.selection = detail;
    } else {
      this.inspectionState.hover = detail;
    }
    this.inspectionState.lastResult = detail;
    this.inspectionState.source = detail?.source || "none";
    this.emit("inspectionchange", {
      runtime: this,
      result,
      detail,
      selection: this.selection,
    });
    return result;
  }

  pickGpu(request, options = {}) {
    if (!this.renderer?.gl || this.pickingIndex.gpuLayerCount <= 0) return null;
    this.lastGpuPickingDebug = null;
    const gl = this.renderer.gl;
    const size = this.renderer.size || {};
    const width = Math.max(1, Math.floor(size.drawingBufferWidth || 1));
    const height = Math.max(1, Math.floor(size.drawingBufferHeight || 1));
    try {
      if (!this.pickingTarget || this.pickingTarget.disposed) {
        this.pickingTarget = this.renderer.createRenderTarget({
          label: "metric-runtime-picking",
          width,
          height,
          depth: true,
          minFilter: gl.NEAREST,
          magFilter: gl.NEAREST,
        });
      } else if (this.pickingTarget.width !== width || this.pickingTarget.height !== height) {
        this.pickingTarget.setSize(width, height);
      }
      if (!this.pickingPass) {
        this.pickingPass = new PickingPass({
          registry: this.pickingRegistry,
          target: this.pickingTarget,
          width,
          height,
        });
      }
      this.pickingPass
        .setTarget(this.pickingTarget)
        .setSize(width, height)
        .setLayers(this.layers);
      this.pickingPass.render({
        ...this.renderer.context,
        runtime: this,
        renderer: this.renderer,
        gl,
        scene: this.scene,
        camera: this.camera,
        size,
      }, this.layers);
      const pickSample = readPickIdNearFramebuffer(gl, request.x, request.y, {
        width,
        height,
        yOrigin: "top",
        radius: gpuPickRadiusPixels(this, options),
      });
      if (options.debugGpuScan === "always" || (!pickSample.numericId && options.debugGpuScan === true)) {
        this.lastGpuPickingDebug = scanPickFramebuffer(gl, {
          width,
          height,
          yOrigin: "top",
          step: options.debugGpuScanStep,
        });
      }
      const numericId = pickSample.numericId;
      if (!numericId) return null;
      return pickResultFromNumericId(numericId, this.pickingRegistry, {
        request,
        source: "gpu-picking",
        raw: pickSample,
      });
    } catch (error) {
      this.warn("gpu-picking-failed", "GPU picking failed; runtime will use deterministic fallback picking.", { error });
      return null;
    } finally {
      gl.bindFramebuffer(gl.FRAMEBUFFER, null);
    }
  }

  pickRelationMatrix(request, options = {}) {
    const point = request.rawPointer || request.pointer || {};
    const cssX = finiteRuntimeNumber(point.x, request.x / positiveRuntimeNumber(this.renderer?.size?.pixelRatio, 1));
    const cssY = finiteRuntimeNumber(point.y, request.y / positiveRuntimeNumber(this.renderer?.size?.pixelRatio, 1));
    for (const entry of this.pickingIndex.relationMatrixPickers) {
      const cell = entry.picker.cellAtCanvasPoint(cssX, cssY, this.renderer?.size?.width, this.renderer?.size?.height);
      if (!cell) continue;
      return new PickResult({
        hit: true,
        kind: "edge",
        id: `${entry.relationId || "relation"}:${cell.rowId}:${cell.columnId}`,
        edgeId: `${cell.rowId}:${cell.columnId}`,
        layerId: entry.descriptorId,
        source: "relation-matrix-picking",
        request,
        raw: cell,
        point: null,
        pixel: { x: request.x, y: request.y },
        entry: {
          numericId: 0,
          kind: "edge",
          id: `${entry.relationId || "relation"}:${cell.rowId}:${cell.columnId}`,
          edgeId: `${cell.rowId}:${cell.columnId}`,
          layerId: entry.descriptorId,
          payload: {
            ...cell,
            relationId: entry.relationId,
            descriptorId: entry.descriptorId,
          },
        },
      });
    }
    return null;
  }

  pickRelationGraph(request, options = {}) {
    const threshold = positiveRuntimeNumber(options.edgeThresholdPx, this.inspectionOptions.edgeThresholdPx, 7)
      * positiveRuntimeNumber(this.renderer?.size?.pixelRatio, 1);
    const maxDistanceSquared = threshold * threshold;
    let best = null;
    const projectionA = {};
    const projectionB = {};
    this.camera?.updateMatrices?.();

    for (const edge of this.pickingIndex.graphEdges) {
      const a = projectRuntimePosition(this.camera, edge.sourcePosition, projectionA, this.renderer?.size);
      const b = projectRuntimePosition(this.camera, edge.targetPosition, projectionB, this.renderer?.size);
      if (!a?.visible || !b?.visible) continue;
      const distanceSquared = distanceToSegmentSquared(request.x, request.y, a.x, a.y, b.x, b.y);
      if (distanceSquared <= maxDistanceSquared && (!best || distanceSquared < best.distanceSquared)) {
        best = {
          edge,
          distanceSquared,
          depth: Math.min(a.depth ?? Infinity, b.depth ?? Infinity),
        };
      }
    }
    if (!best) return null;
    const edge = best.edge;
    return new PickResult({
      hit: true,
      kind: "edge",
      id: edge.edgeId,
      edgeId: edge.edgeId,
      layerId: edge.layerId,
      source: "graph-picking",
      request,
      pixel: { x: request.x, y: request.y },
      distanceSquared: best.distanceSquared,
      distance: Math.sqrt(best.distanceSquared),
      depth: best.depth,
      raw: edge,
      entry: {
        numericId: edge.numericId,
        kind: "edge",
        id: edge.edgeId,
        edgeId: edge.edgeId,
        layerId: edge.layerId,
        payload: edge,
      },
    });
  }

  pickCpuRecord(request, options = {}) {
    if (!this.pickingIndex.recordPoints.length) return null;
    return pickNearestProjectedPoint(request, this.pickingIndex.recordPoints, {
      camera: this.camera,
      registry: this.pickingRegistry,
      radius: positiveRuntimeNumber(options.thresholdPx, this.inspectionOptions.thresholdPx, 34)
        * positiveRuntimeNumber(this.renderer?.size?.pixelRatio, 1),
    });
  }

  applyPickSelection(result, options = {}) {
    if (!result?.hit) return this;
    const pair = pairFromPickResult(result);
    if (pair) {
      return this.selectPair(pair, {
        source: options.source || "inspection",
        pickSource: result.source,
      });
    }
    if (result.recordId != null) {
      return this.selectRecord(result.recordId, {
        source: options.source || "inspection",
        pickSource: result.source,
        focusTarget: result.point?.position || result.raw?.position || undefined,
      });
    }
    return this;
  }

  setFocusTarget(target, detail = {}) {
    this.focusTarget = normalizeCameraFocusTarget(target);
    this.syncFocusTarget();
    if (detail.emit !== false) {
      this.emit("focustargetchange", {
        runtime: this,
        focusTarget: this.focusTarget ? clonePlainObject(this.focusTarget) : null,
        detail: { source: detail.source || "setFocusTarget", recordId: detail.recordId ?? null },
      });
    }
    if (detail.render !== false) this.requestRender();
    return this;
  }

  clearFocusTarget(detail = {}) {
    this.focusTarget = null;
    this.syncFocusTarget();
    if (detail.emit !== false) {
      this.emit("focustargetchange", {
        runtime: this,
        focusTarget: null,
        detail: { source: detail.source || "clearFocusTarget" },
      });
    }
    if (detail.render !== false) this.requestRender();
    return this;
  }

  setCameraFocusPoint(position, detail = {}) {
    return this.setFocusTarget({
      type: "point",
      source: detail.source || "setCameraFocusPoint",
      position,
    }, detail);
  }

  setCameraFocusDistance(focusDistance, detail = {}) {
    return this.setFocusTarget({
      type: "distance",
      source: detail.source || "setCameraFocusDistance",
      focusDistance,
    }, detail);
  }

  setPostprocessEnabled(enabled) {
    this.postprocess.setEnabled(enabled);
    this.requestRender();
    return this;
  }

  setCameraOptions(options = {}) {
    if (!options || typeof options !== "object") return this;
    this.cameraState.setPerspective(options);
    if (options.up) this.cameraState.setUp(options.up);
    if (options.target || options.lookAt) {
      this.cameraState.lookAt(options.target || options.lookAt);
    }
    if (options.position) {
      this.cameraState.setPosition(options.position);
    }
    if (options.yaw !== undefined || options.pitch !== undefined || options.radius !== undefined || options.distance !== undefined) {
      this.cameraState.setOrbit(options);
    }
    this.cameraState.applyTo(this.camera);
    this.renderer.setCamera(this.camera);
    this.syncFocusTarget();
    this.emit("camerachange", {
      runtime: this,
      camera: this.camera,
      state: this.cameraState,
      detail: { source: "setCameraOptions" },
    });
    this.requestRender();
    return this;
  }

  setSceneOptions(options = {}) {
    if (!options || typeof options !== "object") return this;
    const color = options.clearColor || options.backgroundColor || options.background;
    if (color) {
      const normalized = normalizeColor(color, Array.from(this.scene.backgroundColor || DEFAULT_RUNTIME_OPTIONS.background));
      this.scene.setBackground(normalized);
      if (this.renderer.clearColor) {
        this.renderer.clearColor.set(normalized);
      }
    }
    this.emit("scenechange", {
      runtime: this,
      scene: this.scene,
      detail: { source: "setSceneOptions" },
    });
    this.requestRender();
    return this;
  }

  setPostprocessOptions(options = {}) {
    const normalized = normalizePostprocessOptions(options);
    if (this.postprocess && typeof this.postprocess.dispose === "function") {
      this.postprocess.dispose();
    }
    this.options.postprocess = normalized;
    this.postprocess = this.createPostProcessPipeline(normalized);
    this.renderer.setPipeline(this.postprocess);
    this.syncFocusLine();
    this.syncFocusTarget();
    this.emit("postprocesschange", {
      runtime: this,
      postprocess: this.postprocess,
      options: normalized,
    });
    this.requestRender();
    return this;
  }

  applyStagePreset(stage = {}) {
    if (!stage || typeof stage !== "object") return this;
    if (stage.scene) this.setSceneOptions(stage.scene);
    if (stage.camera) this.setCameraOptions(stage.camera);
    if (stage.postprocess) this.setPostprocessOptions(stage.postprocess);
    if (stage.focusTarget || stage.cameraFocus) {
      this.setFocusTarget(stage.focusTarget || stage.cameraFocus, {
        source: "applyStagePreset",
      });
    }
    if (stage.hoverFocus) this.setHoverFocusOptions(stage.hoverFocus, { source: "applyStagePreset" });
    const focusOptions = stage.focus || stage.interaction?.focusLine || stage.postprocess?.tiltShift;
    if (focusOptions) this.setFocusOptions(focusOptions, { source: "applyStagePreset" });
    this.emit("stagechange", {
      runtime: this,
      stage,
    });
    this.requestRender();
    return this;
  }

  getState() {
    return {
      running: this.renderer.loop.running,
      disposed: this.disposed,
      hasDocument: Boolean(this.document),
      selectedRecordId: this.selection.recordId,
      selectedRecord: this.selection.record ? clonePlainObject(this.selection.record) : null,
      selectedRecordPreview: this.selection.recordId != null && this.selection.preview
        ? clonePlainObject(this.selection.preview)
        : null,
      selectedPair: this.selection.pair ? clonePlainObject(this.selection.pair) : null,
      selectedPairPreview: this.selection.pair && this.selection.preview
        ? clonePlainObject(this.selection.preview)
        : null,
      selectionPreview: this.selection.preview ? clonePlainObject(this.selection.preview) : null,
      selectionPresentation: this.selection.presentation ? clonePlainObject(this.selection.presentation) : null,
      selectionFeatures: this.selection.presentation?.features ? clonePlainObject(this.selection.presentation.features) : [],
      selectionSource: this.selection.source || null,
      selectionPickSource: this.selection.pickSource || null,
      focusTarget: this.focusTarget ? clonePlainObject(this.focusTarget) : null,
      inspection: clonePlainObject({
        enabled: this.inspectionOptions.enabled === true,
        source: this.inspectionState.source,
        hover: this.inspectionState.hover,
        selection: this.inspectionState.selection,
        lastResult: this.inspectionState.lastResult,
        availableSources: this.pickingIndex.availableSources,
        gpuPicking: {
          available: this.pickingIndex.gpuLayerCount > 0,
          layerCount: this.pickingIndex.gpuLayerCount,
        },
        relationMatrixPicking: {
          available: this.pickingIndex.relationMatrixPickers.length > 0,
          layerCount: this.pickingIndex.relationMatrixPickers.length,
        },
        graphPicking: {
          available: this.pickingIndex.graphEdges.length > 0,
          edgeCount: this.pickingIndex.graphEdges.length,
        },
        cpuFallback: {
          available: this.pickingIndex.recordPoints.length > 0,
          candidateCount: this.pickingIndex.recordPoints.length,
        },
        runtimeStateKeys: {
          selectedRecord: "selectedRecord",
          selectedRecordId: "selectedRecordId",
          selectedRecordPreview: "selectedRecordPreview",
          selectedPair: "selectedPair",
          selectedPairPreview: "selectedPairPreview",
          selectionPreview: "selectionPreview",
          selectionPresentation: "selectionPresentation",
          selectionFeatures: "selectionFeatures",
        },
      }),
      hoverFocus: {
        enabled: this.hoverFocusOptions.enabled === true,
        target: this.hoverFocusState.target ? clonePlainObject(this.hoverFocusState.target) : null,
      },
      viewCount: this.viewDescriptors.length,
      layerDescriptorCount: this.layerDescriptors.length,
      layerInstanceCount: this.layers.length,
      layerState: { ...this.layerState },
      renderPhases: collectRuntimeRenderPhases(this.layers),
      layerDiagnostics: collectRuntimeLayerDiagnostics(this.layers),
      postprocess: this.postprocess?.getState?.() || null,
      warnings: this.warnings.slice(),
    };
  }

  dispose(options = {}) {
    if (this.disposed) return this;
    this.stop();
    if (this.unsubscribeRendererResize) {
      this.unsubscribeRendererResize();
      this.unsubscribeRendererResize = null;
    }
    if (this.unsubscribeRendererAfterRender) {
      this.unsubscribeRendererAfterRender();
      this.unsubscribeRendererAfterRender = null;
    }
    if (this.controls && typeof this.controls.dispose === "function") {
      this.controls.dispose();
    }
    this.detachHoverFocus();
    this.detachInspection();
    this.clearLayerInstances({ dispose: options.disposeLayers !== false });
    this.disposePickingResources();
    this.events.clear();
    this.renderer.dispose({
      disposeScene: options.disposeScene !== false,
      disposePipeline: options.disposePipeline !== false,
      loseContext: options.loseContext,
    });
    this.disposed = true;
    return this;
  }

  disposePickingResources() {
    if (this.pickingTarget && typeof this.pickingTarget.dispose === "function") {
      this.pickingTarget.dispose();
    }
    if (this.pickingRegistry && typeof this.pickingRegistry.clear === "function") {
      this.pickingRegistry.clear();
    }
    this.pickingPass = null;
    this.pickingTarget = null;
    this.pickingIndex = createEmptyPickingIndex();
    return this;
  }

  createPostProcessPipeline(overrideOptions) {
    const options = normalizePostprocessOptions(overrideOptions ?? this.options.postprocess);
    try {
      return new RuntimePostProcessPipeline(this.renderer.gl, options);
    } catch (error) {
      this.warn("postprocess-create-failed", "Postprocess pipeline could not be created; rendering will continue without postprocess.", {
        error,
      });
      return {
        enabled: false,
        get length() {
          return 0;
        },
        setSize() {
          return this;
        },
        setFocusState() {
          return this;
        },
        setFocusTarget() {
          return this;
        },
        clearFocusTarget() {
          return this;
        },
        setCameraFocusPoint() {
          return this;
        },
        setCameraFocusDistance() {
          return this;
        },
        setEnabled(enabled) {
          this.enabled = Boolean(enabled);
          return this;
        },
        render(context) {
          context.renderer.renderScene(null);
          return null;
        },
        dispose() {},
      };
    }
  }

  createControls() {
    const options = normalizeBooleanOptions(this.options.controls, DEFAULT_RUNTIME_OPTIONS.controls);
    if (options.enabled === false) return null;
    return createCameraControls({
      ...options,
      element: this.canvas,
      camera: this.camera,
      state: this.cameraState,
      focusLine: this.focusLine,
      focusAxis: this.options.focusLine.axis,
      focusRadius: this.options.focusLine.radius,
      autoAttach: options.autoAttach !== false,
      onChange: (state, detail) => {
        if (typeof options.onChange === "function") options.onChange(state, detail);
        this.emit("camerachange", { runtime: this, camera: this.camera, state, detail });
        this.requestRender();
      },
      onFocusChange: (focusLine, pointer) => {
        this.syncFocusLine();
        if (typeof options.onFocusChange === "function") options.onFocusChange(focusLine, pointer);
        this.emit("focuschange", { runtime: this, focusLine, pointer });
        this.requestRender();
      },
    });
  }

  createFocusLineOptions() {
    const size = this.renderer.size;
    const width = size.drawingBufferWidth || 1;
    const height = size.drawingBufferHeight || 1;
    const options = this.options.focusLine || {};
    const resolved = resolveRuntimeFocusLine(options, width, height, null);
    return {
      ...options,
      width,
      height,
      start: resolved.start,
      end: resolved.end,
      radius: resolved.radius ?? Math.max(1, height * (options.radiusRatio ?? DEFAULT_RUNTIME_OPTIONS.focusLine.radiusRatio)),
      focusBand: resolved.focusBand,
      blurCurve: resolved.blurCurve,
      axis: resolved.axis,
    };
  }

  handleResize(size) {
    if (this.focusLine && typeof this.focusLine.setSize === "function") {
      this.focusLine.setSize(size.drawingBufferWidth, size.drawingBufferHeight);
    }
    if (this.controls && typeof this.controls.setSize === "function") {
      this.controls.setSize(size.width, size.height, size.pixelRatio);
    }
    if (this.stageFocusOptions) {
      const resolved = resolveRuntimeFocusLine(
        this.stageFocusOptions,
        size.drawingBufferWidth || size.width || 1,
        size.drawingBufferHeight || size.height || 1,
        this.focusLine,
      );
      this.focusLine.setLine(resolved.start, resolved.end, resolved.radius, {
        axis: resolved.axis,
        focusBand: resolved.focusBand,
        blurCurve: resolved.blurCurve,
      });
      if (this.controls && typeof this.controls.setFocusOptions === "function") {
        this.controls.setFocusOptions({
          axis: resolved.axis,
          radius: resolved.radius,
          updateFocusOnMove: this.stageFocusOptions.pointer?.enabled !== false,
        });
      }
    }
    this.syncFocusLine();
    this.emit("resize", {
      runtime: this,
      size,
    });
  }

  syncFocusLine() {
    if (this.postprocess && typeof this.postprocess.setFocusState === "function") {
      this.postprocess.setFocusState(this.focusLine);
    }
    return this;
  }

  syncFocusTarget() {
    if (this.postprocess && typeof this.postprocess.setFocusTarget === "function") {
      if (this.focusTarget) {
        this.postprocess.setFocusTarget(this.focusTarget);
      } else if (typeof this.postprocess.clearFocusTarget === "function") {
        this.postprocess.clearFocusTarget();
      } else {
        this.postprocess.setFocusTarget(null);
      }
    }
    return this;
  }

  handleAfterRender(context) {
    const postprocessState = this.postprocess?.getState?.() || null;
    const focusDistance = finiteRuntimeNumber(postprocessState?.focusDistance);
    const focusTarget = this.focusTarget ? clonePlainObject(this.focusTarget) : null;
    const previousDistance = this.cameraFocusState.focusDistance;
    const previousTarget = this.cameraFocusState.focusTarget;
    const changed = numericValueChanged(previousDistance, focusDistance, 1e-6)
      || !plainObjectEqual(previousTarget, focusTarget);
    if (!changed) return this;
    this.cameraFocusState = {
      focusTarget: focusTarget ? clonePlainObject(focusTarget) : null,
      focusDistance,
    };
    this.emit("camerafocuschange", {
      runtime: this,
      focusTarget,
      focusDistance,
      postprocess: postprocessState,
      context,
    });
    return this;
  }

  attachHoverFocus() {
    if (this.unsubscribeHoverFocus || this.hoverFocusOptions.enabled !== true || !this.renderer?.on) return this;
    const offPointer = this.renderer.on("pointer", ({ pointer, sourceEvent }) => {
      this.handleHoverFocus(pointer, sourceEvent);
    });
    const offLeave = this.renderer.on("pointerleave", ({ pointer, sourceEvent }) => {
      this.handleHoverFocusLeave(pointer, sourceEvent);
    });
    this.unsubscribeHoverFocus = () => {
      offPointer?.();
      offLeave?.();
    };
    return this;
  }

  detachHoverFocus() {
    if (this.unsubscribeHoverFocus) {
      this.unsubscribeHoverFocus();
      this.unsubscribeHoverFocus = null;
    }
    return this;
  }

  handleHoverFocus(pointer, sourceEvent) {
    const options = this.hoverFocusOptions;
    if (options.enabled !== true || !pointer?.inside) return this;
    if (this.controls?.dragging && options.focusWhileDragging !== true) return this;
    const now = performanceNow();
    if (now - this.hoverFocusState.lastAt < options.throttleMs) return this;
    this.hoverFocusState.lastAt = now;
    const target = resolveHoverFocusTarget(this, pointer, options, sourceEvent);
    if (target) {
      const focusTarget = resolveStableHoverFocusTarget(this, target, options);
      this.hoverFocusState.target = focusTarget;
      this.setFocusTarget(focusTarget, {
        source: "hoverFocus",
        emit: false,
      });
      this.emit("hoverfocuschange", {
        runtime: this,
        focusTarget: clonePlainObject(focusTarget),
        pointer: { ...pointer },
      });
    } else if (options.clearOnMiss === true && this.focusTarget?.source === "hover-focus") {
      this.hoverFocusState.target = null;
      this.clearFocusTarget({ source: "hoverFocus", emit: false });
      this.emit("hoverfocuschange", { runtime: this, focusTarget: null, pointer: { ...pointer } });
    }
    return this;
  }

  handleHoverFocusLeave(pointer, sourceEvent) {
    if (this.hoverFocusOptions.clearOnLeave === true && this.focusTarget?.source === "hover-focus") {
      this.hoverFocusState.target = null;
      this.clearFocusTarget({ source: "hoverFocusLeave", emit: false });
      this.emit("hoverfocuschange", { runtime: this, focusTarget: null, pointer: pointer ? { ...pointer } : null, sourceEvent });
    }
    return this;
  }

  attachInspection() {
    if (this.unsubscribeInspection || this.inspectionOptions.enabled !== true || !this.renderer?.on) return this;
    const offPointer = this.inspectionOptions.hover === false ? null : this.renderer.on("pointer", ({ pointer, sourceEvent }) => {
      this.handleInspectionPointer(pointer, sourceEvent, { mode: "hover", select: false, source: "runtime-hover" });
    });
    const offUp = this.inspectionOptions.click === false ? null : this.renderer.on("pointerup", ({ pointer, sourceEvent }) => {
      this.handleInspectionPointer(pointer, sourceEvent, { mode: "click", select: this.inspectionOptions.selectOnClick !== false, source: "runtime-click" });
    });
    const offLeave = this.renderer.on("pointerleave", ({ pointer, sourceEvent }) => {
      this.handleInspectionLeave(pointer, sourceEvent);
    });
    this.unsubscribeInspection = () => {
      offPointer?.();
      offUp?.();
      offLeave?.();
    };
    return this;
  }

  detachInspection() {
    if (this.unsubscribeInspection) {
      this.unsubscribeInspection();
      this.unsubscribeInspection = null;
    }
    return this;
  }

  handleInspectionPointer(pointer, sourceEvent, detail = {}) {
    if (this.inspectionOptions.enabled !== true || !pointer?.inside) return this;
    if (detail.mode === "click" && sourceEvent?.button != null && sourceEvent.button !== 0) return this;
    if (this.controls?.dragging && detail.mode !== "click") return this;
    this.inspectAt(pointer, {
      ...detail,
      sourceEvent,
      rawPointer: pointer,
    });
    return this;
  }

  handleInspectionLeave(pointer, sourceEvent) {
    if (this.inspectionOptions.clearOnMiss === true) {
      this.inspectionState.hover = null;
      this.inspectionState.lastResult = null;
      this.inspectionState.source = "none";
      this.emit("inspectionchange", { runtime: this, result: null, detail: null, pointer, sourceEvent });
    }
    return this;
  }

  layerDescriptorsFromViews(views) {
    const descriptors = [];
    for (const view of views) {
      if (!view) continue;
      if (typeof view.toLayerDescriptors === "function") {
        descriptors.push(...toArray(view.toLayerDescriptors({
          runtime: this,
          document: this.document,
          visualSpace: this.visualSpace,
        })));
      } else if (Array.isArray(view.layerDescriptors)) {
        descriptors.push(...view.layerDescriptors);
      } else if (Array.isArray(view.layers)) {
        descriptors.push(...view.layers);
      } else if (looksLikeLayerDescriptor(view)) {
        descriptors.push(view);
      }
    }
    return descriptors;
  }

  createLayerFromDescriptor(descriptor) {
    const factory = this.layerFactory;
    const context = {
      runtime: this,
      renderer: this.renderer,
      gl: this.renderer.gl,
      scene: this.scene,
      camera: this.camera,
      document: this.document,
      visualSpace: this.visualSpace,
      selection: this.selection,
    };

    if (typeof factory === "function") {
      return factory(descriptor, context);
    }
    if (factory && typeof factory.createLayerFromDescriptor === "function") {
      return factory.createLayerFromDescriptor(descriptor, context);
    }
    throw new Error("Layer factory must be a function or expose createLayerFromDescriptor().");
  }

  addLayerResult(result, descriptor) {
    for (const layer of toArray(result)) {
      if (!layer) continue;
      layer.descriptor = layer.descriptor || descriptor;
      this.layers.push(layer);
      this.scene.add(layer);
    }
    this.applySelectionToLayers();
    this.refreshPickingIndex();
    return this;
  }

  clearLayerInstances(options = {}) {
    const layers = this.layers.slice();
    this.layers.length = 0;
    for (const layer of layers) {
      this.scene.remove(layer);
      if (options.dispose !== false) {
        disposeLayer(layer);
      }
    }
    return this;
  }

  refreshPickingIndex() {
    const next = createEmptyPickingIndex();
    this.pickingRegistry.clear();

    for (const descriptor of this.layerDescriptors || []) {
      collectDescriptorRecordPickPoints(next, this.pickingRegistry, descriptor, this);
      collectDescriptorRelationMatrixPickers(next, descriptor, this);
      collectDescriptorGraphEdges(next, this.pickingRegistry, descriptor);
    }

    next.gpuLayerCount = (this.layers || []).filter((layer) => (
      layer
      && layer.visible !== false
      && layer.enabled !== false
      && (typeof layer.renderPicking === "function" || typeof layer.renderPickIds === "function")
    )).length;
    if (next.gpuLayerCount > 0) next.availableSources.push("gpu-picking");
    if (next.relationMatrixPickers.length > 0) next.availableSources.push("relation-matrix-picking");
    if (next.graphEdges.length > 0) next.availableSources.push("graph-picking");
    if (next.recordPoints.length > 0) next.availableSources.push("cpu-fallback");
    this.pickingIndex = next;
    return this;
  }

  applySelectionToLayers() {
    for (const layer of this.layers) {
      if (typeof layer.setSelection === "function") {
        layer.setSelection(this.selection, { runtime: this });
      } else if (this.selection.recordId != null && typeof layer.selectRecord === "function") {
        layer.selectRecord(this.selection.recordId, { runtime: this });
      } else if (this.selection.recordId == null && typeof layer.clearSelection === "function") {
        layer.clearSelection({ runtime: this });
      }
    }
    return this;
  }

  createLayerState(status, extras = {}) {
    return {
      status,
      descriptors: this.layerDescriptors.length,
      instances: this.layers.length,
      warning: extras.warning || null,
      errors: extras.errors || [],
    };
  }

  emitLayersChange() {
    this.emit("layerschange", {
      runtime: this,
      layers: this.layers,
      descriptors: this.layerDescriptors,
      state: this.layerState,
    });
    return this;
  }

  warn(code, message, detail = {}) {
    const warning = {
      code,
      message,
      detail,
    };
    this.warnings.push(warning);
    this.emit("warning", {
      runtime: this,
      warning,
    });
    return warning;
  }
}

export const VisualEngineRuntime = MetricVisualRuntime;

export function createMetricVisualRuntime(options = {}) {
  return new MetricVisualRuntime(options);
}

export function createVisualEngineRuntime(options = {}) {
  return new VisualEngineRuntime(options);
}

function mergeRuntimeOptions(options) {
  const merged = {
    ...DEFAULT_RUNTIME_OPTIONS,
    ...options,
    document: {
      ...DEFAULT_RUNTIME_OPTIONS.document,
      ...(options.document || {}),
    },
    renderer: {
      ...DEFAULT_RUNTIME_OPTIONS.renderer,
      ...(options.renderer || {}),
    },
    camera: {
      ...DEFAULT_RUNTIME_OPTIONS.camera,
      ...(options.camera || {}),
      mode: "miniaturePerspective",
    },
    focusLine: {
      ...DEFAULT_RUNTIME_OPTIONS.focusLine,
      ...(options.focusLine || {}),
    },
    postprocess: normalizePostprocessOptions(options.postprocess ?? DEFAULT_RUNTIME_OPTIONS.postprocess),
    controls: normalizeBooleanOptions(options.controls, DEFAULT_RUNTIME_OPTIONS.controls),
    hoverFocus: normalizeHoverFocusOptions(options.hoverFocus ?? options.controls?.hoverFocus, DEFAULT_RUNTIME_OPTIONS.hoverFocus),
    inspection: normalizeInspectionOptions(options.inspection ?? options.picking, DEFAULT_RUNTIME_OPTIONS.inspection),
    layers: {
      ...DEFAULT_RUNTIME_OPTIONS.layers,
      ...(options.layers || {}),
    },
  };
  if (options.layerFactory) {
    merged.layerFactory = options.layerFactory;
  }
  return merged;
}

function normalizeBooleanOptions(value, defaults) {
  if (value === false) return { ...defaults, enabled: false };
  if (value === true || value == null) return { ...defaults };
  return { ...defaults, ...value };
}

function normalizeHoverFocusOptions(value, fallback = DEFAULT_RUNTIME_OPTIONS.hoverFocus) {
  const base = fallback || DEFAULT_RUNTIME_OPTIONS.hoverFocus;
  if (value === false) return { ...base, enabled: false };
  if (value === true) return { ...base, enabled: true };
  const source = value && typeof value === "object" ? value : {};
  const thresholdPx = positiveRuntimeNumber(source.thresholdPx, source.radiusPx, base.thresholdPx, 42);
  const throttleMs = Math.max(0, numberOr(source.throttleMs, base.throttleMs ?? 34));
  const maxCandidates = Math.max(1, Math.floor(numberOr(source.maxCandidates, base.maxCandidates ?? 12000)));
  return {
    ...base,
    ...source,
    enabled: source.enabled === true || base.enabled === true,
    thresholdPx,
    throttleMs,
    maxCandidates,
    smoothDistance: source.smoothDistance === true || source.focusMode === "distance",
    smoothingAlpha: clampRuntimeNumber(source.smoothingAlpha ?? source.focusSmoothing ?? base.smoothingAlpha, 0.01, 1, 0.18),
    clearOnLeave: source.clearOnLeave ?? base.clearOnLeave ?? false,
    clearOnMiss: source.clearOnMiss ?? base.clearOnMiss ?? false,
    focusWhileDragging: source.focusWhileDragging ?? base.focusWhileDragging ?? false,
    positionChannels: toArray(source.positionChannels || source.positionChannel || base.positionChannels || ["position", "targetPosition", "sourcePosition"]),
    recordChannel: source.recordChannel || base.recordChannel || "recordId",
  };
}

function normalizeInspectionOptions(value, fallback = DEFAULT_RUNTIME_OPTIONS.inspection) {
  const base = fallback || DEFAULT_RUNTIME_OPTIONS.inspection;
  if (value === false) return { ...base, enabled: false };
  if (value === true) return { ...base, enabled: true };
  const source = value && typeof value === "object" ? value : {};
  return {
    ...base,
    ...source,
    enabled: source.enabled !== false && base.enabled !== false,
    hover: source.hover !== false && source.hoverPreview !== false && base.hover !== false,
    click: source.click !== false && source.clickSelect !== false && base.click !== false,
    gpu: source.gpu !== false && source.gpuPicking !== false && base.gpu !== false,
    relationMatrix: source.relationMatrix !== false && source.matrix !== false && base.relationMatrix !== false,
    graph: source.graph !== false && source.graphPicking !== false && base.graph !== false,
    cpuFallback: source.cpuFallback !== false && source.cpu !== false && base.cpuFallback !== false,
    thresholdPx: positiveRuntimeNumber(source.thresholdPx, source.radiusPx, base.thresholdPx, 34),
    gpuRadiusPx: Math.max(0, finiteRuntimeNumber(source.gpuRadiusPx, source.gpuRadius, base.gpuRadiusPx, 6)),
    edgeThresholdPx: positiveRuntimeNumber(source.edgeThresholdPx, source.edgeRadiusPx, base.edgeThresholdPx, 7),
    maxCandidates: Math.max(1, Math.floor(numberOr(source.maxCandidates, base.maxCandidates ?? 20000))),
    clearOnMiss: source.clearOnMiss ?? base.clearOnMiss ?? false,
    selectOnClick: source.selectOnClick ?? source.clickSelect ?? base.selectOnClick ?? true,
    recordChannel: source.recordChannel || base.recordChannel || "recordId",
    positionChannels: toArray(source.positionChannels || source.positionChannel || base.positionChannels || ["position", "targetPosition", "sourcePosition"]),
  };
}

function resolveStableHoverFocusTarget(runtime, target, options = {}) {
  if (options.smoothDistance !== true) return target;
  const rawDistance = resolveCameraFocusDistance(target, runtime.camera);
  if (!(rawDistance > 0)) return target;
  const previous = finiteRuntimeNumber(
    runtime.hoverFocusState?.focusDistance,
    runtime.lastFocusDistance,
    rawDistance,
  );
  const alpha = clampRuntimeNumber(options.smoothingAlpha, 0.01, 1, 0.18);
  const focusDistance = previous + (rawDistance - previous) * alpha;
  if (runtime.hoverFocusState) runtime.hoverFocusState.focusDistance = focusDistance;
  return {
    ...target,
    type: "distance",
    source: target.source || "hover-focus",
    focusDistance,
    rawFocusDistance: rawDistance,
    position: target.position,
  };
}

function normalizePostprocessOptions(value) {
  const source = value && typeof value === "object" ? value : {};
  const raw = normalizeBooleanOptions(value, DEFAULT_RUNTIME_OPTIONS.postprocess);
  const explicitPostFxFalse = value && typeof value === "object" && value.postFx === false;
  const postFx = normalizeRuntimePostFxSource(raw, explicitPostFxFalse);
  const tiltShift = normalizeBooleanOptions(raw.tiltShift, {
    enabled: raw.tiltShift !== false,
    blurRadius: raw.blurRadius,
    gradientRadius: raw.gradientRadius,
    focusBand: raw.focusBand,
    blurCurve: raw.blurCurve,
    start: raw.start,
    end: raw.end,
  });
  const fxaa = normalizeBooleanOptions(raw.fxaa, {
    enabled: raw.fxaa === true,
    strength: raw.fxaaStrength,
    edgeThreshold: raw.edgeThreshold,
    edgeThresholdMin: raw.edgeThresholdMin,
  });
  return {
    ...raw,
    tiltShift: tiltShift.enabled !== false,
    blurRadius: own(source, "blurRadius") ? raw.blurRadius : (tiltShift.blurRadius ?? raw.blurRadius),
    gradientRadius: own(source, "gradientRadius") ? raw.gradientRadius : (tiltShift.gradientRadius ?? raw.gradientRadius),
    focusBand: own(source, "focusBand") ? raw.focusBand : (tiltShift.focusBand ?? raw.focusBand),
    blurCurve: own(source, "blurCurve") ? raw.blurCurve : (tiltShift.blurCurve ?? raw.blurCurve),
    start: own(source, "start") ? raw.start : (tiltShift.start ?? raw.start),
    end: own(source, "end") ? raw.end : (tiltShift.end ?? raw.end),
    fxaa: fxaa.enabled === true,
    fxaaStrength: raw.fxaaStrength ?? fxaa.strength,
    edgeThreshold: raw.edgeThreshold ?? fxaa.edgeThreshold,
    edgeThresholdMin: raw.edgeThresholdMin ?? fxaa.edgeThresholdMin,
    postFx,
  };
}

function normalizeRuntimePostFxSource(raw = {}, explicitPostFxFalse = false) {
  if (explicitPostFxFalse) return false;
  if (raw.postFx === true) return true;
  if (raw.postFx && typeof raw.postFx === "object") {
    return {
      ...raw.postFx,
      bloom: raw.postFx.bloom || raw.bloom,
      miniatureFrame: raw.postFx.miniatureFrame || raw.miniatureFrame,
      cameraDof: raw.postFx.cameraDof || raw.cameraDof || raw.depthOfField,
      grade: raw.postFx.grade || raw.postFx.colorGrade || raw.grade || raw.colorGrade,
      vignette: raw.postFx.vignette || raw.vignette,
      filmGrain: raw.postFx.filmGrain || raw.postFx.film || raw.filmGrain || raw.film,
    };
  }

  const hasPostFxInput = Boolean(raw.colorGrade || raw.grade || raw.bloom || raw.vignette || raw.filmGrain || raw.film || raw.cameraDof || raw.depthOfField);
  if (!hasPostFxInput) return false;
  return {
    enabled: true,
    bloom: raw.bloom || { enabled: false },
    miniatureFrame: raw.miniatureFrame || { enabled: true },
    cameraDof: raw.cameraDof || raw.depthOfField || { enabled: false },
    grade: raw.grade || raw.colorGrade || { enabled: true },
    vignette: raw.vignette || { enabled: true, amount: 0.1, radius: 0.78 },
    filmGrain: raw.filmGrain || raw.film || { enabled: false },
  };
}

function normalizePostFxOptions(value) {
  if (value === true) {
    return {
      bloom: { enabled: false },
      miniatureFrame: { enabled: true },
      cameraDof: { enabled: false },
      grade: { enabled: true },
      vignette: { enabled: true, amount: 0.1, radius: 0.78 },
      filmGrain: { enabled: false },
    };
  }
  if (!value || typeof value !== "object") return { enabled: false };
  return {
    ...value,
    bloom: normalizeRuntimeBloomOptions(value.bloom || { enabled: false }),
    miniatureFrame: value.miniatureFrame || { enabled: true },
    cameraDof: value.cameraDof || value.depthOfField || { enabled: false },
    grade: value.grade || value.colorGrade || { enabled: true },
    vignette: value.vignette || { enabled: true, amount: 0.1, radius: 0.78 },
    filmGrain: value.filmGrain || value.film || { enabled: false },
  };
}

function normalizeRuntimeBloomOptions(value) {
  if (value === false) return { enabled: false };
  if (value === true) return { enabled: true };
  if (!value || typeof value !== "object") return { enabled: false };
  const intensity = numberOr(value.intensity, value.strength);
  return Number.isFinite(intensity)
    ? { ...value, intensity, strength: intensity }
    : { ...value };
}

function resolveRuntimeFocusLine(options = {}, width = 1, height = 1, fallback = null) {
  const axis = options.axis === "vertical" ? "vertical" : "horizontal";
  const pixelWidth = Math.max(1, numberOr(width, 1));
  const pixelHeight = Math.max(1, numberOr(height, 1));
  const radius = options.anchor === "viewport-ratio"
    ? positiveRuntimeNumber(
      numberOr(options.radiusRatio, NaN) * pixelHeight,
      pixelHeight / 3,
    )
    : positiveRuntimeNumber(
      options.radius,
      options.gradientRadius,
      numberOr(options.radiusRatio, NaN) * pixelHeight,
      fallback?.radius,
      pixelHeight / 3,
    );
  const focusBandRatio = Number(options.focusBandRatio);
  const focusBandFromRatio = Number.isFinite(focusBandRatio)
    ? Math.max(0, focusBandRatio * pixelHeight)
    : undefined;
  const focusBand = options.anchor === "viewport-ratio" && focusBandFromRatio !== undefined
    ? focusBandFromRatio
    : Math.max(0, numberOr(options.focusBand, focusBandFromRatio ?? fallback?.focusBand ?? 0));
  const blurCurve = Math.max(0.05, numberOr(options.blurCurve, fallback?.blurCurve ?? 1));
  if (options.anchor !== "viewport-ratio" && (options.start || options.end)) {
    return {
      axis,
      radius,
      focusBand,
      blurCurve,
      start: normalizePointObject(options.start, fallback?.start || (axis === "vertical" ? { x: pixelWidth * 0.5, y: 0 } : { x: 0, y: pixelHeight * 0.5 })),
      end: normalizePointObject(options.end, fallback?.end || (axis === "vertical" ? { x: pixelWidth * 0.5, y: pixelHeight } : { x: pixelWidth, y: pixelHeight * 0.5 })),
    };
  }
  if (axis === "vertical") {
    const x = clampRuntimeNumber(
      numberOr(options.x, numberOr(options.xRatio, 0.5) * pixelWidth),
      0,
      pixelWidth,
    );
    return { axis, radius, focusBand, blurCurve, start: { x, y: 0 }, end: { x, y: pixelHeight } };
  }
  const y = clampRuntimeNumber(
    numberOr(options.y, numberOr(options.focusY, numberOr(options.yRatio, 0.5) * pixelHeight)),
    0,
    pixelHeight,
  );
  return { axis, radius, focusBand, blurCurve, start: { x: 0, y }, end: { x: pixelWidth, y } };
}

function normalizePointObject(value, fallback) {
  if (Array.isArray(value) || ArrayBuffer.isView(value)) {
    return { x: numberOr(value[0], fallback.x), y: numberOr(value[1], fallback.y) };
  }
  if (value && typeof value === "object") {
    return { x: numberOr(value.x, fallback.x), y: numberOr(value.y, fallback.y) };
  }
  return { x: fallback.x, y: fallback.y };
}

function focusOptionsForRuntime(options, resolved) {
  return {
    axis: resolved.axis,
    radius: resolved.radius,
    focusBand: resolved.focusBand,
    blurCurve: resolved.blurCurve,
    radiusRatio: options.radiusRatio,
    focusBandRatio: options.focusBandRatio,
    xRatio: options.xRatio,
    yRatio: options.yRatio,
  };
}

function resolveHoverFocusTarget(runtime, pointer, options = {}, sourceEvent = null) {
  if (typeof options.resolver === "function") {
    const resolved = options.resolver({ runtime, pointer, options, sourceEvent });
    return normalizeCameraFocusTarget(resolved);
  }
  return nearestDescriptorFocusTarget(runtime, pointer, options);
}

function nearestDescriptorFocusTarget(runtime, pointer, options = {}) {
  const camera = runtime.camera;
  const descriptors = Array.isArray(runtime.layerDescriptors) ? runtime.layerDescriptors : [];
  const size = runtime.renderer?.size || {};
  const width = Math.max(1, numberOr(size.drawingBufferWidth, camera?.viewport?.pixelWidth, 1));
  const height = Math.max(1, numberOr(size.drawingBufferHeight, camera?.viewport?.pixelHeight, 1));
  const x = numberOr(pointer?.uvX, 0.5) * width;
  const y = numberOr(pointer?.uvY, 0.5) * height;
  const threshold = positiveRuntimeNumber(options.thresholdPx, 42) * positiveRuntimeNumber(size.pixelRatio, 1);
  const thresholdSq = threshold * threshold;
  const positionNames = toArray(options.positionChannels || options.positionChannel || ["position", "targetPosition", "sourcePosition"]);
  const recordNames = toArray(options.recordChannel || ["recordId", "record_id", "id"]);
  const projection = {};
  let best = null;
  let visited = 0;

  camera?.updateMatrices?.();

  for (let descriptorIndex = descriptors.length - 1; descriptorIndex >= 0; descriptorIndex -= 1) {
    const descriptor = descriptors[descriptorIndex];
    const channels = descriptor?.channels || {};
    const recordChannel = getChannel(channels, recordNames);
    const recordIds = getChannelArray(recordChannel);
    if (!recordIds?.length) continue;
    const positionInfo = descriptorFocusPositionInfo(descriptor, positionNames, runtime);
    if (!positionInfo) continue;
    const count = Math.min(recordIds.length, positionInfo.count);
    for (let index = 0; index < count; index += 1) {
      if (visited++ > options.maxCandidates) break;
      const position = positionAt(positionInfo, index);
      if (!position) continue;
      const screen = camera?.projectToPixel
        ? camera.projectToPixel(position, projection)
        : projectWithViewProjection(projection, position, camera?.viewProjectionMatrix, width, height);
      if (!screen?.visible) continue;
      const dx = screen.x - x;
      const dy = screen.y - y;
      const distanceSq = dx * dx + dy * dy;
      if (distanceSq <= thresholdSq && (!best || distanceSq < best.distanceSq || screen.depth < best.depth)) {
        best = {
          distanceSq,
          depth: screen.depth,
          descriptor,
          index,
          recordId: recordIds[index],
          position: [position[0], position[1], position[2]],
        };
      }
    }
    if (visited > options.maxCandidates) break;
  }

  if (!best) return null;
  return {
    type: "point",
    source: "hover-focus",
    recordId: best.recordId,
    descriptorId: best.descriptor.id || null,
    descriptorKind: best.descriptor.kind || best.descriptor.primitive || null,
    position: best.position,
    screenDistancePx: Math.sqrt(best.distanceSq) / positiveRuntimeNumber(size.pixelRatio, 1),
  };
}

function descriptorFocusPositionInfo(descriptor, positionNames, runtime = null) {
  const channels = descriptor?.channels || {};
  const position = getChannel(channels, "position");
  const target = getChannel(channels, "targetPosition");
  if (position && target && descriptorUsesTargetPositionMorph(descriptor)) {
    const sourceArray = getChannelArray(position);
    const targetArray = getChannelArray(target);
    if (sourceArray && targetArray) {
      const itemSize = Math.max(3, getChannelItemSize(position, 3), getChannelItemSize(target, 3));
      return {
        mode: "morph",
        sourceArray,
        targetArray,
        sourceItemSize: getChannelItemSize(position, 3),
        targetItemSize: getChannelItemSize(target, 3),
        count: Math.min(getChannelCount(position, 3), getChannelCount(target, 3)),
        morph: animationProgressForDescriptor(descriptor.animation, runtime),
        itemSize,
      };
    }
  }

  for (const name of positionNames) {
    const channel = getChannel(channels, name);
    const array = getChannelArray(channel);
    if (!array) continue;
    return {
      mode: "static",
      array,
      itemSize: getChannelItemSize(channel, 3),
      count: getChannelCount(channel, 3),
    };
  }
  return null;
}

function descriptorUsesTargetPositionMorph(descriptor) {
  const animation = descriptor?.animation || {};
  const mode = String(animation.mode || "").toLowerCase();
  if (mode.includes("morph")) return true;
  if (animation.channel === "targetPosition") return true;
  if (Array.isArray(animation.requiresChannels) && animation.requiresChannels.includes("targetPosition")) return true;
  return false;
}

function positionAt(info, index) {
  if (info.mode === "morph") {
    const sourceOffset = index * info.sourceItemSize;
    const targetOffset = index * info.targetItemSize;
    const t = info.morph;
    const sx = Number(info.sourceArray[sourceOffset]);
    const sy = Number(info.sourceArray[sourceOffset + 1]);
    const sz = Number(info.sourceArray[sourceOffset + 2]);
    const tx = Number(info.targetArray[targetOffset]);
    const ty = Number(info.targetArray[targetOffset + 1]);
    const tz = Number(info.targetArray[targetOffset + 2]);
    if (![sx, sy, sz, tx, ty, tz].every(Number.isFinite)) return null;
    return [sx + (tx - sx) * t, sy + (ty - sy) * t, sz + (tz - sz) * t];
  }
  const offset = index * info.itemSize;
  const x = Number(info.array[offset]);
  const y = Number(info.array[offset + 1]);
  const z = Number(info.array[offset + 2]);
  return Number.isFinite(x) && Number.isFinite(y) && Number.isFinite(z) ? [x, y, z] : null;
}

function animationProgressForDescriptor(animation = {}, runtime = null) {
  if (Number.isFinite(Number(animation.progress))) return clampRuntimeNumber(Number(animation.progress), 0, 1);
  if (!animation.loop || !Number.isFinite(Number(animation.durationMs))) return 0;
  const duration = Math.max(1, Number(animation.durationMs));
  const timeMs = finiteRuntimeNumber(runtime?.renderer?.loop?.time?.elapsedMs, performanceNow());
  const t = (timeMs % duration) / duration;
  if (animation.direction === "alternate") return t < 0.5 ? t * 2 : 2 - t * 2;
  return t;
}

function projectWithViewProjection(out, position, matrix, width, height) {
  if (!matrix || matrix.length < 16) return null;
  const x = position[0], y = position[1], z = position[2];
  const clipX = matrix[0] * x + matrix[4] * y + matrix[8] * z + matrix[12];
  const clipY = matrix[1] * x + matrix[5] * y + matrix[9] * z + matrix[13];
  const clipZ = matrix[2] * x + matrix[6] * y + matrix[10] * z + matrix[14];
  const clipW = matrix[3] * x + matrix[7] * y + matrix[11] * z + matrix[15];
  if (Math.abs(clipW) < 1e-6) return null;
  const ndcX = clipX / clipW;
  const ndcY = clipY / clipW;
  const ndcZ = clipZ / clipW;
  out.x = (ndcX * 0.5 + 0.5) * width;
  out.y = (-ndcY * 0.5 + 0.5) * height;
  out.depth = ndcZ;
  out.visible = clipW > 0 && ndcX >= -1 && ndcX <= 1 && ndcY >= -1 && ndcY <= 1 && ndcZ >= -1 && ndcZ <= 1;
  return out;
}

function focusTargetForRecord(runtime, recordId, options = {}) {
  const explicit = options.focusTarget || options.cameraFocus || options.focusPoint || options.focusPosition || options.position || options.worldPosition;
  if (explicit) {
    const normalizedExplicit = normalizeCameraFocusTarget(explicit);
    if (normalizedExplicit) return { ...normalizedExplicit, source: "record-selection", recordId };
  }

  const coordinates = runtime.visualSpace?.coordinatesForRecord?.(recordId) || [];
  if (!coordinates.length) return null;
  const coordinateId = options.coordinateId || options.coordinate || options.coordinates;
  const match = coordinateId
    ? coordinates.find((entry) => entry?.state?.id === coordinateId || entry?.state?.coordinate_id === coordinateId)
    : coordinates[0];
  const position = normalizePosition3(match?.position);
  return position ? {
    type: "point",
    source: "record-selection",
    recordId,
    coordinateId: match?.state?.id || match?.state?.coordinate_id || null,
    position,
  } : null;
}

function normalizeSelectionPair(pair) {
  if (!pair || typeof pair !== "object") {
    throw new Error("selectPair() requires a pair object.");
  }
  const rowId = pair.rowId ?? pair.row_id ?? pair.sourceId ?? pair.source_id;
  const columnId = pair.columnId ?? pair.column_id ?? pair.targetId ?? pair.target_id;
  if (rowId == null || columnId == null) {
    throw new Error("selectPair() requires row/column record ids.");
  }
  return {
    relationId: pair.relationId ?? pair.relation_id ?? null,
    relationName: pair.relationName ?? pair.relation_name ?? null,
    relationType: pair.relationType ?? pair.relation_type ?? null,
    graphId: pair.graphId ?? pair.graph_id ?? null,
    edgeId: pair.edgeId ?? pair.edge_id ?? null,
    pairId: pair.pairId ?? pair.pair_id ?? null,
    pairSetId: pair.pairSetId ?? pair.pair_set_id ?? null,
    rowId: String(rowId),
    columnId: String(columnId),
    row: Number.isFinite(Number(pair.row)) ? Number(pair.row) : null,
    column: Number.isFinite(Number(pair.column)) ? Number(pair.column) : null,
    value: pair.value,
    present: pair.present !== false,
    offset: Number.isFinite(Number(pair.offset)) ? Number(pair.offset) : null,
    size: Number.isFinite(Number(pair.size)) ? Number(pair.size) : null,
    properties: pair.properties == null ? null : normalizePreviewProperties(pair.properties),
    nativePair: pair.nativePair ?? pair.native_pair ?? null,
    pickSource: pair.pickSource ?? pair.pick_source ?? null,
    sourceSpaceId: pair.sourceSpaceId ?? pair.source_space_id ?? null,
    targetSpaceId: pair.targetSpaceId ?? pair.target_space_id ?? null,
    sourceCoordinateId: pair.sourceCoordinateId ?? pair.source_coordinate_id ?? null,
    targetCoordinateId: pair.targetCoordinateId ?? pair.target_coordinate_id ?? null,
  };
}

function buildRuntimeRecordPreview(runtime, recordId) {
  if (!runtime) return null;
  return buildMetricRecordPreview({ recordId }, {
    runtime,
    visualSpace: runtime.visualSpace,
    document: runtime.document,
  });
}

function buildRuntimePairPreview(runtime, pair) {
  if (!runtime) return null;
  return buildMetricPairPreview({ pair }, {
    runtime,
    visualSpace: runtime.visualSpace,
    document: runtime.document,
  });
}

function buildRuntimeSelectionPresentation(runtime, selection) {
  if (!runtime) return null;
  return buildLinkedSelectionPresentation(selection, {
    runtime,
    visualSpace: runtime.visualSpace,
    document: runtime.document,
    layerDescriptors: runtime.layerDescriptors,
  });
}

function normalizePreviewProperties(properties) {
  if (!properties) return [];
  if (Array.isArray(properties)) {
    return properties.map(normalizePreviewProperty).filter(Boolean);
  }
  if (properties instanceof Map) {
    return Array.from(properties, ([label, value]) => normalizePreviewProperty({ label, value })).filter(Boolean);
  }
  if (typeof properties === "object") {
    return Object.entries(properties)
      .map(([label, value]) => normalizePreviewProperty({ label, value }))
      .filter(Boolean);
  }
  return [];
}

function normalizePreviewProperty(property) {
  if (!property || typeof property !== "object") return null;
  const label = property.label ?? property.name ?? property.id;
  if (label == null) return null;
  return {
    label: String(label),
    value: property.value,
  };
}

function normalizeCameraFocusTarget(value) {
  if (value == null || value === false) return null;
  if (typeof value === "number") return normalizeFocusDistanceTarget(value);
  if (Array.isArray(value) || ArrayBuffer.isView(value)) {
    const position = normalizePosition3(value);
    return position ? { type: "point", position } : null;
  }
  if (value && typeof value === "object") {
    if (value.type === "distance" || own(value, "focusDistance") || own(value, "distance")) {
      const target = normalizeFocusDistanceTarget(value.focusDistance ?? value.distance);
      return target ? { ...clonePlainObject(value), ...target, type: "distance" } : null;
    }
    const position = normalizePosition3(value.position || value.point || value.worldPosition || value.world || value);
    return position ? { ...clonePlainObject(value), type: "point", position } : null;
  }
  return null;
}

function normalizeFocusDistanceTarget(value) {
  const focusDistance = Number(value);
  return Number.isFinite(focusDistance) && focusDistance > 0
    ? { type: "distance", focusDistance }
    : null;
}

function normalizePosition3(value) {
  if (Array.isArray(value) || ArrayBuffer.isView(value)) {
    const x = Number(value[0]);
    const y = Number(value[1]);
    const z = Number(value[2]);
    return Number.isFinite(x) && Number.isFinite(y) && Number.isFinite(z) ? [x, y, z] : null;
  }
  if (value && typeof value === "object") {
    const x = Number(value.x ?? value[0]);
    const y = Number(value.y ?? value[1]);
    const z = Number(value.z ?? value[2]);
    return Number.isFinite(x) && Number.isFinite(y) && Number.isFinite(z) ? [x, y, z] : null;
  }
  return null;
}

function resolveCameraFocusDistance(focusTarget, camera) {
  if (!focusTarget) return null;
  if (focusTarget.type === "distance") {
    const focusDistance = Number(focusTarget.focusDistance);
    return Number.isFinite(focusDistance) && focusDistance > 0 ? focusDistance : null;
  }
  const position = normalizePosition3(focusTarget.position);
  if (!position) return null;
  return cameraDistanceToPoint(camera, position);
}

function cameraDistanceToPoint(camera, position) {
  if (!camera) return null;
  const view = resolveCameraViewMatrix(camera);
  if (view && view.length >= 16) {
    const z = view[2] * position[0] + view[6] * position[1] + view[10] * position[2] + view[14];
    const distance = -z;
    return Number.isFinite(distance) && distance > 0 ? distance : null;
  }
  const forwardDistance = cameraForwardDistanceToPoint(camera, position);
  if (forwardDistance != null) return forwardDistance;
  return null;
}

function resolveCameraViewMatrix(camera) {
  if (!camera) return null;
  if (typeof camera.updateMatrices === "function") camera.updateMatrices();
  if (typeof camera.updateMatrixWorld === "function") {
    camera.updateMatrixWorld(true);
  } else if (typeof camera.updateWorldMatrix === "function") {
    camera.updateWorldMatrix(true, false);
  }
  return firstMatrix16(
    camera.viewMatrix,
    camera.matrixWorldInverse,
    camera.camera?.viewMatrix,
    camera.camera?.matrixWorldInverse,
  );
}

function firstMatrix16(...candidates) {
  for (const candidate of candidates) {
    const matrix = candidate?.elements || candidate;
    if (matrix && matrix.length >= 16) return matrix;
  }
  return null;
}

function cameraForwardDistanceToPoint(camera, position) {
  const cameraPosition = normalizePosition3(camera.position || camera.worldPosition || camera.eye);
  const target = normalizePosition3(camera.target || camera.lookAtTarget || camera.lookAt);
  if (!cameraPosition || !target) return null;
  const fx = target[0] - cameraPosition[0];
  const fy = target[1] - cameraPosition[1];
  const fz = target[2] - cameraPosition[2];
  const forwardLength = Math.hypot(fx, fy, fz);
  if (forwardLength <= 1e-6) return null;
  const dx = position[0] - cameraPosition[0];
  const dy = position[1] - cameraPosition[1];
  const dz = position[2] - cameraPosition[2];
  const distance = (dx * fx + dy * fy + dz * fz) / forwardLength;
  return Number.isFinite(distance) && distance > 0 ? distance : null;
}

function createEmptyPickingIndex() {
  return {
    recordPoints: [],
    relationMatrixPickers: [],
    graphEdges: [],
    gpuLayerCount: 0,
    availableSources: [],
  };
}

function createEmptyInspectionState() {
  return {
    source: "none",
    hover: null,
    selection: null,
    lastResult: null,
  };
}

function createRuntimePickRequest(runtime, input = {}, options = {}) {
  const size = runtime.renderer?.size || {};
  const pixelRatio = positiveRuntimeNumber(size.pixelRatio, 1);
  const rawPointer = options.rawPointer || options.pointer || input.rawPointer || input.pointer || input;
  const pixel = rawPointer.pixel || rawPointer.screen || null;
  const x = finiteRuntimeNumber(
    options.x,
    input.x,
    pixel?.x,
    rawPointer.x == null ? null : rawPointer.x * pixelRatio,
    rawPointer.css?.x == null ? null : rawPointer.css.x * pixelRatio,
    0,
  );
  const y = finiteRuntimeNumber(
    options.y,
    input.y,
    pixel?.y,
    rawPointer.y == null ? null : rawPointer.y * pixelRatio,
    rawPointer.css?.y == null ? null : rawPointer.css.y * pixelRatio,
    0,
  );
  const request = createPickRequest({
    ...options,
    ...input,
    x,
    y,
    radius: positiveRuntimeNumber(options.thresholdPx, input.radius, DEFAULT_RUNTIME_OPTIONS.inspection.thresholdPx),
    pointer: rawPointer,
    camera: runtime.camera,
    viewport: size,
    registry: runtime.pickingRegistry,
    layers: runtime.layers,
    source: options.source || input.source || "runtime-inspection",
    timestamp: options.timestamp ?? input.timestamp ?? performanceNow(),
  });
  request.rawPointer = rawPointer;
  return request;
}

function gpuPickRadiusPixels(runtime, options = {}) {
  const pixelRatio = positiveRuntimeNumber(runtime?.renderer?.size?.pixelRatio, 1);
  const radiusCss = Math.max(0, finiteRuntimeNumber(
    options.gpuRadiusPx,
    options.gpuRadius,
    runtime?.inspectionOptions?.gpuRadiusPx,
    DEFAULT_RUNTIME_OPTIONS.inspection.gpuRadiusPx,
    0,
  ));
  return Math.round(radiusCss * pixelRatio);
}

function readPickIdNearFramebuffer(gl, x, y, options = {}) {
  const radius = Math.max(0, Math.floor(finiteRuntimeNumber(options.radius, 0)));
  const center = readPickIdFromFramebuffer(gl, x, y, options);
  if (center) return { numericId: center, x, y, dx: 0, dy: 0, samples: 1 };
  let samples = 1;
  for (let ring = 1; ring <= radius; ring += 1) {
    const offsets = [
      [ring, 0],
      [-ring, 0],
      [0, ring],
      [0, -ring],
      [ring, ring],
      [ring, -ring],
      [-ring, ring],
      [-ring, -ring],
    ];
    for (const [dx, dy] of offsets) {
      samples += 1;
      const numericId = readPickIdFromFramebuffer(gl, x + dx, y + dy, options);
      if (numericId) return { numericId, x: x + dx, y: y + dy, dx, dy, samples };
    }
  }
  return { numericId: 0, x, y, dx: 0, dy: 0, samples };
}

function scanPickFramebuffer(gl, options = {}) {
  const width = Math.max(1, Math.floor(finiteRuntimeNumber(options.width, gl?.drawingBufferWidth, 1)));
  const height = Math.max(1, Math.floor(finiteRuntimeNumber(options.height, gl?.drawingBufferHeight, 1)));
  const step = Math.max(1, Math.floor(finiteRuntimeNumber(options.step, 8)));
  let sampled = 0;
  let nonZero = 0;
  let first = null;
  for (let y = 0; y < height; y += step) {
    for (let x = 0; x < width; x += step) {
      sampled += 1;
      const numericId = readPickIdFromFramebuffer(gl, x, y, {
        width,
        height,
        yOrigin: options.yOrigin || "top",
      });
      if (!numericId) continue;
      nonZero += 1;
      if (!first) first = { x, y, numericId };
    }
  }
  return { width, height, step, sampled, nonZero, first };
}

function collectDescriptorRecordPickPoints(out, registry, descriptor, runtime) {
  if (!descriptor || descriptor.visible === false) return out;
  const channels = descriptor.channels || {};
  const recordChannel = getChannel(channels, ["recordId", "record_id", "id", "cellId"]);
  const recordIds = getChannelArray(recordChannel);
  if (!recordIds?.length) return out;
  const positionInfo = descriptorFocusPositionInfo(descriptor, DEFAULT_RUNTIME_OPTIONS.inspection.positionChannels, runtime);
  if (!positionInfo) return out;
  const count = Math.min(recordIds.length, positionInfo.count, runtime.inspectionOptions.maxCandidates);
  for (let index = 0; index < count; index += 1) {
    const recordId = recordIds[index];
    if (recordId == null) continue;
    const position = positionAt(positionInfo, index);
    if (!position) continue;
    const numericId = registry.registerRecord(String(recordId), {
      layerId: descriptor.id || descriptor.primitive || null,
      scope: descriptor.id || descriptor.primitive || null,
      payload: {
        descriptorId: descriptor.id || null,
        descriptorKind: descriptor.kind || descriptor.primitive || null,
        index,
      },
    });
    out.recordPoints.push({
      id: String(recordId),
      recordId: String(recordId),
      layerId: descriptor.id || descriptor.primitive || null,
      scope: descriptor.id || descriptor.primitive || null,
      numericId,
      position,
    });
  }
  return out;
}

function collectDescriptorRelationMatrixPickers(out, descriptor, runtime) {
  if (!descriptor || descriptor.visible === false || descriptor.primitive !== "RelationMatrixLayer") return out;
  try {
    const picker = createRelationMatrixPicker(descriptor, { canvas: runtime.canvas });
    out.relationMatrixPickers.push({
      descriptorId: descriptor.id || null,
      relationId: descriptor.source?.relationId || descriptor.metadata?.relationId || descriptor.relationId || null,
      picker,
    });
  } catch {
    // Incomplete layer descriptors are ignored here; layer construction still
    // reports hard errors through the usual runtime warning path.
  }
  return out;
}

function collectDescriptorGraphEdges(out, registry, descriptor) {
  if (!descriptor || descriptor.visible === false || descriptor.primitive !== "RelationEdgeLayer") return out;
  const graph = descriptor.metadata?.graph || descriptor.source?.graph || null;
  const edges = Array.isArray(graph?.edges) ? graph.edges : [];
  if (!edges.length) return out;
  const sourcePositions = channelVec3Array(descriptor.channels, ["sourcePosition", "source"]);
  const targetPositions = channelVec3Array(descriptor.channels, ["targetPosition", "target"]);
  const values = getChannelArray(getChannel(descriptor.channels || {}, ["relationValue", "value", "weight"]));
  const relationId = descriptor.source?.relationId || graph.relationId || graph.edge_relation_id || null;
  const graphId = descriptor.source?.graphId || graph.id || null;
  const pairSetId = descriptor.source?.pairSetId || descriptor.metadata?.selectionModel?.pairSetId || graph.id || null;
  const count = Math.min(edges.length, sourcePositions.count, targetPositions.count);

  for (let index = 0; index < count; index += 1) {
    const edge = edges[index] || {};
    const rowId = edge.source ?? edge.rowId ?? edge.row_id ?? edge.source_id ?? edge.a;
    const columnId = edge.target ?? edge.columnId ?? edge.column_id ?? edge.target_id ?? edge.b;
    if (rowId == null || columnId == null) continue;
    const edgeId = edge.id || `${relationId || graphId || descriptor.id || "graph"}:${rowId}:${columnId}:${index}`;
    const pair = {
      relationId,
      graphId,
      edgeId,
      pairId: edge.pairId ?? edge.pair_id ?? null,
      pairSetId,
      rowId: String(rowId),
      columnId: String(columnId),
      row: Number.isFinite(Number(edge.sourceIndex)) ? Number(edge.sourceIndex) : null,
      column: Number.isFinite(Number(edge.targetIndex)) ? Number(edge.targetIndex) : null,
      value: edge.value ?? values?.[index],
      present: true,
      sourcePosition: sourcePositions.at(index),
      targetPosition: targetPositions.at(index),
      sourceSpaceId: edge.sourceSpaceId ?? edge.source_space_id ?? null,
      targetSpaceId: edge.targetSpaceId ?? edge.target_space_id ?? null,
      sourceCoordinateId: edge.sourceCoordinateId ?? edge.source_coordinate_id ?? null,
      targetCoordinateId: edge.targetCoordinateId ?? edge.target_coordinate_id ?? null,
      layerId: descriptor.id || null,
    };
    const numericId = registry.registerEdge(edgeId, {
      layerId: descriptor.id || null,
      scope: descriptor.id || descriptor.primitive || null,
      payload: pair,
    });
    out.graphEdges.push({ ...pair, numericId });
  }
  return out;
}

function channelVec3Array(channels = {}, names) {
  const channel = getChannel(channels, names);
  const data = getChannelArray(channel);
  const itemSize = getChannelItemSize(channel, 3);
  const count = getChannelCount(channel, 3);
  return {
    count,
    at(index) {
      const offset = index * itemSize;
      return [
        Number(data?.[offset]) || 0,
        Number(data?.[offset + 1]) || 0,
        Number(data?.[offset + 2]) || 0,
      ];
    },
  };
}

function projectRuntimePosition(camera, position, out = {}, size = {}) {
  if (!position) return null;
  if (camera?.projectToPixel) return camera.projectToPixel(position, out);
  return projectWithViewProjection(
    out,
    position,
    camera?.viewProjectionMatrix,
    Math.max(1, numberOr(size.drawingBufferWidth, 1)),
    Math.max(1, numberOr(size.drawingBufferHeight, 1)),
  );
}

function distanceToSegmentSquared(px, py, ax, ay, bx, by) {
  const vx = bx - ax;
  const vy = by - ay;
  const wx = px - ax;
  const wy = py - ay;
  const lengthSq = vx * vx + vy * vy;
  if (lengthSq <= 1e-9) {
    const dx = px - ax;
    const dy = py - ay;
    return dx * dx + dy * dy;
  }
  const t = clampRuntimeNumber((wx * vx + wy * vy) / lengthSq, 0, 1);
  const x = ax + vx * t;
  const y = ay + vy * t;
  const dx = px - x;
  const dy = py - y;
  return dx * dx + dy * dy;
}

function pairFromPickResult(result) {
  const payload = result?.entry?.payload || result?.raw || null;
  if (!payload) return null;
  const rowId = payload.rowId ?? payload.row_id ?? payload.sourceId ?? payload.source_id;
  const columnId = payload.columnId ?? payload.column_id ?? payload.targetId ?? payload.target_id;
  if (rowId == null || columnId == null) return null;
  return {
    relationId: payload.relationId ?? payload.relation_id ?? null,
    relationName: payload.relationName ?? payload.relation_name ?? null,
    relationType: payload.relationType ?? payload.relation_type ?? null,
    graphId: payload.graphId ?? payload.graph_id ?? null,
    edgeId: payload.edgeId ?? payload.edge_id ?? result.edgeId ?? null,
    pairId: payload.pairId ?? payload.pair_id ?? null,
    pairSetId: payload.pairSetId ?? payload.pair_set_id ?? null,
    rowId: String(rowId),
    columnId: String(columnId),
    row: Number.isFinite(Number(payload.row)) ? Number(payload.row) : null,
    column: Number.isFinite(Number(payload.column)) ? Number(payload.column) : null,
    value: payload.value,
    present: payload.present !== false,
    offset: Number.isFinite(Number(payload.offset)) ? Number(payload.offset) : null,
    size: Number.isFinite(Number(payload.size)) ? Number(payload.size) : null,
    properties: payload.properties == null ? null : normalizePreviewProperties(payload.properties),
    nativePair: payload.nativePair ?? payload.native_pair ?? null,
    pickSource: result.source || null,
    sourceSpaceId: payload.sourceSpaceId ?? payload.source_space_id ?? null,
    targetSpaceId: payload.targetSpaceId ?? payload.target_space_id ?? null,
    sourceCoordinateId: payload.sourceCoordinateId ?? payload.source_coordinate_id ?? null,
    targetCoordinateId: payload.targetCoordinateId ?? payload.target_coordinate_id ?? null,
  };
}

function inspectionDetailFromPickResult(result, options = {}, runtime = null) {
  if (!result?.hit) {
    return {
      hit: false,
      source: result?.source || "none",
      mode: options.mode || null,
    };
  }
  const pair = pairFromPickResult(result);
  const preview = pair
    ? buildRuntimePairPreview(runtime, pair)
    : result.recordId != null
      ? buildRuntimeRecordPreview(runtime, result.recordId)
      : null;
  const presentation = buildRuntimeSelectionPresentation(runtime, {
    recordId: pair ? null : (result.recordId ?? null),
    pair,
    preview,
    source: options.source || "inspection",
    pickSource: result.source || null,
  });
  return {
    hit: true,
    source: result.source || "unknown",
    mode: options.mode || null,
    kind: pair ? "pair" : (result.kind || "record"),
    recordId: result.recordId ?? null,
    pair,
    preview,
    presentation,
    layerId: result.layerId ?? null,
    edgeId: result.edgeId ?? null,
    numericId: result.numericId ?? 0,
    distance: Number.isFinite(result.distance) ? result.distance : null,
  };
}

function collectRuntimeLayerDiagnostics(layers = []) {
  const diagnostics = [];
  for (const layer of layers || []) {
    if (!layer) continue;
    const entry = {
      id: layer.id || null,
      kind: layer.kind || null,
      primitive: layer.primitive || layer.descriptor?.primitive || layer.descriptor?.kind || null,
      visible: layer.visible !== false,
      enabled: layer.enabled !== false,
      renderPhase: resolveLayerRenderPhase(layer),
      resourceCount: Array.isArray(layer.resources) ? layer.resources.length : 0,
    };
    try {
      if (typeof layer.getGrammarDiagnostics === "function") {
        entry.grammar = clonePlainObject(layer.getGrammarDiagnostics());
      }
    } catch (error) {
      entry.grammarError = error instanceof Error ? error.message : String(error);
    }
    try {
      if (typeof layer.describeReadability === "function") {
        entry.readability = clonePlainObject(layer.describeReadability());
      }
    } catch (error) {
      entry.readabilityError = error instanceof Error ? error.message : String(error);
    }
    try {
      if (typeof layer.getDiagnostics === "function") {
        entry.diagnostics = clonePlainObject(layer.getDiagnostics());
      }
    } catch (error) {
      entry.diagnosticsError = error instanceof Error ? error.message : String(error);
    }
    diagnostics.push(entry);
  }
  return diagnostics;
}

function collectRuntimeRenderPhases(layers = []) {
  const counts = {};
  for (const layer of layers || []) {
    if (!layer || layer.enabled === false || layer.visible === false) continue;
    const phase = resolveLayerRenderPhase(layer);
    counts[phase] = (counts[phase] || 0) + 1;
  }
  return {
    phases: Object.keys(counts).sort(),
    counts,
    screenReadableOverlay: counts["screen-readable-overlay"] || 0,
  };
}

function clonePlainObject(value) {
  if (Array.isArray(value)) return value.map(clonePlainObject);
  if (!value || typeof value !== "object") return value;
  const out = {};
  for (const [key, entry] of Object.entries(value)) out[key] = clonePlainObject(entry);
  return out;
}

function finiteRuntimeNumber(...values) {
  for (const value of values) {
    const number = Number(value);
    if (Number.isFinite(number)) return number;
  }
  return null;
}

function numericValueChanged(previous, next, epsilon = 0) {
  if (previous == null || next == null) return previous !== next;
  return Math.abs(previous - next) > epsilon;
}

function plainObjectEqual(left, right) {
  if (left === right) return true;
  if (left == null || right == null) return left === right;
  if (typeof left !== "object" || typeof right !== "object") return false;
  if (Array.isArray(left) || Array.isArray(right)) {
    if (!Array.isArray(left) || !Array.isArray(right) || left.length !== right.length) return false;
    for (let index = 0; index < left.length; index += 1) {
      if (!plainObjectEqual(left[index], right[index])) return false;
    }
    return true;
  }
  const leftKeys = Object.keys(left);
  const rightKeys = Object.keys(right);
  if (leftKeys.length !== rightKeys.length) return false;
  for (const key of leftKeys) {
    if (!Object.prototype.hasOwnProperty.call(right, key)) return false;
    if (!plainObjectEqual(left[key], right[key])) return false;
  }
  return true;
}

function positiveRuntimeNumber(...values) {
  for (const value of values) {
    const number = Number(value);
    if (Number.isFinite(number) && number > 0) return number;
  }
  return 1;
}

function clampRuntimeNumber(value, min, max) {
  return Math.max(min, Math.min(max, value));
}

function performanceNow() {
  return typeof performance !== "undefined" && typeof performance.now === "function" ? performance.now() : Date.now();
}

function normalizeColor(value, fallback) {
  const source = Array.isArray(value) || ArrayBuffer.isView(value) ? value : fallback;
  return [
    numberOr(source[0], fallback[0]),
    numberOr(source[1], fallback[1]),
    numberOr(source[2], fallback[2]),
    numberOr(source[3], fallback[3]),
  ];
}

function numberOr(value, fallback = 0) {
  const number = Number(value);
  return Number.isFinite(number) ? number : fallback;
}

function toArray(value) {
  if (Array.isArray(value)) return value;
  if (value == null) return [];
  return [value];
}

function isPromiseLike(value) {
  return value && typeof value.then === "function";
}

function looksLikeLayerDescriptor(value) {
  return Boolean(value && typeof value === "object" && (
    value.primitive ||
    value.kind?.includes?.("layer") ||
    value.schema?.includes?.("layer_descriptor")
  ));
}

function own(value, key) {
  return Boolean(value && Object.prototype.hasOwnProperty.call(value, key));
}

function disposeLayer(layer) {
  if (layer && typeof layer.dispose === "function") {
    layer.dispose();
  } else if (layer && typeof layer.destroy === "function") {
    layer.destroy();
  }
}
