# Revival Status

This page tracks the current execution status of [REVIVAL_PLAN.md](../REVIVAL_PLAN.md). It is not a replacement for the plan; it records which revival requirements are implemented on `master` and which release actions still require package-index state.

## Local Revival Scope

The current local tree implements the first revival slice:

- public README framing around finite metric-space numerics
- short technical manifesto distinguishing metric-space numerics from vector-search and embedding-only workflows
- CMake presets for `core`, `dev`, `full-tests`, and `release`
- top-level CMake preset for Python core bindings with `python-cmake`
- normalized CMake options with a deprecated `BUILD_TESTS` compatibility path
- FetchContent fallback for core C++ dependencies
- install/export support for `find_package(panda_metric)`
- a downstream CMake consumer under `tests/downstream_consumer/`
- core C++ `Metric`, `Space`, `FiniteSpace`, explicit representation aliases, and `metric::operators`
- Python core facade modules under `metric.metrics`, `metric.spaces`, and `metric.operators`
- Python beta compatibility bridge modules under `metric.mappings` and `metric.transforms`
- promoted C++ examples under `examples/core/`
- promoted Python examples under `python/examples/metric_space/`
- C++ smoke and contract tests under `tests/core_smoke/`
- Python core API and metric-contract tests under `python/tests/core/`
- intrinsic-dimension diagnostics in C++ and Python core operator helpers
- deterministic C++ representative-selection helpers in `metric::operators`
- deterministic Python representative-selection helpers in `metric.operators`
- deterministic C++ and Python medoid representative helpers
- deterministic C++ and Python separated-representative helpers
- graph representation terminology for exact, approximate, directed, symmetrized, weighted, and normalized graph construction
- deterministic C++ and Python exact graph edge helpers
- documentation for concepts, APIs, examples, stability, testing, and release gates
- CI workflows for C++ core, Python wheels, docs/formatting, revived-source formatting, and GitHub Pages artifacts
- release artifact workflow for source archive, Python sdist, Python wheel built from that sdist, and C++ core/downstream evidence
- manual PyPI publishing workflow for checked sdist and cibuildwheel wheel artifacts, with repository-secret and Trusted Publishing authentication paths
- promoted Python examples for strings, structured records, time-series alignment, and histogram transport, with subprocess execution from the core Python API tests
- manual-only legacy workflows for broad historical coverage

## Current Local Verification

The following commands are the local evidence for the revived core:

```shell
cmake --preset core
cmake --build --preset core --parallel 2
ctest --preset core

cmake --preset dev
cmake --build --preset dev --parallel 2
ctest --preset dev

cmake --install build/core --prefix build/install-core --config Debug
cmake -S tests/downstream_consumer -B build/downstream-consumer \
  -DCMAKE_PREFIX_PATH="$PWD/build/install-core"
cmake --build build/downstream-consumer --parallel 2 --config Debug
ctest --test-dir build/downstream-consumer --output-on-failure --build-config Debug

cmake --preset python-cmake
cmake --build --preset python-cmake --parallel 2

python -m pip install --upgrade build
python -m build ./python --sdist --outdir build/wheelhouse-revival
METRIC_PYTHON_USE_BLAS=OFF python -m pip wheel build/wheelhouse-revival/*.tar.gz --no-deps -w build/wheelhouse-revival
python -m pip install --force-reinstall build/wheelhouse-revival/*.whl
for example in python/examples/metric_space/*.py; do
  PYTHONDONTWRITEBYTECODE=1 python "$example"
done
PYTHONDONTWRITEBYTECODE=1 python -m unittest discover -s python/tests/core -v

ruby scripts/check_revival_whitespace.rb
ruby scripts/check_revival_format.rb
ruby scripts/check_markdown_links.rb
ruby -ryaml -e 'Dir[".github/workflows/*.yml"].each { |path| YAML.load_file(path) }; puts "workflow YAML parsed"'
git diff --check
```

## Packaging Decision

The Python package currently uses `pyproject.toml` for the PEP 517 build-system declaration and PEP 621 project metadata. `setup.py` registers the CMake extension build hook and the source-distribution hook that includes the required C++ metric headers in the Python sdist. `scikit-build-core` remains a candidate for a later packaging simplification, but it is not adopted in this revival slice because the current bridge already builds the core wheel and preserves the existing extension layout.

The public Python distribution name is `metric-space`. The import package remains `metric`.

PyPI name availability was checked on 2026-06-19 with the official PyPI JSON API:

- `metric` exists on PyPI and should not be used for this relaunch.
- `metric-py` exists on PyPI as version `0.0.6`, points at the historical `panda-official/metric` repository, and should not be reused unless package ownership and continuity are deliberately restored.
- `metric-space` had no matching visible distribution.
- `metric-space-numerics` had no matching visible distribution.
- `panda-metric` and `panda_metric` had no matching visible distribution.

The release metadata therefore uses `metric-space` to avoid colliding with both the unrelated `metric` project and the historical `metric-py` distribution.

## External State Checked

The revival branch landed through [pull request #326](https://github.com/metric-space-ai/metric/pull/326), merged into `master` as commit `c1bc86f035e132617a111bbf91577f92070ff22e`.

The release metadata update landed through [pull request #327](https://github.com/metric-space-ai/metric/pull/327), merged into `master` as commit `84e87eaf45ef8726f2248f0da761b9d8b59695d6`.

The `v0.3.2` release metadata update landed through [pull request #340](https://github.com/metric-space-ai/metric/pull/340), merged into `master` as commit `62d8e1ec5955b7443254e20e1d60b7b33f489a18`.

The GitHub repository metadata was checked on 2026-06-19 with `gh repo view metric-space-ai/metric`.

- repository description: `Numerical computing framework for finite metric spaces in C++ and Python.`
- homepage: <https://metric-space-ai.github.io/metric/>
- topics: `algorithms`, `cpp`, `finite-metric-spaces`, `metric-space`, `numerical-computing`, `python`

GitHub Pages was checked on 2026-06-19 with `gh api repos/metric-space-ai/metric/pages`. Pages is public, uses `build_type: workflow`, and is built at <https://metric-space-ai.github.io/metric/>.

GitHub Actions was checked on 2026-06-19 with `gh run list --repo metric-space-ai/metric --branch master`. The following `master` runs completed successfully on commit `84e87eaf45ef8726f2248f0da761b9d8b59695d6`:

- Core C++ smoke
- Python core wheel
- Docs and formatting
- Dependency Graph

The manual release-artifact rehearsal was checked on 2026-06-19 with `gh run view 27798350044`. It completed successfully on `master` and uploaded:

- `metric-source-archive`
- `metric-python-artifacts`

## External Release State

The initial release tag and GitHub release were checked on 2026-06-19:

- tag `v0.3.0` points at commit `84e87eaf45ef8726f2248f0da761b9d8b59695d6`
- [GitHub release `v0.3.0`](https://github.com/metric-space-ai/metric/releases/tag/v0.3.0) is published as a non-draft, non-prerelease release
- release artifact workflow run `27798995987` completed successfully from tag `v0.3.0`
- tag CI runs for docs/formatting, C++ core smoke, and Python core wheels completed successfully
- the release assets include `metric-v0.3.0.tar.gz`, `metric_space-0.3.0.tar.gz`, and `metric_space-0.3.0-cp312-cp312-linux_x86_64.whl`

PyPI publishing moved to the `v0.3.1` packaging patch release so Linux wheels use extension-module Python discovery rather than embedded-Python library discovery:

- tag `v0.3.1` points at commit `9d59505b799b5b91ae24b0994173b0df4f336c3b`
- [GitHub release `v0.3.1`](https://github.com/metric-space-ai/metric/releases/tag/v0.3.1) is published as a non-draft, non-prerelease release
- release artifact workflow run `27800464584` completed successfully from tag `v0.3.1`
- tag CI runs for docs/formatting, C++ core smoke, and Python core wheels completed successfully
- the manual PyPI publishing workflow was repaired on `master` through commit `a26ac42abbcfc72550420f868107719beadeec25`
- PyPI publish dry-run `27801076045` completed successfully with one source distribution and 15 checked wheels for CPython 3.10 through 3.14 on Linux, macOS, and Windows
- publish run `27801491154` rebuilt and checked the same distribution set successfully, but the final Twine upload failed with `HTTPError: 403 Forbidden` from `https://upload.pypi.org/legacy/`
- PyPI still returned 404 for `https://pypi.org/pypi/metric-space/json` after the failed upload, so no visible `metric-space` release was published

The remaining external release action is to update or replace the repository PyPI credentials, or configure PyPI Trusted Publishing for owner `metric-space-ai`, repository `metric`, workflow `publish-python.yml`, and environment `pypi`. The workflow supports both repository-secret password uploads and Trusted Publishing uploads; after PyPI access is fixed, rerun `.github/workflows/publish-python.yml` with the current release tag, `publish=true`, and the appropriate `auth_method`. No local Twine credentials were present during the 2026-06-19 release check.

PyPI publishing is now prepared against the `v0.3.2` release, which includes the post-`v0.3.1` revival work:

- tag `v0.3.2` points at commit `62d8e1ec5955b7443254e20e1d60b7b33f489a18`
- [GitHub release `v0.3.2`](https://github.com/metric-space-ai/metric/releases/tag/v0.3.2) is published as a non-draft, non-prerelease release
- release artifact workflow run `27803605815` completed successfully from tag `v0.3.2`
- tag CI runs for docs/formatting, C++ core smoke, Python core wheels, and release artifacts completed successfully
- the release assets include `metric-v0.3.2.tar.gz`, `metric_space-0.3.2.tar.gz`, and `metric_space-0.3.2-cp312-cp312-linux_x86_64.whl`
- PyPI publish dry-run `27803764604` completed successfully with one source distribution artifact and checked wheel artifact sets for CPython 3.10 through 3.14 on Linux, macOS, and Windows
- PyPI still returned 404 for `https://pypi.org/pypi/metric-space/json` after the dry-run, so no visible `metric-space` release has been published

The `v0.3.2` release includes the following revival improvements that landed on `master` after the `v0.3.1` tag:

- Trusted Publishing support for `.github/workflows/publish-python.yml`, merged as `e0f7b94764d3a277315867d28f40d04c6e2a6168`
- promoted Python structured-record example, merged as `0417dd113f2452ec4772e7835cbf86a33e572e81`
- release and PyPI build gates running both promoted Python examples, merged as `3d7b1a6e4cfeb37708615e2eb795a1d66bd8450d`
- Python core metric-contract checks for edit distance, NumPy record callables, and structured-record callables, merged as `3073280344ef66831fbcfcf9197df7d01050b82c`
- revival source-format checks for promoted source and docs files, merged as `cf35266ba5e18b6113f8eb3aa8715ac500050710`
- intrinsic-dimension diagnostics in C++ and Python, merged as `d510e0ceb78a307a4e91545837ee4e6ff81eccc3`

## Post-v0.3.2 Master Progress

The following revival improvements landed on `master` after the `v0.3.2` tag and are unreleased until the next tag:

- promoted Python time-series metric-space example using an alignment-aware callable, merged as `30b75635dfc26010d2f400539dd69952fc787f41`
- promoted Python histogram metric-space example using a one-dimensional transport callable, merged as `dbed0eba25455e420baba79f49d134c126dd6c14`
- Python core API tests now subprocess-run every `python/examples/metric_space/*.py` example, merged as `dbed0eba25455e420baba79f49d134c126dd6c14`
- testing and release-checklist docs now describe the expanded promoted Python example gate, merged as `dbed0eba25455e420baba79f49d134c126dd6c14`
- PyPI publish dry-run `27804901826` completed successfully on `master` commit `1ab3ada019f0460f4ba418291bd2146342ef8064` with one source distribution artifact and checked wheel artifact sets for CPython 3.10 through 3.14 on Linux, macOS, and Windows
- PyPI still returned 404 for `https://pypi.org/pypi/metric-space/json` after the master dry-run, so no visible `metric-space` release has been published
- promotion-gated research roadmap for diagnostics, representative selection, sparse graphs, cross-space dependency discovery, denoising, vector-database adapters, and benchmarks, merged as `b80eaba438a6a45f327951c87f74d88c4af37ed3`
- Python representative-selection helpers using deterministic farthest-first traversal, merged as `78d9d4cdb065555541e10131698452c6d713a257`
- promoted Python representative-selection example and wheel-gate coverage, merged as `78d9d4cdb065555541e10131698452c6d713a257`
- C++ representative-selection helpers using deterministic farthest-first traversal, merged as `2aa8cfbc7cddcb642896de64cdf2d00a6031504f`
- promoted C++ representative-selection example and CTest coverage, merged as `2aa8cfbc7cddcb642896de64cdf2d00a6031504f`
- representative-selection fixtures for process curves and structured mixed records, merged as `dd891b7d32d5f862d1517173a9a5cd8c7d032e36`
- C++ and Python radius-coverage representative helpers with promoted examples and CI coverage, merged as `5eb721b1ae7004dd6643e9d74d9995c6ad0463c7`
- C++ and Python medoid representative helpers with promoted examples and CI coverage, merged as `3022a8485b1df5f8322437bc5d6d9998305082ca`
- C++ and Python separated-representative helpers with promoted examples and CI coverage, merged as `5cfae5fdbb7159aa04c51dfe85734fd295d0f7b9`
- graph representation terminology for exact and approximate graph construction, merged as `5b002c216a437c44c47b252b429268d13b679951`
- C++ and Python exact graph edge helpers with deterministic edge fixtures and CI coverage, merged as `5d20828c09faafb4fc9d623443f3e5421cf64e13`

## Historical Code Policy

The broad historical examples, mappings, transforms, native DNN code, image-processing code, and old test suites remain in-tree. They are not removed in this revival slice. Their support status is documented in the module status matrix in [stability.md](stability.md), and they should move into the release gate only after deterministic fixtures, public docs, and CI-backed examples exist.
