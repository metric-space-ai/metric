from metric import Edit, Space


def main():
    records = ["cat", "cot", "coat", "dog"]
    space = Space(records, Edit())

    print("distance(cat, cot) =", space(0, 1))

    for neighbor in space.neighbors("cut", 2).neighbors:
        print(f"{neighbor.record}: {neighbor.distance}")


if __name__ == "__main__":
    main()
