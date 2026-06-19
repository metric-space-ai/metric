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
- Python engine-style Space grouping intent with KMedoids and DBSCAN strategies
- Python engine-style Space intent facade methods for representatives and structure diagnostics
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
- C++ and Python exact graph result objects with construction metadata
- C++ and Python graph symmetrization helpers with deterministic weighting policies
- C++ and Python graph out-degree pruning helpers with deterministic sparsification metadata
- C++ and Python graph degree diagnostics with deterministic degree-policy metadata
- C++ and Python graph connectivity diagnostics with deterministic component metadata
- C++ and Python graph stretch diagnostics with deterministic shortest-path metadata
- initial C++ engine skeleton with `MetricSpace`, stable `RecordId`s, metric traits, and engine concept traits
- initial C++ engine representation adapters for implicit distance lookup, matrix caching, exact neighbor-index scaffolds, kNN graph adjacency, graph topology edges, and stale detection
- initial C++ engine nearest operators with `NeighborSet`, `operators::knn`, and `operators::range` over spaces, distance providers, and neighbor indexes
- initial C++ engine clustering operators with `ClusteringResult`, deterministic `operators::kmedoids`, and deterministic `operators::dbscan` over spaces and distance providers
- initial C++ engine intent helpers and strategies with semantic `find_neighbors` and `find_groups` entry points over implemented search and clustering paths
- initial C++ engine representative-selection intent with deterministic farthest-first strategy
- initial C++ engine entropy and MGC operators with named result objects over records and metric spaces
- initial C++ engine compare/correlate intent helpers with explicit MGC strategy
- initial C++ engine describe intent with exact finite-space structure diagnostics
- initial C++ engine reduce intent backed by the PCFA strategy
- C++ engine umbrella header exposure for the PCFA-backed reduce intent
- initial C++ engine runtime policy scaffolding for exact lazy/materialized neighbor execution and representation-cache staleness
- initial C++ engine mapping conventions with `MappingResult` lineage metadata and clustered-space derivation
- initial C++ engine PCFA mapping adapter with explicit fit, transform, and inverse-transform support
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
- C++ and Python exact graph result objects with construction metadata and CI coverage, merged as `d1fcdb4243101d316df707b2e658657390bea42c`
- C++ and Python graph symmetrization helpers with deterministic weighting-policy fixtures and CI coverage, merged as `ab09ce4662f0de46c86396752f7d22c8de42c7c9`
- C++ and Python graph out-degree pruning helpers with deterministic sparsification fixtures and CI coverage, merged as `1bcbc7f818efea4014f54f6e8738436fe9d7c25d`
- C++ and Python graph degree diagnostics with deterministic degree-policy fixtures and CI coverage, merged as `2947be9ec6faf2ffa1f1c3428598ea77e18bdbd5`
- C++ and Python graph connectivity diagnostics with deterministic component fixtures and CI coverage, merged as `6e37b20f2c5ec73501c2c66321719f617609001f`
- C++ and Python graph stretch diagnostics with deterministic shortest-path fixtures and CI coverage, merged as `31e2ed52041712427edc9b9bce20b055ed68c345`
- initial C++ engine skeleton with `MetricSpace`, stable `RecordId`s, metric traits, engine concept traits, and core smoke coverage, merged as `8592d1e636e27227ca083220d3d7e3abf4e04bcf`
- initial C++ engine representation adapters with core smoke coverage for matrix caching, exact neighbor-index scaffolds, kNN graph adjacency, graph topology edges, and stale detection, merged as `f5eebadea22e7b4f312632c3a4c10e731fffbdf1`
- initial C++ engine nearest operators with `NeighborSet`, space/provider/index overloads, and core smoke coverage, merged as `9819d89fded5f0947c21de974410c0b9c56393fd`
- initial C++ engine clustering operator with `ClusteringResult`, deterministic k-medoids over spaces and distance providers, and core smoke coverage, merged as `0bb5a0b695c88ef91dd997d4f59275e04908462f`
- deterministic C++ engine DBSCAN clustering over spaces and distance providers with noise/core record metadata and core smoke coverage, merged as `d523f18a34383206f17b1a699d6a4e819da0d801`
- initial C++ engine intent helpers and strategy objects for semantic neighbor and grouping workflows, merged as `d3620cc63a4375ed248575d5a02114d778eb70a6`
- C++ engine representative-selection intent with deterministic farthest-first strategy and core smoke coverage, merged as `936f25fa59055a35a8f9fc5155513bee7990ba5e`
- initial C++ engine entropy and MGC operator wrappers with named result objects and core smoke coverage, merged as `68199bcf7443056b1ebed32fd00b57ca5fea676e`
- C++ engine compare/correlate intent helpers with explicit MGC strategy and core smoke coverage, merged as `d90a7314a453df8b4499cebbfbc5c1f2f9d40c0d`
- C++ engine describe intent with exact finite-space structure diagnostics and core smoke coverage, merged as `5fc5d65f6e13d2b7f1529956265240c592a5b141`
- initial C++ engine mapping conventions with clustered-space derivation, `MappingResult` lineage metadata, and core smoke coverage, merged as `9e4b2d1a349d4b43e76e06ec7374f1d43a919296`
- C++ engine PCFA mapping adapter with explicit inverse reconstruction and LAPACK-gated core smoke coverage, merged as `0aff5aea43433b4fe1b20dbc84cedf71b0e4559e`
- C++ engine reduce intent backed by the PCFA strategy with LAPACK-gated core smoke coverage, merged as `cf1ceef290f73fac95943f0b7132953fe9d0c698`
- C++ engine umbrella header exposure for the PCFA-backed reduce intent with core, downstream, and include-smoke coverage, merged as `82457b5368e49cfbb6a0f3c417fef0fc56c4f5fb`
- initial C++ engine runtime policy scaffolding for exact lazy/materialized neighbor execution, representation diagnostics, and cache staleness coverage, merged as `c9217ec01bc06578dc050a06605df22c44fd62eb`
- Python engine-style Space intent facade methods for representatives and structure diagnostics with named result objects and core test coverage, merged as `1849c29a25122b31605945295c91710cfc6a126f`
- Python engine-style Space grouping intent with KMedoids and DBSCAN strategies, named `ClusteringResult` objects, and core test coverage, merged as `fe2641f8f14e660a994c94ca18d9f5feb426fb2b`

## Historical Code Policy

The broad historical examples, mappings, transforms, native DNN code, image-processing code, and old test suites remain in-tree. They are not removed in this revival slice. Their support status is documented in the module status matrix in [stability.md](stability.md), and they should move into the release gate only after deterministic fixtures, public docs, and CI-backed examples exist.
