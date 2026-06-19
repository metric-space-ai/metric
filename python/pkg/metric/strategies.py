"""Strategy objects for the revived Python engine facade."""

from dataclasses import dataclass


@dataclass(frozen=True)
class FarthestFirst:
    """Deterministic farthest-first representative-selection strategy."""

    seed_index: int = 0


__all__ = ["FarthestFirst"]
