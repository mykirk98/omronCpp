#pragma once
#include <StApi_TL.h>	// TL : Transport Layer

#include "NodeMapUtil.h"
#include "ImageSaverThreadPool.h"

using namespace StApi;

/* @brief This class provides basic camera functionalities such as initialization, image acquisition, and sequential capture. */
class BasicCamera
{
public:
	/* @brief BasicCamera constructor */
	explicit BasicCamera();
	/* @brief BasicCamera destructor */
	~BasicCamera();
	
	/* @brief Initialize camera settings */
	bool Initialize(const CIStSystemPtr& pSystem);
	/* @brief Start image acquisition method */
	void StartAcquisition(uint64_t imageCount);
	/* @brief Stop image acquisition method */
	void StopAcquisition();
	/* @brief Sequential image capture function */
	void SequentialCapture();

	void SetThreadPool(std::shared_ptr<ImageSaverThreadPool> pThreadPool);

protected:
	/*
	@brief Display frame information
	@param pImage : Image pointer containing frame data
	@param frameID : Frame ID of the image
	*/
	void PrintFrameInfo(const CIStStreamBufferPtr& pStreamBuffer);
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
	std::string m_saveRootDir;
	std::shared_ptr<ImageSaverThreadPool> m_pThreadPool;
};