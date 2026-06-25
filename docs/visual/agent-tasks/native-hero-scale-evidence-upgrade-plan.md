# Agent Task: Native Hero Scale Evidence Upgrade Plan

## Owner Scope

This is a planning and audit task. It owns only:

- `docs/visual/reports/native-hero-scale-evidence-upgrade-plan.md`
- optional narrow updates to `docs/visual/native-exporter-work-queue.md`

Do not edit C++ exporters, JavaScript runtime, visual examples, project pages,
GRAE10, generated JSON assets, CMake or tests. Other workers may implement code
in parallel. Do not revert unrelated worktree changes.

## Objective

Turn the current native-scale blockers into concrete implementation tickets.
The output must say exactly which native exporter has to grow, which real
dataset or deterministic native fixture it must use, which metric computations
must be performed in C++, which `metric.visual.v1` fields must be exported, and
which visual grammar will prove the result.

This task must not invent fake heroes. It is acceptable to conclude that a hero
is blocked until a real dataset is available, but the report must identify the
minimum real evidence needed.

## Required Inputs

Read:

- `docs/visual/visual-engine-masterplan.md`
- `docs/visual/metric-visual-engine-implementation-plan.md`
- `docs/visual/native-exporter-work-queue.md`
- `docs/visual/reports/native-exporter-quality-audit.md`
- `visual/hero-visual-briefs.manifest.json`
- `visual/tools/check-native-hero-evidence-scale.mjs`

Run:

```bash
node visual/tools/check-native-hero-evidence-scale.mjs
node visual/tools/check-public-gallery-evidence.mjs
```

## Required Report

Create a table for each public preview:

- `condition-monitoring-hero`
- `mixed-record-hero`
- `cross-space-dependency-hero`
- `relation-matrix-neighborhood`
- `dynamics-noise-hero`
- `mapping-dimensionality-hero`
- `process-curve-external-hero`

For each row include:

- current record count, relation count, record type count
- native/synthetic status
- exact blocker text from the gate
- minimum evidence target from the visual brief
- concrete C++ exporter file to change
- concrete real dataset or deterministic native fixture source
- metric computations that must remain native C++
- required exported properties, relations, coordinates, timelines and previews
- expected visual grammar
- acceptance commands
- risk if the exporter is scaled naively

## Forbidden

- Do not lower evidence thresholds.
- Do not mark any preview as hero accepted.
- Do not propose JavaScript or Python algorithm computation.
- Do not propose synthetic renderer fixtures as public evidence.
- Do not touch GRAE10.
- Do not use vague tasks like "make it better"; every next step must name a
  file, dataset/evidence source, metric computation and validation command.

## Acceptance

The report is acceptable only if a worker could implement each row without
asking what data, file or metric computation is meant.

