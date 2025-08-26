#include "ImageProcess.h"

std::string ImageProcess::PrintFrameInfo(const CIStStreamBufferPtr& pStreamBuffer, std::string userDefinedName)
{
	try
	{
		std::string info = "[" + userDefinedName + "] " +
			"Block ID: " + std::to_string(pStreamBuffer->GetIStStreamBufferInfo()->GetFrameID()) +
			"\tSize: " + std::to_string(pStreamBuffer->GetIStImage()->GetImageWidth()) + " x " + std::to_string(pStreamBuffer->GetIStImage()->GetImageHeight()) +
			"\tFirst byte: " + std::to_string(static_cast<uint32_t>(*reinterpret_cast<uint8_t*>(pStreamBuffer->GetIStImage()->GetImageBuffer()))) +
			"\ttime stamp: " + std::to_string(pStreamBuffer->GetIStStreamBufferInfo()->GetTimestamp());

		return info;
	}
	catch (const GenICam::GenericException& e)
	{
		std::cerr << "[GigECamera] Printing frame info error: " << e.GetDescription() << std::endl;
	}
}

GenICam::gcstring ImageProcess::SetSavePath(const std::string& baseDir, const std::string& cameraName, const std::string& serialNumber, const uint64_t frameID)
{
	try
	{
		// Change frameID to string
		std::string strFrameID = std::to_string(frameID);

#ifdef _WIN32
		std::string filePath = baseDir + cameraName + "\\" + serialNumber + "-" + strFrameID;
#else
		std::string filePath = baseDir + cameraName + "/" + serialNumber + "-" + strFrameID;
#endif

		return GenICam::gcstring(filePath.c_str());
	}
	catch (const GenICam::GenericException& e)
	{
		std::cerr << "[ImageSaverThreadPool] Setting save path error: " << e.GetDescription() << std::endl;
	}
	return GenICam::gcstring();
}

void ImageProcess::ConvertPixelFormat(IStImage* pSrcImage, bool isMono, CIStImageBufferPtr& pDstBuffer)
{
	try
	{
		// Create a data converter object for pixel format conversion.
		CIStPixelFormatConverterPtr pPixelFormatConverter(CreateIStConverter(StConverterType_PixelFormat));
		if (isMono)
		{
			pPixelFormatConverter->SetDestinationPixelFormat(StPFNC_Mono8);
		}
		else
		{
			pPixelFormatConverter->SetDestinationPixelFormat(StPFNC_BGR8);
		}
		// Convert the pixel format of the source image to the destination buffer.
		pPixelFormatConverter->Convert(pSrcImage, pDstBuffer);
	}
	catch (const GenICam::GenericException& e)
	{
		std::cerr << "[ImageSaverThreadPool] Converting pixel format error: " << e.GetDescription() << std::endl;
	}
}

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