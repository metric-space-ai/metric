

#include <iostream>


#include "../../../../modules/mapping.hpp"

#include "../../../../modules/utils/visualizer.hpp"

#include "../../../../modules/transform/discrete_cosine.hpp"

#include "../../assets/helpers.cpp"

#include "../../../../3rdparty/blaze/Blaze.h"


int main() {

    auto csv = read_csv_blaze<double>("assets/830_905_part1.csv", ",");
    blaze::DynamicMatrix<double> all_data = blaze::trans(csv);

    auto model = metric::PCFA_col_factory(all_data, 5);

    std::cout << "\nTraining done. Start encoding\n";

    auto Eigenmodes = model.eigenmodes();
    blaze::DynamicMatrix<double> Eigenmodes_rowwise = blaze::trans(Eigenmodes);
    blaze_dm_to_csv(Eigenmodes_rowwise, "830_905_part1_eigenmodes.csv");

    auto Encoded = model.encode(all_data);
    blaze::DynamicMatrix<double> Encoded_rowwise = blaze::trans(Encoded);
    blaze_dm_to_csv(Encoded_rowwise, "830_905_part1_encoded.csv");

    auto Decoded = model.decode(Encoded);
    blaze::DynamicMatrix<double> Decoded_rowwise = blaze::trans(Decoded);
    blaze_dm_to_csv(Decoded_rowwise, "830_905_part1_decoded.csv");


    std::cout << "Eencoding done\n";




    return 0;
}
