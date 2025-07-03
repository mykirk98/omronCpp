#pragma once
#include <StApi_TL.h>	// TL : Transport Layer

#include "CameraConfigurator.h"
#include "ImageSaverThreadPool.h"

using namespace StApi;

/* @brief This class is responsible for managing camera operations such as initialization, image acquisition, and sequential capture. */
class BasicCamera
{
public:
	/*
	@brief BasicCamera constructor
	@param imageCount : Number of images to capture
	*/
	explicit BasicCamera(uint64_t imageCount = 100);
	/* @brief BasicCamera destructor */
	~BasicCamera();
	
	/* @brief Initialize camera settings */
	bool Initialize(const CIStSystemPtr& pSystem);
	/* @brief Start image acquisition method */
	void StartAcquisition();
	/* @brief Stop image acquisition method */
	void StopAcquisition();
	/* @brief Sequential image capture function */
	void SequentialCapture();

protected:
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
	
	/* @brief system object */
	CIStSystemPtr m_pSystem;
	/* @brief camera device object */
	CIStDevicePtr m_pDevice;
	/* @brief datastream object */
	CIStDataStreamPtr m_pDataStream;

private:
	uint64_t m_imageCount;	// Number of images to capture
	std::string m_saveRootDir;
};