#include "ImageProcess.h"

Mat ImageProcess::ConvertToMat(IStImage* pImage)
{
	{
		// Check the pixelformat of the input image
		const EStPixelFormatNamingConvention_t ePFNC = pImage->GetImagePixelFormat();
		const IStPixelFormatInfo* const pPixelFormatInfo = GetIStPixelFormatInfo(ePFNC);
		if (pPixelFormatInfo->IsMono() || pPixelFormatInfo->IsBayer())
		{
			// Check the size of the image
			//const size_t nImageWidth = pImage->GetImageWidth();
			//const size_t nImageHeight = pImage->GetImageHeight();
			const int width = static_cast<int>(pImage->GetImageWidth());
			const int height = static_cast<int>(pImage->GetImageHeight());
			int type = CV_8UC1;
			if (pPixelFormatInfo->GetEachComponentTotalBitCount() > 8)
			{
				type = CV_16UC1;
			}

			//Mat mat(nImageHeight, nImageWidth, type);
			Mat mat(height, width, type);
			const size_t bufferSize = mat.rows * mat.cols * mat.elemSize() * mat.channels();
			memcpy(mat.ptr(), pImage->GetImageBuffer(), bufferSize);

			// Convert the pixelformat if needed
			if (pPixelFormatInfo->IsBayer())
			{
				int code = 0;
				switch (pPixelFormatInfo->GetPixelColorFilter())
				{
				case(StPixelColorFilter_BayerRG):	code = COLOR_BayerRG2RGB;	break;
				case(StPixelColorFilter_BayerGR):	code = COLOR_BayerGR2RGB;	break;
				case(StPixelColorFilter_BayerGB):	code = COLOR_BayerGB2RGB;	break;
				case(StPixelColorFilter_BayerBG):	code = COLOR_BayerBG2RGB;	break;
				}
				if (code != 0)
				{
					cvtColor(mat, mat, code);
				}
			}
			return mat;
		}
		return Mat();
	}
}
