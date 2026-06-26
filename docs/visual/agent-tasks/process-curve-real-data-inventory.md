# Process-Curve Real Data Inventory

Status: superseded by
`docs/visual/reports/process-curve-external-scale-exporter.md`. The follow-up
scale exporter now derives 576 real UCR source windows and clears the previous
source-record blocker. This task remains only as the historical read-only
inventory contract.

## Purpose

Determine whether this machine already has enough real licensed process-curve
source windows to scale `process-curve-external-hero` from the checked-in 48
source windows to the 500 real-source-window target.

This is an inventory task. Do not synthesize windows, duplicate query records or
pad the current dataset.

## Owner Scope

- read-only inspection of local repository and likely local dataset locations
- report under `docs/visual/reports/process-curve-real-data-inventory.md`

Do not edit native exporters, visual runtime, project page or checked-in assets.

## Required Checks

Inspect at least:

```text
examples/engine/assets/
/Users/michaelwelsch/Downloads/
/Users/michaelwelsch/Documents/
/Users/michaelwelsch/Library/CloudStorage/
```

Search for UCR Time Series Anomaly Detection 2021 archives, extracted CSVs, or
derived gallery CSVs that can legally back a public native exporter.

## Report

The report must state:

- exact files found, sizes and record/window counts where inspectable;
- whether the files look like full UCR source data, derived slices, or unrelated
  examples;
- whether the then-current source-window blocker could be cleared locally;
- the exact next implementation step if enough data exists;
- the exact external data requirement if enough data does not exist.

## Acceptance

Run whatever read-only commands are needed and include the command list in the
report. Do not mark the hero as accepted or scale-ready.
