# Project Site Copy Contract

Date: 2026-06-26

## Result

The public project page now reads as a visual portfolio rather than an internal
review log.

Changed:

- `docs/site/index.html`
- `visual/tools/check-project-site-copy-contract.mjs`

The first viewport embeds the protected 60k MNIST/GRAE10 scene. The next
section is the native visual portfolio. Visible copy no longer uses internal
labels such as `review-pending`, `Visual:`, `Evidence:` or `Notes:`. Internal
status remains in `data-evidence-status` attributes so gates can still enforce
that only GRAE10 is accepted.

Browser screenshots for this check were written to:

```text
output/visual/project-site-copy-contract/first-viewport.png
output/visual/project-site-copy-contract/gallery.png
```

Generated screenshots are review artifacts and are not committed.

## Verification

```bash
node visual/tools/check-project-site-copy-contract.mjs
node visual/tools/check-public-gallery-evidence.mjs
node visual/tools/check-grae10-golden.mjs
node visual/tools/check-visual-command-api.mjs
```

Additional browser check:

```bash
python3 -m http.server 8894 --bind 127.0.0.1
```

Then a Playwright smoke captured the first viewport and gallery viewport. It
reported:

- title: `METRIC | Visual Portfolio`
- first iframe: `../../visual/examples/grae10-metric-engine/index.html`
- visible `review-pending`: false
- iframe count: 8
- console/page errors: none
