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
- promoted Python example under `python/examples/metric_space/`
- C++ smoke and contract tests under `tests/core_smoke/`
- Python core API tests under `python/tests/core/`
- documentation for concepts, APIs, examples, stability, testing, and release gates
- CI workflows for C++ core, Python wheels, docs/formatting, and GitHub Pages artifacts
- release artifact workflow for source archive, Python sdist, Python wheel built from that sdist, and C++ core/downstream evidence
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
PYTHONDONTWRITEBYTECODE=1 python python/examples/metric_space/string_edit_space.py
PYTHONDONTWRITEBYTECODE=1 python -m unittest discover -s python/tests/core -v

ruby scripts/check_revival_whitespace.rb
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

The final release tag and GitHub release were checked on 2026-06-19:

- tag `v0.3.0` points at commit `84e87eaf45ef8726f2248f0da761b9d8b59695d6`
- [GitHub release `v0.3.0`](https://github.com/metric-space-ai/metric/releases/tag/v0.3.0) is published as a non-draft, non-prerelease release
- release artifact workflow run `27798995987` completed successfully from tag `v0.3.0`
- tag CI runs for docs/formatting, C++ core smoke, and Python core wheels completed successfully
- the release assets include `metric-v0.3.0.tar.gz`, `metric_space-0.3.0.tar.gz`, and `metric_space-0.3.0-cp312-cp312-linux_x86_64.whl`

The remaining external release action is to publish the `metric-space` Python source distribution and wheel to PyPI after confirming package ownership or Trusted Publishing. The repository still contains only a legacy manual TestPyPI workflow, and no local Twine credentials were present during the 2026-06-19 release check.

## Historical Code Policy

The broad historical examples, mappings, transforms, native DNN code, image-processing code, and old test suites remain in-tree. They are not removed in this revival slice. Their support status is documented in the module status matrix in [stability.md](stability.md), and they should move into the release gate only after deterministic fixtures, public docs, and CI-backed examples exist.
