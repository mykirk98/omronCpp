#pragma once
#include "CameraWorker.h"
#include "config.h"

// 콜백 함수 : 특정 이벤트가 발생했을 때 자동으로 호출되어, 미리 정의된 동작을 수행하는 것
// 이미지 데이터 스트림에서 새로운 이미지 버퍼가 도착할 때마다 호출된다.
// 카메라에서 새로운 이미지가 수신되면, 이 콜백 함수가 자동으로 호출되어 해당 이미지를 처리할 수 있다.

class CameraWorkerCB : public CameraWorker
{
public:
	/* @brief 클래스 생성자 */
	CameraWorkerCB();
	/* @brief 클래스 소멸자 */
	~CameraWorkerCB();

	/*
	@brief 카메라에 필요한 객체들을 초기화하는 함수
	@param pSystem : 카메라 시스템 객체
	*/
	bool Initialize(const CIStSystemPtr& pSystem);
	/*
	@brief 이미지 획득 시작 함수
	*/
	void StartAcquisition();
	/*
	@brief 이미지 획득 중지 함수
	*/
	void StopAcquisition();
	/*
	@brief 이미지 저장 함수
	@param dstDir : 이미지 저장할 디렉토리 경로
	*/
	void SaveImageToFile(const std::string& dstDir);

	/*
	@brief 소프트웨어 트리거 명령을 위한 ICommand 인터페이스 포인터
	*/
	GenApi::CCommandPtr pICommandTriggerSoftware;
	
private:
	/*
	@brief StApi 콜백 메소드
	@param pIStCallbackParamBase : 콜백 파라미터
	@param pvContext : 콜백 컨텍스트 (this 포인터를 전달하여 멤버 함수 호출)
	*/
	static void __stdcall OnStCallbackMethod(IStCallbackParamBase* pIStCallbackParamBase, void* pvContext);
	/*
	@brief 콜백 처리 함수
	@param pCallbackParam : 콜백 파라미터
	*/
	void OnCallback(IStCallbackParamBase* pCallbackParam);
	/*
	@brief IEnumeration 노드의 값을 설정하는 함수
	@param pInodeMap : INodeMap 인터페이스 포인터
	@param szEnumerationName : 설정할 IEnumeration 노드의 이름
	@param szValueName : 설정할 IEnumEntry의 이름
	*/
	void SetEnumeration(GenApi::INodeMap* pInodeMap, const char* szEnumerationName, const char* szValueName);
	/*
	@brief 트리거 모드를 설정하는 함수
	@param pINodeMap : INodeMap 인터페이스 포인터
	@param triggerSelector : 트리거 셀렉터 노드 설정 값
	@param triggerMode : 트리거 모드 노드 설정 값
	@param triggerSource : 트리거 소스 노드 설정 값
	*/
	void SetTriggerMode(GenApi::CNodeMapPtr& pINodeMap, const char* triggerSelector, const char* triggerMode, const char* triggerSource);

	/* @brief 이미지 객체 포인터 */
	IStImage* m_pImage;
	/* @brief 이미지 프레임 ID */
	uint64_t m_frameID;
};

