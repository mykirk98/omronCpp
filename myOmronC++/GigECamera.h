#pragma once

#include <StApi_TL.h>

#include "GigEConfigurator.h"
#include "ImageSaverThreadPool.h"

using namespace StApi;

class GigECamera
{
public:
	explicit GigECamera();
	~GigECamera();

	bool Initialize(IStInterface* pInterface, uint32_t interfaceDeviceIndex);
	
	void StartAcquisition(uint64_t imageCount);

	void StopAcquisition();

	void SequentialCapture();

	void PrintFrameInfo(const CIStStreamBufferPtr& pStreamBuffer);

	void SetThreadPool(std::shared_ptr<ImageSaverThreadPool> pThreadPool);

protected:

private:
	IStInterface* m_pInterface; // GigE interface pointer
	CIStDevicePtr m_pDevice; // Camera device pointer
	CIStDataStreamPtr m_pDataStream; // Data stream pointer

	std::string m_saveRootDir; // Directory to save images
	std::shared_ptr<ImageSaverThreadPool> m_pThreadPool; // Thread pool for saving images
};

