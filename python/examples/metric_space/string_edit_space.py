"""String edit space — adapter boundary demo.

``Edit`` is a native C++ distance binding. The Python ``Space`` adapts the
string records and exposes explicit distances through that binding. Neighbor
search is a METRIC algorithm and is reached through a native binding, so the
Python facade raises StrategyUnavailableError until it is exposed.
"""

from metric import Edit, Space
from metric.exceptions import StrategyUnavailableError


def main():
    records = ["cat", "cot", "coat", "dog"]
    space = Space(records, Edit())

    # Adapter surface: explicit edit distances via the native Edit binding.
    print("distance(cat, cot) =", space(0, 1))
    print("distance(cat, coat) =", space.distance(0, 2))

    # Native boundary: neighbor search requires the native C++ binding.
    try:
        space.neighbors("cut", 2)
    except StrategyUnavailableError as exc:
        print("neighbors:", str(exc).split(". ")[0])
    else:
        raise AssertionError("neighbors should require a native binding")


if __name__ == "__main__":
    main()
