import { InstancedPointLayer } from "./InstancedPointLayer.js";
import { InstancedGlyphLayer } from "./InstancedGlyphLayer.js";
import { BillboardLabelLayer } from "./BillboardLabelLayer.js";
import { InstancedBoxLayer } from "./InstancedBoxLayer.js";
import { HeatFieldLayer } from "./HeatFieldLayer.js";
import { GroundProjectionLayer } from "./GroundProjectionLayer.js";
import { GroundPlaneLayer } from "./GroundPlaneLayer.js";
import { SurfaceLayer } from "./SurfaceLayer.js";
import { RelationEdgeLayer } from "./RelationEdgeLayer.js";
import { RelationMatrixLayer } from "../relational/RelationMatrixLayer.js";
import { CurveRibbonLayer, CurveTubeMeshLayer } from "../curves/index.js";

export class LayerFactory {
  constructor(registry = defaultLayerRegistry()) {
    this.registry = new Map(registry);
  }

  register(primitive, LayerClass) {
    if (!primitive || typeof primitive !== "string") {
      throw new Error("LayerFactory.register() requires a primitive name.");
    }
    if (typeof LayerClass !== "function") {
      throw new Error(`LayerFactory.register(${primitive}) requires a layer constructor.`);
    }
    this.registry.set(primitive, LayerClass);
    return this;
  }

  create(descriptor, rendererOrGl, options = {}) {
    return createLayerFromRegistry(this.registry, descriptor, rendererOrGl, options);
  }

  has(primitive) {
    return this.registry.has(primitive);
  }

  primitives() {
    return Array.from(this.registry.keys());
  }
}

export function createLayerFromDescriptor(descriptor, rendererOrGl, options = {}) {
  return createLayerFromRegistry(defaultRegistry, descriptor, rendererOrGl, options);
}

export function registerLayer(primitive, LayerClass) {
  if (!primitive || typeof primitive !== "string") {
    throw new Error("registerLayer() requires a primitive name.");
  }
  if (typeof LayerClass !== "function") {
    throw new Error(`registerLayer(${primitive}) requires a layer constructor.`);
  }
  defaultRegistry.set(primitive, LayerClass);
  return LayerClass;
}

export function defaultLayerRegistry() {
  return new Map(defaultRegistry);
}

function createLayerFromRegistry(registry, descriptor, rendererOrGl, options) {
  if (!descriptor || typeof descriptor !== "object") {
    throw new Error("createLayerFromDescriptor() requires a layer descriptor object.");
  }
  const primitive = descriptor.primitive || descriptor.kind;
  const LayerClass = registry.get(primitive);
  if (!LayerClass) {
    throw new Error(`Unsupported layer primitive "${primitive}". Registered primitives: ${Array.from(registry.keys()).join(", ")}`);
  }
  return new LayerClass(descriptor, rendererOrGl, options);
}

const defaultRegistry = new Map([
  ["InstancedPointLayer", InstancedPointLayer],
  ["InstancedGlyphLayer", InstancedGlyphLayer],
  ["BillboardLabelLayer", BillboardLabelLayer],
  ["InstancedBoxLayer", InstancedBoxLayer],
  ["HeatFieldLayer", HeatFieldLayer],
  ["GroundProjectionLayer", GroundProjectionLayer],
  ["GroundPlaneLayer", GroundPlaneLayer],
  ["SurfaceLayer", SurfaceLayer],
  ["RelationEdgeLayer", RelationEdgeLayer],
  ["RelationMatrixLayer", RelationMatrixLayer],
  ["CurveRibbonLayer", CurveRibbonLayer],
  ["CurveTubeMeshLayer", CurveTubeMeshLayer],
]);
