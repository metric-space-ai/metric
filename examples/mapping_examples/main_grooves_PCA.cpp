
//#include "mapping.hpp"  // TODO fix DT.cpp and enable

//#include "../details/encoder.hpp"
#include "mapping/PCAnet.hpp" // temporary // for usage independently on the entire lib

#include "utils/visualizer.hpp"
//#include "../utils/visualizer.hpp"


#include "examples/mapping_examples/assets/helpers.cpp"




int main()
{
    //auto m = blaze::DynamicMatrix<double>(2, 3);
    //read_csv_blaze<blaze::DynamicMatrix, double>("testdata.csv", m, ";");
//    auto m = read_csv_blaze<double>("testdata.csv");

    auto all_data = read_csv_blaze<float>("Pt01_AllGrooves_energy_5.csv", ",");
    blaze::DynamicMatrix<float> training_dataset = submatrix(all_data, 0, 1, all_data.rows(), all_data.columns()-2);
    // std::cout << training_dataset << "\n";

    blaze::DynamicMatrix<float> test_data = submatrix(all_data, 0, all_data.columns()-1, all_data.rows(), 1);

    mat2bmp::blaze2bmp_norm(training_dataset, "training_dataset.bmp");
    mat2bmp::blaze2bmp_norm(test_data, "test_data.bmp");

    auto model = metric::PCAnet(true);
    model.train(training_dataset, 8); // dataset, compressed_code_length

    auto compressed = model.compress(test_data);

    mat2bmp::blaze2bmp_norm(compressed, "compressed.bmp");

    auto restored = model.decompress(compressed);

    mat2bmp::blaze2bmp_norm(restored, "restored.bmp");

    return 0;
}


