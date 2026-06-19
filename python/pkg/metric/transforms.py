"""Transform namespace for the revived Python API.

Transforms are currently a beta/compatibility surface. This module provides a
stable import location without requiring every core-wheel install to expose the
full historical transform bindings.
"""

STABILITY = "beta"


def _load_legacy_module():
    try:
        from metric import transform
    except (ImportError, ModuleNotFoundError):
        return None
    return transform


def available():
    """Return public transform names available in the installed wheel."""
    module = _load_legacy_module()
    if module is None:
        return tuple()
    return tuple(sorted(name for name in dir(module) if not name.startswith("_")))


def legacy_module():
    """Return the legacy transform module when the installed wheel provides it."""
    module = _load_legacy_module()
    if module is None:
        raise ImportError("metric.transform is not available in this wheel")
    return module


__all__ = ["STABILITY", "available", "legacy_module"]
