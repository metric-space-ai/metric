from math import sqrt

from metric import Space


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
    neighbors = space.neighbors(query, count=2)
    assert [names[index] for index, _distance in neighbors] == ["warmup-drift", "warmup-alert"]

    explanation = field_contributions(query, records[neighbors[0][0]])
    assert explanation["status"] == 0.0
    assert explanation["curve"] < 0.01

    groups = space.groups(count=2, representation=matrix)
    assert groups.cluster_count == 2
    assert groups.algorithm == "kmedoids"

    outliers = space.outliers(count=1, representation=matrix)
    assert names[outliers.outliers[0].record_id] == "spike-stop"

    embedding = space.embed(dimensions=2, representation=matrix)
    assert embedding.dimensions == 2
    assert embedding.diagnostics.finite_coordinates

    diagnostics = space.runtime_diagnostics(representation=matrix, intent="mixed_record_demo")
    assert diagnostics.representation == "matrix"

    print("nearest mixed records =", names[neighbors[0][0]], names[neighbors[1][0]])
    print("mixed record groups =", groups.cluster_count)
    print("mixed record outlier =", names[outliers.outliers[0].record_id])


if __name__ == "__main__":
    main()
