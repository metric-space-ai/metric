
//#include "mapping.hpp"  // TODO fix DT.cpp and enable

//#include "../details/encoder.hpp"
#include "mapping/PCAnet.hpp" // temporary // for usage independently on the entire lib

//#include "utils/visualizer/visualizer.hpp"
#include "../utils/visualizer.hpp"


#include "examples/mapping_examples/assets/helpers.cpp"




int main()
{
    //auto m = blaze::DynamicMatrix<double>(2, 3);
    //read_csv_blaze<blaze::DynamicMatrix, double>("testdata.csv", m, ";");
//    auto m = read_csv_blaze<double>("testdata.csv");

    auto training_dataset = read_csv_blaze<float>("Pt01_Ch01_Grooves_1_to_7.csv");
    // std::cout << training_dataset << "\n";

    auto test_data = read_csv_blaze<float>("Pt01_Ch01_Groove09.csv");

    mat2bmp::blaze2bmp(training_dataset, "training_dataset.bmp");
    mat2bmp::blaze2bmp(test_data, "test_data.bmp");

    auto model = metric::PCAnet(true);
    model.train(training_dataset, 8); // dataset, compressed_code_length

    auto compressed = model.compress(test_data);

    mat2bmp::blaze2bmp(compressed, "compressed.bmp");

    auto restored = model.decompress(compressed);

    mat2bmp::blaze2bmp(restored, "restored.bmp");

    return 0;
}


