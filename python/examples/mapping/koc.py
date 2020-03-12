import sys
import numpy
from metric.mapping import KOC_factory


def main():
    best_w_grid_size = 5
    best_h_grid_size = 4

    dataset = numpy.float_([
        [0, 0, 0],
        [0, 1, 0],
        [0, 2, 0],
        [2, 2, 2],
        [2, 2, 2],
        [2, 2, 2],
        [0, 0, 4],
        [0, 0, 4],
        [0, 0, 4],
        [8, 0, 0],
        [8, 0, 0],
        [8, 0, 0],
    ])

    simple_dataset = numpy.float_([8, 0, 1])

    simple_koc_factory = KOC_factory(best_w_grid_size, best_h_grid_size, 0.5, 0.0, 100)
    simple_koc = simple_koc_factory(dataset, 5)

    anomaly_threshold = -0.5

    print('vector anomalies: ', simple_koc.check_if_anomaly(sample=simple_dataset, anomaly_threshold=anomaly_threshold))

    anomalies = simple_koc.check_if_anomaly(samples=dataset, anomaly_threshold=anomaly_threshold)
    print('anomalies:', list(anomalies))

    assignments = simple_koc.result(dataset, anomaly_threshold)
    print('assignments:', list(assignments))


sys.exit(main())

'''
    json reference_data
    reference_data.push_back(assignments)

    auto t1 = std::chrono::steady_clock::now()

        / *Load
        data * /
        auto
        speeds = readEnergies(RAW_DATA_DIRNAME);
        std::cout << "" << std::endl;
        std::cout << "Num records: " << speeds.size() << std::endl;
        std::cout << "Num values in the record: " << speeds[0].size() << std::endl;

        metric::KOC_factory < Record, metric::Grid4, metric::Euclidian < double >, std::uniform_real_distribution < double >>
        koc_factory(best_w_grid_size, best_h_grid_size, 0.8, 0.0, 20);

        auto
        koc = koc_factory(speeds, 5)

        std::vector < std::vector < double >> test_samples;

        std::vector < double > test_sample;
        for (auto i = 0; i < 7 * 8; i++)
        {
            test_sample.push_back(((double)
        rand() / (RAND_MAX)));
        }
        test_samples.push_back(test_sample);

        std::random_device rd; 
        rng(rd()); // random - number

        std::uniform_int_distribution < int > uni(0, speeds.size()); // guaranteed unbiased

        // metric::Grid4
        graph(best_w_grid_size, best_h_grid_size);
        metric::Euclidian < double > distance;
        // std::uniform_real_distribution < double > distribution;

        auto
        random_integer = uni(rng);
        test_samples.push_back(speeds[random_integer]);

        std::vector < double > entropies;
        double
        entropies_sum = 0;
        metric::SOM < Record, metric::Grid4, metric::Euclidian < double >, std::uniform_real_distribution < double >> & som_version = koc;
        for (size_t i = 0; i < speeds.size(); i++)
        {

            auto
        reduced = som_version.encode(speeds[i]);
        std::sort(reduced.begin(), reduced.end());

        std::vector < Record > reduced_reshaped;
        for (size_t j = 0; j < reduced.size(); j++)
        {
        reduced_reshaped.push_back({reduced[j]});
        }
        auto e = entropy(reduced_reshaped, 3, 2.0, distance);
        entropies_sum += e;
        entropies.push_back(e);
        }

        std::sort(entropies.begin(), entropies.end());
        for (size_t i = 0; i < 20; i++)
        {
            std:: cout << "first entropy: " << entropies[i] << std::endl;
        }
        std::cout << "" << std::endl;

        std::cout << "mean entropy: " << entropies_sum / entropies.size() << std::endl;
        auto
        result = std::max_element(entropies.begin(), entropies.end());
        std::cout << "max entropy: " << entropies[std::distance(entropies.begin(), result)] << std::endl;
        result = std::min_element(entropies.begin(), entropies.end());
        std::cout << "min entropy: " << entropies[std::distance(entropies.begin(), result)] << std::endl;
        std::cout << "" << std::endl;

        for (size_t i = 0; i < test_samples.size(); i++)
        {
            auto
        reduced = som_version.encode(test_samples[i]);
        std::sort(reduced.begin(), reduced.end());

        std::vector < Record > reduced_reshaped;
        for (size_t j = 0; j < reduced.size(); j++)
        {
        reduced_reshaped.push_back({reduced[j]});
        }
        auto e = entropy(reduced_reshaped, 3, 2.0, distance);
        std::
            cout << "test entropy: " << e << std::endl;
        }
        std::cout << "" << std::endl;

        anomaly_threshold = -0.1;
        anomalies = koc.check_if_anomaly(test_samples, anomaly_threshold);

        std::cout << std::endl;
        std::cout << "anomalies:" << std::endl;
        vector_print(anomalies);
        std::cout << std::endl;

        assignments = koc.result(test_samples, anomaly_threshold);
        std::cout << std::endl;
        std::cout << "assignments:" << std::endl;
        vector_print(assignments);

        auto
        assigned_clusters = koc.encode(test_samples, anomaly_threshold);
        std::cout << std::endl;
        std::cout << "assigned clusters:" << std::endl;
        vector_print(assigned_clusters);
        std::cout << std::endl;

        auto
        t2 = std::chrono::steady_clock::now();
        std::cout << "(Time = " << double(std::chrono::duration_cast < std::chrono::microseconds > (
        t2 - t1).count()) / 1000000 << " s)" << std::endl;
        std::cout << "" << std::endl;

        return 0
'''