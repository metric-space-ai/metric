from metric import Space
from metric.operators import pairwise_distance_matrix


def structured_record_distance(lhs, rhs):
    status_penalty = 0.0 if lhs["status"] == rhs["status"] else 10.0
    temperature_penalty = abs(lhs["temperature_c"] - rhs["temperature_c"]) / 10.0
    event_penalty = padded_hamming(lhs["events"], rhs["events"])
    return status_penalty + temperature_penalty + event_penalty


def padded_hamming(lhs, rhs):
    width = max(len(lhs), len(rhs))
    padded_lhs = tuple(lhs) + (None,) * (width - len(lhs))
    padded_rhs = tuple(rhs) + (None,) * (width - len(rhs))
    return sum(left != right for left, right in zip(padded_lhs, padded_rhs))


def main():
    records = [
        {"id": "pump-a", "status": "ok", "temperature_c": 62.0, "events": ("start", "load", "idle")},
        {"id": "pump-b", "status": "ok", "temperature_c": 64.5, "events": ("start", "load", "idle")},
        {"id": "valve-c", "status": "warn", "temperature_c": 82.0, "events": ("start", "alarm", "manual")},
        {"id": "pump-d", "status": "ok", "temperature_c": 61.0, "events": ("start", "load", "stop")},
    ]
    query = {"status": "ok", "temperature_c": 63.0, "events": ("start", "load", "idle")}

    space = Space(records, structured_record_distance)
    nearest = space.nearest(query)

    print("nearest structured record =", nearest.record["id"], nearest.distance)
    print("distance(pump-a, valve-c) =", space.distance(0, 2))
    print("pairwise rows =", len(pairwise_distance_matrix(records, structured_record_distance)))


if __name__ == "__main__":
    main()
