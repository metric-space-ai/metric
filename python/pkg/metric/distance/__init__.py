from metric._impl.distance import *
#from metric._impl.voi import *  # commented by MAx F due to removal of VOI
try:
    from metric._impl.standards import *
except ModuleNotFoundError:
    pass


def _install_compatibility_aliases(namespace):
    aliases = {
        "Manhattan": "Manhatten",
        "Minkowski": "P_norm",
        "ThresholdedEuclidean": "Euclidean_thresholded",
    }
    for alias, historical_name in aliases.items():
        if alias not in namespace and historical_name in namespace:
            namespace[alias] = namespace[historical_name]


_install_compatibility_aliases(globals())
