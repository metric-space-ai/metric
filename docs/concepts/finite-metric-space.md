# Finite Metric Spaces

METRIC treats a finite metric space as the central computer object: a finite set
of records plus one true metric that assigns a numeric value to every record
pair.

## Starting Point

METRIC uses the following starting point.

1. There are observations.
2. Observations are different and distinguishable.
3. Records are computer-side recordings of observations.
4. A record and the observation it records are not identical.
5. In the computer we only have records, and the record set under analysis is
   finite.
6. The difference and distinguishability of records is handled by assigning a
   set of possible relations to every record pair.
7. Records themselves need not be ordered, but their possible relations can be
   ordered.

Because the considered record set is finite and the relations for a record pair
are ordered, the relation set for a record pair has a smallest admissible
entry. The C++ rule that assigns this entry to every record pair is the metric.
The assigned value is the metric value of that record pair.

Cost, path length, distance, probability, noise, and compression are
interpretations or derived views. They are not the primitive object METRIC needs
in order to compute.

## Metric-Space Definition

For a finite record set `X = {x0, x1, ..., xn}`, a true metric is a function

```text
d: X x X -> R
```

that returns a finite numeric value for every pair and satisfies:

- non-negativity: `d(x, y) >= 0`
- identity: `d(x, x) = 0`
- symmetry: `d(x, y) = d(y, x)`
- triangle inequality: `d(x, z) <= d(x, y) + d(y, z)`

The pair `(X, d)` is a finite metric space.

## Interpretation

A metric value can be read in more than one way.

In a geometric interpretation, it is the length of a minimal path through an
imagined intermediate space. That intermediate space is an interpretation, not a
required input. Locally, valid metric geometry can often be approximated by
Euclidean coordinate patches; the required dimension and accuracy are execution
questions.

In a non-geometric interpretation, the metric value is the minimal admissible
recoding effort between two records. Edit distance, transport distance, time
alignment, and assignment-style metrics fit this view naturally.

Both interpretations are useful. Neither changes the C++ contract: records plus
an admitted metric define the finite metric space.

## Closed Form Is Not Required

A metric does not need a closed analytic formula. It needs a well-defined
numeric value for each record pair. That value may be computed by:

- a closed formula, such as Euclidean, Manhattan, or Chebyshev distance
- dynamic programming, such as edit distance or TWED-style sequence costs
- shortest paths or assignment algorithms
- optimal transport over an admitted ground metric
- another deterministic solver for the minimal admissible relation value

For algorithmic metrics, the optimal path itself does not have to be unique.
The metric value must be well defined for the declared domain and parameters.

## Continuous And Finite Views

The classical definition of a metric space allows any set, including infinite
or continuous sets such as coordinate spaces, manifolds, function spaces, or
measure spaces. METRIC works with the finite numeric version in software:

- `X` is the finite set of records currently represented in memory or through
  stable IDs.
- `d(a, b)` is a selected native C++ computation.
- graph, table, tree, local coordinate, and mapping forms are execution
  representations derived from `(X, d)`.

Vector spaces are included as a special case: records are coordinate records and
the metric is a coordinate metric. METRIC does not require the user to first
turn every record into a vector.

## Implicit Space, Explicit Representations

The usual representation is implicit: keep records and compute the metric value
when an operation needs it.

METRIC can also build explicit execution forms over the same source space:

- `mtrc::space::storage::LiveDistances`: lazy pairwise access
- `mtrc::space::storage::DistanceTable`: eager or lazy pairwise value table
- `mtrc::space::storage::GraphTopology` and `KnnGraphIndex`: sparse local
  structure
- `mtrc::space::storage::CoverTreeIndex`: exact metric index
- `mtrc::modify::map` outputs: derived spaces, including coordinate spaces when
  useful

These forms trade memory, update cost, and speed. They are not prerequisites for
the metric space to exist.

## C++ Example

```cpp
#include <metric/metric/catalog.hpp>
#include <metric/engine.hpp>

#include <string>
#include <vector>

int main()
{
    std::vector<std::string> records = {"cat", "cot", "coat", "dog"};

    auto space = mtrc::make_space(records, mtrc::Edit<char>{});
    auto nearest = mtrc::stats::search::find_neighbors(
        space,
        std::string("cut"),
        mtrc::count{2});

    return nearest.empty() ? 1 : 0;
}
```

This is a finite metric space over strings. The strings are not embedded into
vectors first; edit distance defines the metric.

## Python Binding Example

```python
from metric import Edit, Space

space = Space(["cat", "cot", "coat", "dog"], Edit())

print(space.distance(0, 1))
print(space.neighbors("cut", k=2))
```

The Python object adapts user data to the native engine surface. The
metric-space object and promoted operations belong to the C++ implementation.

## Why This Is Broader Than Vector Search

Vector-search libraries assume the record is already a vector, or that a
separate coordinate embedding has turned it into one. METRIC treats vectors as
one valid record type among many. When the domain has a better native metric
than an embedding distance, METRIC computes directly on that metric space.

Related concepts:

- [METRIC Vocabulary](metric-vocabulary.md)
- [Metric Spaces](metric-space.md)
- [Metrics as Recoding Costs](metrics-as-recoding-costs.md)
- [Vector Space as a Special Case](vector-space-as-special-case.md)
- [Explicit Representations](explicit-representations.md)
