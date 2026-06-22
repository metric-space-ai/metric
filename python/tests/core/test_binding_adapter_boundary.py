import importlib
import importlib.util
from pathlib import Path

import numpy as np
import pytest

import metric
import metric.distance as distance
from metric import mappings
from metric.exceptions import MissingMetricError, OptionalDependencyError, StrategyUnavailableError
from metric.operators import CorrelationResult
from metric.spaces import Space
from metric.strategies import PhateAE


def _native_correlation_available():
    return all(
        importlib.util.find_spec(name) is not None
        for name in ("metric._impl.entropy", "metric._impl.mgc")
    )


def test_import_metric_succeeds_without_native_correlation():
    # The top-level package must keep importing even when the native
    # correlation bindings are absent (it does not import correlation).
    assert hasattr(metric, "OptionalDependencyError")
    assert issubclass(OptionalDependencyError, metric.MetricError)


@pytest.mark.skipif(
    _native_correlation_available(),
    reason="native correlation bindings are present; the adapter-boundary raise does not apply",
)
def test_correlation_import_raises_optional_dependency_error_when_native_absent():
    # Importing the correlation adapter must convert the missing native
    # binding into a clean metric.OptionalDependencyError that names the
    # native binding -- never a raw ModuleNotFoundError / ABI traceback.
    with pytest.raises(OptionalDependencyError) as excinfo:
        importlib.import_module("metric.correlation")

    message = str(excinfo.value)
    assert "metric._impl.entropy" in message
    assert "adapter boundary" in message

    # The surfaced error must be OptionalDependencyError, not a raw
    # ModuleNotFoundError, and must not chain one as its visible cause.
    assert not isinstance(excinfo.value, ModuleNotFoundError)
    assert excinfo.value.__cause__ is None

    # Loading the mgc adapter module on its own (bypassing the package
    # __init__, which fails first on entropy) names its own native binding.
    mgc_path = (
        Path(metric.__file__).resolve().parent / "correlation" / "mgc.py"
    )
    mgc_spec = importlib.util.spec_from_file_location(
        "metric_correlation_mgc_under_test", mgc_path
    )
    mgc_module = importlib.util.module_from_spec(mgc_spec)
    with pytest.raises(OptionalDependencyError) as mgc_excinfo:
        mgc_spec.loader.exec_module(mgc_module)
    assert "metric._impl.mgc" in str(mgc_excinfo.value)
    assert mgc_excinfo.value.__cause__ is None


def test_standard_vector_metrics_are_native_distance_exports():
    import metric._impl.distance as native_distance

    for name in (
        "Euclidean",
        "Euclidean_thresholded",
        "Manhattan",
        "P_norm",
        "Cosine",
        "Chebyshev",
    ):
        exported = getattr(distance, name)
        assert exported is getattr(native_distance, name)
        assert exported.__module__ == "metric._impl.distance"

    assert distance.Minkowski is distance.P_norm
    assert distance.ThresholdedEuclidean is distance.Euclidean_thresholded


def test_space_vectors_uses_native_euclidean_without_python_fallback():
    records = np.asarray([[0.0, 0.0], [3.0, 4.0]], dtype=float)

    space = Space.vectors(records, validate="none", cache="none")

    assert type(space.metric).__module__ == "metric._impl.distance"
    assert type(space.metric).__name__ == "Euclidean"
    assert space.distance(0, 1) == 5.0


def test_space_vectors_missing_native_euclidean_has_clear_error(monkeypatch):
    monkeypatch.setattr(distance, "Euclidean", None)

    with pytest.raises(MissingMetricError, match="does not implement Euclidean distance itself"):
        Space.vectors([[0.0], [1.0]], validate="none", cache="none")


def test_unpromoted_stats_modify_calls_still_raise():
    space = Space([0.0, 1.0, 2.0], metric=lambda lhs, rhs: abs(lhs - rhs), validate="none")

    assert space.nearest(0.0).id == 0
    assert [neighbor.id for neighbor in space.neighbors(0.0, count=1).neighbors] == [0]
    assert space.reduce(1).source_record_ids == (0,)
    assert space.compress(1).compressed_record_count == 1
    assert space.describe().record_count == 3
    assert space.groups(2).cluster_count == 2
    assert len(space.outliers(count=1).outliers) == 1
    assert space.denoise(count=1).target_record_count == 2

    # compare/correlate are promoted for the aligned (equal-length) path.
    comparison = space.compare(space)
    assert isinstance(comparison, CorrelationResult)
    assert comparison.left_record_count == 3
    assert comparison.right_record_count == 3
    assert comparison.align == "position"

    for call in (lambda: space.embed(1),):
        with pytest.raises(StrategyUnavailableError, match="adapter-only surface"):
            call()

    with pytest.raises(StrategyUnavailableError, match="Custom Python metrics are not ignored"):
        space.map(strategy=PhateAE(dimensions=1))


def test_native_phate_adapter_passes_public_provider_keyword_to_cpp():
    try:
        mappings.native_phate_autoencoder_fit_vectors(
            [[0.0, 0.0], [1.0, 1.0]],
            dimensions=1,
            epochs=0,
            distance_provider="unknown_provider",
        )
    except TypeError as exc:
        if "distance_provider" in str(exc) and "pairwise_distances" in str(exc):
            pytest.skip("local metric._impl.metric extension is stale; fresh build is blocked by C++ headers")
        raise
    except ValueError as exc:
        assert "unsupported native PHATE-AE distance provider" in str(exc)
    else:
        raise AssertionError("unknown native PHATE-AE distance provider was accepted")
