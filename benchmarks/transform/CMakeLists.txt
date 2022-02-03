add_executable(wavelet_benchmarks wavelet_benchmarks.cpp)
target_link_libraries(wavelet_benchmarks ${METRIC_TARGET_NAME} Catch2::Catch2)
