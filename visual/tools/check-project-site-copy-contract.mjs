#!/usr/bin/env node
/*
 * Project site copy contract gate.
 *
 * The public page should read as a visual portfolio. Internal review labels may
 * remain in data-* attributes for tools, but they must not appear as visible
 * copy.
 */

import { access, readFile } from "node:fs/promises";
import { dirname, resolve } from "node:path";
import { fileURLToPath } from "node:url";

const HERE = dirname(fileURLToPath(import.meta.url));
const ROOT = resolve(HERE, "..", "..");
const SITE = resolve(ROOT, "docs", "site", "index.html");

const ACCEPTED_REFERENCE = "grae10-metric-engine";
const EXPECTED_ITEMS = [
  {
    id: "grae10-metric-engine",
    visual: "../../visual/examples/grae10-metric-engine/index.html",
    source: "../../examples/engine/mnist_grae10_integrity.cpp",
    data: "../../visual/examples/grae10-metric-engine/grae10-data.json",
  },
  {
    id: "condition-monitoring-hero",
    visual: "../../visual/examples/condition-monitoring-hero/index.html",
    source: "../../examples/engine/condition_monitoring_visual_export.cpp",
    data: "../../docs/examples/assets/condition-monitoring/metric.visual.json",
  },
  {
    id: "mixed-record-hero",
    visual: "../../visual/examples/mixed-record-hero/index.html",
    source: "../../examples/engine/mixed_finite_records_visual_export.cpp",
    data: "../../docs/examples/assets/mixed-records/metric.visual.json",
  },
  {
    id: "cross-space-dependency-hero",
    visual: "../../visual/examples/cross-space-dependency-hero/index.html",
    source: "../../examples/engine/cross_space_dependency_visual_export.cpp",
    data: "../../docs/examples/assets/cross-space-dependency/metric.visual.json",
  },
  {
    id: "mapping-dimensionality-hero",
    visual: "../../visual/examples/mapping-dimensionality-hero/index.html",
    source: "../../examples/engine/mapping_dimensionality_visual_export.cpp",
    data: "../../docs/examples/assets/mapping-dimensionality/metric.visual.json",
  },
  {
    id: "dynamics-noise-hero",
    visual: "../../visual/examples/dynamics-noise-hero/index.html",
    source: "../../examples/engine/finite_metric_dynamics_visual_export.cpp",
    data: "../../docs/examples/assets/dynamics-noise/metric.visual.json",
  },
  {
    id: "relation-matrix-neighborhood",
    visual: "../../visual/examples/relation-matrix-neighborhood/index.html",
    source: "../../examples/engine/relation_matrix_visual_export.cpp",
    data: "../../docs/examples/assets/relation-matrix/metric.visual.json",
  },
  {
    id: "process-curve-external-hero",
    visual: "../../visual/examples/process-curve-external-hero/index.html",
    source: "../../examples/engine/process_curve_external_visual_export.cpp",
    data: "../../docs/examples/assets/process-curve-external/metric.visual.json",
  },
];

const VISIBLE_FORBIDDEN = [
  ["review-pending", /\breview-pending\b/i],
  ["Visual:", /\bVisual:/],
  ["Evidence:", /\bEvidence:/],
  ["Notes:", /\bNotes:/],
  ["scale-blocked", /\bscale[- ]blocked\b/i],
  ["scale-limited", /\bscale[- ]limited\b/i],
  ["prompt", /\bprompt\b/i],
  ["TODO", /\bTODO\b/],
  ["placeholder", /\bplaceholder\b/i],
  ["fake", /\bfake\b/i],
  ["intent", /\bintent\b/i],
  ["should", /\bshould\b/i],
  ["will", /\bwill\b/i],
  ["relative path", /\.\.\//],
  ["local absolute path", /\/Users\//],
  ["repo docs path", /\bdocs\//],
  ["repo visual path", /\bvisual\//],
  ["repo examples path", /\bexamples\//],
  ["agent task path", /\bagent-tasks\b/i],
  ["raw C++ filename", /\.cpp\b/],
  ["raw JSON filename", /\.json\b/],
  ["metric visual JSON filename", /\bmetric\.visual\.json\b/],
];

async function main() {
  const html = await readFile(SITE, "utf8");
  const visibleText = extractVisibleText(html);
  const hrefs = extractAttributeValues(html, "href");
  const srcs = extractAttributeValues(html, "src");
  const issues = [];

  for (const [label, pattern] of VISIBLE_FORBIDDEN) {
    if (pattern.test(visibleText)) {
      issues.push({ code: "forbidden-visible-copy", label });
    }
  }

  for (const occurrence of rawInternalLabelOccurrences(html, "review-pending")) {
    issues.push({
      code: "review-pending-outside-data-status",
      offset: occurrence.offset,
      context: occurrence.context,
    });
  }

  const firstFrame = firstIframeSource(html);
  if (firstFrame !== EXPECTED_ITEMS[0].visual) {
    issues.push({
      code: "first-iframe-not-grae10-reference",
      expected: EXPECTED_ITEMS[0].visual,
      actual: firstFrame,
    });
  }

  if (!/<\/section>\s*<section\s+id=["']gallery["']\s+class=["']section["']>/m.test(html)) {
    issues.push({
      code: "gallery-does-not-directly-follow-hero",
      message: "The public page should move from the GRAE10 hero directly into the portfolio gallery.",
    });
  }

  const publicExamples = extractPublicExampleNames(html);
  const expectedIds = EXPECTED_ITEMS.map((item) => item.id).sort();
  if (publicExamples.join("\n") !== expectedIds.join("\n")) {
    issues.push({
      code: "unexpected-public-gallery-examples",
      expected: expectedIds,
      actual: publicExamples,
    });
  }

  const acceptedStatuses = extractAcceptedStatusItems(html);
  if (acceptedStatuses.length !== 1 || acceptedStatuses[0] !== ACCEPTED_REFERENCE) {
    issues.push({
      code: "grae10-not-only-accepted-reference",
      expected: [ACCEPTED_REFERENCE],
      actual: acceptedStatuses,
    });
  }

  for (const item of EXPECTED_ITEMS) {
    assertIncludes(issues, html, `data-gallery-item="${item.id}"`, "missing-gallery-item", item.id);
    assertIncludes(issues, hrefs, item.visual, "missing-live-visual-link", item.id);
    assertIncludes(issues, hrefs, item.source, "missing-native-cpp-link", item.id);
    assertIncludes(issues, hrefs, item.data, "missing-evidence-json-link", item.id);
    await assertExistingRelativeHref(issues, item.visual, item.id);
    await assertExistingRelativeHref(issues, item.source, item.id);
    await assertExistingRelativeHref(issues, item.data, item.id);

    if (item.id !== ACCEPTED_REFERENCE) {
      const status = evidenceStatusForItem(html, item.id);
      if (!status) {
        issues.push({ code: "missing-non-grae-status", item: item.id });
      } else if (/\baccepted\b/i.test(status)) {
        issues.push({ code: "non-grae-item-marked-accepted", item: item.id, status });
      } else if (status !== "review-pending") {
        issues.push({
          code: "non-grae-item-not-review-pending",
          item: item.id,
          expected: "review-pending",
          actual: status,
        });
      }
    }
  }

  if (!srcs.includes(EXPECTED_ITEMS[0].visual)) {
    issues.push({
      code: "grae10-live-visual-not-embedded",
      expected: EXPECTED_ITEMS[0].visual,
    });
  }

  const pythonSnippet = /<pre\b[\s\S]*?(?:\bpython\b|\bfrom\s+metric\b|\bimport\s+metric\b|python\/pkg\/metric|\.py\b)[\s\S]*?<\/pre>/i;
  if (pythonSnippet.test(html)) {
    issues.push({
      code: "python-snippet-on-public-page",
      message: "Do not add Python snippets unless real binding paths are verified.",
    });
  }

  const report = {
    ok: issues.length === 0,
    site: "docs/site/index.html",
    acceptedReference: ACCEPTED_REFERENCE,
    publicExamples,
    checkedLinks: EXPECTED_ITEMS.flatMap((item) => [
      { item: item.id, kind: "visual", href: item.visual },
      { item: item.id, kind: "source", href: item.source },
      { item: item.id, kind: "data", href: item.data },
    ]),
    issues,
  };

  console.log(JSON.stringify(report, null, 2));
  if (!report.ok) process.exitCode = 1;
}

function assertIncludes(issues, haystack, needle, code, item) {
  if (Array.isArray(haystack) ? haystack.includes(needle) : haystack.includes(needle)) return;
  issues.push({ code, item, expected: needle });
}

async function assertExistingRelativeHref(issues, href, item) {
  try {
    await access(resolve(dirname(SITE), href));
  } catch {
    issues.push({ code: "linked-file-missing", item, href });
  }
}

function evidenceStatusForItem(html, item) {
  const pattern = new RegExp(`<[^>]+data-gallery-item=["']${escapeRegExp(item)}["'][^>]*data-evidence-status=["']([^"']+)["']`, "m");
  return pattern.exec(html)?.[1] || null;
}

function extractAcceptedStatusItems(html) {
  const pattern = /<[^>]+data-gallery-item=["']([^"']+)["'][^>]*data-evidence-status=["']([^"']*\baccepted\b[^"']*)["'][^>]*>/g;
  return Array.from(html.matchAll(pattern), (match) => match[1]).sort();
}

function extractAttributeValues(html, attribute) {
  const pattern = new RegExp(`\\b${attribute}=["']([^"']+)["']`, "g");
  return Array.from(new Set(Array.from(html.matchAll(pattern), (match) => match[1]))).sort();
}

function extractPublicExampleNames(html) {
  return Array.from(
    new Set(Array.from(html.matchAll(/visual\/examples\/([^/"']+)\/index\.html/g), (match) => match[1])),
  ).sort();
}

function extractVisibleText(html) {
  return decodeEntities(html)
    .replace(/<!--[\s\S]*?-->/g, " ")
    .replace(/<script\b[\s\S]*?<\/script>/gi, " ")
    .replace(/<style\b[\s\S]*?<\/style>/gi, " ")
    .replace(/<[^>]+>/g, " ")
    .replace(/\s+/g, " ")
    .trim();
}

function rawInternalLabelOccurrences(html, label) {
  const occurrences = [];
  const pattern = new RegExp(escapeRegExp(label), "gi");
  for (const match of html.matchAll(pattern)) {
    const offset = match.index ?? 0;
    const tagStart = html.lastIndexOf("<", offset);
    const tagEnd = html.indexOf(">", offset);
    const withinTag = tagStart >= 0 && tagEnd >= offset;
    const tag = withinTag ? html.slice(tagStart, tagEnd + 1) : "";
    if (/data-evidence-status=["'][^"']*review-pending[^"']*["']/.test(tag)) continue;
    occurrences.push({
      offset,
      context: html.slice(Math.max(0, offset - 50), Math.min(html.length, offset + 50)),
    });
  }
  return occurrences;
}

function firstIframeSource(html) {
  return /<iframe\b[\s\S]*?\bsrc=["']([^"']+)["']/i.exec(html)?.[1] || null;
}

function decodeEntities(text) {
  return text
    .replace(/&nbsp;/g, " ")
    .replace(/&amp;/g, "&")
    .replace(/&lt;/g, "<")
    .replace(/&gt;/g, ">")
    .replace(/&quot;/g, '"')
    .replace(/&#39;/g, "'");
}

function escapeRegExp(value) {
  return value.replace(/[.*+?^${}()|[\]\\]/g, "\\$&");
}

main().catch((error) => {
  console.error(error);
  process.exitCode = 1;
});
