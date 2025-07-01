#pragma once
#include "CameraWorker.h"
#include "config.h"

// ī�޶� ��ü�� OnCallback �޼ҵ� �ȿ��� ĸ�Ķ� ������ �� �� �߾��µ�, ����� �и��Ͽ���.
// �� Ŭ������ �и��ϱ� �� �����̴�.

// �ݹ� �Լ� : Ư�� �̺�Ʈ�� �߻����� �� �ڵ����� ȣ��Ǿ�, �̸� ���ǵ� ������ �����ϴ� ��
// �̹��� ������ ��Ʈ������ ���ο� �̹��� ���۰� ������ ������ ȣ��ȴ�.
// ī�޶󿡼� ���ο� �̹����� ���ŵǸ�, �� �ݹ� �Լ��� �ڵ����� ȣ��Ǿ� �ش� �̹����� ó���� �� �ִ�.

class Experiment1 : public CameraWorker
{
public:
	Experiment1();	// ������
	~Experiment1();	// �Ҹ���

	bool initialize();
	void startAcquisition();	// �̹��� ȹ�� ���� �Լ�
	void stopAcquisition();	// �̹��� ȹ�� ���� �Լ�

	GenApi::CCommandPtr pICommandTriggerSoftware;

private:
	static void OnStCallbackMethod(IStCallbackParamBase* pIStCallbackParamBase, void* pvContext);
	void OnCallback(IStCallbackParamBase* pCallbackParam);
	void SetEnumeration(GenApi::INodeMap* pInodeMap, const char* szEnumerationName, const char* szValueName);
	// Ʈ���� ��� ���� �޼ҵ�
	void SetTriggerMode(GenApi::CNodeMapPtr& pINodeMap, const char* triggerSelector, const char* triggerMode, const char* triggerSource);
};

