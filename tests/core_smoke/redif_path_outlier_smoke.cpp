// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <cassert>
#include <cmath>
#include <numeric>
#include <string>
#include <vector>

#include "metric/engine.hpp"

namespace {

struct AbsoluteDistance {
	auto operator()(int lhs, int rhs) const -> int
	{
		const auto difference = lhs - rhs;
		return difference < 0 ? -difference : difference;
	}
};

struct StringLengthDistance {
	auto operator()(const std::string &lhs, const std::string &rhs) const -> double
	{
		return std::abs(static_cast<double>(lhs.size()) - static_cast<double>(rhs.size()));
	}
};

struct DiscreteDistance {
	auto operator()(int lhs, int rhs) const -> int { return lhs == rhs ? 0 : 1; }
};

auto close_to(double lhs, double rhs, double tolerance = 1e-9) -> bool
{
	return std::abs(lhs - rhs) <= tolerance;
}

auto mass(const std::vector<double> &measure) -> double
{
	return std::accumulate(measure.begin(), measure.end(), 0.0);
}

} // namespace

int main()
{
	mtrc::redif_options options;
	options.neighbors = 1;
	options.iterations = 4;
	options.euler_step = 0.25;
	options.adaptive_geometry = true;

	auto line = mtrc::make_space(std::vector<int>{0, 1, 2, 100}, AbsoluteDistance{});
	const auto paths = mtrc::redif_remove_noise(line, options);
	for (const auto &path : paths.paths) {
		assert(close_to(mass(path.terminal_measure()), 1.0));
		assert(path.transport_path_length >= 0.0);
	}

	const auto ranked = mtrc::redif_outliers(line, options);
	assert(ranked.operator_name == "find_outliers");
	assert(ranked.strategy == "redif_transport_path_length");
	assert(ranked.size() == line.size());
	assert(ranked[0].id == line.id(3));
	for (std::size_t i = 1; i < ranked.size(); ++i) {
		assert(ranked[i - 1].score >= ranked[i].score);
	}

	auto reversed = mtrc::make_space(std::vector<int>{100, 2, 1, 0}, AbsoluteDistance{});
	const auto reranked = mtrc::redif_outliers(reversed, options);
	assert(reranked[0].id == reversed.id(0));

	auto clique_like = mtrc::make_space(std::vector<int>{0, 1, 2, 3}, DiscreteDistance{});
	auto clique_options = options;
	clique_options.neighbors = 3;
	clique_options.adaptive_geometry = false;
	const auto clique_ranked = mtrc::redif_outliers(clique_like, clique_options);
	assert(clique_ranked.size() == clique_like.size());
	for (std::size_t i = 1; i < clique_ranked.size(); ++i) {
		assert(close_to(clique_ranked[i].score, clique_ranked[0].score));
	}

	auto bridge = mtrc::make_space(std::vector<int>{0, 1, 2, 6, 10, 11, 12}, AbsoluteDistance{});
	auto bridge_options = options;
	bridge_options.neighbors = 2;
	bridge_options.adaptive_geometry = false;
	const auto bridge_ranked = mtrc::redif_outliers(bridge, bridge_options);
	assert(bridge_ranked[0].id == bridge.id(3));

	// The path functional is defined over atom measures, so the records do not
	// need vector coordinates.
	auto words = mtrc::make_space(
		std::vector<std::string>{"a", "aa", "aaa", "aaaaaaaaaaaa"}, StringLengthDistance{});
	const auto word_ranked = mtrc::redif_outliers(words, options);
	assert(word_ranked[0].id == words.id(3));
	assert(word_ranked[0].score > word_ranked[1].score);

	return 0;
}
