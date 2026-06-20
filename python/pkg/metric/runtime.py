"""Runtime policy objects for the revived Python engine facade."""

from dataclasses import dataclass
import operator

from metric.exceptions import StrategyParameterError, StrategyUnavailableError


_CACHE_MODES = {"auto", "lazy", "materialized", "none"}
_REPRESENTATION_MODES = {"auto", "metric_space", "matrix", "tree", "graph"}


@dataclass(frozen=True)
class CachePolicy:
    """Representation cache preference for promoted Python intent helpers."""

    mode: str = "auto"

    def __post_init__(self):
        if not isinstance(self.mode, str):
            raise StrategyParameterError("cache mode must be a string")
        if self.mode not in _CACHE_MODES:
            raise StrategyParameterError(f"cache mode must be one of {sorted(_CACHE_MODES)!r}")


@dataclass(frozen=True)
class RuntimePolicy:
    """Execution policy for promoted Python intent helpers."""

    exact: bool = True
    parallel: bool = False
    cache: str | CachePolicy = "auto"
    representation: str = "auto"
    graph_count: int | None = None

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
    )


def lazy(policy=None):
    policy = runtime_policy(policy)
    return RuntimePolicy(
        exact=policy.exact,
        parallel=policy.parallel,
        cache="lazy",
        representation=policy.representation,
        graph_count=policy.graph_count,
    )


def parallel(policy=None):
    policy = runtime_policy(policy)
    return RuntimePolicy(
        exact=policy.exact,
        parallel=True,
        cache=policy.cache,
        representation=policy.representation,
        graph_count=policy.graph_count,
    )


def serial(policy=None):
    policy = runtime_policy(policy)
    return RuntimePolicy(
        exact=policy.exact,
        parallel=False,
        cache=policy.cache,
        representation=policy.representation,
        graph_count=policy.graph_count,
    )


def using_implicit(policy=None):
    policy = runtime_policy(policy)
    return RuntimePolicy(
        exact=policy.exact,
        parallel=policy.parallel,
        cache="lazy",
        representation="metric_space",
    )


def using_matrix(policy=None):
    policy = runtime_policy(policy)
    return RuntimePolicy(
        exact=policy.exact,
        parallel=policy.parallel,
        cache="materialized",
        representation="matrix",
    )


def using_tree(policy=None):
    policy = runtime_policy(policy)
    return RuntimePolicy(
        exact=policy.exact,
        parallel=policy.parallel,
        cache="materialized",
        representation="tree",
    )


def using_graph(count=None, policy=None):
    policy = runtime_policy(policy)
    return RuntimePolicy(
        exact=policy.exact,
        parallel=policy.parallel,
        cache="materialized",
        representation="graph",
        graph_count=count,
    )


__all__ = [
    "CachePolicy",
    "RuntimeDiagnostics",
    "RuntimePolicy",
    "approximate",
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
