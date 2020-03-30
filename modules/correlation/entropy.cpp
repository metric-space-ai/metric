/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2019 Panda Team
*/
#ifndef _METRIC_DISTANCE_K_RANDOM_ENTROPY_CPP
#define _METRIC_DISTANCE_K_RANDOM_ENTROPY_CPP

#include <boost/math/special_functions/digamma.hpp>
#include <boost/math/special_functions/gamma.hpp>

#include "modules/utils/type_traits.hpp"
#include "modules/space/tree.hpp"
#include "estimator_helpers.cpp"
#include "epmgp.cpp"

#include <cmath>
#include <vector>

namespace metric {

namespace entropy_details {


// averaged entropy estimation: code COPIED from mgc.*pp with only mgc replaced with entropy, TODO refactor to avoid code dubbing

std::vector<double> linspace(double a, double b, int n)
{
    std::vector<double> array;
    if (n > 1) {
        double step = (b - a) / double(n - 1);
        int count = 0;
        while (count < n) {
            array.push_back(a + count * step);
            ++count;
        }
    } else {
        array.push_back(b);
    }

    return array;
}



double polyeval(const std::vector<double>& poly, const double z)
{
    const int n = poly.size();
    double sum = poly[n - 1];
    for (int i = n - 2; i >= 0; --i) {
        sum *= z;
        sum += poly[i];
    }
    return sum;
}



double erfinv_imp(const double p, const double q)
{
    double result = 0;

    if (p <= 0.5) {
        static const float Y = 0.0891314744949340820313f;
        static const std::vector<double> P = { -0.000508781949658280665617L, -0.00836874819741736770379L,
            0.0334806625409744615033L, -0.0126926147662974029034L, -0.0365637971411762664006L,
            0.0219878681111168899165L, 0.00822687874676915743155L, -0.00538772965071242932965L };
        static const std::vector<double> Q = { 1, -0.970005043303290640362L, -1.56574558234175846809L,
            1.56221558398423026363L, 0.662328840472002992063L, -0.71228902341542847553L, -0.0527396382340099713954L,
            0.0795283687341571680018L, -0.00233393759374190016776L, 0.000886216390456424707504L };

        double g = p * (p + 10);
        double r = polyeval(P, p) / polyeval(Q, p);
        result = g * Y + g * r;
    } else if (q >= 0.25) {
        static const float Y = 2.249481201171875f;
        static const std::vector<double> P = { -0.202433508355938759655L, 0.105264680699391713268L,
            8.37050328343119927838L, 17.6447298408374015486L, -18.8510648058714251895L, -44.6382324441786960818L,
            17.445385985570866523L, 21.1294655448340526258L, -3.67192254707729348546L };
        static const std::vector<double> Q
            = { 1L, 6.24264124854247537712L, 3.9713437953343869095L, -28.6608180499800029974L, -20.1432634680485188801L,
                  48.5609213108739935468L, 10.8268667355460159008L, -22.6436933413139721736L, 1.72114765761200282724L };
        double g = std::sqrt(-2 * std::log(q));
        double xs = q - 0.25;
        double r = polyeval(P, xs) / polyeval(Q, xs);
        result = g / (Y + r);
    } else {
        double x = std::sqrt(-std::log(q));
        if (x < 3) {
            static const float Y = 0.807220458984375f;
            static const std::vector<double> P = { -0.131102781679951906451L, -0.163794047193317060787L,
                0.117030156341995252019L, 0.387079738972604337464L, 0.337785538912035898924L, 0.142869534408157156766L,
                0.0290157910005329060432L, 0.00214558995388805277169L, -0.679465575181126350155e-6L,
                0.285225331782217055858e-7L, -0.681149956853776992068e-9L };
            static const std::vector<double> Q = { 1, 3.46625407242567245975L, 5.38168345707006855425L,
                4.77846592945843778382L, 2.59301921623620271374L, 0.848854343457902036425L, 0.152264338295331783612L,
                0.01105924229346489121L };
            double xs = x - 1.125;
            double R = polyeval(P, xs) / polyeval(Q, xs);
            result = Y * x + R * x;
        } else if (x < 6) {
            static const float Y = 0.93995571136474609375f;
            static const std::vector<double> P
                = { -0.0350353787183177984712L, -0.00222426529213447927281L, 0.0185573306514231072324L,
                      0.00950804701325919603619L, 0.00187123492819559223345L, 0.000157544617424960554631L,
                      0.460469890584317994083e-5L, -0.230404776911882601748e-9L, 0.266339227425782031962e-11L };
            static const std::vector<double> Q
                = { 1L, 1.3653349817554063097L, 0.762059164553623404043L, 0.220091105764131249824L,
                      0.0341589143670947727934L, 0.00263861676657015992959L, 0.764675292302794483503e-4L };
            double xs = x - 3;
            double R = polyeval(P, xs) / polyeval(Q, xs);
            result = Y * x + R * x;
        } else if (x < 18) {
            static const float Y = 0.98362827301025390625f;
            static const std::vector<double> P
                = { -0.0167431005076633737133L, -0.00112951438745580278863L, 0.00105628862152492910091L,
                      0.000209386317487588078668L, 0.149624783758342370182e-4L, 0.449696789927706453732e-6L,
                      0.462596163522878599135e-8L, -0.281128735628831791805e-13L, 0.99055709973310326855e-16L };
            static const std::vector<double> Q
                = { 1L, 0.591429344886417493481L, 0.138151865749083321638L, 0.0160746087093676504695L,
                      0.000964011807005165528527L, 0.275335474764726041141e-4L, 0.282243172016108031869e-6L };
            double xs = x - 6;
            double R = polyeval(P, xs) / polyeval(Q, xs);
            result = Y * x + R * x;
        } else if (x < 44) {
            static const float Y = 0.99714565277099609375f;
            static const std::vector<double> P = { -0.0024978212791898131227L, -0.779190719229053954292e-5L,
                0.254723037413027451751e-4L, 0.162397777342510920873e-5L, 0.396341011304801168516e-7L,
                0.411632831190944208473e-9L, 0.145596286718675035587e-11L, -0.116765012397184275695e-17L };
            static const std::vector<double> Q
                = { 1L, 0.207123112214422517181L, 0.0169410838120975906478L, 0.000690538265622684595676L,
                      0.145007359818232637924e-4L, 0.144437756628144157666e-6L, 0.509761276599778486139e-9L };
            double xs = x - 18;
            double R = polyeval(P, xs) / polyeval(Q, xs);
            result = Y * x + R * x;
        } else {
            static const float Y = 0.99941349029541015625f;
            static const std::vector<double> P = { -0.000539042911019078575891L, -0.28398759004727721098e-6L,
                0.899465114892291446442e-6L, 0.229345859265920864296e-7L, 0.225561444863500149219e-9L,
                0.947846627503022684216e-12L, 0.135880130108924861008e-14L, -0.348890393399948882918e-21L };
            static const std::vector<double> Q
                = { 1L, 0.0845746234001899436914L, 0.00282092984726264681981L, 0.468292921940894236786e-4L,
                      0.399968812193862100054e-6L, 0.161809290887904476097e-8L, 0.231558608310259605225e-11L };
            double xs = x - 44;
            double R = polyeval(P, xs) / polyeval(Q, xs);
            result = Y * x + R * x;
        }
    }
    return result;
}



double erfcinv(const double z)
{
    if ((z < 0) || (z > 2))
        std::cout << "Argument outside range [0,2] in inverse erfc function (got p=%1%)." << std::endl;

    double p, q, s;
    if (z > 1) {
        q = 2 - z;
        p = 1 - q;
        s = -1;
    } else {
        p = 1 - z;
        q = z;
        s = 1;
    }
    return s * erfinv_imp(p, q);
}



std::vector<double> icdf(const std::vector<double>& prob, const double mu, const double sigma)
{
    std::vector<double> synth;
    synth.reserve(prob.size());
    for (auto p : prob) {
        synth.push_back(mu + -1.41421356237309504880 * erfcinv(2 * p) * sigma);
    }

    return synth;
}



double variance(const std::vector<double>& data, const double mean)
{
    double sum = 0;
    for (size_t i = 0; i < data.size(); ++i) {
        sum += (data[i] - mean) * (data[i] - mean);
    }

    return sum;
}



double mean(const std::vector<double>& data)
{
    double sum = 0;
    for (size_t i = 0; i < data.size(); ++i) {
        sum += data[i];
    }
    double value = sum / data.size();

    return value;
}


double peak2ems(const std::vector<double>& data)
{
    double maxAbs = -1;
    double rms = 0;
    for (const auto v : data) {
        const double absV = abs(v);
        if (absV > maxAbs) {
            maxAbs = absV;
        }

        rms += v * v;
    }

    rms /= data.size();
    rms = sqrt(rms);

    return maxAbs / rms;
}


template <typename T>
T conv_diff_entropy(T in) {
    if (in < 1)
        return 1/std::exp(1)*std::exp(in);
    else
        return in;
}

template <typename T>
T conv_diff_entropy_inv(T in) {
    if (in < 1)
        return std::log(in) + 1;
    else
        return in;
}


template <typename T1, typename T2>
T1 log(T1 logbase, T2 x)
{
    return std::log(x) / std::log(logbase);
}


double mvnpdf(blaze::DynamicVector<double> x, blaze::DynamicVector<double> mu, blaze::DynamicMatrix<double> Sigma) {

    size_t n = x.size();
    assert(mu.size() == n && Sigma.columns() == n && Sigma.rows() == n);

    auto centered = x - mu;
    auto p =  blaze::trans(centered) * blaze::inv(Sigma) * centered;
    //return std::exp(-p/2) / ( std::sqrt(blaze::det(Sigma)) * std::pow(2*M_PI, (double)n/2.0) );
    return std::exp(-p/2) / std::sqrt( blaze::det(Sigma) * std::pow(2*M_PI, n) );
}


double mvnpdf(blaze::DynamicVector<double> x) {

    return( mvnpdf(x, blaze::DynamicVector<double>(x.size(), 0), blaze::IdentityMatrix<double>(x.size())) );
}


// averaged entropy estimation: code COPIED from mgc.*pp with only mgc replaced with entropy, TODO refactor to avoid code dubbing
template <typename Container>
double estimate(
        const Container & a,
        const std::function<double(const Container&)>& entropy,
        const size_t sampleSize,
        const double threshold,
        size_t maxIterations
){
    using T = type_traits::underlaying_type_t<Container>;
    const size_t dataSize = a.size();

    // Update maxIterations
    if (maxIterations == 0) {
        maxIterations = dataSize / sampleSize;
    }

    if (maxIterations > dataSize / sampleSize) {
        maxIterations = dataSize / sampleSize;
    }

    if (maxIterations < 1) {
        return entropy(a);
    }

    // Create shuffle indexes
    std::vector<size_t> indexes(dataSize);
    std::iota(indexes.begin(), indexes.end(), 0);

    auto rng = std::default_random_engine();
    std::shuffle(indexes.begin(), indexes.end(), rng);

    // Create vector container for fast random access
    const std::vector<typename Container::value_type> vectorA(a.begin(), a.end());

    // Create samples
    std::vector<typename Container::value_type> sampleA;
    sampleA.reserve(sampleSize);

    std::vector<double> entropyValues;
    double mu = 0;
    for (auto i = 1; i <= maxIterations; ++i) {
        size_t start = (i - 1) * sampleSize;
        size_t end = std::min(i * sampleSize - 1, dataSize - 1);

        // Create samples
        sampleA.clear();

        for (auto j = start; j < end; ++j) {
            sampleA.push_back(vectorA[indexes[j]]);
        }

        // Get sample mgc value
        double sample_entopy = entropy(sampleA);
        entropyValues.push_back(sample_entopy);

        std::sort(entropyValues.begin(), entropyValues.end());

        const size_t n = entropyValues.size();
        const auto p0 = entropy_details::linspace(0.5 / n, 1 - 0.5 / n, n);

        mu = entropy_details::mean(entropyValues);
        double sigma = entropy_details::variance(entropyValues, mu);

        const std::vector<double> synth = entropy_details::icdf(p0, mu, sigma);
        std::vector<double> diff;
        diff.reserve(n);
        for (auto i = 0; i < n; ++i) {
            diff.push_back(entropyValues[i] - synth[i]);
        }

        auto convergence = entropy_details::peak2ems(diff) / n;
        std::cout << n << " " << convergence << " " << sample_entopy << " " << mu << std::endl;

        if (convergence < threshold) {
            return mu;
        }
    }

    return mu;
}


} // namespace entropy_details




// ----------------------------------- entropy

// updated version, for different metric
// averaged entropy estimation: code COPIED from mgc.*pp with only mgc replaced with entropy, TODO refactor to avoid code dubbing
template <typename recType, typename Metric>
template <typename Container>
double entropy_simple<recType, Metric>::operator()(
        const Container& data,
        //std::size_t k,
        //double logbase,
        //Metric metric,
        //bool exp
        ) const
{
    using T = type_traits::underlaying_type_t<Container>;
    using V = type_traits::index_value_type_t<Container>;

    if (data.empty() || data[0].empty()) {
        return 0;
    }
    if (data.size() < k + 1)
        throw std::invalid_argument("number of points in dataset must be larger than k");

    double N = data.size();
    double d = data[0].size();

    //add_noise(data); // TODO test
    metric::Tree<V, Metric> tree(data, -1, metric);

    double entropyEstimate = 0;
    double log_sum = 0;

    for (std::size_t i = 0; i < N; i++) {
        auto res = tree.knn(data[i], k + 1);
        entropyEstimate += std::log(res.back().second);
    }
    entropyEstimate = entropyEstimate * d / (double)N; // mean log * d
    entropyEstimate += boost::math::digamma(N) - boost::math::digamma(k) + d*std::log(2.0);

    if constexpr (!std::is_same<Metric, typename metric::Chebyshev<T>>::value) {
        double p = 1; // Manhatten and other metrics (TODO check if it is correct for them!)
        if constexpr (std::is_same<Metric, typename metric::Euclidian<T>>::value) {
            p = 2; // Euclidean
        } else if constexpr (std::is_same<Metric, typename metric::P_norm<T>>::value) {
            p = metric.p; // general Minkowsky
        }
        entropyEstimate += d * std::log(std::tgamma(1 + 1 / p)) - std::log(std::tgamma(1 + d / p));
    }
    entropyEstimate /= std::log(logbase);
    if (exp)
        return entropy_details::conv_diff_entropy(entropyEstimate); // conversion of values below 1 to exp scale
    else
        return entropyEstimate;
}




// averaged entropy estimation: code COPIED from mgc.*pp with only mgc replaced with entropy, TODO refactor to avoid code dubbing
template <typename recType, typename Metric>
template <typename Container>
double entropy_simple<recType, Metric>::estimate(
        const Container & a,
        const size_t sampleSize,
        const double threshold,
        size_t maxIterations
        //std::size_t k,
        //double logbase,
        //Metric metric,
        //bool exp // TODO apply to returning value!
        )
{
    return entropy_details::estimate_func<Container, entropy_simple<recType, Metric>, Metric>(a, sampleSize, threshold, maxIterations, k, logbase, metric, exp);
}







// ----------------------------------- entropy with kpN approximation




template <typename recType, typename Metric>
template <template <typename, typename> class OuterContainer, typename Container, typename OuterAllocator>
double entropy<recType, Metric>::operator()(const OuterContainer<Container, OuterAllocator>& data) const
{
    size_t n = X.size();
    size_t d = X[0].size();

    assert(p < n);
    assert(k < p);

    double h = 0;
    int got_results = 0;  // absents in Matlab original code

    metric::Tree<Container, Metric> tree (data, -1, metric);
    blaze::DynamicMatrix<double> Nodes (p, d, 0);
    blaze::DynamicVector<double> mu (d, 0);
    blaze::DynamicVector<double> lb (d, 0);
    blaze::DynamicVector<double> ub (d, 0);
    blaze::DynamicVector<double> x_vector (d, 0);
    for (size_t i = 0; i < n; ++i) {

        auto res = tree.knn(data[i], p);
        auto eps = res[k-1].second;

        blaze::reset(mu);
        for (size_t p_idx= 0; p_idx < p; ++p_idx) { // r v realizations from the tree
            for (size_t d_idx = 0; d_idx < d; ++d_idx) { // dimensions
                //Nodes(p_idx, d_idx) = res[p_idx].first->data[d_idx];
                //mu[d_idx] += res[p_idx].first->data[d_idx];
                Nodes(p_idx, d_idx) = res[p_idx].first->get_data()[d_idx];
                mu[d_idx] += res[p_idx].first->get_data()[d_idx];
            }
        }
        mu = mu/p;
        Nodes = Nodes - blaze::expand(blaze::trans(mu), Nodes.rows());
        double offset = 1e-8;
        //double offset = 1e-5; // TODO consider dependence on machine epsilon
        auto K = blaze::evaluate( (blaze::trans(Nodes) * Nodes)*p/(p - 1) + blaze::IdentityMatrix<double>(d)*offset );

        blaze::reset(lb);
        blaze::reset(ub);
        for (size_t d_idx = 0; d_idx < d; ++d_idx) { // dimensions
            lb[d_idx] = data[i][d_idx] - eps;
            ub[d_idx] = data[i][d_idx] + eps;
            x_vector[d_idx] = data[i][d_idx];
        }

        auto g_local = epmgp::local_gaussian_axis_aligned_hyperrectangles<double>(mu, K, lb, ub);
        double logG = std::get<0>(g_local);

        if (!std::isnan(logG)) { // UNLIKE original Matlab code, we exclude points that result in NaN
            double g = entropy_details::mvnpdf(x_vector, mu, K);
            h += logG - std::log(g);
            got_results++;
        }

    }

    double result;
    if (got_results <= 20) // this absents in Matlab original code. TODO adjust min number of points
        result = std::nan("estimation failed");
    result = boost::math::digamma(n) - boost::math::digamma(k) + h/n;
    if (exp)
        return entropy_details::conv_diff_entropy(result); // conversion of values below 1 to exp scale
    return result;
}


// averaged entropy estimation: code COPIED from mgc.*pp with only mgc replaced with entropy, TODO refactor to avoid code dubbing
template <typename recType, typename Metric>
template <typename Container>
double entropy<recType, Metric>::estimate(
        const Container & a,
        const size_t sampleSize,
        const double threshold,
        size_t maxIterations
        )
{
    return entropy_details::estimate_func<Container, entropy<recType, Metric>, Metric>(a, sampleSize, threshold, maxIterations, k, 2, metric, exp);
}







// ---------------------------------- estimators to be debugged
// Kozachenko-Leonenko estimator based on https://hal.archives-ouvertes.fr/hal-00331300/document (Shannon diff. entropy,
// q = 1)
// WARNING: this estimator is sill under construction, it seems to have bugs and needs debugging

template <typename Container, typename Metric = metric::Euclidian<typename Container::value_type>, typename L = double>  // TODO check if L = T is correct
typename std::enable_if<!std::is_integral<typename Container::value_type>::value, double>::type entropy_kl(
    std::vector<Container> data, std::size_t k = 3, L logbase = 2, Metric metric = Metric())
{
    using T = typename Container::value_type;

    if (data.empty() || data[0].empty())
        return 0;
    if (data.size() < k + 1)
        throw std::invalid_argument("number of points in dataset must be larger than k");
    if constexpr (!std::is_same<Metric, typename metric::Euclidian<T>>::value)
        throw std::logic_error("entropy function is now implemented only for Euclidean distance");

    metric::Tree<Container, Metric> tree(data, -1, metric);

    size_t N = data.size();
    size_t m = data[0].size();
    double two = 2.0;  // this is in order to make types match the log template function
    double sum = 0;
    auto Pi = boost::math::constants::pi<T>();
    double half_m = m / two;
    auto coeff = (N - 1) * exp(-boost::math::digamma(k + 1)) * std::pow(Pi, half_m) / boost::math::tgamma(half_m + 1);

    for (std::size_t i = 0; i < N; i++) {
        auto neighbors = tree.knn(data[i], k + 1);
        auto ro = neighbors.back().second;
        sum = sum + entropy_details::log(logbase, coeff * std::pow(ro, m));
    }

    return sum;
} // TODO debug!!


// ported from Julia, not in use
template <typename T>
std::pair<std::vector<double>, std::vector<std::vector<T>>> pluginEstimator(const std::vector<std::vector<T>>& Y)
{
    std::vector<std::vector<T>> uniqueVal = unique(Y);
    std::vector<double> counts(uniqueVal.size());
    for (std::size_t i = 0; i < counts.size(); i++) {
        for (std::size_t j = 0; j < Y.size(); j++) {
            if (Y[j] == uniqueVal[i])
                counts[i]++;
        }
    }
    std::size_t length = Y.size() * Y[0].size();
    std::transform(counts.begin(), counts.end(), counts.begin(), [&length](auto& i) { return i / length; });

    return std::make_pair(counts, uniqueVal);
}



} // namespace metric

#endif
