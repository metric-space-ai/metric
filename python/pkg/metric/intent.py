"""Semantic intent facade for the revived Python engine API."""

from .operators import (
    compress_space,
    compare_spaces,
    correlate_spaces,
    denoise_space,
    describe_structure,
    embed_space,
    find_groups,
    find_outliers,
    find_representatives,
    map_space,
    nearest_neighbors,
    range_neighbors,
    reduce_space,
)

find_neighbors = nearest_neighbors
within_radius = range_neighbors
compress = compress_space
compare = compare_spaces
correlate = correlate_spaces
denoise = denoise_space
describe = describe_structure
embed = embed_space
groups = find_groups
map = map_space
outliers = find_outliers
reduce = reduce_space
representatives = find_representatives

__all__ = [
    "compress",
    "compress_space",
    "compare",
    "compare_spaces",
    "correlate",
    "correlate_spaces",
    "denoise",
    "denoise_space",
    "describe",
    "describe_structure",
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
    "reduce",
    "reduce_space",
    "representatives",
    "within_radius",
]
