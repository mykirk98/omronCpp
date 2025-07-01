#pragma once
#include "CameraWorker.h"
#include "config.h"

// �ݹ� �Լ� : Ư�� �̺�Ʈ�� �߻����� �� �ڵ����� ȣ��Ǿ�, �̸� ���ǵ� ������ �����ϴ� ��
// �̹��� ������ ��Ʈ������ ���ο� �̹��� ���۰� ������ ������ ȣ��ȴ�.
// ī�޶󿡼� ���ο� �̹����� ���ŵǸ�, �� �ݹ� �Լ��� �ڵ����� ȣ��Ǿ� �ش� �̹����� ó���� �� �ִ�.

class CameraWorkerCB : public CameraWorker
{
public:
	/* @brief Ŭ���� ������ */
	CameraWorkerCB();
	/* @brief Ŭ���� �Ҹ��� */
	~CameraWorkerCB();

	/*
	@brief ī�޶� �ʿ��� ��ü���� �ʱ�ȭ�ϴ� �Լ�
	@param pSystem : ī�޶� �ý��� ��ü
	*/
	bool Initialize(const CIStSystemPtr& pSystem);
	/*
	@brief �̹��� ȹ�� ���� �Լ�
	*/
	void StartAcquisition();
	/*
	@brief �̹��� ȹ�� ���� �Լ�
	*/
	void StopAcquisition();
	/*
	@brief �̹��� ���� �Լ�
	@param dstDir : �̹��� ������ ���丮 ���
	*/
	void SaveImageToFile(const std::string& dstDir);

	/*
	@brief ����Ʈ���� Ʈ���� ������ ���� ICommand �������̽� ������
	*/
	GenApi::CCommandPtr pICommandTriggerSoftware;
	
private:
	/*
	@brief StApi �ݹ� �޼ҵ�
	@param pIStCallbackParamBase : �ݹ� �Ķ����
	@param pvContext : �ݹ� ���ؽ�Ʈ (this �����͸� �����Ͽ� ��� �Լ� ȣ��)
	*/
	static void OnStCallbackMethod(IStCallbackParamBase* pIStCallbackParamBase, void* pvContext);
	/*
	@brief �ݹ� ó�� �Լ�
	@param pCallbackParam : �ݹ� �Ķ����
	*/
	void OnCallback(IStCallbackParamBase* pCallbackParam);
	/*
	@brief IEnumeration ����� ���� �����ϴ� �Լ�
	@param pInodeMap : INodeMap �������̽� ������
	@param szEnumerationName : ������ IEnumeration ����� �̸�
	@param szValueName : ������ IEnumEntry�� �̸�
	*/
	void SetEnumeration(GenApi::INodeMap* pInodeMap, const char* szEnumerationName, const char* szValueName);
	/*
	@brief Ʈ���� ��带 �����ϴ� �Լ�
	@param pINodeMap : INodeMap �������̽� ������
	@param triggerSelector : Ʈ���� ������ ��� ���� ��
	@param triggerMode : Ʈ���� ��� ��� ���� ��
	@param triggerSource : Ʈ���� �ҽ� ��� ���� ��
	*/
	void SetTriggerMode(GenApi::CNodeMapPtr& pINodeMap, const char* triggerSelector, const char* triggerMode, const char* triggerSource);

	/* @brief �̹��� ��ü ������ */
	IStImage* m_pImage;
	/* @brief �̹��� ������ ID */
	uint64_t m_frameID;
};

