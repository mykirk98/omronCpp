#pragma once
#include <StApi_TL.h>
#include <iostream>

using namespace StApi;

// 콜백 함수 : 특정 이벤트가 발생했을 때 자동으로 호출되어, 미리 정의된 동작을 수행하는 것
// 이미지 데이터 스트림에서 새로운 이미지 버퍼가 도착할 때마다 호출된다.
// 카메라에서 새로운 이미지가 수신되면, 이 콜백 함수가 자동으로 호출되어 해당 이미지를 처리할 수 있다.

class CameraWorker_CB
{
public:
	CameraWorker_CB();	// 생성자
	~CameraWorker_CB();	// 소멸자

	bool initialize();	// 초기화 함수
	void startAcquisition();	// 이미지 획득 시작 함수
	void stopAcquisition();	// 이미지 획득 중지 함수
	
private:
	static void __stdcall OnStCallbackFunction(IStCallbackParamBase* pIStCallbackParamBase, void* pvContext);
	void handleCallback(IStCallbackParamBase* pCallbackParam);
	
	
private:
	CStApiAutoInit m_StApiAutoInit;	// StApi 초기화 객체
	
	CIStSystemPtr m_pSystem;	// 카메라 시스템 포인터
	CIStDevicePtr m_pDevice;	// 카메라 디바이스 포인터
	CIStDataStreamPtr m_pDataStream;	// 데이터 스트림 포인터

	bool m_initialized;
};

