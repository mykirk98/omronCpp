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
	void StartAcquisition();
	void StopAcquisition();

	void SequentialCapture();

	void SetThreadPool(std::shared_ptr<ImageSaverThreadPool> pThreadPool);

	GenApi::CCommandPtr pICommandTriggerSoftware;

protected:

private:
	static void OnStCallbackMethod(IStCallbackParamBase* pIStCallbackParamBase, void* pvContext);
	void OnCallback(IStCallbackParamBase* pCallbackParam);
	void SetEnumeration(GenApi::INodeMap* pInodeMap, const char* szEnumerationName, const char* szValueName);
	void SetTriggerMode(GenApi::CNodeMapPtr& pINodeMap, const char* triggerSelector, const char* triggerMode, const char* triggerSource);

	void PrintFrameInfo(const CIStStreamBufferPtr& pStreamBuffer);
	void ConvertPixelFormat(IStImage* pSrcImage, bool isColor, CIStImageBufferPtr& pDstBuffer);
	GenICam::gcstring SetSavePath(const std::string& baseDir, const uint64_t frameID);
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
			std::cerr << "[GigECamera] Saving image error: " << e.GetDescription() << std::endl;
		}
	}

	IStInterface* m_pInterface; // GigE interface pointer
	CIStDevicePtr m_pDevice; // Camera device pointer
	CIStDataStreamPtr m_pDataStream; // Data stream pointer

	std::string m_saveRootDir; // Directory to save images
	GenICam::gcstring m_serialNumber;
	std::shared_ptr<ImageSaverThreadPool> m_pThreadPool; // Thread pool for saving images
};

