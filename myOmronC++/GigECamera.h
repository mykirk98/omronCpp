#pragma once

#include <StApi_TL.h>
#include <StApi_IP.h>

#include "FrameQueue.h"
#include "ImageProcess.h"
#include "GigEUtil.h"
#include "NodeMapUtil.h"
#include "config.h"
#ifdef _WIN32
#include <windows.h>  // for Sleep
#else
#include <unistd.h>   // for sleep/usleep
#endif


using namespace StApi;

class GigECamera
{
public:
	explicit GigECamera(std::string saveRootDir);
	~GigECamera();

	bool Initialize(IStInterface* pInterface, uint32_t interfaceDeviceIndex);
	void StartAcquisition();
	void StopAcquisition();

	void SequentialCapture();

	void SetFrameQueue(std::shared_ptr<FrameQueue> pFrameQueue);

	GenApi::CCommandPtr pICommandTriggerSoftware;

	const std::string& GetCameraName() const { return m_cameraName; }

protected:

private:
	static void OnStCallbackMethod(IStCallbackParamBase* pIStCallbackParamBase, void* pvContext);
	void OnCallback(IStCallbackParamBase* pCallbackParam);

	IStInterface* m_pInterface; // GigE interface pointer
	CIStDevicePtr m_pDevice; // Camera device pointer
	CIStDataStreamPtr m_pDataStream; // Data stream pointer

	std::string m_saveRootDir; // Directory to save images
	GenICam::gcstring m_serialNumber;
	std::shared_ptr<FrameQueue> m_queue;
	std::string m_cameraName;
};

