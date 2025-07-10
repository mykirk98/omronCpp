#pragma once

#include <StApi_TL.h>

#include "GigEConfigurator.h"

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

protected:

private:
	IStInterface* m_pInterface; // GigE interface pointer
	CIStDevicePtr m_pDevice; // Camera device pointer
	CIStDataStreamPtr m_pDataStream; // Data stream pointer
};

