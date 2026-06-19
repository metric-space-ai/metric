# Release Checklist

This checklist defines the minimum release gates for METRIC.

## Scope

- confirm the release version and package name
- list user-visible API changes
- list algorithmic behavior changes separately from packaging or documentation changes
- list compatibility API changes
- confirm [docs/stability.md](stability.md) reflects the release contents
- confirm [testing-and-ci.md](testing-and-ci.md) reflects required and manual-only workflows
- confirm [revival-status.md](revival-status.md) distinguishes local evidence from external release work
- confirm public Pages content in `docs/site/` matches the release framing
- confirm the final Python package name and PyPI ownership or availability
- confirm [CHANGELOG.md](../CHANGELOG.md) has a release entry using the categories below

## Required Gates

- core C++ CI passes on Linux, macOS, and Windows
- C++ install/export consumer passes with `find_package(panda_metric)`
- Python wheel CI passes on supported CPython versions
- PyPI publishing workflow builds and checks supported wheels before upload
- release artifact workflow passes for the release tag
- promoted C++ and Python examples run in CI
- docs and formatting CI passes
- target engine API examples are either backed by CI or explicitly labeled as roadmap/target API
- representative strategy tests pass for any target intent before it is promoted from roadmap to release API
- `git diff --check` reports no whitespace errors for local changes
- release notes distinguish API, algorithm, packaging, docs, and compatibility changes
- `CHANGELOG.md` includes known limitations and external release actions that are not yet complete
- release artifacts include a source archive, Python source distribution, and Python wheel built from that source distribution

## Local Verification

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
```

Python wheel verification:

```shell
python -m pip wheel ./python --no-deps -w wheelhouse
python -m pip install --force-reinstall wheelhouse/*.whl
for example in python/examples/metric_space/*.py python/examples/engine/*.py; do
  python "$example"
done
PYTHONDONTWRITEBYTECODE=1 python -m unittest discover -s python/tests/core -v
```

Docs and formatting verification:

```shell
ruby scripts/check_revival_whitespace.rb
ruby scripts/check_revival_format.rb
ruby scripts/check_markdown_links.rb
ruby -ryaml -e 'Dir[".github/workflows/*.yml"].each { |path| YAML.load_file(path) }; puts "workflow YAML parsed"'
git diff --check
```

Release artifact workflow:

```shell
git archive --format=tar.gz --prefix="metric-local/" -o /tmp/metric-local.tar.gz HEAD
python -m pip install --upgrade build
python -m build ./python --sdist --outdir wheelhouse
METRIC_PYTHON_USE_BLAS=OFF python -m pip wheel wheelhouse/*.tar.gz --no-deps -w wheelhouse
```

PyPI publishing workflow:

```shell
gh workflow run publish-python.yml \
  --repo metric-space-ai/metric \
  -f ref=v0.3.2 \
  -f publish=false
```

Use `publish=true` only after confirming the target package index, credentials, and package ownership. If PyPI returns `403 Forbidden`, rotate the repository PyPI credentials or configure PyPI Trusted Publishing, confirm the package name is still available, and rerun the same workflow with `ref=v0.3.2` and `publish=true`.

For Trusted Publishing, configure PyPI with:

- owner: `metric-space-ai`
- repository: `metric`
- workflow: `publish-python.yml`
- environment: `pypi`

Then run:

```shell
gh workflow run publish-python.yml \
  --repo metric-space-ai/metric \
  -f ref=v0.3.2 \
  -f publish=true \
  -f auth_method=trusted-publishing
```

## Artifacts

- source archive from the release tag, produced by `.github/workflows/release-artifacts.yml`
- C++ install/export smoke evidence from CI
- Python wheel artifacts for supported CPython versions
- PyPI-compatible wheels from `.github/workflows/publish-python.yml`
- Python source distribution artifact
- public Pages artifact from `docs/site/`
- release notes

## Release Notes Template

```markdown
# METRIC <version>

## API Changes

## Algorithm Changes

## Packaging and Build

## Documentation and Examples

## Compatibility Notes

## Known Limitations
```
