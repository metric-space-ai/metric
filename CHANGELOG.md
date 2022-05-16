# Changelog

## [0.2.1] - 2022-05-16

### Added
- PCFA average() and weights() getter methods and constructor that uses what they return, so as user in enabled to export and re-create the PCFA model [#324](https://github.com/panda-official/metric/pull/324)
- Minimal test for PCFA [#324](https://github.com/panda-official/metric/pull/324)
- Save and load example lines at `PCFA_example.cpp` [#324](https://github.com/panda-official/metric/pull/324)

### Changed
- Add build ability without LAPACK [#325](https://github.com/panda-official/metric/pull/325)
- Add link to cereal::cereal target [#325](https://github.com/panda-official/metric/pull/325)
- Change cereal version to 1.3.2 in CI [#325](https://github.com/panda-official/metric/pull/325)


## [0.2.0] - 2022-02-10

### Added
- Zero derivative padding with tests [#313](https://github.com/panda-official/metric/pull/313)
- Cmake `metric` target [#308](https://github.com/panda-official/metric/pull/308)
- Cmake variables for benchmarks, examples and tests [#319](https://github.com/panda-official/metric/pull/319)
- Cmake install instructions [#319](https://github.com/panda-official/metric/pull/319)
- `.clang-format`
- Changelog

### Changed
- Metric headers moved from `modules` to `metric` directory [#318](https://github.com/panda-official/metric/pull/318)
  [#319](https://github.com/panda-official/metric/pull/319)
- Move buildin dependencies (`blaze`, `json`, `cereal`) to external [#315](https://github.com/panda-official/metric/pull/315)
- Wavelet `dbwavf` refactor [#317](https://github.com/panda-official/metric/pull/317)
- Fix dozens spelling errors, layout errors, etc in `README.md` [#319](https://github.com/panda-official/metric/pull/319)
- Put in order CI workflows [#311](https://github.com/panda-official/metric/pull/311)
  [#312](https://github.com/panda-official/metric/pull/312)
- Reformat code according to `.clang-format`

### Removed
- `crossfilter`, `n-api`[#315](https://github.com/panda-official/metric/pull/315)
