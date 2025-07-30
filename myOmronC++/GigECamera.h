#pragma once

#include <StApi_TL.h>

#include "ThreadSafeQueue.h"
#include "ImageProcess.h"
#include "GigEUtil.h"
#include "NodeMapUtil.h"
#include "config.h"
#include "Logger.h"
#ifdef _WIN32
#include <windows.h>  // for Sleep
#else
#include <unistd.h>   // for sleep/usleep
#endif

using namespace StApi;

class GigECamera
{
public:
	explicit GigECamera(std::string saveRootDir, std::shared_ptr<Logger> logger);
	~GigECamera();

	bool Initialize(IStInterface* pInterface, uint32_t iFaceDeviceIdx);
	void StartAcquisition();
	void StopAcquisition();

	void SequentialCapture();

	void ExecuteTrigger();

	void SetFrameQueue(std::shared_ptr<ThreadSafeQueue<FrameData>> pFrameQueue);

	const std::string& GetUserDefinedName();
	const std::string& GetSerialNumber();

protected:

private:
	static void OnStCallbackMethod(IStCallbackParamBase* pIStCallbackParamBase, void* pvContext);
	void OnCallback(IStCallbackParamBase* pCallbackParam);

	IStInterface* m_pInterface; // GigE interface pointer
	CIStDevicePtr m_pDevice; // Camera device pointer
	CIStDataStreamPtr m_pDataStream; // Data stream pointer

	GenApi::CCommandPtr pICommandTriggerSoftware;
	std::shared_ptr<ThreadSafeQueue<FrameData>> m_pFrameQueue;

	std::string m_strSaveRootDir;
	std::string m_strSerialNumber;
	std::string m_strUserDefinedName;

	std::shared_ptr<Logger> m_logger; // Logger for logging camera events
};