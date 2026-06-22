# Benchmark Report Scaffold

METRIC now has a small native C++ scaffold for benchmark reports:
`mtrc::benchmarks::BenchmarkReport`.

The scaffold does not run benchmarks by itself. It gives CI-safe examples,
smoke tests, and future benchmark jobs a common Markdown shape for four kinds of
evidence:

- run metadata: suite, source, build profile, platform, artifact name, and notes
- representation cost: representation name, record count, distance evaluations,
  cached distances, dense slots, memory estimate, exactness, materialization
- application workflow evidence: query count, metric-space matches, vector-comparison mismatches, optional
  latent hits, average metric margin, and named diagnostics
- performance evidence (work reduction): workload, record count, shared operator
  passes, naive vs materialized distance evaluations, cache-miss evaluations,
  evaluation reduction factor, and an exact-match flag

Minimal shape:

```cpp
#include <metric/engine.hpp>

mtrc::benchmarks::BenchmarkReport report("METRIC Engine Benchmark Report");
report.set_run_metadata({"engine application workflow suite",
                         "examples/engine/benchmark_report.cpp",
                         "core preset",
                         "portable deterministic fixture",
                         "docs/examples/engine-benchmark-report.md",
                         "not a timing benchmark"});

report.add_representation_cost(mtrc::benchmarks::representation_cost_row(
    "process gallery distance table",
    distance_table.diagnostics(),
    "eager all-pairs materialization"));

mtrc::benchmarks::WorkflowEvidenceRow workflow;
workflow.benchmark = "process curve PHATE gallery";
workflow.query_count = 6;
workflow.metric_matches = 6;
workflow.vector_mismatches = 6;
workflow.latent_matches = 6;
workflow.reports_latent_matches = true;
workflow.average_metric_margin = 2.16667;
workflow.reports_average_metric_margin = true;
workflow.diagnostics = "OOS rank and distance penalty";
report.add_workflow_evidence(workflow);

std::string markdown = report.to_markdown();
```

The core smoke gate
`benchmark_report_smoke` verifies that this Markdown includes run metadata,
representation-cost, application-workflow, and performance-evidence sections.
Benchmark jobs write the rendered Markdown as a non-blocking artifact while
keeping ordinary correctness CI focused on deterministic smoke assertions.

The performance-evidence section records deterministic *work reduction* through
amortization. It is measured with a shared-counter metric wrapper: a naive
pipeline re-evaluates the domain metric once per distance-consuming operator
pass over a hero gallery, while a materialized `DistanceTable` evaluates the
pairwise structure once and serves every later pass from cache with zero
additional metric evaluations. The counts are integers and reproducible across
platforms, so no wall-clock timing enters the checked-in artifact. Optional
indicative wall-clock timing is emitted out-of-band when
`METRIC_BENCHMARK_REPORT_TIMING=1` is set and is never asserted in CI.

The first promoted generator is
`build/core/examples/engine/engine_benchmark_report`; its checked-in artifact is
[Engine Benchmark Report](engine-benchmark-report.md). It currently consolidates
native string, histogram, process-curve PHATE gallery, mixed-record,
distribution/image recoding, cross-space MGC, and condition-monitoring
application workflow evidence, plus work-reduction performance evidence for the
mixed-record, condition-monitoring, cross-space, and PHATE gallery heroes. The
manual `benchmark.yml` workflow runs the same generator with
`METRIC_BENCHMARK_REPORT_*` environment variables so archived artifacts carry
runner/build/commit metadata.
