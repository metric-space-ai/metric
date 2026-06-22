"""String edit space — native edit metric with exact search.

``Edit`` is a native C++ distance binding. The Python ``Space`` adapts the
string records and exposes explicit distances plus native exact-scan neighbor
search through that binding surface.
"""

from metric import Edit, Space


def main():
    records = ["cat", "cot", "coat", "dog"]
    space = Space(records, Edit())

    # Adapter surface: explicit edit distances via the native Edit binding.
    print("distance(cat, cot) =", space(0, 1))
    print("distance(cat, coat) =", space.distance(0, 2))

    neighbors = space.neighbors("cut", 2)
    assert [neighbor.record for neighbor in neighbors.neighbors] == ["cat", "cot"]
    print("neighbors(cut) =", [neighbor.record for neighbor in neighbors.neighbors])


if __name__ == "__main__":
    main()
