import { BaseView } from "./BaseView.js";
import { defaultCoordinateId } from "./MetricSpaceView.js";
import { SpacePropertiesView } from "./SpacePropertiesView.js";
import {
  VISUAL_LAYER_HIERARCHY_BANDS,
  createVisualLayerHierarchy,
} from "./TrajectoryPathView.js";
import {
  extractCoordinatePositions,
  extractPropertyValues,
  recordsFor,
  resolveCollectionItem,
} from "./view-utils.js";

const PROPERTY_FIELD_VIEW_EVIDENCE_SCHEMA = "metric.visual.property_field_evidence_ref.v1";

/**
 * PropertyFieldView is the reusable property-field grammar adapter. It turns an
 * exported scalar record property plus exported/fitted record coordinates into a
 * HeatFieldLayer descriptor by reusing SpacePropertiesView's field grammar.
 * It does not compute density, entropy, anomaly, or residual values.
 */
export class PropertyFieldView extends BaseView {
  constructor(options = {}) {
    super({ ...options, kind: "property-field" });
    this.recordIds = options.recordIds || [];
    this.positions = options.positions || new Map();
    this.scalarValues = options.scalarValues || new Map();
    this.propertyName = options.propertyName || options.name || this.propertyId;
    this.coordinateId = options.coordinateId ?? options.coordinate_id ?? this.coordinateId;
    this.order = Number.isFinite(Number(options.order)) ? Number(options.order) : -2;
    this.nativeEvidence = options.nativeEvidence || createPropertyFieldNativeEvidence(options.document, {
      datasetId: this.datasetId,
      spaceId: this.spaceId,
      coordinateId: this.coordinateId,
      propertyId: this.propertyId,
      propertyName: this.propertyName,
      propertyValueType: options.propertyValueType,
      recordCount: this.recordIds.length,
    });
    this.fieldOptions = {
      fieldMode: options.fieldMode || options.fieldGrammar || "ground",
      fieldAlpha: Number.isFinite(Number(options.fieldAlpha ?? options.alpha)) ? Number(options.fieldAlpha ?? options.alpha) : 0.34,
      fieldRadius: Number.isFinite(Number(options.fieldRadius ?? options.radius)) ? Number(options.fieldRadius ?? options.radius) : 0.22,
      fieldLift: Number.isFinite(Number(options.fieldLift)) ? Number(options.fieldLift) : 0.32,
      fieldMaterial: options.fieldMaterial || options.material || {},
      groundY: Number.isFinite(Number(options.groundY)) ? Number(options.groundY) : 0,
    };
  }

  static fromVisualSpace(document, options = {}) {
    const propertyResolution = resolveProperty(document, options);
    const coordinateResolution = resolveCoordinate(document, options, null);
    const property = propertyResolution.item;
    const coordinate = coordinateResolution.item;
    const space = resolveSpace(document, options, coordinate);
    const datasetId = options.datasetId ?? property?.dataset_id ?? property?.datasetId ?? coordinate?.dataset_id ?? coordinate?.datasetId ?? space?.dataset_id;
    const records = recordsFor(document, { ...options, datasetId });
    const positions = extractCoordinatePositions(coordinate, {
      records,
      recordIds: options.recordIds || space?.record_ids,
    });

    return new PropertyFieldView({
      ...options,
      document,
      records,
      recordIds: positions.ids,
      positions: positions.positions,
      scalarValues: extractPropertyValues(property, { records, recordIds: positions.ids }),
      datasetId,
      spaceId: options.spaceId ?? options.space_id ?? space?.id ?? coordinate?.space_id ?? coordinate?.spaceId,
      coordinateId: coordinate?.id,
      propertyId: property?.id,
      propertyName: property?.name,
      propertyValueType: property?.value_type ?? property?.valueType ?? null,
      nativeEvidence: createPropertyFieldNativeEvidence(document, {
        datasetId,
        spaceId: options.spaceId ?? options.space_id ?? space?.id ?? coordinate?.space_id ?? coordinate?.spaceId,
        coordinateId: coordinate?.id,
        propertyId: property?.id,
        propertyName: property?.name,
        propertyValueType: property?.value_type ?? property?.valueType ?? null,
        recordCount: positions.ids.length,
        propertyDefaulted: propertyResolution.defaulted,
        coordinateDefaulted: coordinateResolution.defaulted,
      }),
    });
  }

  static fromMetricSpaceView(document, metricSpaceView, options = {}) {
    const propertyResolution = resolveProperty(document, options);
    const coordinateResolution = resolveCoordinate(document, options, metricSpaceView);
    const property = propertyResolution.item;
    const coordinate = coordinateResolution.item;
    const space = resolveSpace(document, options, coordinate);
    const datasetId = options.datasetId
      ?? metricSpaceView?.datasetId
      ?? property?.dataset_id
      ?? property?.datasetId
      ?? coordinate?.dataset_id
      ?? coordinate?.datasetId
      ?? space?.dataset_id;
    const records = recordsFor(document, { ...options, datasetId });
    const recordIds = (options.recordIds || metricSpaceView?.recordIds || []).map(String);
    const positions = metricSpaceView?.positions || extractCoordinatePositions(coordinate, {
      records,
      recordIds: recordIds.length ? recordIds : space?.record_ids,
    }).positions;
    const ids = recordIds.length ? recordIds : extractCoordinatePositions(coordinate, { records, recordIds: space?.record_ids }).ids;

    return new PropertyFieldView({
      ...options,
      document,
      records,
      recordIds: ids,
      positions,
      scalarValues: extractPropertyValues(property, { records, recordIds: ids }),
      datasetId,
      spaceId: options.spaceId ?? options.space_id ?? metricSpaceView?.spaceId ?? space?.id ?? coordinate?.space_id ?? coordinate?.spaceId,
      coordinateId: coordinate?.id ?? metricSpaceView?.coordinateId,
      propertyId: property?.id,
      propertyName: property?.name,
      propertyValueType: property?.value_type ?? property?.valueType ?? null,
      groundY: options.groundY ?? metricSpaceView?.groundY,
      nativeEvidence: createPropertyFieldNativeEvidence(document, {
        datasetId,
        spaceId: options.spaceId ?? options.space_id ?? metricSpaceView?.spaceId ?? space?.id ?? coordinate?.space_id ?? coordinate?.spaceId,
        coordinateId: coordinate?.id ?? metricSpaceView?.coordinateId,
        propertyId: property?.id,
        propertyName: property?.name,
        propertyValueType: property?.value_type ?? property?.valueType ?? null,
        recordCount: ids.length,
        propertyDefaulted: propertyResolution.defaulted,
        coordinateDefaulted: coordinateResolution.defaulted,
        fittedCoordinateSource: Boolean(metricSpaceView?.positions),
      }),
    });
  }

  toLayerDescriptors() {
    const descriptor = this.toSpacePropertiesFieldDescriptor();
    if (!descriptor) return [];
    descriptor.order = this.order;
    const visualHierarchy = createVisualLayerHierarchy({
      band: VISUAL_LAYER_HIERARCHY_BANDS.supportField,
      role: "property-field",
      viewClass: "PropertyFieldView",
      order: descriptor.order,
      drawsBelow: [
        VISUAL_LAYER_HIERARCHY_BANDS.trajectoryPath,
        VISUAL_LAYER_HIERARCHY_BANDS.currentState,
        VISUAL_LAYER_HIERARCHY_BANDS.sceneLabels,
      ],
      depthPolicy: {
        depthTest: descriptor.material?.depthTest ?? null,
        depthWrite: descriptor.material?.depthWrite ?? false,
        depthBias: descriptor.material?.depthBias ?? null,
      },
      purpose: "keep exported scalar support fields below paths, current states and labels",
    });
    descriptor.source = {
      ...(descriptor.source || {}),
      viewId: this.id,
      viewKind: "property-field",
      viewClass: "PropertyFieldView",
      datasetId: this.datasetId,
      spaceId: this.spaceId,
      coordinateId: this.coordinateId,
      propertyId: this.propertyId,
      nativeEvidence: this.nativeEvidence,
    };
    descriptor.metadata = {
      ...(descriptor.metadata || {}),
      ...this.metadata,
      viewClass: "PropertyFieldView",
      role: "property-field",
      algorithmicComputation: false,
      propertyId: this.propertyId,
      coordinateId: this.coordinateId,
      recordCount: this.recordIds.length,
      nativeEvidence: this.nativeEvidence,
      visualPriority: visualHierarchy,
      visualHierarchy,
      semanticRenderPriority: visualHierarchy.sortPriority,
    };
    return [descriptor];
  }

  toSpacePropertiesFieldDescriptor() {
    const view = new SpacePropertiesView({
      ...this.fieldOptions,
      id: this.id,
      name: this.name,
      datasetId: this.datasetId,
      spaceId: this.spaceId,
      coordinateId: this.coordinateId,
      propertyId: this.propertyId,
      propertyName: this.propertyName,
      records: this.records,
      recordIds: this.recordIds,
      positions: this.positions,
      scalarValues: this.scalarValues,
      field: true,
      ground: false,
      groundProjection: false,
      metadata: {
        ...(this.metadata || {}),
        viewClass: "PropertyFieldView",
        nativeEvidence: this.nativeEvidence,
        algorithmicComputation: false,
      },
    });
    return view.fieldDescriptor();
  }
}

export function createPropertyFieldView(options) {
  return new PropertyFieldView(options);
}

function resolveProperty(document, options = {}) {
  const explicitRef = options.propertyId
    ?? options.property
    ?? options.property_id
    ?? (typeof options.propertyField === "string" ? options.propertyField : undefined)
    ?? (typeof options.groundField === "string" ? options.groundField : undefined);
  const propertyRef = explicitRef
    ?? options.scalarProperty
    ?? options.scalarPropertyId
    ?? firstScalarPropertyId(document);
  const property = resolveCollectionItem(document, "properties", propertyRef, {
    required: propertyRef != null,
    label: explicitRef == null ? "default property" : "property",
  });
  if (!property) {
    throw new Error("PropertyFieldView requires a scalar property or a document default scalar property.");
  }
  const valueType = property.value_type ?? property.valueType;
  if (valueType != null && valueType !== "scalar" && valueType !== "rank" && valueType !== "number") {
    throw new Error(`PropertyFieldView requires a scalar property; "${property.id}" has value_type "${valueType}".`);
  }
  return { item: property, defaulted: explicitRef == null };
}

function resolveCoordinate(document, options = {}, metricSpaceView = null) {
  const explicitRef = options.coordinate ?? options.coordinateId ?? options.coordinate_id;
  const space = resolveSpace(document, options, null);
  const coordinateRef = explicitRef
    ?? metricSpaceView?.coordinateId
    ?? defaultCoordinateId(document, space, { dimension: 3 });
  const coordinate = resolveCollectionItem(document, "coordinates", coordinateRef, {
    required: coordinateRef != null,
    label: explicitRef == null ? "default coordinate" : "coordinate",
  });
  if (!coordinate) {
    throw new Error("PropertyFieldView requires coordinates or a document default coordinate.");
  }
  return { item: coordinate, defaulted: explicitRef == null };
}

function resolveSpace(document, options = {}, coordinate = null) {
  const spaceRef = options.space ?? options.spaceId ?? options.space_id ?? coordinate?.space_id ?? coordinate?.spaceId;
  return resolveCollectionItem(document, "spaces", spaceRef, {
    required: options.space != null || options.spaceId != null || options.space_id != null,
    label: "space",
  });
}

function firstScalarPropertyId(document) {
  const properties = Array.isArray(document?.properties) ? document.properties : [];
  const property = properties.find((entry) => {
    const valueType = entry?.value_type ?? entry?.valueType;
    return valueType === "scalar" || valueType === "rank" || valueType === "number";
  }) || properties[0];
  return property?.id;
}

function createPropertyFieldNativeEvidence(document, options = {}) {
  return {
    schema: PROPERTY_FIELD_VIEW_EVIDENCE_SCHEMA,
    source: "exported-record-property",
    documentSchema: document?.schema || null,
    provenance: document?.provenance || null,
    datasetId: options.datasetId || null,
    spaceId: options.spaceId || null,
    coordinateId: options.coordinateId || null,
    propertyId: options.propertyId || null,
    propertyName: options.propertyName || null,
    propertyValueType: options.propertyValueType || null,
    recordCount: Number.isFinite(Number(options.recordCount)) ? Number(options.recordCount) : 0,
    propertyDefaulted: options.propertyDefaulted === true,
    coordinateDefaulted: options.coordinateDefaulted === true,
    fittedCoordinateSource: options.fittedCoordinateSource === true,
  };
}
