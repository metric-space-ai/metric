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


def exact_knn_graph_edges(records, metric, k):
    """Build exact directed kNN graph edges as source, target, distance tuples."""
    records = list(records)
    try:
        k = operator.index(k)
    except TypeError:
        raise TypeError("k must be an integer") from None

    if k < 0:
        raise ValueError("k must be non-negative")
    if k == 0:
        return []

    max_neighbors = max(0, len(records) - 1)
    if k > max_neighbors:
        raise ValueError("k cannot exceed the number of non-self neighbors")

    space = FiniteMetricSpace(records, metric)
    edges = []

    for source_index in range(len(records)):
        candidates = []
        for target_index in range(len(records)):
            if source_index == target_index:
                continue
            candidates.append((
                space.distance(source_index, target_index),
                target_index,
            ))

        candidates.sort()
        for distance, target_index in candidates[:k]:
            edges.append((source_index, target_index, distance))

    return edges


def exact_radius_graph_edges(records, metric, radius):
    """Build exact directed radius graph edges as source, target, distance tuples."""
    records = list(records)
    if radius < 0:
        raise ValueError("radius must be non-negative")

    space = FiniteMetricSpace(records, metric)
    edges = []

    for source_index in range(len(records)):
        for target_index in range(len(records)):
            if source_index == target_index:
                continue
            distance = space.distance(source_index, target_index)
            if distance <= radius:
                edges.append((source_index, target_index, distance))

    return edges


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


def medoid_index(records, metric):
    """Select the record ID with the smallest total distance to all records."""
    records = list(records)
    if not records:
        raise ValueError("cannot select a medoid from an empty record set")

    space = FiniteMetricSpace(records, metric)
    best_index = 0
    best_total_distance = None

    for candidate_index in range(len(records)):
        total_distance = sum(
            space.distance(candidate_index, other_index)
            for other_index in range(len(records))
        )
        if best_total_distance is None or total_distance < best_total_distance:
            best_index = candidate_index
            best_total_distance = total_distance

    return best_index


def medoid(records, metric):
    """Select the record with the smallest total distance to all records."""
    records = list(records)
    return records[medoid_index(records, metric)]


def separated_representative_indices(records, metric, minimum_distance):
    """Select representatives greedily separated by a minimum distance."""
    records = list(records)
    if minimum_distance < 0:
        raise ValueError("minimum_distance must be non-negative")
    if not records:
        return []

    space = FiniteMetricSpace(records, metric)
    selected = []

    for candidate_index in range(len(records)):
        if all(
            space.distance(candidate_index, selected_index) >= minimum_distance
            for selected_index in selected
        ):
            selected.append(candidate_index)

    return selected


def separated_representatives(records, metric, minimum_distance):
    """Select representative records greedily separated by a minimum distance."""
    records = list(records)
    return [
        records[index]
        for index in separated_representative_indices(records, metric, minimum_distance)
    ]


def coverage_representative_indices(records, metric, radius):
    """Select representatives that greedily cover records within a radius."""
    records = list(records)
    if radius < 0:
        raise ValueError("radius must be non-negative")
    if not records:
        return []

    space = FiniteMetricSpace(records, metric)
    selected = []
    covered = [False] * len(records)

    while not all(covered):
        seed_index = next(index for index, is_covered in enumerate(covered) if not is_covered)
        selected.append(seed_index)

        for index in range(len(records)):
            if space.distance(seed_index, index) <= radius:
                covered[index] = True

    return selected


def coverage_representatives(records, metric, radius):
    """Select representative records that greedily cover records within a radius."""
    records = list(records)
    return [
        records[index]
        for index in coverage_representative_indices(records, metric, radius)
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
    "coverage_representative_indices",
    "coverage_representatives",
    "exact_knn_graph_edges",
    "exact_radius_graph_edges",
    "medoid",
    "medoid_index",
    "pairwise_distance_matrix",
    "nearest_neighbors",
    "range_neighbors",
    "representative_indices",
    "representatives",
    "separated_representative_indices",
    "separated_representatives",
]
