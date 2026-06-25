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
  const document = context.document || context.runtime?.document || visualSpace?.document || {};
  const recordId = input.recordId || input.focusTarget?.recordId;
  if (!visualSpace || recordId == null) return null;
  const record = visualSpace.getRecord?.(recordId);
  if (!record) return null;
  const payload = record.payload || {};
  const payloadSnippet = payloadSnippetFields(payload, context.maxPayloadFields ?? 6);
  const payloadPreview = describeRecordPayload(payload, context);
  const recordProperties = collectRecordProperties(document, visualSpace, record.id, context.maxRecordProperties ?? 8);
  const linkedViews = collectRecordCoordinateLinks(document, visualSpace, record.id, context.maxLinkedViews ?? 6);
  const fields = [
    { label: "id", value: record.id },
    { label: "type", value: record.record_type },
    { label: "dataset", value: record.dataset_id },
  ];
  if (payloadPreview.kind) fields.push({ label: "payload", value: payloadPreview.kind });
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
      properties: recordProperties,
    },
    payloadKind: payloadPreview.kind,
    payloadSnippet,
    linkedViews,
    fields: fields.concat(featureFields),
    sections: [],
  };
  const payloadFields = payloadPreview.fields.length ? payloadPreview.fields : payloadSnippet;
  if (payloadFields.length > 0 || payloadPreview.sectionSeries.length > 0 || payloadPreview.image) {
    preview.sections.push({
      title: "payload",
      fields: payloadFields,
      series: payloadPreview.sectionSeries,
      image: payloadPreview.image,
    });
  }
  preview.sections.push(...payloadPreview.sections);
  if (recordProperties.length > 0) {
    preview.sections.push({
      title: "record properties",
      fields: recordProperties,
    });
  }
  if (linkedViews.length > 0) {
    preview.sections.push({
      title: "linked views",
      fields: linkedViews.map((link) => ({
        label: link.name || link.coordinateId,
        value: `${link.spaceId || "space"} ${formatPosition(link.position)}`,
      })),
    });
  }
  if (payloadPreview.series.length > 0) {
    preview.series = payloadPreview.series;
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

  const hasExplicitRelation = Object.prototype.hasOwnProperty.call(pair, "relationId")
    || Object.prototype.hasOwnProperty.call(pair, "relation_id");
  const relationId = hasExplicitRelation ? (pair.relationId ?? pair.relation_id ?? null) : firstRelationId(document);
  const relation = relationId ? visualSpace.getRelation?.(relationId) : null;
  const relationName = pair.relationName ?? pair.relation_name ?? relation?.name ?? null;
  const entryMatch = relationId ? relationEntryForPair(visualSpace, relation, relationId, rowId, columnId) : null;
  const entry = pair.value !== undefined ? pair : entryMatch?.entry || null;
  const rowRecord = visualSpace.getRecord?.(rowId);
  const columnRecord = visualSpace.getRecord?.(columnId);
  const rowRecordProperties = collectRecordProperties(document, visualSpace, rowId, context.maxPairRecordProperties ?? 5);
  const columnRecordProperties = collectRecordProperties(document, visualSpace, columnId, context.maxPairRecordProperties ?? 5);
  const value = entry?.value ?? pair.value;
  const present = pair.present ?? Boolean(entry);
  const directPairProperties = normalizePairPropertyFields(pair.properties);
  const documentPairProperties = collectPairProperties(document, visualSpace, {
    relationId,
    rowId,
    columnId,
    symmetric: isSymmetricRelation(relation),
  });
  const pairProperties = mergePairPropertyFields(directPairProperties, documentPairProperties);
  const rowRecordSummary = summarizeRecordForPair(rowRecord, rowRecordProperties);
  const columnRecordSummary = summarizeRecordForPair(columnRecord, columnRecordProperties);
  const fields = [
    { label: "relation id", value: relationId || "relation" },
    { label: "relation name", value: relationName || relation?.id || relationId || "relation" },
    { label: "row id", value: rowId },
    { label: "column id", value: columnId },
    { label: "row record", value: labelForRecord(rowRecord, rowId) },
    { label: "column record", value: labelForRecord(columnRecord, columnId) },
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
      row: Number.isFinite(Number(pair.row)) ? Number(pair.row) : (entryMatch?.reversed ? null : numericOrNull(entry?.row)),
      column: Number.isFinite(Number(pair.column)) ? Number(pair.column) : (entryMatch?.reversed ? null : numericOrNull(entry?.column)),
      value,
      present: Boolean(present),
      properties: pairProperties,
      reversedLookup: Boolean(entryMatch?.reversed),
    },
    records: {
      row: rowRecordSummary,
      column: columnRecordSummary,
    },
    fields,
    sections: pairPreviewSections({
      pairProperties,
      rowRecord,
      columnRecord,
      rowRecordProperties,
      columnRecordProperties,
      context,
    }),
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
  if (preview.image) {
    element.appendChild(renderImagePreview(preview.image));
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
    if (section?.image) sectionElement.appendChild(renderImagePreview(section.image));
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

function renderImagePreview(image) {
  const figure = document.createElement("figure");
  figure.className = "mtrc-record-preview__image";
  const img = document.createElement("img");
  img.src = stringValue(image.href || image.src || "");
  img.alt = stringValue(image.alt || image.label || "record image");
  if (Number.isFinite(Number(image.width))) img.width = Number(image.width);
  if (Number.isFinite(Number(image.height))) img.height = Number(image.height);
  figure.appendChild(img);
  if (image.label) {
    const caption = document.createElement("figcaption");
    caption.textContent = stringValue(image.label);
    figure.appendChild(caption);
  }
  return figure;
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

    .mtrc-record-preview__image {
      display: grid;
      gap: 5px;
      margin: 8px 0 0;
    }

    .mtrc-record-preview__image img {
      display: block;
      width: 100%;
      max-height: 120px;
      object-fit: contain;
      border-radius: 8px;
      background: rgba(255, 252, 235, 0.42);
    }

    .mtrc-record-preview__image figcaption {
      color: #607073;
      font-size: 10px;
    }
  `;
  document.head.appendChild(style);
}

function formatValue(value) {
  if (Array.isArray(value)) return value.map((entry) => formatNumber(entry)).join(", ");
  if (typeof value === "number") return formatNumber(value);
  if (value && typeof value === "object") return summarizePreviewValue(value);
  if (value == null) return "";
  return String(value);
}

function formatNumber(value) {
  return Number.isFinite(value) ? value.toFixed(Math.abs(value) >= 10 ? 2 : 4) : String(value);
}

function formatPosition(position) {
  if (!Array.isArray(position)) return "";
  return `[${position.slice(0, 3).map((value) => formatNumber(Number(value))).join(", ")}]`;
}

function numericOrNull(value) {
  return Number.isFinite(Number(value)) ? Number(value) : null;
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

function describeRecordPayload(payload, context = {}, depth = 0) {
  const maxFields = context.maxPayloadFields ?? 6;
  const maxComponents = context.maxPayloadComponents ?? 4;
  const maxNamedValues = context.maxNamedPayloadValues ?? 6;
  const kind = payloadKind(payload);
  const preview = {
    kind,
    fields: [],
    sections: [],
    series: [],
    sectionSeries: [],
    image: null,
  };

  if (!payload || typeof payload !== "object") return preview;
  if (kind) preview.fields.push({ label: "kind", value: kind });

  if (kind === "string" || typeof payload.text === "string") {
    preview.fields.push({ label: "text", value: payload.text ?? payload.value ?? "" });
  }

  if (Array.isArray(payload.values)) {
    preview.fields.push({ label: "values", value: summarizePreviewValue(payload.values) });
    const named = namedNumericFields(payload.values, payload.names, maxNamedValues);
    preview.fields.push(...named);
    if (payload.values.every((value) => Number.isFinite(Number(value)))) {
      preview.series.push({ label: "values", values: payload.values, color: "rgba(32, 118, 132, 0.86)" });
    }
  } else if (payload.values !== undefined) {
    preview.fields.push({ label: "value", value: summarizePreviewValue(payload.values) });
  }

  if (Array.isArray(payload.series)) {
    preview.fields.push({ label: "samples", value: payload.series.length });
    if (payload.sample_rate_hz !== undefined) preview.fields.push({ label: "sample rate", value: `${payload.sample_rate_hz} Hz` });
    if (payload.unit !== undefined) preview.fields.push({ label: "unit", value: payload.unit });
    preview.series.push({ label: "series", values: payload.series, color: "rgba(32, 118, 132, 0.86)" });
  }

  if (Array.isArray(payload.bins)) {
    preview.fields.push({ label: "bins", value: payload.bins.length });
    if (Array.isArray(payload.edges)) preview.fields.push({ label: "edges", value: payload.edges.length });
    preview.sectionSeries.push({ label: "bins", values: payload.bins, color: "rgba(116, 81, 154, 0.82)" });
  }

  const href = payload.href || payload.url || payload.src;
  if (kind === "image" || kind === "image_ref" || href) {
    if (href) preview.fields.push({ label: "href", value: href });
    if (payload.width !== undefined || payload.height !== undefined) {
      preview.fields.push({ label: "size", value: `${payload.width ?? "?"} x ${payload.height ?? "?"}` });
    }
    if (payload.alt) preview.fields.push({ label: "alt", value: payload.alt });
    if (href) {
      preview.image = {
        href,
        width: payload.width,
        height: payload.height,
        alt: payload.alt,
        label: payload.label,
      };
    }
  }

  const components = payloadComponents(payload);
  if (components.length > 0) {
    preview.fields.push({ label: "components", value: components.length });
    if (depth < 2) {
      for (const component of components.slice(0, maxComponents)) {
        const child = describeRecordPayload(component.payload, context, depth + 1);
        const fields = child.fields.length ? child.fields : payloadSnippetFields(component.payload, maxFields);
        preview.sections.push({
          title: component.label ? `component: ${component.label}` : "component",
          fields,
          series: child.series.concat(child.sectionSeries),
          image: child.image,
        });
      }
      if (components.length > maxComponents) {
        preview.fields.push({ label: "more components", value: components.length - maxComponents });
      }
    }
  }

  if (payload.features && typeof payload.features === "object" && !Array.isArray(payload.features)) {
    for (const [label, value] of Object.entries(payload.features).slice(0, maxFields)) {
      if (!preview.fields.some((field) => field.label === label)) {
        preview.fields.push({ label, value: summarizePreviewValue(value) });
      }
    }
  }

  if (preview.fields.length <= (kind ? 1 : 0)) {
    preview.fields.push(...payloadSnippetFields(payload, maxFields));
  }

  preview.fields = preview.fields.slice(0, Math.max(maxFields + 2, maxFields));
  return preview;
}

function payloadKind(payload) {
  if (!payload || typeof payload !== "object") return null;
  if (payload.kind) return String(payload.kind);
  if (typeof payload.text === "string") return "string";
  if (Array.isArray(payload.series)) return "time_series";
  if (Array.isArray(payload.bins)) return "histogram";
  if (Array.isArray(payload.values)) return "vector";
  if (payload.fields || payload.components || payload.parts) return "composed";
  if (payload.href || payload.url || payload.src) return "external_ref";
  return null;
}

function namedNumericFields(values, names, limit) {
  if (!Array.isArray(values) || !Array.isArray(names)) return [];
  const fields = [];
  const count = Math.min(values.length, names.length, limit);
  for (let index = 0; index < count; index += 1) {
    fields.push({ label: String(names[index]), value: values[index] });
  }
  return fields;
}

function payloadComponents(payload) {
  if (!payload || typeof payload !== "object") return [];
  if (Array.isArray(payload.parts)) {
    return payload.parts
      .map((part, index) => ({ label: part?.label ?? part?.name ?? `part ${index + 1}`, payload: part }))
      .filter((part) => part.payload && typeof part.payload === "object");
  }
  const source = payload.fields || payload.components;
  if (source && typeof source === "object" && !Array.isArray(source)) {
    return Object.entries(source)
      .map(([label, value]) => ({ label, payload: value }))
      .filter((part) => part.payload && typeof part.payload === "object");
  }
  return [];
}

function summarizePreviewValue(value) {
  if (Array.isArray(value)) return `${value.length} values`;
  if (value && typeof value === "object") return `${Object.keys(value).length} fields`;
  return value;
}

function collectPairProperties(document, visualSpace, pair) {
  const fields = [];
  const targets = pairPropertyTargets(pair);
  for (const property of document.properties || []) {
    if (property.target_type !== "pair") continue;
    for (const target of targets) {
      const propertyValue = visualSpace.propertyValue?.(property.id, target);
      if (!propertyValue) continue;
      fields.push({
        id: property.id,
        label: property.name || property.id,
        value: propertyValue.value,
        valueType: property.value_type || null,
      });
      break;
    }
  }
  return fields;
}

function collectRecordProperties(document, visualSpace, recordId, maxProperties = 8) {
  const fields = [];
  if (recordId == null) return fields;
  for (const property of document.properties || []) {
    if (property.target_type !== "record") continue;
    const propertyValue = visualSpace.propertyValue?.(property.id, { record_id: recordId });
    if (!propertyValue) continue;
    fields.push({
      id: property.id,
      label: property.name || property.id,
      value: propertyValue.value,
      valueType: property.value_type || null,
    });
    if (fields.length >= maxProperties) break;
  }
  return fields;
}

function collectRecordCoordinateLinks(document, visualSpace, recordId, maxLinks = 6) {
  const links = [];
  if (!visualSpace || recordId == null) return links;
  for (const coordinate of document.coordinates || []) {
    const state = visualSpace.getCoordinateState?.(coordinate.id);
    const position = state?.rawPositionByRecordId?.get(recordId) || state?.positionByRecordId?.get(recordId);
    if (!position) continue;
    links.push({
      coordinateId: coordinate.id,
      name: coordinate.name || coordinate.id,
      spaceId: coordinate.space_id || coordinate.spaceId || null,
      position: Array.from(position),
    });
    if (links.length >= maxLinks) break;
  }
  return links;
}

function relationEntryForPair(visualSpace, relation, relationId, rowId, columnId) {
  const direct = visualSpace.relationValue?.(relationId, rowId, columnId);
  if (direct) return { entry: direct, reversed: false };
  if (!isSymmetricRelation(relation)) return null;
  const reverse = visualSpace.relationValue?.(relationId, columnId, rowId);
  return reverse ? { entry: reverse, reversed: true } : null;
}

function isSymmetricRelation(relation) {
  if (!relation) return false;
  const storage = String(relation.storage || "");
  const lawCheck = relation.metadata?.law_check || {};
  return relation.metadata?.symmetric === true
    || storage.includes("symmetric")
    || lawCheck.symmetric === true
    || lawCheck.symmetry === true
    || lawCheck.symmetry === "checked_exhaustive"
    || lawCheck.symmetry === "symmetric";
}

function pairPropertyTargets(pair = {}) {
  const targets = [];
  const relationId = pair.relationId ?? pair.relation_id;
  const rowId = pair.rowId ?? pair.row_id;
  const columnId = pair.columnId ?? pair.column_id;
  if (rowId == null || columnId == null) return targets;

  pushUniquePairTarget(targets, { relation_id: relationId, row_id: rowId, column_id: columnId });
  pushUniquePairTarget(targets, { row_id: rowId, column_id: columnId });
  if (pair.symmetric) {
    pushUniquePairTarget(targets, { relation_id: relationId, row_id: columnId, column_id: rowId });
    pushUniquePairTarget(targets, { row_id: columnId, column_id: rowId });
  }
  return targets;
}

function pushUniquePairTarget(targets, target) {
  const key = `${target.relation_id ?? ""}\u0000${target.row_id ?? ""}\u0000${target.column_id ?? ""}`;
  if (targets.some((candidate) => `${candidate.relation_id ?? ""}\u0000${candidate.row_id ?? ""}\u0000${candidate.column_id ?? ""}` === key)) return;
  targets.push(target);
}

function summarizeRecordForPair(record, properties = []) {
  if (!record) return null;
  return {
    id: record.id,
    label: record.label || null,
    type: record.record_type || null,
    datasetId: record.dataset_id || null,
    payloadKind: payloadKind(record.payload),
    properties,
  };
}

function pairPreviewSections({ pairProperties, rowRecord, columnRecord, rowRecordProperties, columnRecordProperties, context }) {
  const sections = [];
  if (pairProperties.length > 0) {
    sections.push({ title: "pair properties", fields: pairProperties });
  }
  const rowFields = recordContextFields(rowRecord, rowRecordProperties, context);
  if (rowFields.length > 0) {
    sections.push({ title: "row record", fields: rowFields });
  }
  const columnFields = recordContextFields(columnRecord, columnRecordProperties, context);
  if (columnFields.length > 0) {
    sections.push({ title: "column record", fields: columnFields });
  }
  return sections;
}

function recordContextFields(record, properties, context = {}) {
  if (!record) return [];
  const fields = [
    { label: "id", value: record.id },
    { label: "label", value: record.label || record.id },
    { label: "type", value: record.record_type },
  ];
  const payload = describeRecordPayload(record.payload || {}, {
    ...context,
    maxPayloadFields: Math.min(context.maxPairPayloadFields ?? 4, 4),
    maxPayloadComponents: Math.min(context.maxPairPayloadComponents ?? 2, 2),
  });
  if (payload.kind) fields.push({ label: "payload", value: payload.kind });
  for (const property of properties.slice(0, context.maxPairRecordProperties ?? 5)) {
    fields.push(property);
  }
  for (const field of payloadSnippetFields(record.payload || {}, context.maxPairPayloadFields ?? 4)) {
    if (!fields.some((existing) => existing.label === field.label)) fields.push(field);
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
