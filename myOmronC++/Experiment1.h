#pragma once
#include "CameraWorker.h"
#include "config.h"

// 카메라 객체의 OnCallback 메소드 안에서 캡쳐랑 저장을 둘 다 했었는데, 현재는 분리하였다.
// 이 클래스는 분리하기 전 상태이다.

// 콜백 함수 : 특정 이벤트가 발생했을 때 자동으로 호출되어, 미리 정의된 동작을 수행하는 것
// 이미지 데이터 스트림에서 새로운 이미지 버퍼가 도착할 때마다 호출된다.
// 카메라에서 새로운 이미지가 수신되면, 이 콜백 함수가 자동으로 호출되어 해당 이미지를 처리할 수 있다.

class Experiment1 : public CameraWorker
{
public:
	Experiment1();	// 생성자
	~Experiment1();	// 소멸자

	bool initialize();
	void startAcquisition();	// 이미지 획득 시작 함수
	void stopAcquisition();	// 이미지 획득 중지 함수

	GenApi::CCommandPtr pICommandTriggerSoftware;

private:
	static void __stdcall OnStCallbackMethod(IStCallbackParamBase* pIStCallbackParamBase, void* pvContext);
	void OnCallback(IStCallbackParamBase* pCallbackParam);
	void SetEnumeration(GenApi::INodeMap* pInodeMap, const char* szEnumerationName, const char* szValueName);
	// 트리거 모드 설정 메소드
	void SetTriggerMode(GenApi::CNodeMapPtr& pINodeMap, const char* triggerSelector, const char* triggerMode, const char* triggerSource);
};

