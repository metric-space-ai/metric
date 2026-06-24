import {
  collectRecordIds,
  indexRecordIds,
  pairSourceId,
  pairTargetId,
  pairValue,
  relationPairs,
} from "./relation-source.js";

/**
 * Check reciprocal relation values for symmetric visualization diagnostics.
 *
 * The check reports missing reciprocals and mismatched reciprocal values. It
 * does not repair or infer missing values because the renderer must preserve
 * exported relation evidence.
 *
 * @param {object|Array} source
 * @param {object} [options]
 * @param {number} [options.tolerance=1e-9]
 * @returns {object}
 */
export function checkSymmetricRelations(source, options = {}) {
  const tolerance = Number.isFinite(options.tolerance) ? Math.max(0, options.tolerance) : 1e-9;
  const recordIds = collectRecordIds(source, options);
  const indexById = indexRecordIds(recordIds);
  const values = relationMap(source, { ...options, recordIds });
  const missingReciprocals = [];
  const mismatches = [];
  let reciprocalCount = 0;
  let comparedCount = 0;
  let maxDelta = 0;

  for (const [key, entry] of values.entries()) {
    if (entry.source === entry.target) continue;
    const reverse = values.get(pairKey(entry.target, entry.source));
    if (!reverse) {
      missingReciprocals.push({ source: entry.source, target: entry.target, value: entry.value });
      continue;
    }

    reciprocalCount += 1;
    if (indexById.get(entry.source) > indexById.get(entry.target)) continue;
    comparedCount += 1;
    const delta = Math.abs(entry.value - reverse.value);
    if (delta > maxDelta) maxDelta = delta;
    if (delta > tolerance) {
      mismatches.push({
        source: entry.source,
        target: entry.target,
        value: entry.value,
        reverseValue: reverse.value,
        delta,
      });
    }
  }

  return {
    kind: "symmetric-relation-check",
    recordCount: recordIds.length,
    pairCount: values.size,
    reciprocalCount,
    comparedCount,
    tolerance,
    maxDelta,
    missingReciprocalCount: missingReciprocals.length,
    mismatchCount: mismatches.length,
    isSymmetric: missingReciprocals.length === 0 && mismatches.length === 0,
    missingReciprocals,
    mismatches,
  };
}

/**
 * Diagnose directed/asymmetric relation evidence for visualization.
 *
 * Useful for deciding whether matrix rendering should show the full directed
 * matrix, whether mirroring would hide meaningful evidence, and how many
 * reciprocal pairs are numerically identical.
 *
 * @param {object|Array} source
 * @param {object} [options]
 * @returns {object}
 */
export function checkAsymmetricRelations(source, options = {}) {
  const tolerance = Number.isFinite(options.tolerance) ? Math.max(0, options.tolerance) : 1e-9;
  const values = relationMap(source, options);
  const asymmetricPairs = [];
  let reciprocalCount = 0;
  let identicalReciprocalCount = 0;
  let directedOnlyCount = 0;
  let maxDelta = 0;

  for (const entry of values.values()) {
    if (entry.source === entry.target) continue;
    const reverse = values.get(pairKey(entry.target, entry.source));
    if (!reverse) {
      directedOnlyCount += 1;
      continue;
    }
    reciprocalCount += 1;
    const delta = Math.abs(entry.value - reverse.value);
    if (delta <= tolerance) identicalReciprocalCount += 1;
    if (delta > maxDelta) maxDelta = delta;
    if (delta > tolerance) {
      asymmetricPairs.push({
        source: entry.source,
        target: entry.target,
        value: entry.value,
        reverseValue: reverse.value,
        delta,
      });
    }
  }

  return {
    kind: "asymmetric-relation-check",
    pairCount: values.size,
    reciprocalCount,
    directedOnlyCount,
    identicalReciprocalCount,
    asymmetricPairCount: asymmetricPairs.length,
    tolerance,
    maxDelta,
    hasDirectedEvidence: directedOnlyCount > 0 || asymmetricPairs.length > 0,
    asymmetricPairs,
  };
}

/**
 * Run both symmetry diagnostics and return a compact summary.
 *
 * @param {object|Array} source
 * @param {object} [options]
 * @returns {object}
 */
export function diagnoseRelationSymmetry(source, options = {}) {
  const symmetric = checkSymmetricRelations(source, options);
  const asymmetric = checkAsymmetricRelations(source, options);
  return {
    kind: "relation-symmetry-diagnostics",
    symmetric,
    asymmetric,
    recommendedMatrixMode: symmetric.isSymmetric ? "symmetric" : "directed",
  };
}

function relationMap(source, options = {}) {
  const recordIds = options.recordIds || collectRecordIds(source, options);
  const out = new Map();
  for (const pair of relationPairs(source)) {
    const sourceId = pairSourceId(pair, recordIds);
    const targetId = pairTargetId(pair, recordIds);
    const value = pairValue(pair, options.valueKey);
    if (!sourceId || !targetId || !Number.isFinite(value)) continue;
    out.set(pairKey(sourceId, targetId), { source: sourceId, target: targetId, value, pair });
  }
  return out;
}

function pairKey(source, target) {
  return `${source}\u0000${target}`;
}

