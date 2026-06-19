from metric import Edit, Space


def main():
    records = ["cat", "cot", "coat", "dog"]
    space = Space(records, Edit())

    print("distance(cat, cot) =", space(0, 1))

    for record_id, distance in space.neighbors("cut", 2):
        print(f"{records[record_id]}: {distance}")


if __name__ == "__main__":
    main()
