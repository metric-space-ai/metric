
//#include "mapping.hpp"  // TODO fix DT.cpp and enable

//#include "../details/encoder.hpp"
#include "mapping/PCFA.hpp" // temporary // for usage independently on the entire lib

#include "utils/visualizer.hpp"
//#include "../utils/visualizer.hpp"


#include "examples/mapping_examples/assets/helpers.cpp"


// toy class template
/*

template <typename ElementType>
class ToyClass
{

  public:

    template <template <typename, typename> typename ContainerType, typename Allocator>
    ToyClass(std::vector<ContainerType<ElementType, Allocator>> in) {
        blaze::DynamicMatrix<ElementType> blaze_in(in.size(), in[0].size(), 0);
        for (size_t i = 0; i < in.size(); ++i)
            for (size_t j = 0; j < in[0].size(); ++j)
                blaze_in(i, j) = in[i][j];
        blaze::DynamicVector<ElementType, blaze::rowVector> avgs;
        inner_data = metric::PCA(blaze_in, 4, avgs);
    }

    //template <template <typename, bool> typename ContainerType

    blaze::DynamicMatrix<ElementType> get_inner_data() {
        return inner_data;
    }

  private:
    blaze::DynamicMatrix<ElementType> inner_data;

};



template <typename ContainerType>
class ToyClass2
{

  private:

    template<typename>
    struct determine_element_type  // TODO replace with value_type/ElementTypew field detector!
    {
        using type = void;
    };

    template <typename ValueType, typename Allocator>
    struct determine_element_type<std::vector<ValueType, Allocator>>
    {
        using type = typename std::vector<ValueType, Allocator>::value_type;
    };

    template <typename ValueType, bool F>
    struct determine_element_type<blaze::DynamicVector<ValueType, F>>
    {
        using type = typename blaze::DynamicVector<ValueType, F>::ElementType;
    };


    template <typename ValueType, bool F>
    struct determine_element_type<blaze::DynamicMatrix<ValueType, F>>
    {
        using type = typename blaze::DynamicMatrix<ValueType, F>::ElementType;
    };


  public:

    //using ElementType = typename ContainerType::value_type;
    using ElementType = typename determine_element_type<ContainerType>::type;

    ToyClass2(std::vector<ContainerType> in) {
        blaze::DynamicMatrix<ElementType> blaze_in(in.size(), in[0].size(), 0);
        for (size_t i = 0; i < in.size(); ++i)
            for (size_t j = 0; j < in[0].size(); ++j)
                blaze_in(i, j) = in[i][j];
        blaze::DynamicVector<ElementType, blaze::rowVector> avgs;
        inner_data = metric::PCA(blaze_in, 4, avgs);
    }

    ToyClass2(blaze::DynamicMatrix<ElementType> in) {
        blaze::DynamicVector<ElementType, blaze::rowVector> avgs;
        inner_data = metric::PCA(in, 4, avgs);
    }

    blaze::DynamicMatrix<ElementType> get_inner_data() {
        return inner_data;
    }

  private:
    blaze::DynamicMatrix<ElementType> inner_data;

};


template <typename ContainerType>
ToyClass2<ContainerType> ToyClass2_factory(std::vector<ContainerType> in)
{
    return ToyClass2<ContainerType>(in);
}

template <typename ElementType>
ToyClass2<blaze::DynamicMatrix<ElementType>> ToyClass2_factory(blaze::DynamicMatrix<ElementType> in)
{
    return ToyClass2<blaze::DynamicMatrix<ElementType>>(in);
}

//*/




int main()
{

    // toy calls
    /*

    std::vector<float> d0 = {0, 1, 2};
    std::vector<float> d1 = {0, 1, 3};
    auto d = std::vector{d0, d1};

    //auto toy_object = ToyClass<float>(d);
    //auto toy_object = ToyClass2<std::vector<float>>(d);
    auto toy_object = ToyClass2_factory(d);
    //auto toy_object = ToyClass<determine_element_type<std::vector<float>>::type>(d);
    std::cout << toy_object.get_inner_data() << "\n";

    std::cout << "\n";

    blaze::DynamicVector<float, blaze::rowVector> d0_blaze {0, 1, 2};
    blaze::DynamicVector<float, blaze::rowVector> d1_blaze {0, 1, 3};
    auto d_blaze = std::vector{d0_blaze, d1_blaze};
    //auto toy_object_blaze = ToyClass2<blaze::DynamicVector<float, blaze::rowVector>>(d_blaze);
    auto toy_object_blaze = ToyClass2_factory(d_blaze);
    std::cout << toy_object_blaze.get_inner_data() << "\n";

    std::cout << "\n";

    blaze::DynamicMatrix<float, blaze::rowMajor> d_blaze_matrix {{0, 1, 2}, {0, 1, 3}};
    auto toy_object_blaze_matrix = ToyClass2_factory(d_blaze_matrix);
    std::cout << toy_object_blaze_matrix.get_inner_data() << "\n";

    return 0;
    //*/


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

    blaze::DynamicMatrix<V, blaze::rowMajor> training_dataset_r_t = trans(training_dataset_r);
    auto model_r = metric::PCFA_factory(training_dataset_r_t, n_features); // dataset, compressed_code_length

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


