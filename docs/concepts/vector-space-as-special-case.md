# Vector Space as a Special Case

Vector spaces are important, but they are not the foundation of METRIC. They are one kind of record domain where standard metrics such as Euclidean, Manhattan, cosine-derived, or other norm-induced distances may be appropriate.

METRIC is useful when the best metric is not naturally a vector norm:

- strings compared by edit operations
- distributions compared by transport cost
- time series compared by alignment cost
- images compared by structural or perceptual distance
- industrial records compared by domain-specific penalties

Embedding these records into vectors can be useful, but it is a modeling choice. METRIC keeps the original metric visible so that algorithms can operate on the geometry the domain actually defines.

## Relationship to Vector Search

Vector-search systems usually start after an embedding has already been produced. METRIC can operate before that step, after that step, or without it. A vector embedding can be treated as a record type with a vector metric, while a native metric can be used when it better preserves domain meaning.

## Practical Rule

Use vector methods when the vector representation is meaningful and stable. Use direct metric-space methods when the distance function is more trustworthy than an embedding.
