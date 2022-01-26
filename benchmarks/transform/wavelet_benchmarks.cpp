#define CATCH_CONFIG_MAIN

#define CATCH_CONFIG_ENABLE_BENCHMARKING

#include <catch2/catch.hpp>

#include <metric/transform/wavelet.hpp>

using T = double;


TEST_CASE("Daubechies matrix construction", "[transform][wavelet]")
{
  BENCHMARK("Daubechies matrix construction")
  {
    return wavelet::DaubechiesMat<T>(48000, 2);
  };
}

