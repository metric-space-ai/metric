"""Small metric-space operators covered by the core Python smoke path."""

import math
import operator

from metric.spaces import FiniteMetricSpace


def pairwise_distance_matrix(records, metric):
    return FiniteMetricSpace(records, metric).pairwise_distances()


def nearest_neighbors(records, metric, query, k=1):
    return FiniteMetricSpace(records, metric).knn(query, k)


def range_neighbors(records, metric, query, radius):
    return FiniteMetricSpace(records, metric).rnn(query, radius)


def representative_indices(records, metric, k, seed_index=0):
    """Select representative record IDs with deterministic farthest-first traversal."""
    records = list(records)
    try:
        k = operator.index(k)
        seed_index = operator.index(seed_index)
    except TypeError:
        raise TypeError("k and seed_index must be integers") from None

    if k < 0:
        raise ValueError("k must be non-negative")
    if k == 0:
        return []
    if not records:
        raise ValueError("cannot select representatives from an empty record set")
    if k > len(records):
        raise ValueError("k cannot exceed the number of records")
    if seed_index < 0 or seed_index >= len(records):
        raise IndexError("seed_index is outside the record set")

    space = FiniteMetricSpace(records, metric)
    selected = [seed_index]
    selected_set = {seed_index}
    nearest_selected_distances = [
        space.distance(index, seed_index)
        for index in range(len(records))
    ]

    while len(selected) < k:
        next_index = None
        next_distance = None
        for index, distance in enumerate(nearest_selected_distances):
            if index in selected_set:
                continue
            if next_distance is None or distance > next_distance:
                next_index = index
                next_distance = distance

        if next_index is None:
            raise RuntimeError("failed to select the next representative")

        selected.append(next_index)
        selected_set.add(next_index)
        for index in range(len(records)):
            nearest_selected_distances[index] = min(
                nearest_selected_distances[index],
                space.distance(index, next_index),
            )

    return selected


def representatives(records, metric, k, seed_index=0):
    """Select representative records with deterministic farthest-first traversal."""
    records = list(records)
    return [
        records[index]
        for index in representative_indices(records, metric, k, seed_index)
    ]


def intrinsic_dimension(records, metric):
    """Estimate expansion dimension from finite metric-space distance growth."""
    return intrinsic_dimension_from_distances(pairwise_distance_matrix(records, metric))


def intrinsic_dimension_from_distances(distances):
    maximum_dimension = 0.0
    for row in distances:
        for radius in row:
            if radius <= 0:
                continue

            inner_count = sum(1 for distance in row if distance <= radius)
            outer_count = sum(1 for distance in row if distance <= radius * 2)
            if inner_count and outer_count >= inner_count:
                maximum_dimension = max(maximum_dimension, math.log2(outer_count / inner_count))
    return maximum_dimension


__all__ = [
    "intrinsic_dimension",
    "intrinsic_dimension_from_distances",
    "pairwise_distance_matrix",
    "nearest_neighbors",
    "range_neighbors",
    "representative_indices",
    "representatives",
]
