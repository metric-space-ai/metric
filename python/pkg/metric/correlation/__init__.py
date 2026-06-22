from metric.exceptions import OptionalDependencyError

try:
    from .entropy import Entropy
    from .mgc import MGC, MGC_direct
except ModuleNotFoundError as error:
    # The Entropy/MGC bindings (metric._impl.entropy / metric._impl.mgc) are part
    # of the FULL native build, which is OFF by default; the core wheel does not
    # ship them. Surface a clear, actionable OptionalDependencyError instead of a
    # bare ModuleNotFoundError so a user knows exactly what to enable.
    raise OptionalDependencyError(
        "metric.correlation (Entropy / MGC) requires the FULL native build, which the "
        "default core wheel does not include. Rebuild the extension with "
        "-DMETRIC_PYTHON_BUILD_FULL=ON (Boost available), or install a full wheel. "
        f"Underlying import error: {error}"
    ) from error

__all__ = ["Entropy", "MGC", "MGC_direct"]
