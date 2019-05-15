#include "esn.hpp"



namespace esn
{



// ---------------------------------  math functions:


blaze::DynamicMatrix<double> get_readout_no_echo( // for ESN with disables echo
        const blaze::DynamicMatrix<double> & Slices,
        const blaze::DynamicMatrix<double> & W_in
        )
{
    size_t slice_size = W_in.columns();
    assert(slice_size > 0);
    slice_size = slice_size - 1; // size of input vector. first column is for offset and will be used separately

    assert(Slices.rows() == slice_size);

    auto W_in_submatrix = submatrix(W_in, 0UL, 1UL, W_in.rows(), slice_size); // all except first column
    auto w_in_offset = submatrix(W_in, 0UL, 0UL, W_in.rows(), 1UL); // first column
    auto Ones = blaze::DynamicMatrix<double>(w_in_offset.columns(), Slices.columns(), 1);

    return evaluate(tanh(W_in_submatrix * Slices + w_in_offset * Ones));
}




blaze::DynamicMatrix<double> get_readout( // echo mode
        const blaze::DynamicMatrix<double> & Slices,
        const blaze::DynamicMatrix<double> & W_in,
        const blaze::DynamicMatrix<double> & W, // TODO make sparse
        double alpha, // = 0.5,
        size_t washout // = 0
        )
{

    if (washout == 0 && alpha == 1) // if echo disabled, we run faster overload without sample loop
        return get_readout_no_echo(Slices, W_in);

    size_t x_size = W.rows(); // size of inter-step echo buffer
    assert(x_size == W.columns()); // W must be square

    size_t slice_size = W_in.columns();
    assert(slice_size > 0);
    slice_size = slice_size - 1; // size of input vector. first column is for offset and will be used separately

    assert(Slices.rows() == slice_size);

    auto x = blaze::DynamicMatrix<double>(x_size, 1UL, 0.0); // matrix type is due to impossibility of addition of matrix and vector
    // TODO set initial random values, if needed

    //  here we only define symbolyc expressions, all computations are deferred
    auto W_in_submatrix = submatrix(W_in, 0UL, 1UL, W_in.rows(), slice_size); // all except first column
    auto w_in_offset = submatrix(W_in, 0UL, 0UL, W_in.rows(), 1UL); // first column

    size_t n = 0;
    blaze::DynamicMatrix<double> current_slice = columns(Slices, {n}); // will be updated inside loop. Type is set in order to force evaluation
    // TODO refactor: try n of vector type
    auto input_summand = W_in_submatrix * current_slice + w_in_offset; // define all symbolic expressions out of loop (but dependent on n ones must be updated)
    auto x_prev_summand = W * x;

    assert(Slices.columns() > washout); // TODO consider >= x_size in order to avoid undetermined system
    auto Output = blaze::DynamicMatrix<double>(x_size, Slices.columns() - washout);

    for  (n=0; n<Slices.columns(); n++)
    {
//        std::cout << "new iter" << "\n";
//        std::cout << "x:\n" << x << "\n";

        //std::cout << "current_slice before udate:\n" << current_slice << "\n";
        current_slice = columns(Slices, {n}); // update for each n // TODO consider making n also Blaze expression?
        // std::cout << "current_slice updated:\n" << current_slice << "\n";

//        std::cout << "input_summand:\n" << input_summand << "\n";
//        std::cout << "x_prev_summand:\n" << x_prev_summand << "\n";

        x = evaluate(tanh(input_summand + x_prev_summand) * alpha + x * (1 - alpha));

        if (n >= washout)
            columns(Output, {n - washout}) = x; // we output all readout

//        std::cout << "new_x:\n" << x << "\n";
    }

//    std::cout << "Slices:\n" << Slices << "\n";

    return Output;
}



blaze::DynamicMatrix<double> ridge(
        const blaze::DynamicMatrix<double> & Target,
        const blaze::DynamicMatrix<double> & Readout,
        double beta // = 0.5
        )
{
    auto I = blaze::IdentityMatrix<double>(Readout.rows());
    auto TR = trans(Readout);
    return Target * TR * inv(Readout*TR + I*beta);
}






// -------------------------------- caller:




    void ESN::create_W(size_t w_size, double w_connections, double w_sr)
    {
        W = blaze::CompressedMatrix<double>(w_size, w_size, 0.0); // TODO make sparse

        if (w_sr > 0)
        {
            // TODO disable the following W fullfilling code if considered to use graph
            auto uniform_int = std::uniform_int_distribution<int>(0, w_size-1);
            auto uniform_double = std::uniform_real_distribution<double>(-1, 1);
            int count;
            size_t r_row, r_col;
            for (r_col = 0; r_col < W.columns(); r_col++)
                for (count = 0; count < w_connections; count++)
                {
                    r_row = uniform_int(rgen);
                    if (W.find(r_row, r_col) == W.end(r_row)) // find(..) works for compressed matrix only
                        W.insert(r_row, r_col, uniform_double(rgen));
                    else
                        count--; // retry
                }

            // spectral radius normalization
            blaze::DynamicVector<blaze::complex<double>, blaze::columnVector> eig(0UL, 0.0);

            blaze::DynamicMatrix<double> Wd = W; // DynamicMatrix needed for eigen call // TODO disable this line if considered to use graph
            // TODO enable the following 3 lines if considered to use graph
            //auto graph = metric::graph::RandomUniform<double, false>(w_size, -1, 1, w_connections);
            //blaze::DynamicMatrix<double> Wd = graph.get_matrix(); // DynamicMAtrix needed for eigen call
            //W = Wd;

            eigen(Wd, eig);
            auto sr = max(sqrt(pow(real(eig), 2) + pow(imag(eig), 2)));
            W = W * w_sr / sr;
        }
    }



    ESN::ESN(
            size_t w_size, // = 500, // number of elements in reservoir
            double w_connections, // = 10, // number of interconnections (for each reservoir element)
            double w_sr, // = 0.6, // desired spectral radius of the reservoir
            double alpha_, // = 0.5, // leak rate, number of slices excluded from output for washout
            size_t washout_, // = 1,
            double beta_ // = 0.5, // ridge solver metaparameter
            ) : alpha(alpha_), beta(beta_), washout(washout_)
    {
        assert(w_connections/(float)w_size < 0.5);
        create_W(w_size, w_connections, w_sr);
    }



    void ESN::train(
            const blaze::DynamicMatrix<double> & Slices,
            const blaze::DynamicMatrix<double> & Target
            )
    {

        size_t in_size = Slices.rows();

        auto uniform_double = std::uniform_real_distribution<double>(-1, 1);
        size_t r_row, r_col;
        W_in = blaze::DynamicMatrix<double>(W.rows(), in_size+1, 0.0);
        for (r_row=0; r_row<W.rows(); r_row++)
            for (r_col=0; r_col<=in_size; r_col++)
            {
                assert(r_row<W_in.rows() && r_col<W_in.columns());
                W_in(r_row, r_col) = uniform_double(rgen);
            }

        blaze::DynamicMatrix<double> Readout = esn::get_readout(
                    Slices, // input signal
                    W_in, // input weights
                    W, // reservoir internal weights (square matrix)
                    alpha, washout // leak rate, number of slices excluded from output for washout
                    );

        W_out = esn::ridge(
                    submatrix(Target, 0UL, washout, Target.rows(), Target.columns() - washout),
                    Readout,
                    beta
                    );
        trained = true;

    }




    blaze::DynamicMatrix<double> ESN::predict(
            const blaze::DynamicMatrix<double> & Slices
            )
    {
        assert(trained);

        blaze::DynamicMatrix<double> Readout = esn::get_readout(
                    Slices, // input signal
                    W_in, // input weights
                    W, // reservoir internal weights (square matrix)
                    alpha, washout // leak rate, number of slices excluded from output for washout
                    );

        return W_out * Readout;
    }




} // namespace esn

