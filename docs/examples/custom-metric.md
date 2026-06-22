# Custom Metric Example

METRIC can build a finite metric space from any record type and callable metric.

Core pattern:

```cpp
#include <metric/concepts.hpp>

struct PaddedHamming {
    auto operator()(const std::string &lhs, const std::string &rhs) const -> double;
};

std::vector<std::string> records = {"red", "reed", "road", "blue"};

auto distance = mtrc::make_metric<std::string>(PaddedHamming{});
auto space = mtrc::Space::from_records(records, distance);
```

The metric determines the geometry. No embedding step is required before nearest-neighbor search:

```cpp
auto nearest = space.neighbors(std::string("read"), 2);
```

The raw callable can also be used directly with the compatibility representation name:

```cpp
mtrc::Matrix<std::string, PaddedHamming> matrix(records);
```

`mtrc::Space` already exposes intent names such as `neighbors` over the underlying representation, as the core pattern above shows. `mtrc::Matrix` is the lower-level representation name kept for direct representation access.
