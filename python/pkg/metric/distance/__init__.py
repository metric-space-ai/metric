try:
    from metric._impl.distance import *
except ModuleNotFoundError as exc:
    if exc.name in {"metric._impl", "metric._impl.distance"}:
        raise ImportError(
            "metric.distance requires the native C++ extension metric._impl.distance. "
            "Build or install the METRIC Python wheel; the Python package does not "
            "implement distance algorithms as a fallback."
        ) from exc
    raise

# Full legacy builds still expose standard vector metrics through a separate
# extension. Default builds export them directly from metric._impl.distance.
if "Euclidean" not in globals():
    try:
        from metric._impl.standards import *
    except ModuleNotFoundError:
        pass


def _install_compatibility_aliases(namespace):
    aliases = {
        "Minkowski": "P_norm",
        "ThresholdedEuclidean": "Euclidean_thresholded",
    }
    for alias, historical_name in aliases.items():
        if alias not in namespace and historical_name in namespace:
            namespace[alias] = namespace[historical_name]


_install_compatibility_aliases(globals())
