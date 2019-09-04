
//#include "mapping.hpp"  // TODO fix DT.cpp and enable

//#include "../details/encoder.hpp"
#include "mapping/PCFA.hpp" // temporary // for usage independently on the entire lib

#include "utils/visualizer.hpp"
//#include "../utils/visualizer.hpp"


#include "examples/mapping_examples/assets/helpers.cpp"




int main()
{

    using V = float; // double;

    size_t n_features = 8;

    auto all_data = read_csv_blaze<V>("PtAll_AllGrooves_energy_5.csv", ","); // all parts  all unmixed channels
    blaze::DynamicMatrix<V> training_dataset = submatrix(all_data, 0, 1, all_data.rows(), all_data.columns()-2);
    // std::cout << training_dataset << "\n";

    blaze::DynamicMatrix<V> test_data = read_csv_blaze<V>("test_data_input.csv", ",");

//    blaze::DynamicMatrix<float> test_data = submatrix(all_data, 0, all_data.columns()-1, all_data.rows(), 1);

    mat2bmp::blaze2bmp_norm(training_dataset, "training_dataset.bmp");
    mat2bmp::blaze2bmp_norm(test_data, "test_data.bmp");
    blaze_dm_to_csv(training_dataset, "training_dataset.csv");
    blaze_dm_to_csv(test_data, "test_data.csv");

    auto model = metric::PCFA<V>    (training_dataset, n_features); // dataset, compressed_code_length

    auto compressed = model.encode(test_data);

    mat2bmp::blaze2bmp_norm(compressed, "compressed.bmp");
    blaze_dm_to_csv(compressed, "compressed.csv");

    auto restored = model.decode(compressed);

    mat2bmp::blaze2bmp_norm(restored, "restored.bmp");
    blaze_dm_to_csv(restored, "restored.csv");


    // also making feature output for the training dataset

    auto all_features = model.encode(training_dataset);

    mat2bmp::blaze2bmp_norm(all_features, "all_features.bmp");
    blaze_dm_to_csv(all_features, "all_features.csv");


    // view contribution of each feature

    auto I = blaze::IdentityMatrix<V>(n_features);

    for (size_t feature_idx=0; feature_idx<n_features; ++feature_idx) {
        blaze::DynamicMatrix<V> unit_feature = submatrix(I, 0, feature_idx, I.rows(), 1);
        auto unit_waveform = model.decode(unit_feature, false);
        mat2bmp::blaze2bmp_norm(unit_waveform, "unit_waveform_" + std::to_string(feature_idx) + ".bmp");
        blaze_dm_to_csv(unit_waveform, "unit_waveform_" + std::to_string(feature_idx) + ".csv");
    }



    return 0;
}


