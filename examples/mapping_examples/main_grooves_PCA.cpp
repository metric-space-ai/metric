
//#include "mapping.hpp"  // TODO fix DT.cpp and enable

//#include "../details/encoder.hpp"
#include "mapping/PCAnet.hpp" // temporary // for usage independently on the entire lib

#include "utils/visualizer.hpp"
//#include "../utils/visualizer.hpp"


#include "examples/mapping_examples/assets/helpers.cpp"




int main()
{

    size_t n_features = 8;

    //auto m = blaze::DynamicMatrix<double>(2, 3);
    //read_csv_blaze<blaze::DynamicMatrix, double>("testdata.csv", m, ";");
//    auto m = read_csv_blaze<double>("testdata.csv");

    //auto all_data = read_csv_blaze<float>("Pt01_AllGrooves_energy_5.csv", ","); // part 1
    //auto all_data = read_csv_blaze<float>("Pt02_AllGrooves_energy_0.csv", ","); // part 2
    //auto all_data = read_csv_blaze<float>("Pt03_AllGrooves_energy_0.csv", ","); // part 3
    auto all_data = read_csv_blaze<float>("PtAll_AllGrooves_energy_5.csv", ","); // all parts  all unmixed channels
    blaze::DynamicMatrix<float> training_dataset = submatrix(all_data, 0, 1, all_data.rows(), all_data.columns()-2);
    // std::cout << training_dataset << "\n";

    blaze::DynamicMatrix<float> test_data = submatrix(all_data, 0, all_data.columns()-1, all_data.rows(), 1);

    mat2bmp::blaze2bmp_norm(training_dataset, "training_dataset.bmp");
    mat2bmp::blaze2bmp_norm(test_data, "test_data.bmp");
    blaze_dm_to_csv(training_dataset, "training_dataset.csv");
    blaze_dm_to_csv(test_data, "test_data.csv");

    auto model = metric::PCAnet(true);
    model.train(training_dataset, n_features); // dataset, compressed_code_length

    auto compressed = model.compress(test_data);

    mat2bmp::blaze2bmp_norm(compressed, "compressed.bmp");
    blaze_dm_to_csv(compressed, "compressed.csv");

    auto restored = model.decompress(compressed);

    mat2bmp::blaze2bmp_norm(restored, "restored.bmp");
    blaze_dm_to_csv(restored, "restored.csv");


    // also making feature output for the training dataset

    auto all_features = model.compress(training_dataset);

    mat2bmp::blaze2bmp_norm(all_features, "all_features.bmp");
    blaze_dm_to_csv(all_features, "all_features.csv");


    // view contribution of each feature

    auto I = blaze::IdentityMatrix<float>(n_features);

    for (size_t feature_idx=0; feature_idx<n_features; ++feature_idx) {
        blaze::DynamicMatrix<float> unit_feature = submatrix(I, 0, feature_idx, I.rows(), 1);
        auto unit_waveform = model.decompress(unit_feature);
        mat2bmp::blaze2bmp_norm(unit_waveform, "unit_waveform_" + std::to_string(feature_idx) + ".bmp");
        blaze_dm_to_csv(unit_waveform, "unit_waveform_" + std::to_string(feature_idx) + ".csv");
    }



    return 0;
}


