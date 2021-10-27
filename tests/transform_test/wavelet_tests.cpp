#include <catch2/catch.hpp>

#include "modules/transform/wavelet.hpp"

TEMPLATE_TEST_CASE("DaubechiesMat", "[transform][wavelet]", float)
{
  const auto dmat = wavelet::DaubechiesMat<TestType>(12, 6);
  std::cout << dmat << std::endl;

  const auto sumLow = blaze::sum(blaze::row(dmat, 0));

  for (size_t i = 0; i < dmat.rows() / 2; ++i) {
    REQUIRE(Approx(sumLow) == blaze::sum(blaze::row(dmat, i)));
  }
  const auto sumHigh = blaze::sum(blaze::row(dmat, 0));
  for (size_t i = dmat.rows() / 2; i < dmat.rows(); ++i) {
    REQUIRE(Approx(sumHigh) == blaze::sum(blaze::row(dmat, i)));
  }
}
