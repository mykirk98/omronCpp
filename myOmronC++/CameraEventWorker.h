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
	// explicit 키워드를 사용한 이유 : 단일 인자 생성자에서 암시적 변환을 방지하기 위함,
	// = 연산자로 초기화되는 순간 예상치 못한 생성자 호출이 일어날 수 있기 때문
	// CameraEventWorker cam = 500; // 암시적 변환 (허용되지 않음)
	// CameraEventWorker cam(500); // 명시적 생성자 호출 (허용됨)
	~CameraEventWorker();

	bool initialize();
	void startAcquisition();
	void stopAcquisition();

private:
	void enableExposureEndEvent();
	static void __stdcall OnNodeCallbackFunction(GenApi::INode* pINode, uint32_t pParam);
	static void handleNodeCallback(GenApi::INode* pINode);
	//NOTE:
	// static 멤버 함수 : 객체(인스턴스)를 생성하지 않아도 호출할 수 있는 함수로,
	//						클래스의 모든 인스턴스에서 공유된다.
	// 인스턴스 멤버 함수는, 암묵적으로 this 포인터를 필요로 하지만,
	//						static 멤버 함수는 this 포인터가 필요하지 않다.

private:
	uint64_t m_imageCount;
	bool m_initialized;

	CStApiAutoInit m_stApiAutoInit;
	CIStSystemPtr m_pSystem;
	CIStDevicePtr m_pDevice;
	CIStDataStreamPtr m_pDataStream;

	GenApi::CNodeMapPtr m_pNodeMap;
};

