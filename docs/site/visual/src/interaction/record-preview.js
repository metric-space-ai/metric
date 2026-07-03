const STYLE_ID = "mtrc-record-preview-panel-style";
export const PREVIEW_PRESENTATION_DIAGNOSTICS_SCHEMA = "metric.visual.preview_presentation_diagnostics.v1";

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
    diagnostics: createRecordPreviewDiagnostics({
      payloadPreview,
      payloadSnippet,
      recordProperties,
      linkedViews,
    }),
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
    diagnostics: createPairPreviewDiagnostics({
      relationId,
      relation,
      relationName,
      rowRecord,
      columnRecord,
      pairProperties,
      present,
      entryMatch,
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
  if (Array.isArray(image?.values) || Array.isArray(image?.pixels)) {
    const canvas = document.createElement("canvas");
    const width = Math.max(1, Math.min(256, Math.round(Number(image.renderWidth || image.width) || 1)));
    const height = Math.max(1, Math.min(256, Math.round(Number(image.renderHeight || image.height) || 1)));
    canvas.width = width;
    canvas.height = height;
    canvas.setAttribute("aria-label", stringValue(image.alt || image.label || "record image array"));
    drawCompactImage(canvas, image);
    figure.appendChild(canvas);
    if (image.label) {
      const caption = document.createElement("figcaption");
      caption.textContent = stringValue(image.label);
      figure.appendChild(caption);
    }
    return figure;
  }
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

function drawCompactImage(canvas, image) {
  const ctx = canvas.getContext?.("2d");
  if (!ctx) return;
  const values = (image.values || image.pixels || []).map(Number);
  const width = canvas.width;
  const height = canvas.height;
  const channels = Math.max(1, Math.round(Number(image.channels) || 1));
  const range = image.valueRange || finiteRange(values);
  const data = ctx.createImageData(width, height);
  const sourceWidth = Math.max(1, Math.round(Number(image.sourceWidth || image.width || width) || width));
  const sourceHeight = Math.max(1, Math.round(Number(image.sourceHeight || image.height || height) || height));
  for (let y = 0; y < height; y += 1) {
    const sourceY = Math.min(sourceHeight - 1, Math.floor((y / Math.max(1, height)) * sourceHeight));
    for (let x = 0; x < width; x += 1) {
      const sourceX = Math.min(sourceWidth - 1, Math.floor((x / Math.max(1, width)) * sourceWidth));
      const sourceIndex = (sourceY * sourceWidth + sourceX) * channels;
      const targetIndex = (y * width + x) * 4;
      if (channels >= 3) {
        data.data[targetIndex] = scaleImageChannel(values[sourceIndex], range);
        data.data[targetIndex + 1] = scaleImageChannel(values[sourceIndex + 1], range);
        data.data[targetIndex + 2] = scaleImageChannel(values[sourceIndex + 2], range);
        data.data[targetIndex + 3] = channels >= 4 ? scaleImageChannel(values[sourceIndex + 3], { min: 0, max: 1 }) : 255;
      } else {
        const gray = scaleImageChannel(values[sourceIndex], range);
        data.data[targetIndex] = gray;
        data.data[targetIndex + 1] = gray;
        data.data[targetIndex + 2] = gray;
        data.data[targetIndex + 3] = 255;
      }
    }
  }
  ctx.putImageData(data, 0, 0);
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
      max-height: min(440px, calc(100vh - 28px));
      padding: 12px 13px;
      border: 1px solid rgba(40, 58, 59, 0.18);
      border-radius: 8px;
      color: #243034;
      background: rgb(246, 246, 238);
      box-shadow: 0 18px 48px rgba(44, 55, 53, 0.20);
      font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", system-ui, sans-serif;
      overflow: hidden auto;
      pointer-events: none;
      opacity: 0;
      transition: opacity 120ms ease;
      will-change: transform, opacity;
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

    .mtrc-record-preview__image canvas {
      display: block;
      width: 100%;
      height: auto;
      max-height: 130px;
      image-rendering: pixelated;
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
  const text = String(value);
  return text.length > 220 ? `${text.slice(0, 217)}...` : text;
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
  const maxTextLength = context.maxPayloadTextLength ?? 180;
  const maxSeriesValues = context.maxPayloadSeriesValues ?? 512;
  const kind = payloadKind(payload);
  const renderer = payloadRendererFor(payload, kind);
  const preview = {
    kind,
    renderer,
    fields: [],
    sections: [],
    series: [],
    sectionSeries: [],
    image: null,
    diagnostics: createPayloadDiagnostics({ kind, renderer }),
  };

  if (!payload || typeof payload !== "object") {
    preview.diagnostics.unavailable = true;
    preview.diagnostics.notes.push("payload unavailable");
    return preview;
  }
  if (kind) preview.fields.push({ label: "kind", value: kind });

  const textValue = payload.text ?? (isTextPayloadKind(kind) ? payload.value : undefined);
  if (renderer === "text") {
    if (typeof textValue === "string") {
      preview.fields.push({ label: "text", value: boundedTextValue(textValue, maxTextLength, preview.diagnostics) });
    } else {
      preview.fields.push({ label: "text", value: "" });
      preview.diagnostics.unavailable = true;
      preview.diagnostics.notes.push("text payload missing text/value");
    }
  }

  if (renderer === "image-reference") {
    const href = payload.href || payload.url || payload.src;
    if (href) preview.fields.push({ label: "href", value: boundedTextValue(href, maxTextLength, preview.diagnostics) });
    if (payload.width !== undefined || payload.height !== undefined) {
      preview.fields.push({ label: "size", value: `${payload.width ?? "?"} x ${payload.height ?? "?"}` });
    }
    if (payload.alt) preview.fields.push({ label: "alt", value: boundedTextValue(payload.alt, maxTextLength, preview.diagnostics) });
    if (href) {
      preview.image = {
        href,
        width: payload.width,
        height: payload.height,
        alt: payload.alt,
        label: payload.label,
      };
    } else {
      preview.diagnostics.unavailable = true;
      preview.diagnostics.notes.push("image reference missing href/url/src");
    }
  }

  if (renderer === "compact-image") {
    const compactImage = compactImageFromPayload(payload, context, preview.diagnostics);
    if (compactImage) {
      preview.image = compactImage;
      preview.fields.push({ label: "size", value: `${compactImage.sourceWidth} x ${compactImage.sourceHeight}` });
      preview.fields.push({ label: "channels", value: compactImage.channels });
      preview.fields.push({ label: "pixels", value: compactImage.sourceLength });
      if (compactImage.valueRange) {
        preview.fields.push({ label: "range", value: [compactImage.valueRange.min, compactImage.valueRange.max] });
      }
    } else {
      preview.diagnostics.unavailable = true;
      preview.diagnostics.notes.push("compact image array missing numeric pixel data");
    }
  }

  const seriesValues = seriesValuesForPayload(payload, renderer);
  if (renderer === "time-series" && Array.isArray(seriesValues)) {
    const values = summarizeNumericSeries(seriesValues, maxSeriesValues, preview.diagnostics);
    preview.fields.push({ label: "samples", value: seriesValues.length });
    if (payload.sample_rate_hz !== undefined) preview.fields.push({ label: "sample rate", value: `${payload.sample_rate_hz} Hz` });
    if (payload.unit !== undefined) preview.fields.push({ label: "unit", value: payload.unit });
    preview.series.push({ label: "series", values, color: "rgba(32, 118, 132, 0.86)" });
    preview.diagnostics.summarized = true;
  }

  const histogramValues = histogramValuesForPayload(payload, renderer);
  if (renderer === "histogram" && Array.isArray(histogramValues)) {
    const values = summarizeNumericSeries(histogramValues, maxSeriesValues, preview.diagnostics);
    preview.fields.push({ label: "bins", value: histogramValues.length });
    if (Array.isArray(payload.edges)) preview.fields.push({ label: "edges", value: payload.edges.length });
    preview.sectionSeries.push({ label: "bins", values, color: "rgba(116, 81, 154, 0.82)" });
    preview.diagnostics.summarized = true;
  }

  if (renderer === "vector" && Array.isArray(payload.values)) {
    preview.fields.push({ label: "values", value: summarizePreviewValue(payload.values) });
    const named = namedNumericFields(payload.values, payload.names, maxNamedValues);
    preview.fields.push(...named);
    if (Array.isArray(payload.names) && payload.names.length > maxNamedValues) {
      preview.diagnostics.truncated = true;
      preview.diagnostics.notes.push("named vector fields truncated");
    }
    if (payload.values.every((value) => Number.isFinite(Number(value)))) {
      preview.series.push({
        label: "values",
        values: summarizeNumericSeries(payload.values, maxSeriesValues, preview.diagnostics),
        color: "rgba(32, 118, 132, 0.86)",
      });
      preview.diagnostics.summarized = true;
    }
  } else if (renderer === "vector" && payload.values !== undefined) {
    preview.fields.push({ label: "value", value: summarizePreviewValue(payload.values) });
    if (Array.isArray(payload.values)) preview.diagnostics.summarized = true;
  }

  if (renderer === "properties") {
    const propertyFields = genericPayloadFields(payload, maxFields, preview.diagnostics);
    preview.fields.push(...propertyFields);
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
        preview.diagnostics.truncated = true;
        preview.diagnostics.notes.push("component sections truncated");
      }
    }
    preview.diagnostics.summarized = true;
  }

  if (payload.features && typeof payload.features === "object" && !Array.isArray(payload.features)) {
    const featureEntries = Object.entries(payload.features);
    for (const [label, value] of featureEntries.slice(0, maxFields)) {
      if (!preview.fields.some((field) => field.label === label)) {
        preview.fields.push({ label, value: summarizePreviewValue(value) });
      }
    }
    if (featureEntries.length > maxFields) {
      preview.diagnostics.truncated = true;
      preview.diagnostics.notes.push("feature fields truncated");
    }
  }

  if (preview.fields.length <= (kind ? 1 : 0)) {
    preview.fields.push(...payloadSnippetFields(payload, maxFields));
  }

  const fieldLimit = Math.max(maxFields + 2, maxFields);
  if (preview.fields.length > fieldLimit) {
    preview.diagnostics.truncated = true;
    preview.diagnostics.notes.push("payload fields truncated");
  }
  preview.fields = preview.fields.slice(0, fieldLimit);
  preview.diagnostics.fieldCount = preview.fields.length;
  preview.diagnostics.sectionCount = preview.sections.length;
  preview.diagnostics.seriesCount = preview.series.length + preview.sectionSeries.length;
  preview.diagnostics.image = Boolean(preview.image);
  return preview;
}

function createPayloadDiagnostics({ kind, renderer }) {
  return {
    schema: PREVIEW_PRESENTATION_DIAGNOSTICS_SCHEMA,
    source: "metric.visual.v1",
    scope: "record-payload",
    payloadKind: kind || "unknown",
    renderer: renderer || "unavailable",
    truncated: false,
    summarized: false,
    unavailable: false,
    notes: [],
  };
}

function createRecordPreviewDiagnostics({ payloadPreview, payloadSnippet, recordProperties, linkedViews }) {
  const payloadDiagnostics = payloadPreview?.diagnostics || {};
  return {
    schema: PREVIEW_PRESENTATION_DIAGNOSTICS_SCHEMA,
    source: "metric.visual.v1",
    kind: "record",
    renderer: "record-preview",
    payloadRenderer: payloadDiagnostics.renderer || payloadPreview?.renderer || "unavailable",
    payloadKind: payloadPreview?.kind || "unknown",
    truncated: Boolean(payloadDiagnostics.truncated),
    summarized: Boolean(payloadDiagnostics.summarized),
    unavailable: Boolean(payloadDiagnostics.unavailable),
    payload: payloadDiagnostics,
    counts: {
      payloadFields: payloadPreview?.fields?.length || 0,
      payloadSnippetFields: payloadSnippet?.length || 0,
      payloadSections: payloadPreview?.sections?.length || 0,
      payloadSeries: (payloadPreview?.series?.length || 0) + (payloadPreview?.sectionSeries?.length || 0),
      recordProperties: recordProperties?.length || 0,
      linkedViews: linkedViews?.length || 0,
    },
  };
}

function createPairPreviewDiagnostics({
  relationId,
  relation,
  relationName,
  rowRecord,
  columnRecord,
  pairProperties,
  present,
  entryMatch,
}) {
  return {
    schema: PREVIEW_PRESENTATION_DIAGNOSTICS_SCHEMA,
    source: "metric.visual.v1",
    kind: "pair",
    renderer: "pair-preview",
    payloadRenderer: "pair-relation",
    payloadKind: relation?.relation_type || "pair",
    truncated: false,
    summarized: false,
    unavailable: !present,
    relation: {
      id: relationId || null,
      name: relationName || relation?.name || null,
      type: relation?.relation_type || null,
      symmetric: isSymmetricRelation(relation),
      reversedLookup: Boolean(entryMatch?.reversed),
      valuePresent: Boolean(present),
    },
    fields: {
      rowIdentity: Boolean(rowRecord),
      columnIdentity: Boolean(columnRecord),
      pairProperties: pairProperties?.length || 0,
    },
  };
}

function payloadRendererFor(payload, kind) {
  if (!payload || typeof payload !== "object") return "unavailable";
  const components = payloadComponents(payload);
  const normalizedKind = normalizePayloadKind(kind);
  const href = payload.href || payload.url || payload.src;
  if (isComposedPayloadKind(kind) || components.length > 0) return "composed";
  if (isTextPayloadKind(kind) || typeof payload.text === "string") return "text";
  if (isTimeSeriesPayloadKind(kind)
    || Array.isArray(payload.series)
    || Array.isArray(payload.samples)
    || Array.isArray(payload.curve)) return "time-series";
  if (isHistogramPayloadKind(kind)
    || Array.isArray(payload.bins)
    || Array.isArray(payload.counts)
    || Array.isArray(payload.distribution)) return "histogram";
  if (href && (isImagePayloadKind(kind) || normalizedKind === "external-ref")) return "image-reference";
  if (isImagePayloadKind(kind)
    || payload.pixels
    || payload.pixel_values
    || payload.matrix
    || payload.image
    || (hasImageDimensions(payload) && (payload.values || payload.data))) return "compact-image";
  if (isVectorPayloadKind(kind) || Array.isArray(payload.values)) return "vector";
  if (hasGenericPayloadFields(payload)) return "properties";
  if (href) return "image-reference";
  if (Object.keys(payload).some((key) => key !== "kind")) return "properties";
  return "unavailable";
}

function normalizePayloadKind(kind) {
  return String(kind || "").trim().toLowerCase().replace(/[_\s]+/g, "-");
}

function isTextPayloadKind(kind) {
  return ["string", "text", "code", "log", "log-line"].includes(normalizePayloadKind(kind));
}

function isTimeSeriesPayloadKind(kind) {
  return ["time-series", "timeseries", "series", "curve", "process-curve", "process-series"].includes(normalizePayloadKind(kind));
}

function isHistogramPayloadKind(kind) {
  return ["histogram", "distribution", "spectrum", "bins", "probability-distribution"].includes(normalizePayloadKind(kind));
}

function isImagePayloadKind(kind) {
  return [
    "image",
    "image-ref",
    "image-reference",
    "image-like",
    "pixels",
    "pixel-array",
    "matrix",
    "spectrogram",
  ].includes(normalizePayloadKind(kind));
}

function isComposedPayloadKind(kind) {
  return ["composed", "mixed", "structured", "record", "parts"].includes(normalizePayloadKind(kind));
}

function isVectorPayloadKind(kind) {
  return ["vector", "embedding", "values", "feature-vector", "numeric-vector"].includes(normalizePayloadKind(kind));
}

function hasGenericPayloadFields(payload) {
  if (!payload || typeof payload !== "object") return false;
  return payload.value !== undefined
    || payload.scalar !== undefined
    || payload.category !== undefined
    || payload.label !== undefined
    || payload.features && typeof payload.features === "object";
}

function hasImageDimensions(payload) {
  return payload
    && (payload.width !== undefined
      || payload.height !== undefined
      || payload.rows !== undefined
      || payload.columns !== undefined
      || payload.cols !== undefined
      || Array.isArray(payload.shape)
      || Array.isArray(payload.dimensions));
}

function boundedTextValue(value, maxLength, diagnostics) {
  const text = String(value ?? "");
  if (text.length <= maxLength) return text;
  diagnostics.truncated = true;
  diagnostics.notes.push("text value truncated");
  return `${text.slice(0, Math.max(0, maxLength - 3))}...`;
}

function seriesValuesForPayload(payload, renderer) {
  if (renderer !== "time-series") return null;
  if (Array.isArray(payload.series)) return payload.series;
  if (Array.isArray(payload.samples)) return payload.samples;
  if (Array.isArray(payload.curve)) return payload.curve;
  if (Array.isArray(payload.values) && isTimeSeriesPayloadKind(payload.kind)) return payload.values;
  return null;
}

function histogramValuesForPayload(payload, renderer) {
  if (renderer !== "histogram") return null;
  if (Array.isArray(payload.bins)) return payload.bins;
  if (Array.isArray(payload.counts)) return payload.counts;
  if (Array.isArray(payload.distribution)) return payload.distribution;
  if (Array.isArray(payload.values) && isHistogramPayloadKind(payload.kind)) return payload.values;
  return null;
}

function summarizeNumericSeries(values, maxValues, diagnostics) {
  const numericValues = values.map(Number).filter(Number.isFinite);
  if (numericValues.length <= maxValues) return numericValues;
  diagnostics.summarized = true;
  diagnostics.notes.push("numeric series sampled for preview");
  const sampled = [];
  const step = (numericValues.length - 1) / Math.max(1, maxValues - 1);
  for (let index = 0; index < maxValues; index += 1) {
    sampled.push(numericValues[Math.round(index * step)]);
  }
  return sampled;
}

function genericPayloadFields(payload, maxFields, diagnostics) {
  const fields = [];
  const skip = new Set([
    "kind",
    "type",
    "features",
    "fields",
    "components",
    "parts",
    "series",
    "values",
    "bins",
    "edges",
    "pixels",
    "matrix",
  ]);
  for (const label of ["value", "scalar", "category", "label"]) {
    if (payload[label] !== undefined) fields.push({ label, value: summarizePreviewValue(payload[label]) });
  }
  if (payload.features && typeof payload.features === "object" && !Array.isArray(payload.features)) {
    for (const [label, value] of Object.entries(payload.features)) {
      fields.push({ label, value: summarizePreviewValue(value) });
    }
  }
  for (const [label, value] of Object.entries(payload)) {
    if (skip.has(label) || fields.some((field) => field.label === label)) continue;
    fields.push({ label, value: summarizePreviewValue(value) });
  }
  if (fields.length > maxFields) {
    diagnostics.truncated = true;
    diagnostics.notes.push("property fields truncated");
  }
  if (fields.some((field) => typeof field.value === "string" && /fields|values/.test(field.value))) {
    diagnostics.summarized = true;
  }
  return fields.slice(0, maxFields);
}

function compactImageFromPayload(payload, context, diagnostics) {
  const source = payload.pixels ?? payload.pixel_values ?? payload.values ?? payload.matrix ?? payload.image ?? payload.data;
  const shape = Array.isArray(payload.shape) ? payload.shape : Array.isArray(payload.dimensions) ? payload.dimensions : null;
  const dimensions = compactImageDimensions(payload, source, shape);
  const maxValues = context.maxCompactImageValues ?? 4096;
  const values = flattenNumericValues(source, maxValues);
  if (!values.length) return null;
  const requiredValues = dimensions.sourceWidth * dimensions.sourceHeight * dimensions.channels;
  if (requiredValues > 0 && values.length < Math.min(requiredValues, maxValues)) {
    diagnostics.truncated = true;
    diagnostics.notes.push("compact image has fewer values than dimensions require");
  }
  if (requiredValues > maxValues || values.truncated) {
    diagnostics.summarized = true;
    diagnostics.truncated = true;
    diagnostics.notes.push("compact image values truncated for preview");
  }
  const valueRange = finiteRange(values);
  return {
    kind: "compact-array",
    values,
    width: dimensions.renderWidth,
    height: dimensions.renderHeight,
    renderWidth: dimensions.renderWidth,
    renderHeight: dimensions.renderHeight,
    sourceWidth: dimensions.sourceWidth,
    sourceHeight: dimensions.sourceHeight,
    channels: dimensions.channels,
    sourceLength: dimensions.sourceLength || values.length,
    valueRange,
    alt: payload.alt || payload.label || "compact image array",
    label: payload.label || `${dimensions.sourceWidth} x ${dimensions.sourceHeight} array`,
  };
}

function compactImageDimensions(payload, source, shape) {
  let channels = positiveInteger(payload.channels ?? payload.channel_count ?? payload.channelCount ?? shape?.[2]) || 1;
  let sourceWidth = positiveInteger(payload.width ?? payload.columns ?? payload.cols ?? shape?.[1]) || null;
  let sourceHeight = positiveInteger(payload.height ?? payload.rows ?? shape?.[0]) || null;
  if (Array.isArray(source) && Array.isArray(source[0])) {
    sourceHeight ||= source.length;
    sourceWidth ||= Math.max(1, ...source.map((row) => Array.isArray(row) ? row.length : 1));
  }
  const sourceLength = countNestedValues(source);
  if (sourceWidth && !sourceHeight) sourceHeight = Math.max(1, Math.floor(sourceLength / Math.max(1, sourceWidth * channels)));
  if (sourceHeight && !sourceWidth) sourceWidth = Math.max(1, Math.floor(sourceLength / Math.max(1, sourceHeight * channels)));
  if (!sourceWidth || !sourceHeight) {
    const side = Math.max(1, Math.round(Math.sqrt(sourceLength / channels)));
    sourceWidth = side;
    sourceHeight = Math.max(1, Math.ceil((sourceLength / channels) / side));
  }
  const cellCount = Math.max(1, sourceWidth * sourceHeight);
  if (channels === 1 && sourceLength === cellCount * 3) channels = 3;
  if (channels === 1 && sourceLength === cellCount * 4) channels = 4;
  const maxRenderSide = 96;
  const scale = Math.min(1, maxRenderSide / Math.max(sourceWidth, sourceHeight));
  return {
    sourceWidth,
    sourceHeight,
    renderWidth: Math.max(1, Math.round(sourceWidth * scale)),
    renderHeight: Math.max(1, Math.round(sourceHeight * scale)),
    channels,
    sourceLength,
  };
}

function flattenNumericValues(source, limit, out = []) {
  if (out.length >= limit) {
    out.truncated = true;
    return out;
  }
  if (Array.isArray(source) || ArrayBuffer.isView(source)) {
    for (const value of source) {
      flattenNumericValues(value, limit, out);
      if (out.length >= limit) {
        out.truncated = true;
        break;
      }
    }
    return out;
  }
  const number = Number(source);
  if (Number.isFinite(number)) out.push(number);
  return out;
}

function countNestedValues(source) {
  if (Array.isArray(source) || ArrayBuffer.isView(source)) {
    return Array.from(source).reduce((count, value) => count + countNestedValues(value), 0);
  }
  return Number.isFinite(Number(source)) ? 1 : 0;
}

function positiveInteger(value) {
  const number = Number(value);
  return Number.isFinite(number) && number > 0 ? Math.round(number) : null;
}

function finiteRange(values) {
  const numericValues = values.map(Number).filter(Number.isFinite);
  if (!numericValues.length) return { min: 0, max: 1 };
  const min = Math.min(...numericValues);
  const max = Math.max(...numericValues);
  return { min, max: max === min ? min + 1 : max };
}

function scaleImageChannel(value, range) {
  const number = Number(value);
  if (!Number.isFinite(number)) return 0;
  if (range.min >= 0 && range.max <= 1) return clamp(Math.round(number * 255), 0, 255);
  if (range.min >= 0 && range.max <= 255) return clamp(Math.round(number), 0, 255);
  return clamp(Math.round(((number - range.min) / Math.max(1e-9, range.max - range.min)) * 255), 0, 255);
}

function payloadKind(payload) {
  if (!payload || typeof payload !== "object") return null;
  if (payload.kind) return String(payload.kind);
  if (typeof payload.text === "string") return "string";
  if (Array.isArray(payload.series)) return "time_series";
  if (Array.isArray(payload.bins)) return "histogram";
  if (payload.pixels
    || payload.pixel_values
    || payload.matrix
    || payload.image
    || (hasImageDimensions(payload) && (payload.values || payload.data))) return "image";
  if (Array.isArray(payload.values)) return "vector";
  if (payload.fields || payload.components || payload.parts) return "composed";
  if (payload.href || payload.url || payload.src) return "external_ref";
  if (hasGenericPayloadFields(payload)) return "properties";
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
  if (targets.some((candidate) => {
    return `${candidate.relation_id ?? ""}\u0000${candidate.row_id ?? ""}\u0000${candidate.column_id ?? ""}` === key;
  })) return;
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
