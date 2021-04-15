
#include "../helpers/csv.hpp"
#include "../ortho_to_persp/ortho_to_persp.hpp"

#include "../../../../../3rdparty/blaze/Blaze.h"

#include <iostream>


int main()
{

    blaze::DynamicMatrix<double> z_orth (720, 2800, 0);
    readCsvBlaze("z_orth.csv", z_orth);

    blaze::DynamicMatrix<double> K (3, 3, 0);
    readCsvBlaze("k.csv", K);

    auto x_y = orthoToPersp(z_orth, K);

    writeCsvBlaze("z_orth_out.csv", z_orth);
    writeCsvBlaze("x.csv", std::get<0>(x_y));
    writeCsvBlaze("y.csv", std::get<1>(x_y));

    return 0;
}
