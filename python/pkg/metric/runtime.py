"""Runtime policy objects for the revived Python engine facade."""

from dataclasses import dataclass

from metric.exceptions import StrategyParameterError, StrategyUnavailableError


_CACHE_MODES = {"auto", "lazy", "materialized", "none"}


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

    def __post_init__(self):
        if not isinstance(self.exact, bool):
            raise StrategyParameterError("exact must be a bool")
        if not isinstance(self.parallel, bool):
            raise StrategyParameterError("parallel must be a bool")
        cache = self.cache if isinstance(self.cache, CachePolicy) else CachePolicy(self.cache)
        object.__setattr__(self, "cache", cache)

    @property
    def cache_mode(self):
        return self.cache.mode

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


def runtime_diagnostics(runtime=None, *, representation="metric_space", intent=None):
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
        representation="metric_space" if representation is None else representation,
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
    return RuntimePolicy(exact=policy.exact, parallel=policy.parallel, cache="materialized")


def lazy(policy=None):
    policy = runtime_policy(policy)
    return RuntimePolicy(exact=policy.exact, parallel=policy.parallel, cache="lazy")


def parallel(policy=None):
    policy = runtime_policy(policy)
    return RuntimePolicy(exact=policy.exact, parallel=True, cache=policy.cache)


def serial(policy=None):
    policy = runtime_policy(policy)
    return RuntimePolicy(exact=policy.exact, parallel=False, cache=policy.cache)


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
]
