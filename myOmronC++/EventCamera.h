#pragma once

#include <StApi_TL.h>
#include <string>
#include <iostream>

using namespace StApi;

class EventCamera
{
public:
	explicit EventCamera(uint64_t imageCount = 1000);
	~EventCamera();

	bool initialize();
	void startAcquisition();
	void stopAcquisition();

private:
	void enableExposureEndEvent();
	static void OnNodeCallbackFunction(GenApi::INode* pINode, uint32_t pParam);
	static void handleNodeCallback(GenApi::INode* pINode);

private:
	uint64_t m_imageCount;
	bool m_initialized;

	CStApiAutoInit m_stApiAutoInit;
	CIStSystemPtr m_pSystem;
	CIStDevicePtr m_pDevice;
	CIStDataStreamPtr m_pDataStream;

	GenApi::CNodeMapPtr m_pNodeMap;
};

