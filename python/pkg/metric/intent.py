"""Semantic intent facade for the revived Python engine API."""

from .operators import (
    add_noise_space,
    compress_space,
    compare_spaces,
    correlate_spaces,
    density_filter_space,
    describe_structure,
    equalize_space,
    embed_space,
    find_groups,
    find_outliers,
    find_representatives,
    map_space,
    nearest_neighbors,
    range_neighbors,
    redif_dynamics,
    reduce_space,
    remove_noise_space,
)

add_noise = add_noise_space
find_neighbors = nearest_neighbors
within_radius = range_neighbors
compress = compress_space
compare = compare_spaces
correlate = correlate_spaces
density_filter = density_filter_space
describe = describe_structure
dynamics = redif_dynamics
equalize = equalize_space
embed = embed_space
groups = find_groups
map = map_space
outliers = find_outliers
reduce = reduce_space
remove_noise = remove_noise_space
representatives = find_representatives

__all__ = [
    "add_noise",
    "add_noise_space",
    "compress",
    "compress_space",
    "compare",
    "compare_spaces",
    "correlate",
    "correlate_spaces",
    "density_filter",
    "density_filter_space",
    "describe",
    "describe_structure",
    "dynamics",
    "equalize",
    "equalize_space",
    "embed",
    "embed_space",
    "find_groups",
    "find_neighbors",
    "find_outliers",
    "find_representatives",
    "groups",
    "map",
    "map_space",
    "outliers",
    "redif_dynamics",
    "reduce",
    "reduce_space",
    "remove_noise",
    "remove_noise_space",
    "representatives",
    "within_radius",
]
