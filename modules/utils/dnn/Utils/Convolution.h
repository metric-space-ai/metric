#ifndef UTILS_CONVOLUTION_H_
#define UTILS_CONVOLUTION_H_


namespace dnn
{
	namespace internal
	{
		struct ConvDims
		{
			// Input parameters
			const int in_channels;
			const int out_channels;
			const int channel_rows;
			const int channel_cols;
			const int filter_rows;
			const int filter_cols;
			// Image dimension -- one observation with all channels
			const int img_rows;
			const int img_cols;
			// Dimension of the convolution result for each output channel
			const int conv_rows;
			const int conv_cols;

			ConvDims(
				const int in_channels_, const int out_channels_,
				const int channel_rows_, const int channel_cols_,
				const int filter_rows_, const int filter_cols_
			) :
				in_channels(in_channels_), out_channels(out_channels_),
				channel_rows(channel_rows_), channel_cols(channel_cols_),
				filter_rows(filter_rows_), filter_cols(filter_cols_),
				img_rows(channel_rows_), img_cols(in_channels_ * channel_cols_),
				conv_rows(channel_rows_ - filter_rows_ + 1),
				conv_cols(channel_cols_ - filter_cols_ + 1)
			{}
		};
	} // namespace internal
} // namespace dnn


#endif /* UTILS_CONVOLUTION_H_ */
