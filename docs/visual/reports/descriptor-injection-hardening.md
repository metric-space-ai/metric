# Descriptor Injection Hardening

Date: 2026-06-26

## API Boundary

`MetricVisualSurface` no longer exposes public `setLayerDescriptors()` or
`addLayerDescriptors()` methods. Public callers should use semantic `show*`
commands or `setViews()` with semantic view objects that implement
`toLayerDescriptors()`.

`setViews()` rejects raw layer descriptor objects by default instead of
silently installing them. Raw descriptor installation is internal-only through
`MetricVisualSurface._setLayerDescriptors()`, with an explicit internal
diagnostics option available for `setViews()` if an internal diagnostic path
must pass raw descriptors.

`MetricVisualRuntime.setLayerDescriptors()` remains available as the internal
runtime method used by the facade and runtime-owned composition paths.

## Validation Boundary

`visual/tools/check-visual-command-api.mjs` now verifies that:

- `MetricVisualSurface.prototype.setLayerDescriptors` is not public.
- `MetricVisualSurface.prototype.addLayerDescriptors` is not public.
- `_setLayerDescriptors()` remains available as the internal facade helper.
- `setViews()` accepts semantic view objects with `toLayerDescriptors()`.
- `setViews()` rejects raw descriptor objects by default.
- public examples do not call public or private descriptor injection methods.

The final descriptor-injection search may show internal facade/runtime calls,
but it must not show public example or project-site calls.
