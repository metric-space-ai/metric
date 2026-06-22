

#include <iostream>

#include "metric/mapping.hpp"

#include "metric/utils/visualizer.hpp"

#include "metric/transform/discrete_cosine.hpp"

#include "../../assets/helpers.cpp"

#include <metric/numeric.hpp>

int main()
{

	auto csv = read_csv_matrix<double>("assets/830_905_part1.csv", ",");
	mtrc::numeric::DynamicMatrix<double> all_data = mtrc::numeric::trans(csv);

	auto model = mtrc::PCFA_col_factory(all_data, 5);

	std::cout << "\nTraining done. Start encoding\n";

	auto Eigenmodes = model.eigenmodes();
	mtrc::numeric::DynamicMatrix<double> Eigenmodes_rowwise = mtrc::numeric::trans(Eigenmodes);
	matrix_to_csv(Eigenmodes_rowwise, "830_905_part1_eigenmodes.csv");

	auto Encoded = model.encode(all_data);
	mtrc::numeric::DynamicMatrix<double> Encoded_rowwise = mtrc::numeric::trans(Encoded);
	matrix_to_csv(Encoded_rowwise, "830_905_part1_encoded.csv");

	auto Decoded = model.decode(Encoded);
	mtrc::numeric::DynamicMatrix<double> Decoded_rowwise = mtrc::numeric::trans(Decoded);
	matrix_to_csv(Decoded_rowwise, "830_905_part1_decoded.csv");

	std::cout << "Eencoding done\n";

	return 0;
}
