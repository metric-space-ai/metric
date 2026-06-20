"""Compatibility helpers for legacy Python METRIC import paths.

This module keeps compatibility discovery explicit and lazy. It does not import
heavy historical extension modules unless callers request them.
"""

from importlib import import_module


STABILITY = "compatibility"

LEGACY_MODULES = ("correlation", "distance", "mapping", "space", "transform")


def legacy_module(name):
    """Return a legacy ``metric.<name>`` module when it is available."""
    if name not in LEGACY_MODULES:
        raise ValueError(f"unknown legacy METRIC module: {name}")
    try:
        return import_module(f"metric.{name}")
    except (ImportError, ModuleNotFoundError) as exc:
        raise ImportError(f"metric.{name} is not available in this wheel") from exc


def available_modules():
    """Return legacy module names importable in the installed wheel."""
    available = []
    for name in LEGACY_MODULES:
        try:
            legacy_module(name)
        except ImportError:
            continue
        available.append(name)
    return tuple(available)


__all__ = ["LEGACY_MODULES", "STABILITY", "available_modules", "legacy_module"]
