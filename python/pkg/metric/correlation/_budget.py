import operator

from metric.exceptions import MetricComputationError, StrategyUnavailableError


def runtime_policy_or_none(runtime):
    if runtime is None:
        return None
    from metric.runtime import runtime_policy

    return runtime_policy(runtime)


def resolve_runtime_budget(policy, value, name):
    if value is not None or policy is None:
        return value
    return getattr(policy, name)


def effective_budgets(
    *,
    runtime=None,
    max_memory_bytes=None,
    max_distance_evaluations=None,
    max_dense_records=None,
):
    policy = runtime_policy_or_none(runtime)
    max_memory_bytes = resolve_runtime_budget(policy, max_memory_bytes, "max_memory_bytes")
    max_distance_evaluations = resolve_runtime_budget(
        policy,
        max_distance_evaluations,
        "max_distance_evaluations",
    )
    max_dense_records = resolve_runtime_budget(policy, max_dense_records, "max_dense_records")

    from metric.spaces import (
        _DEFAULT_MAX_DENSE_RECORDS,
        _DEFAULT_MAX_DISTANCE_EVALUATIONS,
        _DEFAULT_MAX_MEMORY_BYTES,
        _normalize_optional_budget,
    )

    return (
        policy,
        _normalize_optional_budget(max_memory_bytes, "max_memory_bytes", _DEFAULT_MAX_MEMORY_BYTES),
        _normalize_optional_budget(
            max_distance_evaluations,
            "max_distance_evaluations",
            _DEFAULT_MAX_DISTANCE_EVALUATIONS,
        ),
        _normalize_optional_budget(max_dense_records, "max_dense_records", _DEFAULT_MAX_DENSE_RECORDS),
    )


def budget_overrides(defaults, runtime, max_memory_bytes, max_distance_evaluations, max_dense_records):
    default_runtime, default_memory, default_distance, default_records = defaults
    return {
        "runtime": default_runtime if runtime is None else runtime,
        "max_memory_bytes": default_memory if max_memory_bytes is None else max_memory_bytes,
        "max_distance_evaluations": (
            default_distance if max_distance_evaluations is None else max_distance_evaluations
        ),
        "max_dense_records": default_records if max_dense_records is None else max_dense_records,
    }


def record_count_or_none(records):
    try:
        return operator.index(len(records))
    except TypeError:
        return None


def materialize_unsized_records(operation, records, max_dense_records):
    materialized = []
    for index, record in enumerate(records):
        if index >= max_dense_records:
            raise MetricComputationError(
                f"{operation} refused unsized iterator materialization before native calls: "
                f"records>{max_dense_records}, max_dense_records={max_dense_records}. "
                "Reason: the native adapter builds exact dense correlation/entropy work below the Space planner. "
                "Suggested fallback: pass a sized container, pass a bounded sample, "
                "use metric.Space APIs where available, or raise max_dense_records explicitly."
            )
        materialized.append(record)
    return materialized


def require_low_level_exact_budget(
    operation,
    *record_sets,
    runtime=None,
    max_memory_bytes=None,
    max_distance_evaluations=None,
    max_dense_records=None,
    distance_multiplier=1,
    memory_multiplier=1,
    dense_cell_bytes=32,
):
    policy, memory_budget, distance_budget, dense_record_budget = effective_budgets(
        runtime=runtime,
        max_memory_bytes=max_memory_bytes,
        max_distance_evaluations=max_distance_evaluations,
        max_dense_records=max_dense_records,
    )
    if policy is not None and not policy.exact:
        raise StrategyUnavailableError(
            f"{operation} has no promoted approximate strategy yet. "
            "Use RuntimePolicy(exact=True), use metric.Space correlation APIs, "
            "or raise exact low-level budgets explicitly."
        )

    checked_record_sets = []
    known_counts = []
    for records in record_sets:
        count = record_count_or_none(records)
        if count is None:
            records = materialize_unsized_records(operation, records, dense_record_budget)
            count = len(records)
        checked_record_sets.append(records)
        known_counts.append(count)

    record_count = max(known_counts)
    dense_cells = record_count * record_count
    estimated_distance_evaluations = dense_cells * distance_multiplier
    estimated_bytes = dense_cells * dense_cell_bytes * memory_multiplier
    if (
        record_count <= dense_record_budget
        and estimated_bytes <= memory_budget
        and estimated_distance_evaluations <= distance_budget
    ):
        return tuple(checked_record_sets)

    raise MetricComputationError(
        f"{operation} refused low-level exact correlation work before native calls: "
        f"records={record_count}, estimated_bytes={estimated_bytes}, "
        f"budget_bytes={memory_budget}, "
        f"estimated_distance_evaluations={estimated_distance_evaluations}, "
        f"distance_evaluation_budget={distance_budget}, "
        f"max_dense_records={dense_record_budget}. "
        "Reason: the native adapter builds exact dense correlation/entropy work below the Space planner. "
        "Suggested fallback: use metric.Space compare/correlate/entropy APIs where available, "
        "pass a bounded sample, or raise the low-level budgets explicitly."
    )
