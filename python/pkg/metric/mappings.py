"""Mapping namespace for the revived Python API.

Mapping algorithms are currently a beta/compatibility surface. This module
provides a stable import location without forcing the core wheel to import the
broader legacy compiled mapping bindings.
"""

STABILITY = "beta"


def _load_legacy_module():
    try:
        from metric import mapping
    except (ImportError, ModuleNotFoundError):
        return None
    return mapping


def available():
    """Return public mapping names available in the installed wheel."""
    module = _load_legacy_module()
    if module is None:
        return tuple()
    return tuple(sorted(name for name in dir(module) if not name.startswith("_")))


def legacy_module():
    """Return the legacy mapping module when the installed wheel provides it."""
    module = _load_legacy_module()
    if module is None:
        raise ImportError("metric.mapping is not available in this wheel")
    return module


__all__ = ["STABILITY", "available", "legacy_module"]
