#pragma once
#include <StApi_TL.h>	// TL : Transport Layer

#include "ImageProcess.h"
#include "NodeMapUtil.h"
#include "ImageSaverThreadPool.h"
#include "config.h" 
#include "CSVWriter.h"

using namespace StApi;

/* @brief This class provides basic camera functionalities such as initialization, image acquisition, and sequential capture. */
class BasicCamera
{
public:
	/* @brief BasicCamera constructor */
	explicit BasicCamera(std::string rootDir);
	/* @brief BasicCamera destructor */
	~BasicCamera();
	
	/* @brief Initialize camera settings */
	bool Initialize(const CIStSystemPtr& pSystem);
	/* @brief Start image acquisition method */
	void StartAcquisition(uint64_t imageCount);
	/* @brief Stop image acquisition method */
	void StopAcquisition();
	/* @brief Sequential image capture function */
	void FreeRunCapture0();
	void FreeRunCapture1();

protected:
	/* @brief system object */
	//CIStSystemPtr m_pSystem;
	/* @brief camera device object */
	CIStDevicePtr m_pDevice;
	/* @brief datastream object */
	CIStDataStreamPtr m_pDataStream;

private:
	std::string m_strRootDir;
	std::unique_ptr<CSVWriter> m_csv;
	std::string m_strCameraName;
	std::string m_strSerialNumber;
	//std::shared_ptr<ImageSaverThreadPool> m_pThreadPool;
};