#pragma once
#include "CameraWorker.h"
#include "config.h"


class Experiment1 : public CameraWorker
{
public:
	Experiment1();
	~Experiment1();

	bool initialize();
	void startAcquisition();
	void stopAcquisition();

	GenApi::CCommandPtr pICommandTriggerSoftware;

private:
	static void OnStCallbackMethod(IStCallbackParamBase* pIStCallbackParamBase, void* pvContext);
	void OnCallback(IStCallbackParamBase* pCallbackParam);
	void SetEnumeration(GenApi::INodeMap* pInodeMap, const char* szEnumerationName, const char* szValueName);
	void SetTriggerMode(GenApi::CNodeMapPtr& pINodeMap, const char* triggerSelector, const char* triggerMode, const char* triggerSource);
};

