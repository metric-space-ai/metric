# MNIST Babyplots Quarantine

Date: 2026-06-26

## Outcome

The active standalone MNIST Babyplots/Babylon render page was deleted:

- `visual/examples/mnist-dimension-reduction/index.html`

No replacement standalone renderer, fake placeholder, generated screenshot, or
new engine path was added. MNIST remains represented by the protected
METRIC-owned GRAE10 engine page:

- `visual/examples/grae10-metric-engine/index.html`

The protected GRAE10 page was not changed.

## Validation

Required checks passed:

```bash
node visual/tools/check-public-gallery-evidence.mjs
node visual/tools/check-direct-runtime-harness-quarantine.mjs
node visual/tools/check-grae10-golden.mjs
rg -n "Baby;|Baby\\.|doRender\\(|mnist-dimension-reduction|bbp\\." visual docs/site
```

The final `rg` command returned no active visual or project-site matches.
