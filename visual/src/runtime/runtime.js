import { VisualRenderer, VisualScene } from "../engine/index.js";
import { createMiniaturePerspectiveCamera } from "../camera/index.js";
import { createFocusLineState, createCameraControls } from "../interaction/index.js";
import { VisualSpace } from "../data/index.js";
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

const RUNTIME_RENDER_PASS = "Scene(RenderPass)";
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
    return {
      enabled: this.enabled,
      length: this.length,
      nativePasses,
      enabledNativePasses,
      pipelineOrder: [
        RUNTIME_RENDER_PASS,
        ...compatibilityPasses,
        ...postFxEnabledOrder.map((name) => `postFx.${name}`),
      ],
      pipelinePhases: {
        scene: RUNTIME_RENDER_PASS,
        compatibility: compatibilityPasses,
        postFx: postFxEnabledOrder,
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
    if (!this.enabled || this.length === 0) {
      context.renderer.renderScene(null);
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
      context.renderer.renderScene(sceneContext.target || null);
    }, {
      width,
      height,
      target: null,
      focusState: this.focusState,
      camera: context.camera || context.renderer?.camera || null,
    });
    return null;
  }

  dispose() {
    if (this.native) this.native.dispose();
  }
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
      record: null,
      source: null,
    };
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
    this.selection = {
      recordId,
      record: this.visualSpace?.getRecord ? this.visualSpace.getRecord(recordId) : null,
      source: options.source || null,
      focusTarget,
    };
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
      record: null,
      source: options.source || null,
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
      focusTarget: this.focusTarget ? clonePlainObject(this.focusTarget) : null,
      hoverFocus: {
        enabled: this.hoverFocusOptions.enabled === true,
        target: this.hoverFocusState.target ? clonePlainObject(this.hoverFocusState.target) : null,
      },
      viewCount: this.viewDescriptors.length,
      layerDescriptorCount: this.layerDescriptors.length,
      layerInstanceCount: this.layers.length,
      layerState: { ...this.layerState },
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
    this.clearLayerInstances({ dispose: options.disposeLayers !== false });
    this.events.clear();
    this.renderer.dispose({
      disposeScene: options.disposeScene !== false,
      disposePipeline: options.disposePipeline !== false,
      loseContext: options.loseContext,
    });
    this.disposed = true;
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
    const positionInfo = descriptorFocusPositionInfo(descriptor, positionNames);
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

function descriptorFocusPositionInfo(descriptor, positionNames) {
  const channels = descriptor?.channels || {};
  const position = getChannel(channels, "position");
  const target = getChannel(channels, "targetPosition");
  if (position && target && descriptor?.animation?.mode === "coordinate-morph") {
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
        morph: animationProgressForDescriptor(descriptor.animation),
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

function animationProgressForDescriptor(animation = {}) {
  if (Number.isFinite(Number(animation.progress))) return clampRuntimeNumber(Number(animation.progress), 0, 1);
  if (!animation.loop || !Number.isFinite(Number(animation.durationMs))) return 0;
  const duration = Math.max(1, Number(animation.durationMs));
  const t = (performanceNow() % duration) / duration;
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

function clonePlainObject(value) {
  if (Array.isArray(value)) return value.map(clonePlainObject);
  if (!value || typeof value !== "object") return value;
  const out = {};
  for (const [key, entry] of Object.entries(value)) out[key] = clonePlainObject(entry);
  return out;
}

function finiteRuntimeNumber(value) {
  const number = Number(value);
  return Number.isFinite(number) ? number : null;
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
