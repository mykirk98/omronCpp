#pragma once

#include <StApi_TL.h>
#include <string>
#include <iostream>

using namespace StApi;

class CameraEventWorker
{
public:
	explicit CameraEventWorker(uint64_t imageCount = 1000);
	//NOTE:
	// explicit Ű���带 ����� ���� : ���� ���� �����ڿ��� �Ͻ��� ��ȯ�� �����ϱ� ����,
	// = �����ڷ� �ʱ�ȭ�Ǵ� ���� ����ġ ���� ������ ȣ���� �Ͼ �� �ֱ� ����
	// CameraEventWorker cam = 500; // �Ͻ��� ��ȯ (������ ����)
	// CameraEventWorker cam(500); // ������ ������ ȣ�� (����)
	~CameraEventWorker();

	bool initialize();
	void startAcquisition();
	void stopAcquisition();

private:
	void enableExposureEndEvent();
	static void OnNodeCallbackFunction(GenApi::INode* pINode, uint32_t pParam);
	static void handleNodeCallback(GenApi::INode* pINode);
	//NOTE:
	// static ��� �Լ� : ��ü(�ν��Ͻ�)�� �������� �ʾƵ� ȣ���� �� �ִ� �Լ���,
	//						Ŭ������ ��� �ν��Ͻ����� �����ȴ�.
	// �ν��Ͻ� ��� �Լ���, �Ϲ������� this �����͸� �ʿ�� ������,
	//						static ��� �Լ��� this �����Ͱ� �ʿ����� �ʴ�.

private:
	uint64_t m_imageCount;
	bool m_initialized;

	CStApiAutoInit m_stApiAutoInit;
	CIStSystemPtr m_pSystem;
	CIStDevicePtr m_pDevice;
	CIStDataStreamPtr m_pDataStream;

	GenApi::CNodeMapPtr m_pNodeMap;
};

