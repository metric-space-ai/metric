"""Public exception hierarchy for the revived Python API."""


class MetricError(Exception):
    """Base class for public METRIC Python errors."""


class MissingMetricError(MetricError):
    """Raised when a metric-space operation needs an explicit metric."""


class MetricInputError(MetricError):
    """Raised when records or operation inputs are invalid."""


class MetricContractError(MetricError):
    """Raised when a metric callable violates metric-space contracts."""


class MetricComputationError(MetricError):
    """Raised when a metric-space computation cannot be completed."""


class AmbiguousIntentError(MetricError):
    """Raised when a semantic intent call is ambiguous."""


class IncompatibleSpaceError(MetricError):
    """Raised when operations receive incompatible metric spaces."""


class StrategyError(MetricError):
    """Base class for strategy selection and execution errors."""


class StrategyUnavailableError(StrategyError):
    """Raised when a requested strategy is unavailable."""


class StrategyParameterError(StrategyError):
    """Raised when strategy parameters are invalid."""


class RepresentationError(MetricError):
    """Base class for representation-specific errors."""


class StaleRepresentationError(RepresentationError):
    """Raised when a representation no longer matches its source space."""


class OptionalDependencyError(MetricError, ImportError):
    """Raised when an optional dependency is required but unavailable.

    It is also an :class:`ImportError` because it is raised exclusively for
    unavailable optional imports/native bindings (e.g. the correlation adapter
    boundary or a missing ``numpy``). This lets import-aware compatibility
    discovery (``metric.compat``) treat an unavailable optional binding the
    same way it treats any other unimportable legacy module, while keeping the
    public ``MetricError`` hierarchy intact.
    """


class NotFittedError(MetricError):
    """Raised when a model is used before fitting."""


class UnsupportedOperationError(MetricError):
    """Raised when an operation is explicit but unsupported by the result."""


__all__ = [
    "AmbiguousIntentError",
    "IncompatibleSpaceError",
    "MetricComputationError",
    "MetricContractError",
    "MetricError",
    "MetricInputError",
    "MissingMetricError",
    "NotFittedError",
    "OptionalDependencyError",
    "RepresentationError",
    "StaleRepresentationError",
    "StrategyError",
    "StrategyParameterError",
    "StrategyUnavailableError",
    "UnsupportedOperationError",
]
