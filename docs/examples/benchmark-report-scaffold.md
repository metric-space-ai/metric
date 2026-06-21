# Benchmark Report Scaffold

METRIC now has a small native C++ scaffold for benchmark reports:
`metric::benchmarks::BenchmarkReport`.

The scaffold does not run benchmarks by itself. It gives CI-safe examples,
smoke tests, and future benchmark jobs a common Markdown shape for two kinds of
evidence:

- run metadata: suite, source, build profile, platform, artifact name, and notes
- representation cost: representation name, record count, distance evaluations,
  cached distances, dense slots, memory estimate, exactness, materialization
- hero quality: query count, metric hits, vector-baseline misses, optional
  latent hits, average metric margin, and named diagnostics

Minimal shape:

```cpp
#include <metric/engine.hpp>

metric::benchmarks::BenchmarkReport report("METRIC Engine Benchmark Report");
report.set_run_metadata({"engine hero baseline suite",
                         "examples/engine/benchmark_report.cpp",
                         "core preset",
                         "portable deterministic fixture",
                         "docs/examples/engine-benchmark-report.md",
                         "not a timing benchmark"});

report.add_representation_cost(metric::benchmarks::representation_cost_row(
    "process gallery matrix cache",
    matrix_cache.diagnostics(),
    "eager all-pairs materialization"));

metric::benchmarks::HeroQualityRow hero;
hero.benchmark = "process curve PHATE gallery";
hero.query_count = 6;
hero.metric_hits = 6;
hero.vector_misses = 6;
hero.latent_hits = 6;
hero.reports_latent_hits = true;
hero.average_metric_margin = 2.16667;
hero.reports_average_metric_margin = true;
hero.diagnostics = "OOS rank and distance penalty";
report.add_hero_quality(hero);

std::string markdown = report.to_markdown();
```

The core smoke gate
`benchmark_report_smoke` verifies that this Markdown includes run metadata,
representation-cost, and hero-quality sections. Benchmark jobs write the
rendered Markdown as a non-blocking artifact while keeping ordinary correctness
CI focused on deterministic smoke assertions.

The first promoted generator is
`build/core/examples/engine/engine_benchmark_report`; its checked-in artifact is
[Engine Benchmark Report](engine-benchmark-report.md). It currently consolidates
native string, histogram, process-curve PHATE gallery, mixed-record,
distribution/image recoding, and cross-space MGC hero evidence.
The manual `benchmark.yml` workflow runs the same generator with
`METRIC_BENCHMARK_REPORT_*` environment variables so archived artifacts carry
runner/build/commit metadata.
