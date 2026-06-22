"""Correlation adapter boundary.

``metric.correlation`` exposes ``Entropy`` and ``MGC``/``MGC_direct`` as thin
adapters over the native correlation bindings (``metric._impl.entropy`` and
``metric._impl.mgc``). Those bindings are not promoted in the default core
wheel. When they are absent, importing this package raises
``metric.OptionalDependencyError`` (naming the missing native binding) rather
than leaking a raw ``ModuleNotFoundError``. This is an adapter-only boundary: it
promotes no correlation algorithm.
"""

from .entropy import Entropy
from .mgc import MGC, MGC_direct

__all__ = ["Entropy", "MGC", "MGC_direct"]
