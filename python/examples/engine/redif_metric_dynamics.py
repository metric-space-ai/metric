"""Redif metric dynamics over several finite metric spaces.

Redif evolves atom measures over the metric structure. The records below are
strings, histograms, aligned process curves, mixed records, and scalar atoms;
all Redif paths use only the supplied distance.
"""

from metric import Space
from metric.operators import find_outliers
from metric.strategies import DBSCAN, ForwardDynamics, InverseDynamics, TransportPath


def edit_distance(lhs, rhs):
    previous = list(range(len(rhs) + 1))
    for left_index, left_value in enumerate(lhs, start=1):
        current = [left_index]
        for right_index, right_value in enumerate(rhs, start=1):
            insert = current[-1] + 1
            delete = previous[right_index] + 1
            replace = previous[right_index - 1] + (0 if left_value == right_value else 1)
            current.append(min(insert, delete, replace))
        previous = current
    return previous[-1]


def cumulative_transport_distance(lhs, rhs):
    cumulative = 0.0
    distance = 0.0
    for left, right in zip(lhs, rhs):
        cumulative += left - right
        distance += abs(cumulative)
    return distance


def aligned_curve_distance(lhs, rhs):
    gap_cost = 2.0
    previous = [index * gap_cost for index in range(len(rhs) + 1)]
    for left_index, left_value in enumerate(lhs, start=1):
        current = [left_index * gap_cost] + [0.0] * len(rhs)
        for right_index, right_value in enumerate(rhs, start=1):
            substitute = previous[right_index - 1] + min(abs(left_value - right_value), 2 * gap_cost)
            delete = previous[right_index] + gap_cost
            insert = current[right_index - 1] + gap_cost
            current[right_index] = min(substitute, delete, insert)
        previous = current
    return previous[-1]


def mixed_record_distance(lhs, rhs):
    status = 0.0 if lhs["status"] == rhs["status"] else 2.0
    message = edit_distance(lhs["message"], rhs["message"]) / max(
        len(lhs["message"]),
        len(rhs["message"]),
        1,
    )
    curve = aligned_curve_distance(lhs["curve"], rhs["curve"]) / max(len(lhs["curve"]), len(rhs["curve"]))
    return status + message + curve + abs(lhs["level"] - rhs["level"]) / 10.0


def assert_measure_result(label, result, expected_count):
    assert result.record_count == expected_count
    assert len(result.paths) == expected_count
    assert len(result.summaries) == expected_count
    assert result.transport_diagnostics["solver"] == "exact_discrete_wasserstein"
    assert result.operator_diagnostics[0]["spectral_gap_proxy"] == "minimum_transition_escape_probability"
    print(label, "paths =", len(result.paths), "proxy =", round(result.operator_diagnostics[0]["spectral_gap_proxy_value"], 6))


def run_redif_space(label, records, metric, ids, expected_singular_id):
    space = Space(records, metric=metric, ids=ids, name=label)
    inverse = space.remove_noise(InverseDynamics(neighbors=1, iterations=2, euler_step=0.2, adaptive_geometry=False))
    assert_measure_result(label + " inverse", inverse, len(records))

    forward = space.add_noise(ForwardDynamics(neighbors=1, iterations=2, euler_step=0.2, adaptive_geometry=False))
    assert_measure_result(label + " forward", forward, len(records))
    assert max(summary.terminal_shannon_entropy for summary in forward.summaries) > 0.0

    ranked = space.outliers(TransportPath(neighbors=1, iterations=2, euler_step=0.2, adaptive_geometry=False))
    assert ranked.outliers[0].record_id == expected_singular_id
    print(label, "transport path leader =", ranked.outliers[0].record_id)


def main():
    run_redif_space(
        "strings",
        ["cat", "cot", "coat", "encyclopaedia"],
        edit_distance,
        ("cat", "cot", "coat", "long-word"),
        "long-word",
    )

    run_redif_space(
        "histograms",
        [
            (1.0, 0.0, 0.0, 0.0),
            (0.8, 0.2, 0.0, 0.0),
            (0.0, 0.2, 0.8, 0.0),
            (0.0, 0.0, 0.0, 1.0),
        ],
        cumulative_transport_distance,
        ("left-edge", "left-shift", "right-shift", "right-edge"),
        "right-shift",
    )

    run_redif_space(
        "process-curves",
        [
            (0.0, 1.0, 1.0, 1.0, 2.0),
            (0.0, 1.0, 1.2, 1.0, 2.1),
            (0.0, 1.0, 6.0, 1.0, 2.0),
            (3.0, 3.0, 3.0, 3.0),
        ],
        aligned_curve_distance,
        ("baseline", "small-drift", "spike", "flat"),
        "flat",
    )

    run_redif_space(
        "mixed-records",
        [
            {"status": "stable", "message": "nominal flow", "curve": (1.0, 1.0, 1.1), "level": 2.0},
            {"status": "stable", "message": "nominal flows", "curve": (1.0, 1.0, 1.2), "level": 2.5},
            {"status": "warmup", "message": "valve drift", "curve": (1.0, 1.4, 1.8), "level": 5.0},
            {"status": "alert", "message": "pressure spike stop", "curve": (1.0, 7.0, 1.0), "level": 10.0},
        ],
        mixed_record_distance,
        ("stable-a", "stable-b", "warmup", "alert"),
        "alert",
    )

    bridge_records = [0, 1, 2, 6, 10, 11, 12]
    bridge_space = Space(bridge_records, metric=lambda lhs, rhs: abs(lhs - rhs), ids=tuple(f"atom-{x}" for x in bridge_records))
    redif_ranked = bridge_space.outliers(
        TransportPath(neighbors=2, iterations=4, euler_step=0.25, adaptive_geometry=False)
    )
    assert redif_ranked.outliers[0].record_id == "atom-6"
    density_ranked = find_outliers(
        bridge_records,
        lambda lhs, rhs: abs(lhs - rhs),
        DBSCAN(radius=4.0, min_points=2),
    )
    assert not density_ranked.outliers
    print("bridge Redif leader =", redif_ranked.outliers[0].record_id)
    print("bridge density-unassigned count =", len(density_ranked.outliers))


if __name__ == "__main__":
    main()
