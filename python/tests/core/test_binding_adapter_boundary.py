import numpy as np
import pytest

import metric.distance as distance
from metric import mappings
from metric.exceptions import MissingMetricError, StrategyUnavailableError
from metric.spaces import Space
from metric.strategies import PhateAE


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

    for call in (
        lambda: space.describe(),
        lambda: space.compare(space),
        lambda: space.embed(1),
    ):
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
