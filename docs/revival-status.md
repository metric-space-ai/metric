# Revival Status

This page tracks the current execution status of [REVIVAL_PLAN.md](../REVIVAL_PLAN.md). It is not a replacement for the plan; it records which revival requirements are locally implemented and which release actions still require external repository or package-index state.

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

The package name is currently `metric-py`. A public relaunch still needs an explicit package-name decision before publishing to PyPI.

PyPI name availability was checked on 2026-06-19 with `python -m pip index versions`:

- `metric` exists on PyPI and should not be used for this relaunch.
- `metric-py` had no matching visible distribution.
- `metric-space` had no matching visible distribution.
- `metric-space-numerics` had no matching visible distribution.
- `panda-metric` and `panda_metric` had no matching visible distribution.

The current local package metadata stays on `metric-py` until the release owner chooses whether continuity or clearer public naming is more important.

## External State Checked

The GitHub repository metadata was checked on 2026-06-19 with `gh repo view metric-space-ai/metric`.

- repository description: `Numerical computing framework for finite metric spaces in C++ and Python.`
- homepage: <https://metric-space-ai.github.io/metric/>
- topics: `algorithms`, `cpp`, `finite-metric-spaces`, `metric-space`, `numerical-computing`, `python`

GitHub Pages was checked on 2026-06-19 with `gh api repos/metric-space-ai/metric/pages`. Pages is public and built at <https://metric-space-ai.github.io/metric/>, currently from the legacy `gh-pages` source branch. The new checked-in Pages workflow remains local until the revival branch is committed and pushed.

GitHub Actions workflows were checked on 2026-06-19 with `gh api repos/metric-space-ai/metric/actions/workflows`. The remote repository currently exposes only the pre-revival workflows. The new revival workflows become remote release evidence only after these local workflow files are committed and pushed.

## External Release Work

These plan items cannot be proven by local files alone and remain external release actions:

- keep the GitHub repository description, homepage, and topics aligned with the finite metric-space framing
- switch or confirm GitHub Pages publishing for the checked-in `docs/site/` artifact after the revival branch lands
- run the new GitHub Actions matrix on Linux, macOS, and Windows in the repository
- run `.github/workflows/release-artifacts.yml` from the final release tag
- choose the final public Python package name from the checked candidates or another owner-approved name
- publish release wheels to PyPI
- finalize the [CHANGELOG.md](../CHANGELOG.md) revival draft into release notes using the template in [release-checklist.md](release-checklist.md)

## Historical Code Policy

The broad historical examples, mappings, transforms, native DNN code, image-processing code, and old test suites remain in-tree. They are not removed in this revival slice. Their support status is documented in the module status matrix in [stability.md](stability.md), and they should move into the release gate only after deterministic fixtures, public docs, and CI-backed examples exist.
