#pragma once
#include <StApi_TL.h>	// TL : Transport Layer
#include <StApi_IP.h>	// IP : Image Processing

#include "CameraConfigurator.h"

using namespace StApi;

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

/*
@brief Basic Camera Worker Class
@brief This class is responsible for managing camera operations such as initialization, image acquisition, and saving images in various formats.
*/
class BasicCamera
{
public:
	/*
	@brief Class constructor
	@param imageCount : Number of images to capture
	*/
	explicit BasicCamera(uint64_t imageCount = 100);
	/* @brief Class destructor */
	~BasicCamera();
	
	/* @brief Initialize camera settings */
	bool Initialize(const CIStSystemPtr& pSystem);
	/* @brief Start image acquisition method */
	void StartAcquisition();
	/* @brief Stop image acquisition method */
	void StopAcquisition();
	/*
	@brief Save image to file wrapper
	@param dstDir : Directory path where the image will be saved
	*/
	void SaveImageToFile(const std::string& dstDir);

	/*
	@brief Convert and save image
	@param pSrcImage : Source image pointer to be converted and saved
	@param isColor : Whether the image is in color format
	@param dstDir : Directory path where the image will be saved
	@param frameID : Frame ID of the image to be saved
	@tparam FORMAT : Image format type for saving (e.g., StApiRaw, BMP, TIFF, PNG, JPEG, CSV)
	*/
	template<typename FORMAT>
	void ConvertAndSaveImage(IStImage* pSrcImage, bool isColor, std::string dstDir, const uint64_t frameID);

protected:
	void PrintFrameInfo(const IStImage* pImage, CIStStreamBufferPtr& pStreamBuffer);
	/*
	@brief Display frame information
	@param pImage : Image pointer containing frame data
	@param frameID : Frame ID of the image
	*/
	void PrintFrameInfo(const IStImage* pImage, const uint64_t frameID);
	/*
	@brief Load saved image from a directory
	@param pImageBuffer : Image buffer pointer to load the image into
	@param srcDir : Directory path from which image comes
	*/
	void LoadSavedImage(CIStImageBufferPtr& pImageBuffer, const GenICam::gcstring& srcDir);
	
	/*
	@brief system object
	@brief CIStSystemPtr is a pointer to the system object that manages the camera and its resources.
	*/
	CIStSystemPtr m_pSystem;
	/*
	@brief camera device object
	@brief CIStDevicePtr is a pointer to the camera device object that represents the physical camera.
	*/
	CIStDevicePtr m_pDevice;
	/*
	@brief datastream object
	@brief CIStDataStreamPtr is a pointer to the data stream object that handles the image data flow from the camera.
	@brief datastream is used to retrieve image buffers from the camera.
	*/
	CIStDataStreamPtr m_pDataStream;
	IStImage* m_pImage;	// Pointer to the current image being processed
	uint64_t m_frameID;	// Frame ID of the current image

private:
	/* @brief Sequential image capture function */
	void SequentialCapture();
	/*
	@brief Set save path for images
	@param savePath: directory path where images will be saved
	@param frameID : frame ID of the image to be saved
	@return : absolute path where the image will be saved
	*/
	GenICam::gcstring SetSavePath(const std::string& savePath, const uint64_t frameID);
	/*
	@brief pixel format conversion function
	@param pSrcImage : source image pointer to be converted
	@param isColor : whether the image is in color format
	@param pDstBuffer : destination image buffer pointer to hold the converted image
	*/
	void ConvertPixelFormat(IStImage* pSrcImage, bool isColor, CIStImageBufferPtr& pDstBuffer);
	/*
	@brief save image function
	@param pImageBuffer : image buffer pointer containing the image to be saved
	@param dstDir : directory path where the image will be saved
	@tparam FORMAT : image format type for saving (e.g., StApiRaw, BMP, TIFF, PNG, JPEG, CSV)
	*/
	template<typename FORMAT>
	void SaveImage(CIStImageBufferPtr& pImageBuffer, GenICam::gcstring& dstDir);

	uint64_t m_imageCount;	// Number of images to capture
};