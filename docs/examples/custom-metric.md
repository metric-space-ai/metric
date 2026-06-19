# Custom Metric Example

METRIC can build a finite metric space from any record type and callable metric.

Core pattern:

```cpp
#include <metric/concepts.hpp>

struct PaddedHamming {
    auto operator()(const std::string &lhs, const std::string &rhs) const -> double;
};

std::vector<std::string> records = {"red", "reed", "road", "blue"};

auto distance = metric::make_metric<std::string>(PaddedHamming{});
auto space = metric::Space::from_records(records, distance);
```

The metric determines the geometry. No embedding step is required before nearest-neighbor search:

```cpp
auto nearest = space.neighbors(std::string("read"), 2);
```

The raw callable can also be used directly with the compatibility representation name:

```cpp
metric::Matrix<std::string, PaddedHamming> matrix(records);
```

The forthcoming engine facade will put intent names above these representations. The current promoted example uses the representation API because it is compiled in CI.
