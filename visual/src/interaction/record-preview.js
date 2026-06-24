const STYLE_ID = "mtrc-record-preview-panel-style";

export class RecordPreviewPanel {
  constructor(options = {}) {
    const {
      root = document.body,
      className = "mtrc-record-preview",
      offset = [18, 18],
      maxFields = 8,
      resolver = null,
      render = null,
    } = options;
    this.root = root;
    this.className = className;
    this.offset = offset;
    this.maxFields = maxFields;
    this.resolver = resolver;
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

function clamp(value, min, max) {
  return Math.min(max, Math.max(min, value));
}
