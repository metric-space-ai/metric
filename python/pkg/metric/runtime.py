"""Runtime policy objects for the revived Python engine facade."""

from dataclasses import dataclass
import operator

from metric.exceptions import OptionalDependencyError, StrategyParameterError, StrategyUnavailableError


_CACHE_MODES = {"auto", "lazy", "materialized", "none"}
_REPRESENTATION_MODES = {"auto", "metric_space", "matrix", "tree", "graph"}


def _normalize_optional_budget(value, name):
    if value is None:
        return None
    if isinstance(value, bool):
        raise StrategyParameterError(f"{name} must be a non-negative integer")
    try:
        value = operator.index(value)
    except TypeError:
        raise StrategyParameterError(f"{name} must be a non-negative integer") from None
    if value < 0:
        raise StrategyParameterError(f"{name} must be non-negative")
    return value


@dataclass(frozen=True)
class CachePolicy:
    """Representation cache preference for promoted Python intent helpers."""

    mode: str = "auto"

    def __post_init__(self):
        if not isinstance(self.mode, str):
            raise StrategyParameterError("cache mode must be a string")
        if self.mode not in _CACHE_MODES:
            raise StrategyParameterError(f"cache mode must be one of {sorted(_CACHE_MODES)!r}")

    def to_dict(self):
        return {"mode": self.mode}


@dataclass(frozen=True)
class RuntimePolicy:
    """Execution policy for promoted Python intent helpers."""

    exact: bool = True
    parallel: bool = False
    cache: str | CachePolicy = "auto"
    representation: str = "auto"
    graph_count: int | None = None
    max_memory_bytes: int | None = None
    max_distance_evaluations: int | None = None
    max_dense_records: int | None = None

    def __post_init__(self):
        if not isinstance(self.exact, bool):
            raise StrategyParameterError("exact must be a bool")
        if not isinstance(self.parallel, bool):
            raise StrategyParameterError("parallel must be a bool")
        cache = self.cache if isinstance(self.cache, CachePolicy) else CachePolicy(self.cache)
        if not isinstance(self.representation, str):
            raise StrategyParameterError("representation must be a string")
        if self.representation not in _REPRESENTATION_MODES:
            raise StrategyParameterError(
                f"representation must be one of {sorted(_REPRESENTATION_MODES)!r}"
            )
        graph_count = self.graph_count
        if graph_count is not None:
            try:
                graph_count = operator.index(graph_count)
            except TypeError:
                raise StrategyParameterError("graph_count must be an integer") from None
            if graph_count < 0:
                raise StrategyParameterError("graph_count must be non-negative")
        object.__setattr__(self, "cache", cache)
        object.__setattr__(self, "graph_count", graph_count)
        object.__setattr__(
            self,
            "max_memory_bytes",
            _normalize_optional_budget(self.max_memory_bytes, "max_memory_bytes"),
        )
        object.__setattr__(
            self,
            "max_distance_evaluations",
            _normalize_optional_budget(
                self.max_distance_evaluations,
                "max_distance_evaluations",
            ),
        )
        object.__setattr__(
            self,
            "max_dense_records",
            _normalize_optional_budget(self.max_dense_records, "max_dense_records"),
        )

    @property
    def cache_mode(self):
        return self.cache.mode

    @property
    def representation_preference(self):
        if self.representation == "matrix":
            return "matrix"
        if self.representation == "tree":
            return "exact_tree_index"
        if self.representation == "graph":
            return "exact_knn_graph"
        if self.representation == "metric_space":
            return "metric_space"
        return "matrix" if self.cache_mode == "materialized" else "metric_space"

    @property
    def name(self):
        accuracy = "exact" if self.exact else "approximate"
        execution = "parallel" if self.parallel else "serial"
        return f"{accuracy}_{self.cache_mode}_{execution}"

    def to_dict(self):
        return {
            "name": self.name,
            "exact": self.exact,
            "parallel": self.parallel,
            "cache": self.cache.to_dict(),
            "cache_mode": self.cache_mode,
            "representation": self.representation,
            "representation_preference": self.representation_preference,
            "graph_count": self.graph_count,
            "max_memory_bytes": self.max_memory_bytes,
            "max_distance_evaluations": self.max_distance_evaluations,
            "max_dense_records": self.max_dense_records,
        }


@dataclass(frozen=True)
class RuntimeDiagnostics:
    """Inspectable runtime policy and representation decision metadata."""

    policy_name: str
    exact: bool
    parallel: bool
    cache_mode: str
    representation: str
    intent: str | None = None
    supported: bool = True
    reason: str = ""
    max_memory_bytes: int | None = None
    max_distance_evaluations: int | None = None
    max_dense_records: int | None = None

    def to_dict(self):
        return {
            "policy_name": self.policy_name,
            "exact": self.exact,
            "parallel": self.parallel,
            "cache_mode": self.cache_mode,
            "representation": self.representation,
            "intent": self.intent,
            "supported": self.supported,
            "reason": self.reason,
            "max_memory_bytes": self.max_memory_bytes,
            "max_distance_evaluations": self.max_distance_evaluations,
            "max_dense_records": self.max_dense_records,
        }

    def to_pandas(self):
        try:
            import pandas as pd
        except ModuleNotFoundError:
            raise OptionalDependencyError(
                "RuntimeDiagnostics.to_pandas() requires pandas. Install pandas or use to_dict()."
            ) from None

        return pd.DataFrame.from_records([self.to_dict()])


def runtime_policy(runtime=None):
    """Return a RuntimePolicy from None, a RuntimePolicy, or a mapping."""

    if runtime is None:
        return RuntimePolicy()
    if isinstance(runtime, RuntimePolicy):
        return runtime
    if isinstance(runtime, dict):
        return RuntimePolicy(**runtime)
    raise TypeError("runtime must be a metric.runtime.RuntimePolicy")


def require_exact_runtime(runtime=None):
    """Normalize a runtime policy and reject unsupported approximate execution."""

    policy = runtime_policy(runtime)
    if not policy.exact:
        raise StrategyUnavailableError(
            "The promoted Python facade currently supports exact runtime policies only. "
            "Use RuntimePolicy(exact=True) or omit runtime=."
        )
    return policy


def runtime_diagnostics(runtime=None, *, representation=None, intent=None):
    """Describe the normalized runtime policy and chosen representation."""

    policy = runtime_policy(runtime)
    supported = policy.exact
    reason = ""
    if not supported:
        reason = "approximate runtime policies are not promoted in the Python facade yet"
    return RuntimeDiagnostics(
        policy_name=policy.name,
        exact=policy.exact,
        parallel=policy.parallel,
        cache_mode=policy.cache_mode,
        representation=(
            policy.representation_preference if representation is None else representation
        ),
        intent=intent,
        supported=supported,
        reason=reason,
        max_memory_bytes=policy.max_memory_bytes,
        max_distance_evaluations=policy.max_distance_evaluations,
        max_dense_records=policy.max_dense_records,
    )


def exact(cache="auto", parallel=False):
    return RuntimePolicy(exact=True, parallel=parallel, cache=cache)


def approximate(cache="auto", parallel=False):
    return RuntimePolicy(exact=False, parallel=parallel, cache=cache)


def materialized(policy=None):
    policy = runtime_policy(policy)
    return RuntimePolicy(
        exact=policy.exact,
        parallel=policy.parallel,
        cache="materialized",
        representation=policy.representation,
        graph_count=policy.graph_count,
        max_memory_bytes=policy.max_memory_bytes,
        max_distance_evaluations=policy.max_distance_evaluations,
        max_dense_records=policy.max_dense_records,
    )


def lazy(policy=None):
    policy = runtime_policy(policy)
    return RuntimePolicy(
        exact=policy.exact,
        parallel=policy.parallel,
        cache="lazy",
        representation=policy.representation,
        graph_count=policy.graph_count,
        max_memory_bytes=policy.max_memory_bytes,
        max_distance_evaluations=policy.max_distance_evaluations,
        max_dense_records=policy.max_dense_records,
    )


def parallel(policy=None):
    policy = runtime_policy(policy)
    return RuntimePolicy(
        exact=policy.exact,
        parallel=True,
        cache=policy.cache,
        representation=policy.representation,
        graph_count=policy.graph_count,
        max_memory_bytes=policy.max_memory_bytes,
        max_distance_evaluations=policy.max_distance_evaluations,
        max_dense_records=policy.max_dense_records,
    )


def serial(policy=None):
    policy = runtime_policy(policy)
    return RuntimePolicy(
        exact=policy.exact,
        parallel=False,
        cache=policy.cache,
        representation=policy.representation,
        graph_count=policy.graph_count,
        max_memory_bytes=policy.max_memory_bytes,
        max_distance_evaluations=policy.max_distance_evaluations,
        max_dense_records=policy.max_dense_records,
    )


def using_implicit(policy=None):
    policy = runtime_policy(policy)
    return RuntimePolicy(
        exact=policy.exact,
        parallel=policy.parallel,
        cache="lazy",
        representation="metric_space",
        graph_count=policy.graph_count,
        max_memory_bytes=policy.max_memory_bytes,
        max_distance_evaluations=policy.max_distance_evaluations,
        max_dense_records=policy.max_dense_records,
    )


def using_matrix(policy=None):
    policy = runtime_policy(policy)
    return RuntimePolicy(
        exact=policy.exact,
        parallel=policy.parallel,
        cache="materialized",
        representation="matrix",
        graph_count=policy.graph_count,
        max_memory_bytes=policy.max_memory_bytes,
        max_distance_evaluations=policy.max_distance_evaluations,
        max_dense_records=policy.max_dense_records,
    )


def using_tree(policy=None):
    policy = runtime_policy(policy)
    return RuntimePolicy(
        exact=policy.exact,
        parallel=policy.parallel,
        cache="materialized",
        representation="tree",
        graph_count=policy.graph_count,
        max_memory_bytes=policy.max_memory_bytes,
        max_distance_evaluations=policy.max_distance_evaluations,
        max_dense_records=policy.max_dense_records,
    )


def using_graph(count=None, policy=None):
    policy = runtime_policy(policy)
    return RuntimePolicy(
        exact=policy.exact,
        parallel=policy.parallel,
        cache="materialized",
        representation="graph",
        graph_count=count,
        max_memory_bytes=policy.max_memory_bytes,
        max_distance_evaluations=policy.max_distance_evaluations,
        max_dense_records=policy.max_dense_records,
    )


def _native_core_available():
    """True iff the native ``metric._impl.metric`` extension imports."""
    try:
        from metric._impl import metric as _native  # noqa: F401
    except (ImportError, ModuleNotFoundError):
        return False
    return True


def _distance_metrics_available():
    """True iff at least one distance-metric constructor is exposed."""
    try:
        from metric import metrics
    except (ImportError, ModuleNotFoundError):
        return False
    try:
        return len(metrics.available()) > 0
    except Exception:
        return False


def _correlation_package_available():
    """True iff ``import metric.correlation`` succeeds without raising.

    A parallel task makes this import raise ``OptionalDependencyError`` when the
    native correlation extension is missing; today it raises
    ``ModuleNotFoundError``. Either way the probe reports ``False`` rather than
    leaking the underlying error.
    """
    try:
        import metric.correlation  # noqa: F401
    except Exception:
        return False
    return True


def capabilities():
    """Report the installed wheel's actual metric-space capabilities.

    This is factual, installed-capability introspection: every flag is computed
    by probing the live native bindings and public adapter state at call time,
    not from a hardcoded snapshot. The returned mapping has stable string keys
    and ``bool`` values. ``True`` means the path is reachable in this build;
    ``False`` means it is not promoted (or its dependency is absent) here.

    No optional or missing module is imported in a way that surfaces a raw
    ``ModuleNotFoundError``: every probe swallows import failures and reports
    ``False`` instead.
    """
    from metric.operators import native_binding_available

    return {
        # The native C++ extension itself.
        "native_core": _native_core_available(),
        # At least one distance-metric constructor (Edit is guaranteed).
        "distance_metrics": _distance_metrics_available(),
        # Exact-scan neighbor search.
        "neighbors": native_binding_available("exact_scan_neighbors"),
        # Explicit all-pairs distance matrix.
        "pairwise": native_binding_available("pairwise_distance_matrix"),
        # Farthest-first representative selection.
        "representatives": native_binding_available("representative_indices"),
        # Representative-based reduction/compression (reuses representative
        # selection plus native assignment).
        "reduce_compress": (
            native_binding_available("representative_indices")
            and native_binding_available("assign_to_representatives")
        ),
        # Exact finite-space structure description.
        "structure": native_binding_available("describe_structure"),
        # Clustering grouping (k-medoids / DBSCAN).
        "groups": (
            native_binding_available("kmedoids")
            and native_binding_available("dbscan")
        ),
        # Singular-record scoring (nearest-neighbor isolation / DBSCAN density-unassigned records).
        "outliers": (
            native_binding_available("nearest_neighbor_outliers")
            and native_binding_available("dbscan_outliers")
        ),
        # DBSCAN density filtering into a derived finite metric space.
        "density_filter": native_binding_available("dbscan"),
        # Metric-space embedding (never promoted in this batch).
        "embed": native_binding_available("embed"),
        # Cross-space distance-profile comparison/correlation. The native
        # binding promoted for compare_spaces/correlate_spaces is exposed as
        # metric._impl.metric.distance_profile_correlation.
        "compare_correlate": native_binding_available("distance_profile_correlation"),
        # The optional native correlation package.
        "correlation_package": _correlation_package_available(),
    }


def available():
    """Alias for :func:`capabilities` exposed as top-level ``metric.available``."""
    return capabilities()


__all__ = [
    "CachePolicy",
    "RuntimeDiagnostics",
    "RuntimePolicy",
    "approximate",
    "available",
    "capabilities",
    "exact",
    "lazy",
    "materialized",
    "parallel",
    "require_exact_runtime",
    "runtime_diagnostics",
    "runtime_policy",
    "serial",
    "using_graph",
    "using_implicit",
    "using_matrix",
    "using_tree",
]
