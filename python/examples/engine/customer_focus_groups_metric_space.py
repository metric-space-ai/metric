"""Customer focus groups as finite metric-space modification.

The point of this example is not to build a customer-ML pipeline. It shows the
finite metric-space route when rows contain mixed data and the representatives
must remain real records.
"""

from dataclasses import dataclass

from metric import Space
from metric.strategies import KMedoids, RadiusCoverage, UniformDensity


@dataclass(frozen=True)
class Customer:
    customer_id: str
    region: str
    plan: str
    monthly_revenue: float
    seats: int
    products: frozenset
    journey: tuple


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


def jaccard_distance(lhs, rhs):
    union = lhs | rhs
    if not union:
        return 0.0
    return 1.0 - (len(lhs & rhs) / len(union))


def customer_distance(lhs, rhs):
    """Weighted sum of finite-record metrics over heterogeneous fields."""
    revenue = abs(lhs.monthly_revenue - rhs.monthly_revenue) / 10000.0
    seats = abs(lhs.seats - rhs.seats) / 500.0
    region = 0.0 if lhs.region == rhs.region else 1.0
    plan = 0.0 if lhs.plan == rhs.plan else 1.0
    product = jaccard_distance(lhs.products, rhs.products)
    journey = edit_distance(lhs.journey, rhs.journey) / 5.0
    return (
        0.8 * revenue
        + 0.5 * seats
        + 0.4 * region
        + 1.0 * plan
        + 1.2 * product
        + 1.0 * journey
    )


def customer_records():
    return [
        Customer("acme", "us", "enterprise", 18000, 420, frozenset({"crm", "sso", "audit"}), ("trial", "security", "procurement", "expand")),
        Customer("globex", "us", "enterprise", 16500, 390, frozenset({"crm", "sso", "audit"}), ("trial", "security", "procurement", "expand")),
        Customer("initech", "eu", "enterprise", 14200, 310, frozenset({"crm", "sso"}), ("trial", "security", "legal", "expand")),
        Customer("umbrella", "eu", "enterprise", 15500, 340, frozenset({"crm", "sso", "data"}), ("trial", "security", "legal", "expand")),
        Customer("northwind", "us", "growth", 4200, 90, frozenset({"crm", "support"}), ("signup", "import", "team", "renew")),
        Customer("soylent", "us", "growth", 3900, 80, frozenset({"crm", "support"}), ("signup", "import", "team", "renew")),
        Customer("wonka", "eu", "growth", 4700, 100, frozenset({"crm", "billing"}), ("signup", "import", "billing", "renew")),
        Customer("stark", "apac", "startup", 950, 12, frozenset({"billing"}), ("signup", "card", "activate")),
        Customer("wayne", "us", "startup", 1100, 16, frozenset({"billing", "support"}), ("signup", "card", "activate")),
        Customer("hooli", "apac", "startup", 850, 10, frozenset({"billing"}), ("signup", "card", "activate")),
    ]


def metric_personas(space):
    compressed = space.compress(strategy=KMedoids(groups=3))
    personas = [
        {
            "persona_id": record.customer_id,
            "plan": record.plan,
            "region": record.region,
            "products": tuple(sorted(record.products)),
            "multiplicity": compressed.representative_multiplicities[position],
            "weight": compressed.representative_weights[position],
        }
        for position, record in enumerate(compressed.space.records)
    ]
    return compressed, personas


def numeric_projection(customer):
    """A deliberately lossy vector-first projection used as a contrast."""
    plan_order = {"startup": 0.0, "growth": 1.0, "enterprise": 2.0}
    return (
        customer.monthly_revenue / 10000.0,
        customer.seats / 500.0,
        plan_order[customer.plan],
    )


def vector_centroids(records, assignments, group_count):
    vectors = [numeric_projection(record) for record in records]
    centroids = []
    for group in range(group_count):
        members = [vector for vector, assignment in zip(vectors, assignments) if assignment == group]
        centroid = tuple(sum(values) / len(members) for values in zip(*members))
        centroids.append(centroid)
    return centroids


def subspace(records, predicate):
    selected = [record for record in records if predicate(record)]
    return Space(selected, metric=customer_distance, ids=[record.customer_id for record in selected])


def main():
    records = customer_records()
    space = Space(records, metric=customer_distance, ids=[record.customer_id for record in records])

    compressed, personas = metric_personas(space)
    assert compressed.strategy == "k_medoids"
    assert compressed.source_record_ids == tuple(persona["persona_id"] for persona in personas)
    assert abs(sum(compressed.representative_weights) - 1.0) < 1e-12
    assert "weighted metric-measure compression" in compressed.validity

    print("metric personas =", [persona["persona_id"] for persona in personas])
    print("persona weights =", [round(persona["weight"], 3) for persona in personas])

    enterprise = subspace(records, lambda customer: customer.plan == "enterprise")
    growth = subspace(records, lambda customer: customer.plan == "growth")
    startup = subspace(records, lambda customer: customer.plan == "startup")
    enterprise_persona = enterprise.compress(strategy=KMedoids(groups=1)).space.records[0]
    growth_persona = growth.compress(strategy=KMedoids(groups=1)).space.records[0]
    startup_persona = startup.compress(strategy=KMedoids(groups=1)).space.records[0]
    assert enterprise_persona.plan == "enterprise"
    assert growth_persona.plan == "growth"
    assert startup_persona.plan == "startup"
    print("split personas =", [enterprise_persona.customer_id, growth_persona.customer_id, startup_persona.customer_id])

    coverage = space.compress(strategy=RadiusCoverage(radius=0.9))
    assert coverage.compressed_record_count >= 3
    assert coverage.validity
    print("radius coverage panel =", list(coverage.source_record_ids))

    equalized = space.equalize(strategy=UniformDensity(radius=0.9))
    assert equalized.diagnostics["empirical_density_preserved"] is False
    print("uniform-density panel =", list(equalized.source_record_ids))

    centroids = vector_centroids(records, compressed.assignments, compressed.compressed_record_count)
    customer_vectors = {numeric_projection(record) for record in records}
    assert all(centroid not in customer_vectors for centroid in centroids)
    print("vector-first centers are synthetic =", [tuple(round(value, 3) for value in centroid) for centroid in centroids])


if __name__ == "__main__":
    main()
