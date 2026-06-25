# Release Readiness Notes

Date: 2026-06-25
Status: Production ready for the current verified snapshot
Primary tracker: [Production Readiness Roadmap](engine/production-readiness-roadmap.md)

This file records the current release-positioning notes and known limitations
for the finite metric-space refactor. It is intentionally short and editable:
agents should update the status, evidence, and blocker notes as the release
state changes.

## Release Positioning

METRIC is a framework for computing with finite metric spaces. The public model
is:

- records and metrics define a finite metric space `(X, d)`;
- storage and indexes represent that same space without changing the object;
- measures, entropy, local volume, correlation, and structural diagnostics are
  finite-space quantities;
- transformations, Redif dynamics, coordinate maps, and compressed spaces create
  derived finite metric spaces with explicit lineage.

Concrete application workflows are downstream uses of these objects. They should
not determine the public vocabulary.

## Current Evidence

- Full local build passed with `cmake --build build/core -j2`.
- Full local CTest passed with 193 tests discovered, 190 executed tests passed,
  3 MNIST IDX-data skips, and 0 failed tests.
- A clean workspace snapshot in `/tmp` configured from scratch, built from an
  empty build directory, and passed CTest with 190 executed tests, 3 MNIST
  IDX-data skips, and 0 failed tests.
- A clean clone from temporary committed snapshot `2d5bcdb` configured from
  scratch, built from an empty build directory, and passed CTest with 190
  executed tests, 3 MNIST IDX-data skips, and 0 failed tests.
- Real MNIST IDX-backed smoke, gallery, and GRAE10 integrity runs passed with
  temporary IDX files extracted to `/tmp/metric-mnist-idx`.
- The Python wheel smoke script builds an isolated wheel, installs it, verifies
  the promoted package surface, runs core tests, and runs promoted examples.
- The Python 3.12 clean wheel smoke passed from a fresh temporary venv.
- Public docs and promoted Engine examples now use coordinate-solver and
  coordinate-calibration terms instead of concrete solver internals.
- Redif is documented and tested as metric-induced dynamics and inverse
  dynamics.

## Known Limitations

Status: documented

- The required MNIST IDX calibration files are intentionally not part of the new
  Engine asset surface; release hosts must provide or mount them when re-running
  the IDX-backed examples.
- Repeat clean-clone and wheel evidence after any release-candidate edit.

## Agent Progress Log

Agents should append dated entries here when changing release readiness status.

- 2026-06-25: Added release-readiness notes for the finite metric-space refactor.
  At creation, real MNIST IDX-backed runs and clean-host release evidence were
  still open.
- 2026-06-25: Captured clean workspace snapshot evidence in `/tmp`: configure,
  build, and CTest passed with the same MNIST IDX-data skips as the local build.
- 2026-06-25: Captured real MNIST IDX-backed evidence from temporary IDX files:
  diffusion-coordinate smoke, diffusion-coordinate gallery, and GRAE10 integrity
  all passed without skips.
- 2026-06-25: Captured clean Python 3.12 wheel evidence from a fresh temporary
  venv; wheel build, install, package-surface probe, core tests, and promoted
  Python examples passed.
- 2026-06-25: Captured clean clone evidence from temporary committed snapshot
  `2d5bcdb`; configure, build, and CTest passed from an empty build directory.
