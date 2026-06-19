from metric import Space


def absolute_distance(lhs, rhs):
    return abs(lhs - rhs)


def upper_triangle(matrix):
    values = []
    for row_index, row in enumerate(matrix):
        for column_index in range(row_index + 1, len(row)):
            values.append(row[column_index])
    return values


def pearson(lhs, rhs):
    lhs_mean = sum(lhs) / len(lhs)
    rhs_mean = sum(rhs) / len(rhs)
    numerator = sum((left - lhs_mean) * (right - rhs_mean) for left, right in zip(lhs, rhs))
    lhs_scale = sum((left - lhs_mean) ** 2 for left in lhs) ** 0.5
    rhs_scale = sum((right - rhs_mean) ** 2 for right in rhs) ** 0.5
    if lhs_scale == 0 or rhs_scale == 0:
        raise ValueError("distance profiles must have non-zero variance")
    return numerator / (lhs_scale * rhs_scale)


def main():
    process_records = [0, 1, 2, 3, 4, 5]
    quality_records = [0, 1, 4, 9, 16, 25]

    process_space = Space(process_records, absolute_distance)
    quality_space = Space(quality_records, absolute_distance)

    dependency = pearson(
        upper_triangle(process_space.to_matrix().pairwise_distances()),
        upper_triangle(quality_space.to_matrix().pairwise_distances()),
    )
    assert dependency > 0.8

    print("cross-space distance-profile correlation =", round(dependency, 3))


if __name__ == "__main__":
    main()
