# Agent Task: Exporter Quality Audit

## Owner

This is a read-mostly task. The worker should not edit exporter files unless
the parent explicitly asks for a patch.

## Goal

After native exporter sources land, audit them for correctness, consistency and
schema validity.

## Audit Checklist

For each exporter:

- Does it compute evidence in C++?
- Does it avoid Python/JavaScript algorithm work?
- Does it avoid dependencies?
- Does it emit `metric.visual.v1` by default?
- Does `--export-dir <dir>` write `metric.visual.json`?
- Does provenance identify native C++ export and avoid
  `provenance.synthetic: true`?
- Are records, relations, spaces, coordinates, properties and diagnostics
  internally consistent?
- Does `node visual/tools/check-visual-document.mjs` pass?
- Does the source have a small enough scope to maintain?

## Output

Produce a Markdown report with:

- one section per exporter
- pass/fail table
- exact compile/run/validate commands used
- blocking issues
- recommended parent integration steps

