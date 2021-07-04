#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include "modules/utils/solver/ups/ups_solver/ups_solver.hpp"

#include <limits>
//#include <vector>
//#include <iostream>



TEMPLATE_TEST_CASE("ups: overall", "[utils]", float, double) {

    blaze::CompressedMatrix<unsigned char, blaze::columnMajor> M {
        {0, 0, 1, 1, 0},
        {0, 1, 1, 1, 1},
        {0, 1, 1, 0, 0},
        {0, 0, 1, 0, 0}
    };

    blaze::DynamicMatrix<TestType, blaze::columnMajor> Z {
        {0.8, 0.8, 0.8, 0.8, 0.8},
        {0.8, 0.8, 0.8, 0.9, 0.8},
        {0.8, 1.0, 0.8, 0.8, 0.8},
        {0.8, 0.8, 0.8, 0.8, 0.8}};

    blaze::DynamicMatrix<TestType> K {
        {1, 0, 1},
        {0, 1, 1},
        {0, 0, 1}};


    // images

    blaze::DynamicMatrix<TestType, blaze::columnMajor> I1R {
        {0,  5,  10, 5,  0},
        {5,  10, 15, 10, 5},
        {10, 15, 20, 15, 10},
        {5,  10, 15, 10, 0}
    };

    blaze::DynamicMatrix<TestType, blaze::columnMajor> I2R {
        {0,  5,  10, 5,  0},
        {5,  10, 30, 20, 5},
        {10, 15, 20, 15, 10},
        {5,  10, 15, 10, 0}
    };

    std::vector<blaze::DynamicMatrix<TestType, blaze::columnMajor>> I1 {I1R, I1R, I1R};
    std::vector<blaze::DynamicMatrix<TestType, blaze::columnMajor>> I2 {I2R, I2R, I2R};
    std::vector<std::vector<blaze::DynamicMatrix<TestType, blaze::columnMajor>>> I {I1, I2};


    auto r = ups_solver(Z, M, K, I);

//    std::cout << std::endl << "final result:" << std::endl
//              << "z: " << std::endl << std::get<0>(r) << std::endl
//              << "rho1: " << std::endl << std::get<1>(r)[0] << std::endl
//              << "rho2: " << std::endl << std::get<1>(r)[1] << std::endl
//              << "rho3: " << std::endl << std::get<1>(r)[2] << std::endl;

//    blaze::DynamicMatrix<float, blaze::columnMajor> z_ref = {  // c++ result on some stage of debugging
//        {            0,            0,       1.2021,      1.05274,            0 },
//        {            0,      1.04852,      1.53897,     0.717244,     0.732883 },
//        {            0,     0.995618,     0.793853,            0,            0 },
//        {            0,            0,     0.821557,            0,            0 }
//    };

    blaze::DynamicMatrix<TestType, blaze::columnMajor> z_ref = {  // Octave reference result
        { 0.00000,     0.00000,     1.20301,     1.05178,     0.00000 },
        { 0.00000,     1.04842,     1.53990,     0.71446,     0.73173 },
        { 0.00000,     0.99260,     0.79102,     0.00000,     0.00000 },
        { 0.00000,     0.00000,     0.82003,     0.00000,     0.00000 }
    };

    blaze::DynamicMatrix<TestType> albedo_ref = {  // Octave reference result
        { 0.00000,     0.00000,    17.97467,    53.25992,     0.00000 },
        { 0.00000,    25.61316,    40.16718,    31.92726,    36.94323 },
        { 0.00000,   261.87056,    74.76535,     0.00000,     0.00000 },
        { 0.00000,     0.00000,   110.63529,     0.00000,     0.00000 }
    };

    auto z_diff = blaze::abs(std::get<0>(r) - z_ref);
    auto albedo1_diff = blaze::abs(std::get<1>(r)[0] - albedo_ref);
    auto albedo_channels_diff = blaze::abs(std::get<1>(r)[1] - std::get<1>(r)[0]) +
                                blaze::abs(std::get<1>(r)[2] - std::get<1>(r)[0]);

//    std::cout << std::endl << "differences:" << std::endl
//              << "z: " << std::endl << z_diff << std::endl
//              << "albedo: " << std::endl << albedo1_diff << std::endl;

    REQUIRE(blaze::max(z_diff) < 0.1);
    REQUIRE(blaze::max(albedo1_diff) < 2);
    REQUIRE(blaze::max(albedo_channels_diff) < std::numeric_limits<TestType>::epsilon() * 1000);

}



TEMPLATE_TEST_CASE("ups: nabla operator matrices", "[utils]", float, double) {

    blaze::CompressedMatrix<unsigned char, blaze::columnMajor> M {
        {0, 0, 1, 1, 0},
        {0, 1, 1, 1, 1},
        {0, 1, 1, 0, 0},
        {0, 0, 1, 0, 0}
    };

    blaze::CompressedMatrix<TestType> nCN_x = {
        {     0,    0,    0,    0,    0,    0,    0,    0,    0},
        {     0,    0,    0,    0,    0,    0,    0,    0,    0},
        {     0,    0,    0,    0,    0,    0,    0,    0,    0},
        {  -0.5,    0,    0,    0,    0,    0,    0,  0.5,    0},
        {     0,    0,    0,    0,    0,    0,    0,    0,    0},
        {     0,    0,    0,    0,    0,    0,    0,    0,    0},
        {     0,    0,    0,    0,    0,    0,    0,    0,    0},
        {     0,    0,    0, -0.5,    0,    0,    0,    0,  0.5},
        {     0,    0,    0,    0,    0,    0,    0,    0,    0}
    };
    blaze::CompressedMatrix<TestType> nCN_y = {
        {     0,    0,    0,    0,    0,    0,    0,    0,    0},
        {     0,    0,    0,    0,    0,    0,    0,    0,    0},
        {     0,    0,    0,    0,    0,    0,    0,    0,    0},
        {     0,    0, -0.5,    0,  0.5,    0,    0,    0,    0},
        {     0,    0,    0, -0.5,    0,  0.5,    0,    0,    0},
        {     0,    0,    0,    0,    0,    0,    0,    0,    0},
        {     0,    0,    0,    0,    0,    0,    0,    0,    0},
        {     0,    0,    0,    0,    0,    0,    0,    0,    0},
        {     0,    0,    0,    0,    0,    0,    0,    0,    0}
    };

    blaze::CompressedMatrix<TestType> nCD_x = {
        {     0,    0,    0,  0.5,    0,    0,    0,    0,    0},
        {     0,    0,    0,    0,  0.5,    0,    0,    0,    0},
        {     0,    0,    0,    0,    0,    0,  0.5,    0,    0},
        {  -0.5,    0,    0,    0,    0,    0,    0,  0.5,    0},
        {     0, -0.5,    0,    0,    0,    0,    0,    0,    0},
        {     0,    0,    0,    0,    0,    0,    0,    0,    0},
        {     0,    0, -0.5,    0,    0,    0,    0,    0,    0},
        {     0,    0,    0, -0.5,    0,    0,    0,    0,  0.5},
        {     0,    0,    0,    0,    0,    0,    0, -0.5,    0}
    };
    blaze::CompressedMatrix<TestType> nCD_y = {
        {     0,  0.5,    0,    0,    0,    0,    0,    0,    0},
        {  -0.5,    0,    0,    0,    0,    0,    0,    0,    0},
        {     0,    0,    0,  0.5,    0,    0,    0,    0,    0},
        {     0,    0, -0.5,    0,  0.5,    0,    0,    0,    0},
        {     0,    0,    0, -0.5,    0,  0.5,    0,    0,    0},
        {     0,    0,    0,    0, -0.5,    0,    0,    0,    0},
        {     0,    0,    0,    0,    0,    0,    0,  0.5,    0},
        {     0,    0,    0,    0,    0,    0, -0.5,    0,    0},
        {     0,    0,    0,    0,    0,    0,    0,    0,    0}
    };

    blaze::CompressedMatrix<TestType> nFD_x = {
        {    -1,    0,    0,    1,    0,    0,    0,    0,    0},
        {     0,   -1,    0,    0,    1,    0,    0,    0,    0},
        {     0,    0,   -1,    0,    0,    0,    1,    0,    0},
        {     0,    0,    0,   -1,    0,    0,    0,    1,    0},
        {     0,    0,    0,    0,   -1,    0,    0,    0,    0},
        {     0,    0,    0,    0,    0,   -1,    0,    0,    0},
        {     0,    0,    0,    0,    0,    0,   -1,    0,    0},
        {     0,    0,    0,    0,    0,    0,    0,   -1,    1},
        {     0,    0,    0,    0,    0,    0,    0,    0,   -1}
    };
    blaze::CompressedMatrix<TestType> nFD_y = {
        {    -1,    1,    0,    0,    0,    0,    0,    0,    0},
        {     0,   -1,    0,    0,    0,    0,    0,    0,    0},
        {     0,    0,   -1,    1,    0,    0,    0,    0,    0},
        {     0,    0,    0,   -1,    1,    0,    0,    0,    0},
        {     0,    0,    0,    0,   -1,    1,    0,    0,    0},
        {     0,    0,    0,    0,    0,   -1,    0,    0,    0},
        {     0,    0,    0,    0,    0,    0,   -1,    1,    0},
        {     0,    0,    0,    0,    0,    0,    0,   -1,    0},
        {     0,    0,    0,    0,    0,    0,    0,    0,   -1}
    };

    blaze::CompressedMatrix<TestType> nBD_x = {
        {     1,    0,    0,    0,    0,    0,    0,    0,    0},
        {     0,    1,    0,    0,    0,    0,    0,    0,    0},
        {     0,    0,    1,    0,    0,    0,    0,    0,    0},
        {    -1,    0,    0,    1,    0,    0,    0,    0,    0},
        {     0,   -1,    0,    0,    1,    0,    0,    0,    0},
        {     0,    0,    0,    0,    0,    1,    0,    0,    0},
        {     0,    0,   -1,    0,    0,    0,    1,    0,    0},
        {     0,    0,    0,   -1,    0,    0,    0,    1,    0},
        {     0,    0,    0,    0,    0,    0,    0,   -1,    1}
    };
    blaze::CompressedMatrix<TestType> nBD_y = {
        {     1,    0,    0,    0,    0,    0,    0,    0,    0},
        {    -1,    1,    0,    0,    0,    0,    0,    0,    0},
        {     0,    0,    1,    0,    0,    0,    0,    0,    0},
        {     0,    0,   -1,    1,    0,    0,    0,    0,    0},
        {     0,    0,    0,   -1,    1,    0,    0,    0,    0},
        {     0,    0,    0,    0,   -1,    1,    0,    0,    0},
        {     0,    0,    0,    0,    0,    0,    1,    0,    0},
        {     0,    0,    0,    0,    0,    0,   -1,    1,    0},
        {     0,    0,    0,    0,    0,    0,    0,    0,    1}
    };


    auto nablas = nablaMat<TestType>(M, Central, NeumannHomogeneous);

//    std::cout << std::endl << "Central, NeumannHomogeneous:" << std::endl
//              << "nabla_x: " << std::endl << std::get<0>(nablas) << std::endl
//              << "nabla_y: " << std::endl << std::get<1>(nablas) << std::endl;
    REQUIRE(std::get<0>(nablas) == nCN_x);
    REQUIRE(std::get<1>(nablas) == nCN_y);


    nablas = nablaMat<TestType>(M, Central, DirichletHomogeneous);

//    std::cout << std::endl << "Central, DirichletHomogeneous:" << std::endl
//              << "nabla_x: " << std::endl << std::get<0>(nablas) << std::endl
//              << "nabla_y: " << std::endl << std::get<1>(nablas) << std::endl;
    REQUIRE(std::get<0>(nablas) == nCD_x);
    REQUIRE(std::get<1>(nablas) == nCD_y);


    nablas = nablaMat<TestType>(M, Forward, DirichletHomogeneous);

//    std::cout << std::endl << "Forward, DirichletHomogeneous:" << std::endl
//              << "nabla_x: " << std::endl << std::get<0>(nablas) << std::endl
//              << "nabla_y: " << std::endl << std::get<1>(nablas) << std::endl;
    REQUIRE(std::get<0>(nablas) == nFD_x);
    REQUIRE(std::get<1>(nablas) == nFD_y);


    nablas = nablaMat<TestType>(M, Backward, DirichletHomogeneous);

//    std::cout << std::endl << "Backward, DirichletHomogeneous:" << std::endl
//              << "nabla_x: " << std::endl << std::get<0>(nablas) << std::endl
//              << "nabla_y: " << std::endl << std::get<1>(nablas) << std::endl;
    REQUIRE(std::get<0>(nablas) == nBD_x);
    REQUIRE(std::get<1>(nablas) == nBD_y);

}



TEMPLATE_TEST_CASE("ups: normals & indexing", "[utils]", float, double) {

    blaze::CompressedMatrix<unsigned char, blaze::columnMajor> M {
        {0, 0, 1, 1, 0},
        {0, 1, 1, 1, 1},
        {0, 1, 1, 0, 0},
        {0, 0, 1, 0, 0}
    };

    blaze::DynamicMatrix<TestType, blaze::columnMajor> Z {
        {0.8, 0.8, 0.8, 0.8, 0.8},
        {0.8, 0.8, 0.8, 0.9, 0.8},
        {0.8, 1.0, 0.8, 0.8, 0.8},
        {0.8, 0.8, 0.8, 0.8, 0.8}};

    blaze::DynamicMatrix<TestType> K {
        {1, 0, 1},
        {0, 1, 1},
        {0, 0, 1}};

    blaze::CompressedMatrix<TestType> nFD_x = {
        {    -1,    0,    0,    1,    0,    0,    0,    0,    0},
        {     0,   -1,    0,    0,    1,    0,    0,    0,    0},
        {     0,    0,   -1,    0,    0,    0,    1,    0,    0},
        {     0,    0,    0,   -1,    0,    0,    0,    1,    0},
        {     0,    0,    0,    0,   -1,    0,    0,    0,    0},
        {     0,    0,    0,    0,    0,   -1,    0,    0,    0},
        {     0,    0,    0,    0,    0,    0,   -1,    0,    0},
        {     0,    0,    0,    0,    0,    0,    0,   -1,    1},
        {     0,    0,    0,    0,    0,    0,    0,    0,   -1}
    };
    blaze::CompressedMatrix<TestType> nFD_y = {
        {    -1,    1,    0,    0,    0,    0,    0,    0,    0},
        {     0,   -1,    0,    0,    0,    0,    0,    0,    0},
        {     0,    0,   -1,    1,    0,    0,    0,    0,    0},
        {     0,    0,    0,   -1,    1,    0,    0,    0,    0},
        {     0,    0,    0,    0,   -1,    1,    0,    0,    0},
        {     0,    0,    0,    0,    0,   -1,    0,    0,    0},
        {     0,    0,    0,    0,    0,    0,   -1,    1,    0},
        {     0,    0,    0,    0,    0,    0,    0,   -1,    0},
        {     0,    0,    0,    0,    0,    0,    0,    0,   -1}
    };

    blaze::DynamicMatrix<TestType> N_unnormalized_ref = {
        {            0,          0.2,         -0.8 },
        {         -0.2,           -1,           -0 },
        {            0,            0,         -0.8 },
        {          0.1,            0,         -0.9 },
        {         -0.8,            0,           -0 },
        {         -0.8,         -0.8,          1.6 },
        {         -0.8,          0.1,          0.9 },
        {         -0.1,         -0.9,         -0.7 },
        {         -0.8,         -0.8,          1.6 }
    };

    blaze::DynamicVector<TestType> dz_ref = {
        0.824621,
          1.0198,
             0.8,
        0.905538,
             0.8,
         1.95959,
          1.2083,
         1.14455,
         1.95959
    };

    blaze::DynamicMatrix<TestType> N_normalized_ref = {
        {            0,     0.242536,    -0.970142 },
        {    -0.196116,    -0.980581,           -0 },
        {            0,            0,           -1 },
        {     0.110431,            0,    -0.993884 },
        {           -1,            0,           -0 },
        {    -0.408248,    -0.408248,     0.816497 },
        {    -0.662085,    0.0827606,     0.744845 },
        {   -0.0873704,    -0.786334,    -0.611593 },
        {    -0.408248,    -0.408248,     0.816497 }
    };

    blaze::DynamicMatrix<TestType> sh4_ref = {
        {            0,     0.118504,    -0.474014,     0.282095 },
        {   -0.0958228,    -0.479114,           -0,     0.282095 },
        {            0,            0,    -0.488603,     0.282095 },
        {    0.0539571,            0,    -0.485614,     0.282095 },
        {    -0.488603,            0,           -0,     0.282095 },
        {    -0.199471,    -0.199471,     0.398942,     0.282095 },
        {    -0.323496,     0.040437,     0.363933,     0.282095 },
        {   -0.0426894,    -0.384205,    -0.298826,     0.282095 },
        {    -0.199471,    -0.199471,     0.398942,     0.282095 }
    };

        blaze::DynamicMatrix<TestType> sh9_ref = {
        {            0,     0.118504,    -0.474014,     0.282095,            0,           -0,     -0.25707,   -0.0321338,     0.575126 },
        {   -0.0958228,    -0.479114,           -0,     0.282095,     0.210105,            0,            0,    -0.504253,    -0.315392 },
        {            0,            0,    -0.488603,     0.282095,            0,           -0,           -0,            0,     0.630783 },
        {    0.0539571,            0,    -0.485614,     0.282095,            0,    -0.119914,           -0,   0.00666188,     0.619244 },
        {    -0.488603,            0,           -0,     0.282095,           -0,            0,           -0,     0.546274,    -0.315392 },
        {    -0.199471,    -0.199471,     0.398942,     0.282095,     0.182091,    -0.364183,    -0.364183,            0,     0.315392 },
        {    -0.323496,     0.040437,     0.363933,     0.282095,   -0.0598657,    -0.538791,    0.0673489,     0.235721,     0.209541 },
        {   -0.0426894,    -0.384205,    -0.298826,     0.282095,    0.0750606,    0.0583805,     0.525424,    -0.333603,    0.0385211 },
        {    -0.199471,    -0.199471,     0.398942,     0.282095,     0.182091,    -0.364183,    -0.364183,            0,     0.315392 }

    };


    std::vector<size_t> z_idc = indicesCwStd(M);
    blaze::DynamicVector<TestType> z_vector = flattenToCol(Z);
    blaze::DynamicVector<TestType> z_vector_masked = blaze::elements(z_vector, z_idc);  // Matlab z = z(mask);

    blaze::DynamicVector<TestType> zx = nFD_x * z_vector_masked;  // nabla applied via matrix multiplication
    blaze::DynamicVector<TestType> zy = nFD_y * z_vector_masked;

    auto xxyy = indices2dCw(M);
    blaze::DynamicVector<TestType> xx = std::get<0>(xxyy) - K(0, 2);
    blaze::DynamicVector<TestType> yy = std::get<1>(xxyy) - K(1, 2);

    blaze::DynamicMatrix<TestType> N_unnormalized = pixNormals(z_vector_masked, zx, zy, xx, yy, K);
    blaze::DynamicVector<TestType> dz = blaze::sqrt(blaze::sum<blaze::rowwise>(N_unnormalized % N_unnormalized)); // TODO compare to Eps if needed
    blaze::DynamicMatrix<TestType> N_normalized = normalizePixNormals(N_unnormalized, dz);

//    std::cout << std::endl << "normals:" << std::endl
//              << "N_unnormalized: " << std::endl << N_unnormalized << std::endl
//              << "dz: " << std::endl << dz << std::endl
//              << "N_normalized: " << std::endl << N_normalized << std::endl;

    REQUIRE(blaze::sum(blaze::abs(N_unnormalized - N_unnormalized_ref)) < std::numeric_limits<TestType>::epsilon() * 100000);
    REQUIRE(blaze::sum(blaze::abs(dz - dz_ref)) < 0.0005);
    REQUIRE(blaze::sum(blaze::abs(N_normalized - N_normalized_ref)) < 0.00001);


    blaze::DynamicMatrix<TestType> normals_theta (N_unnormalized.rows(), N_unnormalized.columns());
    for (size_t c = 0; c < N_unnormalized.columns(); ++c) {
        blaze::column(normals_theta, c) = blaze::column(N_unnormalized, c) / dz;
    }

    auto sh_lo = normalsToSh(normals_theta, ho_low);
    auto sh_hi = normalsToSh(normals_theta, ho_high);

//    std::cout << std::endl << "augmented normals:" << std::endl
//              << "sh 4: " << std::endl << sh_lo << std::endl
//              << "sh 9: " << std::endl << sh_hi << std::endl;

    REQUIRE(blaze::sum(blaze::abs(sh_lo - sh4_ref)) < 0.0005);
    REQUIRE(blaze::sum(blaze::abs(sh_hi - sh9_ref)) < 0.0005);
}
