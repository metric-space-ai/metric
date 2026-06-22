# Migration

New code uses the `mtrc` namespace and the Level-1 source layout.

## Include Paths

Use:

```cpp
#include <metric/metric/catalog.hpp>
#include <metric/engine.hpp>
#include <metric/record.hpp>
#include <metric/space.hpp>
```

Avoid older umbrella paths that describe the pre-revival layout.

## Namespace Direction

Use:

- `mtrc::record`
- `mtrc::space`
- `mtrc::metric`
- `mtrc::stats`
- `mtrc::modify`
- `mtrc::solve`
- `mtrc::numeric`

Do not add new public C++ code under generic algorithm folders. Put it in the
namespace that describes its finite metric-space role.

## Binding Rule

Python and other languages adapt user data to the native C++ implementation.
They must not implement real metric math, solvers, or algorithms outside C++.
