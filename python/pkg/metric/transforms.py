"""Transform namespace for the revived Python API.

Transforms are currently a beta namespace. It is intentionally empty until
finite metric-space transform contracts are promoted.
"""

STABILITY = "beta"


def available():
    """Return public transform names available in the installed wheel."""
    return tuple()


__all__ = ["STABILITY", "available"]
