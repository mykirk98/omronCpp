#pragma once

#include "ImageSaveQueue.h"
#include <thread>
#include <vector>
#include <atomic>
#include <string>
#include <StApi_IP.h>

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
@brief ImageSaverThreadPool class
@brief This class manages a pool of threads for saving images.
*/
class ImageSaverThreadPool
{
public:
	/*
	@brief ImageSaverThreadPool constructor
	@param threadCount : Number of threads in the pool
	@brief saveRootDir : Root directory where images will be saved
	@brief convertToColor : Flag to indicate whether to convert images to color format
	*/
	ImageSaverThreadPool(size_t threadCount, const std::string& saveRootDir, bool convertToColor = false);
	/* @brief ImageSaverThreadPool destructor */
	~ImageSaverThreadPool();

	/* @brief Start the thread pool */
	void Start();
	/* @brief Stop the thread pool */
	void Stop();
	/*
	@brief Enqueue an image frame for saving
	@param frame : FrameData object containing image data and metadata
	*/
	void Enqueue(const FrameData& frame);

private:
	/* @brief WorkerLoop function */
	void WorkerLoop();
	/*
	@brief Convert pixel format of the image to the buffer
	@param pSrcImage : Source image pointer to be converted
	@param isColor : Flag that decides whether the image is saved in color or mono format
	*/
	void ConvertPixelFormat(IStImage* pSrcImage, bool isColor, CIStImageBufferPtr& pDstBuffer);
	/*
	@brief Set the save path for the image
	@param baseDir : Base directory where images will be saved
	@param cameraName : Name of the camera
	@param frameID : Frame ID of the image to be saved
	*/
	GenICam::gcstring SetSavePath(const std::string& baseDir, const std::string& cameraName, const uint64_t frameID);
	template<typename FORMAT>
	void SaveImage(CIStImageBufferPtr& pImageBuffer, GenICam::gcstring& dstDir)
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

	/* @brief Thread pool for saving images */
	std::vector<std::thread> m_workers;
	/* @brief ImageSaveQueue object for managing image frames */
	ImageSaveQueue m_queue;\
	/* @brief Flag to indicate whether the thread pool is running */
	std::atomic<bool> m_running;
	/* @brief Root directory where images will be saved */
	std::string m_saveRootDir;
	/* @brief Flag to indicate whether to convert images to color format */
	bool m_convertToColor;
};