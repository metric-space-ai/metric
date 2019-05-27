#include "encoder.hpp"


namespace linear_compressor
{



template <class BlazeMatrix>
blaze::DynamicMatrix<double> PCA(const BlazeMatrix In, int n_components, bool visualize)
{
    auto Result = blaze::DynamicMatrix<double>(n_components, In.rows(), 0);

    auto avgs = blaze::sum<blaze::rowwise>(In) / In.columns();
    auto CenteredInput = blaze::DynamicMatrix<double>(In.rows(), In.columns(), 0);
    for (size_t col = 0; col<In.columns(); col++)
        column(CenteredInput, col) = column(In, col) - avgs;

    blaze::SymmetricMatrix<blaze::DynamicMatrix<double>> CovMat = blaze::evaluate(CenteredInput * trans(CenteredInput));

    //if (visualize)
    //{
        //mat2bmp::blaze2bmp(CenteredInput, "CenteredInput.bmp");
        //mat2bmp::blaze2bmp_norm(CovMat, "CovMat.bmp");
    //}

    blaze::DynamicVector<double, blaze::columnVector> w( In.rows() ); // for eigenvalues
    blaze::DynamicMatrix<double, blaze::rowMajor> V( In.rows(), In.rows() );  // for eigenvectors

    eigen( CovMat, w, V );

    //if (visualize)
        //mat2bmp::blaze2bmp_norm(V, "V_raw.bmp");

    for (size_t row = 0; row<V.rows(); row++)
        blaze::row(V, row) = blaze::row(V, row) * w[row];

    //if (visualize)
        //mat2bmp::blaze2bmp_norm(V, "V_ready.bmp");

    // sort and select
    size_t lower_idx = 0;
    size_t upper_idx = w.size() - 1;
    double spectral_radius; // also we get spectral radius for normalization: we process the first eigenvalue specially
    if ((-w[lower_idx] > w[upper_idx]))
        spectral_radius = w[lower_idx];
    else
        spectral_radius = w[upper_idx];
    int count = 0;
    while (count < n_components && upper_idx > lower_idx)
    {
        if (-w[lower_idx] > w[upper_idx])
        {
            blaze::row(Result, count) = blaze::row(V, lower_idx) / spectral_radius; // add eigenpair
            lower_idx++;
        }
        else
        {
            blaze::row(Result, count) = blaze::row(V, upper_idx) / spectral_radius; // add eigenpair
            upper_idx--;
        }
        count++;
    }

    //if (visualize)
        //mat2bmp::blaze2bmp_norm(Result, "Result.bmp");

    return Result;
}




// simple linear encoder based on PCA

DirectMapping::DirectMapping(bool visualize_)
{
    visualize = visualize_;
}

void DirectMapping::train(const blaze::DynamicMatrix<double> & Slices, size_t n_features)
{

    W_encode = linear_compressor::PCA(Slices, n_features, visualize);

    auto encoded = compress(Slices);

    W_decode = trans(W_encode);

    //if (visualize)
    //{
        //mat2bmp::blaze2bmp_norm(W_encode, "W_encode.bmp");
        //mat2bmp::blaze2bmp_norm(encoded, "encoded_training_dataset.bmp");
        //mat2bmp::blaze2bmp_norm(W_decode, "W_decode.bmp");
        // //std::cout << "W_encode:\n" << W_encode << "\n";
        // //std::cout << "W_decode:\n" << W_decode << "\n";
    //}
}


blaze::DynamicMatrix<double> DirectMapping::compress(
        const blaze::DynamicMatrix<double> & Slices
        )
{
    return W_encode * Slices;
    //return evaluate(W_encode * Slices);
}


blaze::DynamicMatrix<double> DirectMapping::decompress(
        const blaze::DynamicMatrix<double> & Codes
        )
{
    return W_decode * Codes;
}



} // namespace linear_compressor

