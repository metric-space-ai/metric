/*
 * Shared provenance helpers for metric.visual.v1 documents.
 *
 * Native exported evidence is intentionally recognized only by explicit
 * boolean provenance flags. The snake_case spelling is emitted by the current
 * native writers; the camelCase spelling is accepted for equivalent serializers.
 * Writer paths, runtime labels, and text that mentions C++ are ignored.
 */

export const EXPLICIT_NATIVE_METRIC_VISUAL_EXPORT_KEYS = Object.freeze([
  "native_export",
  "nativeExport",
]);

export function explicitNativeMetricVisualExportSignal(provenance = {}) {
  if (!provenance || typeof provenance !== "object") return null;
  for (const key of EXPLICIT_NATIVE_METRIC_VISUAL_EXPORT_KEYS) {
    if (provenance[key] === true) return key;
  }
  return null;
}

export function isExplicitNativeMetricVisualExport(provenance = {}) {
  return explicitNativeMetricVisualExportSignal(provenance) !== null;
}

export function isSyntheticMetricVisualEvidence(provenance = {}) {
  return provenance?.synthetic === true
    || provenance?.synthetic_js === true
    || provenance?.synthetic_fixture === true;
}

export function isNativeMetricVisualDocument(document) {
  if (document?.schema !== "metric.visual.v1") return false;
  const provenance = document?.provenance || {};
  return !isSyntheticMetricVisualEvidence(provenance)
    && isExplicitNativeMetricVisualExport(provenance);
}
