#include <algorithm>
#include <cassert>
#include <cstddef>
#include <string>
#include <type_traits>
#include <vector>

#include "metric/concepts.hpp"
#include "metric/distance.hpp"

struct PaddedHamming {
    auto operator()(const std::string &lhs, const std::string &rhs) const -> std::size_t
    {
        const auto limit = std::max(lhs.size(), rhs.size());
        std::size_t distance = 0;

        for (std::size_t i = 0; i < limit; ++i) {
            const char left = i < lhs.size() ? lhs[i] : '\0';
            const char right = i < rhs.size() ? rhs[i] : '\0';
            if (left != right) {
                ++distance;
            }
        }

        return distance;
    }
};

struct DirectedIntegerDistance {
    auto operator()(int lhs, int rhs) const -> int
    {
        if (lhs <= rhs) {
            return rhs - lhs;
        }
        return 2 * (lhs - rhs);
    }
};

template <typename Record, typename Metric>
void assert_metric_contracts(const std::vector<Record> &records, Metric metric)
{
    for (const auto &lhs : records) {
        assert(metric(lhs, lhs) == 0);

        for (const auto &rhs : records) {
            const auto lhs_rhs = metric(lhs, rhs);
            const auto rhs_lhs = metric(rhs, lhs);

            assert(lhs_rhs >= 0);
            assert(lhs_rhs == rhs_lhs);

            for (const auto &through : records) {
                assert(metric(lhs, through) <= lhs_rhs + metric(rhs, through));
            }
        }
    }
}

template <typename Record, typename Metric>
void assert_declared_metric_contracts(const std::vector<Record> &records, Metric metric)
{
    for (const auto &lhs : records) {
        assert(metric(lhs, lhs) == 0);

        for (const auto &rhs : records) {
            const auto lhs_rhs = metric(lhs, rhs);
            assert(lhs_rhs >= 0);

            if constexpr (metric::metric_traits<Metric>::law == metric::metric_law::metric ||
                          metric::metric_traits<Metric>::law == metric::metric_law::pseudo_metric) {
                assert(lhs_rhs == metric(rhs, lhs));

                for (const auto &through : records) {
                    assert(metric(lhs, through) <= lhs_rhs + metric(rhs, through));
                }
            }

            if constexpr (metric::metric_traits<Metric>::law == metric::metric_law::metric) {
                if (lhs_rhs == 0) {
                    assert(lhs == rhs);
                }
            }
        }
    }
}

int main()
{
    static_assert(std::is_same_v<metric::Manhattan<double>, metric::Manhatten<double>>);
    static_assert(std::is_same_v<metric::Manhattan_standardized<double>, metric::Manhatten_standardized<double>>);
    static_assert(metric::is_metric_callable_v<PaddedHamming, std::string>);
    static_assert(metric::metric_traits<metric::Euclidean<double>>::law == metric::metric_law::metric);
    static_assert(metric::metric_traits<metric::Euclidean<double>>::records == metric::record_kind::aligned_vector);
    static_assert(metric::metric_thread_safe_v<metric::Euclidean<double>>);
    static_assert(metric::metric_traits<DirectedIntegerDistance>::law == metric::metric_law::distance);

    const auto padded_hamming = metric::make_metric<std::string>(PaddedHamming{});
    static_assert(metric::is_metric_callable_v<decltype(padded_hamming), std::string>);

    assert_metric_contracts(
        std::vector<std::string>{"cat", "cot", "coat", "dog"},
        metric::Edit<std::string>{});

    assert_metric_contracts(
        std::vector<std::vector<double>>{{0.0, 1.0}, {1.0, 1.0}, {2.0, 3.0}},
        metric::Manhattan<double>{});

    assert_metric_contracts(
        std::vector<std::string>{"red", "reed", "road", "blue"},
        padded_hamming);

    assert_declared_metric_contracts(
        std::vector<std::vector<double>>{{0.0, 0.0}, {1.0, 0.0}, {1.0, 2.0}, {3.0, 3.0}},
        metric::Euclidean<double>{});

    assert_declared_metric_contracts(
        std::vector<int>{0, 1, 3},
        DirectedIntegerDistance{});

    return 0;
}
