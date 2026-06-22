"""Mixed structured records — exact search over heterogeneous records.

A rich toy domain metric that combines numeric, categorical, string, spectrum,
and curve fields. The Python ``Space`` adapts these heterogeneous records and
exposes explicit distances, native exact-scan neighbor search, and a runtime-
policy view. Clustering and outlier detection run through native bindings;
embedding remains unavailable until its binding is promoted.
"""

from math import sqrt

from metric import RuntimePolicy, Space
from metric.exceptions import StrategyUnavailableError
from metric.operators import pairwise_distance_matrix
from metric.strategies import DBSCAN, MDS


WEIGHTS = {
    "temperature_summary": 0.20,
    "status": 0.15,
    "message": 0.15,
    "spectrum": 0.25,
    "curve": 0.25,
}


def euclidean(lhs, rhs):
    return sqrt(sum((left - right) ** 2 for left, right in zip(lhs, rhs)))


def edit_distance(lhs, rhs):
    previous = list(range(len(rhs) + 1))
    for lhs_index, lhs_char in enumerate(lhs, start=1):
        current = [lhs_index] + [0] * len(rhs)
        for rhs_index, rhs_char in enumerate(rhs, start=1):
            substitute = previous[rhs_index - 1] + (0 if lhs_char == rhs_char else 1)
            delete = previous[rhs_index] + 1
            insert = current[rhs_index - 1] + 1
            current[rhs_index] = min(substitute, delete, insert)
        previous = current
    return previous[-1]


def transport_distance(lhs, rhs):
    cumulative_delta = 0.0
    distance = 0.0
    for lhs_mass, rhs_mass in zip(lhs, rhs):
        cumulative_delta += lhs_mass - rhs_mass
        distance += abs(cumulative_delta)
    return distance


def aligned_curve_distance(lhs, rhs):
    gap_cost = 2.0
    previous = [index * gap_cost for index in range(len(rhs) + 1)]
    for lhs_index, lhs_value in enumerate(lhs, start=1):
        current = [lhs_index * gap_cost] + [0.0] * len(rhs)
        for rhs_index, rhs_value in enumerate(rhs, start=1):
            substitute = previous[rhs_index - 1] + min(
                abs(lhs_value - rhs_value),
                2 * gap_cost,
            )
            delete = previous[rhs_index] + gap_cost
            insert = current[rhs_index - 1] + gap_cost
            current[rhs_index] = min(substitute, delete, insert)
        previous = current
    return previous[-1]


def field_contributions(lhs, rhs):
    message_scale = max(len(lhs["message"]), len(rhs["message"]), 1)
    curve_scale = max(len(lhs["curve"]), len(rhs["curve"]), 1)
    return {
        "temperature_summary": WEIGHTS["temperature_summary"]
        * euclidean(lhs["temperature_summary"], rhs["temperature_summary"]),
        "status": WEIGHTS["status"] * (0.0 if lhs["status"] == rhs["status"] else 2.0),
        "message": WEIGHTS["message"] * edit_distance(lhs["message"], rhs["message"]) / message_scale,
        "spectrum": WEIGHTS["spectrum"] * transport_distance(lhs["spectrum"], rhs["spectrum"]),
        "curve": WEIGHTS["curve"] * aligned_curve_distance(lhs["curve"], rhs["curve"]) / curve_scale,
    }


def mixed_record_distance(lhs, rhs):
    return sum(field_contributions(lhs, rhs).values())


def make_records():
    return [
        {
            "temperature_summary": (68.0, 69.0, 70.0),
            "status": "stable",
            "message": "nominal flow",
            "spectrum": (0.6, 0.3, 0.1, 0.0),
            "curve": (1.0, 1.0, 1.1, 1.0),
        },
        {
            "temperature_summary": (72.0, 73.0, 75.0),
            "status": "warmup",
            "message": "valve drift warning",
            "spectrum": (0.1, 0.3, 0.4, 0.2),
            "curve": (1.0, 1.2, 1.5, 1.4, 1.2),
        },
        {
            "temperature_summary": (72.5, 73.2, 74.5),
            "status": "warmup",
            "message": "valve drift alert",
            "spectrum": (0.1, 0.25, 0.45, 0.2),
            "curve": (1.0, 1.2, 1.4, 1.4, 1.3),
        },
        {
            "temperature_summary": (64.0, 63.5, 63.0),
            "status": "cooldown",
            "message": "pump normal",
            "spectrum": (0.0, 0.2, 0.5, 0.3),
            "curve": (2.0, 1.7, 1.4, 1.1),
        },
        {
            "temperature_summary": (91.0, 96.0, 101.0),
            "status": "alert",
            "message": "pressure spike stop",
            "spectrum": (0.0, 0.0, 0.2, 0.8),
            "curve": (1.0, 6.5, 1.0, 6.0, 1.0),
        },
    ]


def requires_native(label, call):
    try:
        call()
    except StrategyUnavailableError:
        print(f"{label}: requires native C++ binding")
    else:
        raise AssertionError(f"{label} should require a native binding")


def main():
    names = ["stable-flow", "warmup-drift", "warmup-alert", "cooldown", "spike-stop"]
    records = make_records()
    query = {
        "temperature_summary": (72.2, 73.1, 75.1),
        "status": "warmup",
        "message": "valve drift warning",
        "spectrum": (0.1, 0.3, 0.4, 0.2),
        "curve": (1.0, 1.2, 1.5, 1.4, 1.3),
    }

    space = Space(records, metric=mixed_record_distance, name="mixed_structured_records")
    matrix = space.to_matrix()

    # Adapter surface: explicit distances and a runtime-policy view.
    print("records =", ", ".join(names))
    print("matrix rows =", len(pairwise_distance_matrix(records, mixed_record_distance)))
    explanation = field_contributions(query, records[1])
    print("warmup-drift status penalty =", explanation["status"])
    diagnostics = space.runtime_diagnostics(
        representation=matrix,
        runtime=RuntimePolicy(exact=True, cache="materialized"),
        intent="mixed_record_demo",
    )
    assert diagnostics.representation == "matrix"
    print("runtime representation =", diagnostics.representation)

    neighbors = space.neighbors(query, count=2)
    assert [neighbor.id for neighbor in neighbors.neighbors] == [1, 2]
    print("neighbors(query) =", [names[neighbor.id] for neighbor in neighbors.neighbors])

    groups = space.groups(count=2, representation=matrix)
    assert groups.cluster_count == 2
    print("group medoids =", [names[index] for index in groups.medoids])

    outliers = space.outliers(strategy=DBSCAN(radius=2, min_points=2), representation=matrix)
    print("outliers =", [names[outlier.record_id] for outlier in outliers.outliers])

    # Native boundary: embedding lives in C++ but is not promoted here.
    requires_native("embed", lambda: space.embed(strategy=MDS(dimensions=2), representation=matrix))


if __name__ == "__main__":
    main()
