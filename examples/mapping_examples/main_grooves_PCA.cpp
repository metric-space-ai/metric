
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


    //* // good code, may be enabled
    // code for old PCFA_col

    auto all_data = read_csv_blaze<V>("PtAll_AllGrooves_energy_5.csv", ","); // all parts  all unmixed channels
    blaze::DynamicMatrix<V> training_dataset = submatrix(all_data, 0, 1, all_data.rows(), all_data.columns()-2);
    // std::cout << training_dataset << "\n";

    blaze::DynamicMatrix<V> test_data = read_csv_blaze<V>("test_data_input.csv", ",");

//    blaze::DynamicMatrix<float> test_data = submatrix(all_data, 0, all_data.columns()-1, all_data.rows(), 1);

    mat2bmp::blaze2bmp_norm(training_dataset, "training_dataset.bmp");
    mat2bmp::blaze2bmp_norm(test_data, "test_data.bmp");
    blaze_dm_to_csv(training_dataset, "training_dataset.csv");
    blaze_dm_to_csv(test_data, "test_data.csv");

    auto model = metric::PCFA_col_factory(training_dataset, n_features); // dataset, compressed_code_length

    auto avg = model.average();
    mat2bmp::blaze2bmp_norm(avg, "averages.bmp");
    blaze_dm_to_csv(avg, "averages.csv");

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

    // same using eigenmodes getter
    auto Eigenmodes = model.eigenmodes();
    blaze_dm_to_csv(Eigenmodes, "eigenmodes.csv");

    //*/


    // row_wise PCFA with trans() on each input and output

    auto all_data_r = read_csv_blaze<V>("PtAll_AllGrooves_energy_5.csv", ","); // all parts  all unmixed channels
    blaze::DynamicMatrix<V> training_dataset_r = submatrix(all_data_r, 0, 1, all_data_r.rows(), all_data_r.columns()-2);
    // std::cout << training_dataset << "\n";

    blaze::DynamicMatrix<V> test_data_r = read_csv_blaze<V>("test_data_input.csv", ",");

    mat2bmp::blaze2bmp_norm(training_dataset_r, "training_dataset_r.bmp");
    mat2bmp::blaze2bmp_norm(test_data_r, "test_data_r.bmp");
    blaze_dm_to_csv(training_dataset_r, "training_dataset_r.csv");
    blaze_dm_to_csv(test_data_r, "test_data_r.csv");

    auto model_r = metric::PCFA_factory(trans(training_dataset_r), n_features); // dataset, compressed_code_length

    blaze::DynamicMatrix<V> avg_r_out = trans(model_r.average());
    mat2bmp::blaze2bmp_norm(avg_r_out, "averages_r.bmp");
    blaze_dm_to_csv(avg_r_out, "averages_r.csv");

    auto compressed_r = model_r.encode(trans(test_data_r));

    blaze::DynamicMatrix<V> compressed_r_out = trans(compressed_r);
    mat2bmp::blaze2bmp_norm(compressed_r_out, "compressed_r.bmp");
    blaze_dm_to_csv(compressed_r_out, "compressed_r.csv");

    auto restored_r = model_r.decode(compressed_r);

    blaze::DynamicMatrix<V> restored_r_out = trans(restored_r);
    mat2bmp::blaze2bmp_norm(restored_r_out, "restored_r.bmp");
    blaze_dm_to_csv(restored_r_out, "restored_r.csv");


    // also making feature output for the training dataset

    auto all_features_r = model_r.encode(trans(training_dataset_r));

    blaze::DynamicMatrix<V> all_features_r_out = trans(all_features_r);
    mat2bmp::blaze2bmp_norm(all_features_r_out, "all_features_r.bmp");
    blaze_dm_to_csv(all_features_r_out, "all_features_r.csv");


//    // view contribution of each feature

//    auto I = blaze::IdentityMatrix<V>(n_features);

//    for (size_t feature_idx=0; feature_idx<n_features; ++feature_idx) {
//        blaze::DynamicMatrix<V> unit_feature = submatrix(I, 0, feature_idx, I.rows(), 1);
//        auto unit_waveform = model.decode(unit_feature, false);
//        mat2bmp::blaze2bmp_norm(unit_waveform, "unit_waveform_" + std::to_string(feature_idx) + ".bmp");
//        blaze_dm_to_csv(unit_waveform, "unit_waveform_" + std::to_string(feature_idx) + ".csv");
//    }

//    // same using eigenmodes getter
//    auto Eigenmodes = model.eigenmodes();
//    blaze_dm_to_csv(Eigenmodes, "eigenmodes.csv");


    return 0;
}


