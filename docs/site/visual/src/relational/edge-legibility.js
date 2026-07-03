export const RELATION_EDGE_LEGIBILITY_SCHEMA = "metric.visual.relation_edge_legibility.v1";

export function createRelationEdgeLegibilityProfile(options = {}) {
  const edgeCount = positiveInteger(options.edgeCount ?? options.visibleEdgeCount ?? options.count, 0);
  const sourceEdgeCount = positiveInteger(options.sourceEdgeCount ?? options.candidateEdgeCount ?? edgeCount, edgeCount);
  const role = String(options.role || options.profile || "relation-edges");
  const densityClass = classifyEdgeDensity(edgeCount);
  const laneModulo = positiveInteger(options.laneModulo, defaultLaneModulo(role, edgeCount));
  const laneOffsetScale = numberOption(options.laneOffsetScale, defaultLaneOffsetScale(role, edgeCount));
  const alphaScale = clamp(numberOption(options.alphaScale, defaultAlphaScale(role, edgeCount)), 0.16, 1);
  const minimumAlpha = clamp(numberOption(options.minimumAlpha, defaultMinimumAlpha(role)), 0, 1);

  return {
    schema: RELATION_EDGE_LEGIBILITY_SCHEMA,
    role,
    strategy: "density-aware-alpha-with-deterministic-world-lanes",
    source: "render-legibility-only",
    edgeCount,
    sourceEdgeCount,
    densityClass,
    preservesEdgeCount: true,
    laneStrategy: laneOffsetScale > 0 ? "deterministic-edge-id-world-offset" : "none",
    laneModulo,
    laneOffsetScale,
    alphaScale,
    minimumAlpha,
    alphaSource: "per-edge-color-alpha-scaled-before-global-material-alpha",
    rankSource: options.rankSource || "exported-edge-emphasis-or-native-relation-value",
    algorithmicComputation: false,
  };
}

export function applyRelationEdgeLegibilityDescriptor(descriptor, options = {}) {
  if (!descriptor) return descriptor;
  const profile = createRelationEdgeLegibilityProfile({
    edgeCount: descriptor.metadata?.graph?.edgeCount
      ?? descriptor.metadata?.edgeCount
      ?? descriptor.channels?.sourcePosition?.count
      ?? descriptor.channels?.position?.count
      ?? 0,
    sourceEdgeCount: descriptor.metadata?.nativeEvidence?.sourceEdgeCount
      ?? descriptor.metadata?.graph?.candidateCount
      ?? descriptor.metadata?.graph?.edgeCount
      ?? descriptor.metadata?.edgeCount
      ?? 0,
    role: descriptor.metadata?.role || descriptor.source?.role || options.role,
    ...options,
  });

  return {
    ...descriptor,
    geometry: {
      ...(descriptor.geometry || {}),
      laneOffsetScale: profile.laneOffsetScale,
      laneModulo: profile.laneModulo,
      edgeLegibility: profile.strategy,
    },
    material: {
      ...(descriptor.material || {}),
      edgeDensityAlphaScale: profile.alphaScale,
      edgeMinimumAlpha: profile.minimumAlpha,
    },
    metadata: {
      ...(descriptor.metadata || {}),
      edgeLegibility: profile,
    },
  };
}

function classifyEdgeDensity(edgeCount) {
  if (edgeCount >= 5000) return "very-dense";
  if (edgeCount >= 1000) return "dense";
  if (edgeCount >= 250) return "medium";
  if (edgeCount >= 80) return "light";
  return "sparse";
}

function defaultAlphaScale(role, edgeCount) {
  if (edgeCount <= 0) return 1;
  if (/residual|error/i.test(role)) return Math.min(1, Math.sqrt(180 / Math.max(180, edgeCount)));
  if (/bridge|paired|cross-space|dependence/i.test(role)) return Math.min(1, Math.sqrt(160 / Math.max(160, edgeCount)));
  if (/cross-type|mixed/i.test(role)) return Math.min(1, Math.sqrt(900 / Math.max(900, edgeCount)));
  return Math.min(1, Math.sqrt(700 / Math.max(700, edgeCount)));
}

function defaultMinimumAlpha(role) {
  if (/residual|error/i.test(role)) return 0.12;
  if (/bridge|paired|dependence/i.test(role)) return 0.1;
  return 0.06;
}

function defaultLaneModulo(role, edgeCount) {
  if (/residual|error/i.test(role)) return 1;
  if (/bridge|paired|dependence/i.test(role)) return edgeCount >= 80 ? 11 : 7;
  if (/cross-type|mixed/i.test(role)) return edgeCount >= 1000 ? 13 : 7;
  return edgeCount >= 250 ? 9 : 5;
}

function defaultLaneOffsetScale(role, edgeCount) {
  if (/residual|error/i.test(role)) return 0;
  if (edgeCount <= 1) return 0;
  if (/bridge|paired|dependence/i.test(role)) return 0.014;
  if (/cross-type|mixed/i.test(role)) return 0.009;
  return 0.007;
}

function positiveInteger(value, fallback) {
  const number = Number(value);
  return Number.isFinite(number) && number > 0 ? Math.floor(number) : fallback;
}

function numberOption(...values) {
  for (const value of values) {
    const number = Number(value);
    if (Number.isFinite(number)) return number;
  }
  return 0;
}

function clamp(value, min, max) {
  return Math.max(min, Math.min(max, Number(value) || 0));
}
