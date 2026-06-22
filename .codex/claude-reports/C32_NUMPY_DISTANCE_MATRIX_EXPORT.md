# C32 — One Practical Python Distance-Matrix Export

## Goal

Add one practical, user-facing export: convert an existing `Space` pairwise
matrix to a NumPy array through the adapter layer, with no new C++ algorithm and
no change to metric-space semantics.

## Changed files

- `python/pkg/metric/spaces.py`
  - Added module-level `_numpy()` helper mirroring `metric.operators._numpy`:
    lazy `import numpy`; raises `OptionalDependencyError` on `ModuleNotFoundError`
    so degraded installs without numpy still import the module.
  - Added method `FiniteMetricSpace.distance_matrix_numpy()` (inherited by both
    `Space` and `MatrixSpace`, since `MatrixSpace = FiniteMetricSpace`).
- `python/pkg/metric/operators.py`
  - No change required. The existing `pairwise_distance_matrix` binding and the
    `_numpy()` lazy-import pattern were reused as-is. (Listed allowed file;
    editing it was unnecessary.)
- `python/tests/core/test_revival_api.py`
  - Added `import metric.spaces as spaces_module` and `OptionalDependencyError`
    to the existing imports.
  - Added two additive, self-contained test methods (see below).
- `docs/api/python.md`
  - Added `space.distance_matrix_numpy()` to the Core Methods listing and a
    focused paragraph describing its return contract.
- `docs/python/space.md`
  - Added a focused "NumPy Distance Matrix" section.
- `.codex/claude-reports/C32_NUMPY_DISTANCE_MATRIX_EXPORT.md` (this report).

## Method name and return contract

`Space.distance_matrix_numpy()` (defined on `FiniteMetricSpace`, inherited by
`Space` and `MatrixSpace`):

- Returns a square `numpy.ndarray` of shape `(n, n)` where `n == len(space)`.
- Built from the existing pairwise matrix (`self.pairwise_distances()`, which is
  in record order); preserves record order so `matrix[i][j]` equals
  `space.distance(i, j)`.
- For a symmetric metric the result is symmetric; for an identity-respecting
  metric the diagonal is zero. Both follow from the underlying pairwise values
  and are not special-cased.
- Imports numpy lazily; raises `metric.exceptions.OptionalDependencyError` with a
  clear message when numpy is not installed.

No new pairwise algorithm exists in Python. The method only marshals the
already-promoted pairwise path (`self.pairwise_distances()`) into numpy via
`numpy.asarray(..., dtype=float)`.

## Tests added (additive, in `RevivalApiTest`)

- `test_distance_matrix_numpy_marshals_pairwise_in_record_order`: builds a small
  Space over scalars `[0.0, 1.0, 4.0]` with `metric = lambda l, r: abs(l - r)`;
  asserts the result is an `np.ndarray`, is square `(n, n)`, preserves record
  order (`matrix[i][j] == abs(records[i] - records[j])`), is symmetric
  (`np.array_equal(matrix, matrix.T)`), and has a zero diagonal.
- `test_distance_matrix_numpy_requires_numpy`: monkeypatches
  `spaces_module._numpy` to raise `OptionalDependencyError` (mirroring how
  `operators._numpy()` surfaces a missing numpy), then asserts
  `distance_matrix_numpy()` raises `OptionalDependencyError`; restores the
  original helper in a `finally` block.

## Invocation note

`PYTHONPATH=python/pkg` was REQUIRED for the pytest run. The literal task-listed
command omits it and fails on the namespace collision between `import metric`
(the Python package under `python/pkg/metric`) and the repo-root C++ `metric/`
namespace directory. The commands below include the prepend.

## Exact test output

```
$ PYTHONDONTWRITEBYTECODE=1 PYTHONPATH=python/pkg uv run --python 3.12 --with pytest --with numpy python -m pytest python/tests/core/test_revival_api.py -q
...........................                                 [100%]
27 passed, 13 subtests passed in 1.18s
```

```
$ PYTHONDONTWRITEBYTECODE=1 uv run --python 3.12 python -m compileall -q python/pkg
(no output; exit 0)
```

```
$ git diff --check
(no output; exit 0)
```

## Out-of-scope export paths

The following remain intentionally out of scope (no work done; no stubs added):

- Pandas / DataFrame export of the distance matrix (no pandas dependency added).
- Sub-selected / id-ordered matrix export (`pairwise(ids=...)` exists, but no
  numpy variant was added — only the full record-order matrix).
- File-format exports (CSV/TSV/`.npy` write) of the distance matrix; record
  export via `to_csv`/`to_tsv` is unchanged and is record data, not the matrix.
- Any new native C++ binding or new pairwise algorithm. The method strictly
  reuses the already-promoted native pairwise loop via `pairwise_distances()`.
- SciPy condensed / sparse forms.
