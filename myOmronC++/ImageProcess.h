#pragma once
#include <STApi_TL.h>
#include <StApi_IP.h>
#include <opencv2/opencv.hpp>

using namespace StApi;
using namespace cv;

struct StApiRaw
{
	static constexpr const char* extension = ".StApiRaw";
	static constexpr EStStillImageFileFormat_t fileFormat = StStillImageFileFormat_StApiRaw;
};
struct BMP
{
	static constexpr const char* extension = ".bmp";
	static constexpr EStStillImageFileFormat_t fileFormat = StStillImageFileFormat_Bitmap;
};
struct TIFF
{
	static constexpr const char* extension = ".tif";
	static constexpr EStStillImageFileFormat_t fileFormat = StStillImageFileFormat_TIFF;
};
struct PNG
{
	static constexpr const char* extension = ".png";
	static constexpr EStStillImageFileFormat_t fileFormat = StStillImageFileFormat_PNG;
};
struct JPEG
{
	static constexpr const char* extension = ".jpg";
	static constexpr EStStillImageFileFormat_t fileFormat = StStillImageFileFormat_JPEG;
};
struct CSV
{
	static constexpr const char* extension = ".csv";
	static constexpr EStStillImageFileFormat_t fileFormat = StStillImageFileFormat_CSV;
};

class ImageProcess
{
public:
	/*	@brief Print the frame information of the image
	@param pStreamBuffer : Stream buffer containing the image data
	@param cameraName : Name of the camera */
	static void PrintFrameInfo(const CIStStreamBufferPtr& pStreamBuffer, std::string cameraName);
	/*	@brief Set the save path for the image
	@param baseDir : Base directory where images will be saved
	@param cameraName : Name of the camera
	@param serialNumber : Serial number of the camera
	@param frameID : Frame ID of the image to be saved
	@return : Returns the full path where the image will be saved */
	static GenICam::gcstring SetSavePath(const std::string& baseDir, const std::string& cameraName, const std::string& serialNumber, const uint64_t frameID);
	/*	@brief Convert pixel format of the image to the buffer
	@param pSrcImage : Source image pointer to be converted
	@param isMono : Flag indicating if the image is monochrome
	@param pDstBuffer : buffer to store the converted image	*/
	static void ConvertPixelFormat(IStImage* pSrcImage, bool isMono, CIStImageBufferPtr& pDstBuffer);

	/* @brief Save the image in the specified format
	@param pImageBuffer : Image buffer containing the image data
	@param dstDir : Destination directory where the image will be saved */
	template<typename FORMAT>
	static void SaveImage(CIStImageBufferPtr& pImageBuffer, GenICam::gcstring& dstDir)
	{
		try
		{
			dstDir.append(FORMAT::extension);

			CIStStillImageFilerPtr pStillImageFiler(CreateIStFiler(StFilerType_StillImage));
			pStillImageFiler->Save(pImageBuffer->GetIStImage(), FORMAT::fileFormat, dstDir);
		}
		catch (const GenICam::GenericException& e)
		{
			std::cerr << "[ImageSaverThreadPool] Saving image error: " << e.GetDescription() << std::endl;
		}
	}
	/* @brief Convert IStImage to cv::Mat
	@param pImage : Pointer to the IStImage to be converted
	@return cv::Mat object containing the image data */
	static Mat ConvertToMat(IStImage* pImage);
};