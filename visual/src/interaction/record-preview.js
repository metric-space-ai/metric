const STYLE_ID = "mtrc-record-preview-panel-style";

export class RecordPreviewPanel {
  constructor(options = {}) {
    const {
      root = document.body,
      className = "mtrc-record-preview",
      offset = [18, 18],
      maxFields = 8,
      resolver = null,
      runtime = null,
      visualSpace = null,
      document: evidenceDocument = null,
      render = null,
    } = options;
    this.root = root || document.body || document.documentElement;
    this.className = className;
    this.offset = offset;
    this.maxFields = maxFields;
    this.runtime = runtime;
    this.visualSpace = visualSpace || runtime?.visualSpace || null;
    this.document = evidenceDocument || runtime?.document || this.visualSpace?.document || null;
    this.resolver = resolver || (this.visualSpace ? createMetricEvidencePreviewResolver({
      runtime,
      visualSpace: this.visualSpace,
      document: this.document,
    }) : null);
    this.customRender = typeof render === "function" ? render : null;
    this.element = document.createElement("aside");
    this.element.className = className;
    this.element.setAttribute("aria-hidden", "true");
    this.element.dataset.visible = "false";
    this.root.appendChild(this.element);
    installRecordPreviewStyle();
  }

  setResolver(resolver) {
    this.resolver = typeof resolver === "function" ? resolver : null;
    return this;
  }

  setEvidenceContext(context = {}) {
    this.runtime = context.runtime || this.runtime || null;
    this.visualSpace = context.visualSpace || this.runtime?.visualSpace || this.visualSpace || null;
    this.document = context.document || this.runtime?.document || this.visualSpace?.document || this.document || null;
    if (!context.resolver && this.visualSpace) {
      this.resolver = createMetricEvidencePreviewResolver({
        runtime: this.runtime,
        visualSpace: this.visualSpace,
        document: this.document,
      });
    }
    return this;
  }

  attachToRuntime(runtime, options = {}) {
    this.detachRuntime?.();
    this.setEvidenceContext({ runtime, ...options });
    const offInspection = runtime?.on?.("inspectionchange", ({ detail, result }) => {
      if (!detail?.hit) {
        if (options.hideOnMiss !== false) this.hide();
        return;
      }
      const pointer = result?.request?.rawPointer || result?.request?.pointer || {};
      const pair = detail.pair || null;
      this.show({
        recordId: detail.recordId,
        pair,
        focusTarget: pair ? { pair } : { recordId: detail.recordId },
        x: pointer.client?.x ?? pointer.x ?? options.x ?? 0,
        y: pointer.client?.y ?? pointer.y ?? options.y ?? 0,
      });
    });
    const offSelection = options.selection === false ? null : runtime?.on?.("selectionchange", ({ selection }) => {
      if (selection?.pair) {
        this.show({
          pair: selection.pair,
          x: options.selectionX ?? viewportAnchorX(0.58),
          y: options.selectionY ?? viewportAnchorY(0.34),
        });
      } else if (selection?.recordId) {
        this.show({
          recordId: selection.recordId,
          x: options.selectionX ?? viewportAnchorX(0.58),
          y: options.selectionY ?? viewportAnchorY(0.34),
        });
      }
    });
    this.detachRuntime = () => {
      offInspection?.();
      offSelection?.();
      this.detachRuntime = null;
    };
    return this;
  }

  setRenderer(render) {
    this.customRender = typeof render === "function" ? render : null;
    return this;
  }

  show(input) {
    const preview = this.resolvePreview(input);
    if (!preview) {
      this.hide();
      return;
    }
    if (this.customRender) {
      const rendered = this.customRender(preview, input);
      this.element.replaceChildren();
      if (rendered instanceof Node) {
        this.element.appendChild(rendered);
      } else {
        renderPreview(this.element, preview, this.maxFields);
      }
    } else {
      renderPreview(this.element, preview, this.maxFields);
    }
    this.moveTo(input?.x ?? input?.clientX ?? 0, input?.y ?? input?.clientY ?? 0);
    this.element.dataset.visible = "true";
    this.element.setAttribute("aria-hidden", "false");
  }

  moveTo(x, y) {
    const [offsetX, offsetY] = this.offset;
    const width = this.element.offsetWidth || 260;
    const height = this.element.offsetHeight || 120;
    const margin = 14;
    const nextX = clamp(x + offsetX, margin, window.innerWidth - width - margin);
    const nextY = clamp(y + offsetY, margin, window.innerHeight - height - margin);
    this.element.style.transform = `translate3d(${Math.round(nextX)}px, ${Math.round(nextY)}px, 0)`;
  }

  hide() {
    this.element.dataset.visible = "false";
    this.element.setAttribute("aria-hidden", "true");
  }

  dispose() {
    this.detachRuntime?.();
    this.element.remove();
  }

  resolvePreview(input) {
    const value = this.resolver ? this.resolver(input) : input?.preview;
    if (!value) return null;
    if (typeof value === "string") {
      return { title: value, fields: [] };
    }
    return value;
  }
}

export function createMetricEvidencePreviewResolver(context = {}) {
  return (input = {}) => buildMetricEvidencePreview(input, context);
}

export function buildMetricEvidencePreview(input = {}, context = {}) {
  if (input.pair || input.pairId || input.rowId || input.row_id || input.focusTarget?.pair) {
    return buildMetricPairPreview(input, context);
  }
  return buildMetricRecordPreview(input, context);
}

export function buildMetricRecordPreview(input = {}, context = {}) {
  const visualSpace = resolveVisualSpace(context);
  const recordId = input.recordId || input.focusTarget?.recordId;
  if (!visualSpace || recordId == null) return null;
  const record = visualSpace.getRecord?.(recordId);
  if (!record) return null;
  const payload = record.payload || {};
  const payloadSnippet = payloadSnippetFields(payload, context.maxPayloadFields ?? 6);
  const fields = [
    { label: "id", value: record.id },
    { label: "type", value: record.record_type },
    { label: "dataset", value: record.dataset_id },
  ];
  const featureFields = payload.features && typeof payload.features === "object"
    ? Object.entries(payload.features).slice(0, 5).map(([label, value]) => ({ label, value }))
    : [];
  const preview = {
    kind: "record",
    source: "metric.visual.v1",
    recordId: record.id,
    title: record.label || record.id,
    subtitle: record.record_type || "record",
    record: {
      id: record.id,
      label: record.label || null,
      type: record.record_type || null,
      datasetId: record.dataset_id || null,
    },
    payloadSnippet,
    fields: fields.concat(featureFields),
    sections: [],
  };
  if (payloadSnippet.length > 0) {
    preview.sections.push({
      title: "payload",
      fields: payloadSnippet,
    });
  }
  if (Array.isArray(payload.series)) {
    preview.series = [{ label: "series", values: payload.series, color: "rgba(32, 118, 132, 0.86)" }];
  } else if (Array.isArray(payload.values) && payload.values.every((value) => Number.isFinite(Number(value)))) {
    preview.series = [{ label: "values", values: payload.values, color: "rgba(32, 118, 132, 0.86)" }];
  } else if (payload.values !== undefined) {
    preview.sections.push({
      title: "payload",
      fields: [{ label: "value", value: summarizePreviewValue(payload.values) }],
    });
  }
  return preview;
}

export function buildMetricPairPreview(input = {}, context = {}) {
  const visualSpace = resolveVisualSpace(context);
  const document = context.document || context.runtime?.document || visualSpace?.document || {};
  const pair = input.pair || input.focusTarget?.pair || input;
  const rowId = pair.rowId ?? pair.row_id ?? pair.sourceId ?? pair.source_id;
  const columnId = pair.columnId ?? pair.column_id ?? pair.targetId ?? pair.target_id;
  if (!visualSpace || rowId == null || columnId == null) return null;

  const relationId = pair.relationId ?? pair.relation_id ?? firstRelationId(document);
  const relation = relationId ? visualSpace.getRelation?.(relationId) : null;
  const relationName = pair.relationName ?? pair.relation_name ?? relation?.name ?? null;
  const entry = pair.value !== undefined
    ? pair
    : relationId
      ? visualSpace.relationValue?.(relationId, rowId, columnId)
      : null;
  const rowRecord = visualSpace.getRecord?.(rowId);
  const columnRecord = visualSpace.getRecord?.(columnId);
  const value = entry?.value ?? pair.value;
  const present = pair.present ?? Boolean(entry);
  const directPairProperties = normalizePairPropertyFields(pair.properties);
  const documentPairProperties = collectPairProperties(document, visualSpace, {
    relationId,
    rowId,
    columnId,
  });
  const pairProperties = mergePairPropertyFields(directPairProperties, documentPairProperties);
  const fields = [
    { label: "relation id", value: relationId || "relation" },
    { label: "relation name", value: relationName || relation?.id || relationId || "relation" },
    { label: "row id", value: rowId },
    { label: "column id", value: columnId },
    { label: "value", value: present && Number.isFinite(Number(value)) ? Number(value).toFixed(4) : (present ? value : "no direct pair") },
  ];
  if (relation?.relation_type) fields.splice(2, 0, { label: "relation type", value: relation.relation_type });

  return {
    kind: "pair",
    source: "metric.visual.v1",
    title: `${labelForRecord(rowRecord, rowId)} <-> ${labelForRecord(columnRecord, columnId)}`,
    subtitle: relation?.relation_type || "pair relation",
    pair: {
      relationId,
      relationName: relationName || null,
      relationType: relation?.relation_type || null,
      rowId,
      columnId,
      rowLabel: labelForRecord(rowRecord, rowId),
      columnLabel: labelForRecord(columnRecord, columnId),
      row: Number.isFinite(Number(pair.row ?? entry?.row)) ? Number(pair.row ?? entry?.row) : null,
      column: Number.isFinite(Number(pair.column ?? entry?.column)) ? Number(pair.column ?? entry?.column) : null,
      value,
      present: Boolean(present),
      properties: pairProperties,
    },
    fields,
    sections: pairProperties.length > 0
      ? [{ title: "pair properties", fields: pairProperties }]
      : [],
  };
}

function renderPreview(element, preview, maxFields) {
  element.replaceChildren();

  const header = document.createElement("header");
  const title = document.createElement("strong");
  title.textContent = stringValue(preview.title || "Record");
  header.appendChild(title);
  if (preview.subtitle) {
    const subtitle = document.createElement("span");
    subtitle.textContent = stringValue(preview.subtitle);
    header.appendChild(subtitle);
  }
  element.appendChild(header);

  const fields = Array.isArray(preview.fields) ? preview.fields.slice(0, maxFields) : [];
  if (fields.length > 0) {
    element.appendChild(renderFieldList(fields));
  }

  const sections = Array.isArray(preview.sections) ? preview.sections : [];
  sections.forEach((section) => {
    const sectionElement = document.createElement("section");
    sectionElement.className = "mtrc-record-preview__section";
    if (section?.title) {
      const title = document.createElement("b");
      title.textContent = stringValue(section.title);
      sectionElement.appendChild(title);
    }
    const sectionFields = Array.isArray(section?.fields) ? section.fields.slice(0, maxFields) : [];
    if (sectionFields.length > 0) sectionElement.appendChild(renderFieldList(sectionFields));
    if (Array.isArray(section?.series)) {
      section.series.slice(0, 2).forEach((series) => sectionElement.appendChild(renderSparkline(series)));
    }
    element.appendChild(sectionElement);
  });

  if (Array.isArray(preview.series)) {
    preview.series.slice(0, 2).forEach((series) => element.appendChild(renderSparkline(series)));
  }
}

function renderFieldList(fields) {
  const list = document.createElement("dl");
  fields.forEach((field) => {
    const label = document.createElement("dt");
    label.textContent = stringValue(field?.label ?? "");
    const value = document.createElement("dd");
    value.textContent = formatValue(field?.value);
    list.append(label, value);
  });
  return list;
}

function renderSparkline(series) {
  const values = Array.isArray(series?.values) ? series.values.map(Number).filter(Number.isFinite) : [];
  const canvas = document.createElement("canvas");
  canvas.className = "mtrc-record-preview__sparkline";
  canvas.width = 280;
  canvas.height = 54;
  canvas.setAttribute("aria-hidden", "true");
  if (values.length < 2) return canvas;

  const ctx = canvas.getContext("2d");
  const min = Math.min(...values);
  const max = Math.max(...values);
  const span = Math.max(1e-9, max - min);
  ctx.clearRect(0, 0, canvas.width, canvas.height);
  ctx.strokeStyle = "rgba(37, 53, 56, 0.18)";
  ctx.lineWidth = 1;
  ctx.beginPath();
  ctx.moveTo(0, canvas.height - 8);
  ctx.lineTo(canvas.width, canvas.height - 8);
  ctx.stroke();
  ctx.strokeStyle = series?.color || "rgba(34, 112, 125, 0.82)";
  ctx.lineWidth = 2;
  ctx.beginPath();
  values.forEach((value, index) => {
    const x = (index / (values.length - 1)) * canvas.width;
    const y = canvas.height - 9 - ((value - min) / span) * (canvas.height - 16);
    if (index === 0) ctx.moveTo(x, y);
    else ctx.lineTo(x, y);
  });
  ctx.stroke();
  return canvas;
}

function installRecordPreviewStyle() {
  if (document.getElementById(STYLE_ID)) return;
  const style = document.createElement("style");
  style.id = STYLE_ID;
  style.textContent = `
    .mtrc-record-preview {
      position: fixed;
      left: 0;
      top: 0;
      z-index: 12;
      width: min(320px, calc(100vw - 28px));
      padding: 12px 13px;
      border: 1px solid rgba(40, 58, 59, 0.18);
      border-radius: 12px;
      color: #243034;
      background: rgba(239, 238, 221, 0.94);
      box-shadow: 0 18px 48px rgba(44, 55, 53, 0.20);
      pointer-events: none;
      opacity: 0;
      transition: opacity 120ms ease;
      will-change: transform, opacity;
      backdrop-filter: blur(9px);
    }

    .mtrc-record-preview[data-visible="true"] {
      opacity: 0.96;
    }

    .mtrc-record-preview header {
      display: grid;
      gap: 2px;
      margin-bottom: 9px;
    }

    .mtrc-record-preview strong {
      font-size: 13px;
      font-weight: 760;
      letter-spacing: 0;
    }

    .mtrc-record-preview header span {
      color: #607073;
      font-size: 12px;
    }

    .mtrc-record-preview dl {
      display: grid;
      grid-template-columns: max-content 1fr;
      gap: 5px 12px;
      margin: 0;
    }

    .mtrc-record-preview dt {
      color: #6a7678;
      font-size: 11px;
      font-weight: 660;
    }

    .mtrc-record-preview dd {
      min-width: 0;
      margin: 0;
      color: #253538;
      font-size: 11px;
      font-variant-numeric: tabular-nums;
      overflow-wrap: anywhere;
      text-align: right;
    }

    .mtrc-record-preview__section {
      display: grid;
      gap: 7px;
      margin-top: 10px;
      padding-top: 9px;
      border-top: 1px solid rgba(42, 58, 59, 0.12);
    }

    .mtrc-record-preview__section b {
      color: #435053;
      font-size: 11px;
      font-weight: 760;
      letter-spacing: 0;
    }

    .mtrc-record-preview__sparkline {
      display: block;
      width: 100%;
      height: 54px;
      border-radius: 8px;
      background: rgba(255, 252, 235, 0.32);
    }
  `;
  document.head.appendChild(style);
}

function formatValue(value) {
  if (Array.isArray(value)) return value.map((entry) => formatNumber(entry)).join(", ");
  if (typeof value === "number") return formatNumber(value);
  if (value == null) return "";
  return String(value);
}

function formatNumber(value) {
  return Number.isFinite(value) ? value.toFixed(Math.abs(value) >= 10 ? 2 : 4) : String(value);
}

function stringValue(value) {
  return value == null ? "" : String(value);
}

function resolveVisualSpace(context = {}) {
  return context.visualSpace || context.runtime?.visualSpace || null;
}

function firstRelationId(document = {}) {
  return Array.isArray(document.relations) && document.relations.length ? document.relations[0].id : null;
}

function labelForRecord(record, fallback) {
  return record?.label || record?.id || fallback;
}

function payloadSnippetFields(payload, maxFields) {
  if (!payload || typeof payload !== "object") return [];
  const fields = [];
  for (const [label, value] of Object.entries(payload)) {
    if (label === "series" || label === "values") continue;
    if (label === "features" && value && typeof value === "object" && !Array.isArray(value)) {
      for (const [featureLabel, featureValue] of Object.entries(value)) {
        fields.push({ label: featureLabel, value: summarizePreviewValue(featureValue) });
        if (fields.length >= maxFields) return fields;
      }
      continue;
    }
    fields.push({ label, value: summarizePreviewValue(value) });
    if (fields.length >= maxFields) return fields;
  }
  if (Array.isArray(payload.values)) fields.push({ label: "values", value: summarizePreviewValue(payload.values) });
  if (Array.isArray(payload.series)) fields.push({ label: "series", value: summarizePreviewValue(payload.series) });
  return fields.slice(0, maxFields);
}

function summarizePreviewValue(value) {
  if (Array.isArray(value)) return `${value.length} values`;
  if (value && typeof value === "object") return `${Object.keys(value).length} fields`;
  return value;
}

function collectPairProperties(document, visualSpace, pair) {
  const fields = [];
  for (const property of document.properties || []) {
    if (property.target_type !== "pair") continue;
    const propertyValue = visualSpace.propertyValue?.(property.id, {
      relation_id: pair.relationId,
      row_id: pair.rowId,
      column_id: pair.columnId,
    });
    if (propertyValue) fields.push({ label: property.name || property.id, value: propertyValue.value });
  }
  return fields;
}

function normalizePairPropertyFields(properties) {
  if (!properties) return [];
  if (Array.isArray(properties)) {
    return properties.map(normalizePairPropertyField).filter(Boolean);
  }
  if (properties instanceof Map) {
    return Array.from(properties, ([label, value]) => normalizePairPropertyField({ label, value })).filter(Boolean);
  }
  if (typeof properties === "object") {
    return Object.entries(properties)
      .map(([label, value]) => normalizePairPropertyField({ label, value }))
      .filter(Boolean);
  }
  return [];
}

function normalizePairPropertyField(property) {
  if (!property || typeof property !== "object") return null;
  const label = property.label ?? property.name ?? property.id;
  if (label == null) return null;
  return {
    label: String(label),
    value: property.value,
  };
}

function mergePairPropertyFields(...groups) {
  const fields = [];
  const seen = new Set();
  for (const group of groups) {
    for (const field of group || []) {
      const key = `${field.label}\u0000${String(field.value)}`;
      if (seen.has(key)) continue;
      seen.add(key);
      fields.push(field);
    }
  }
  return fields;
}

function viewportAnchorX(ratio) {
  return (Number(globalThis.innerWidth) || 0) * ratio;
}

function viewportAnchorY(ratio) {
  return (Number(globalThis.innerHeight) || 0) * ratio;
}

function clamp(value, min, max) {
  return Math.min(max, Math.max(min, value));
}
