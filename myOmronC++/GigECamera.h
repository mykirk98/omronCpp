#pragma once

#include <StApi_TL.h>

#include "YCQueue.h"
#include "ImageProcess.h"
#include "GigEUtil.h"
#include "NodeMapUtil.h"
#include "config.h"
#include "CamLogger.h"
#ifdef _WIN32
#include <windows.h>  // for Sleep
#else
#include <unistd.h>   // for sleep/usleep
#endif

using namespace StApi;

/* @brief GigE 카메라 클래스 */
class GigECamera
{
public:
	/* @brief GigECamera 생성자 
	@param rootDir 설정 파일의 루트 디렉토리
	@param logger 카메라 이벤트 로깅을 위한 로거 */
	explicit GigECamera(std::string rootDir, std::shared_ptr<CamLogger> logger);
	/* @brief GigECamera 소멸자 */
	~GigECamera();

	/* @brief 카메라 초기화
	@param pInterface GigE 인터페이스 포인터
	@param iFaceDeviceIdx 인터페이스 내 장치 인덱스 */
	bool Initialize(IStInterface* pInterface, uint32_t iFaceDeviceIdx);
	/* @brief 이미지 획득 시작 */
	void StartAcquisition();
	/* @brief 이미지 획득 중지 */
	void StopAcquisition();
	/* @brief 연속 촬영 */
	void FreeRunCapture();
	/* @brief 소프트웨어 트리거 실행 */
	void ExecuteTrigger();
	/* @brief 소프트웨어 트리거 실행 (상세 정보 포함)
	@param detailInfo 이미지 저장 시 사용할 상세 정보 (예: "TOP", "BOTTOM", "hole1", ..., "hole10") */
	void ExecuteTrigger(const std::string& detailInfo);

	/* @brief 프레임 큐 설정
	@param pFrameQueue 프레임 데이터를 위한 YCQueue 포인터 */
	void SetFrameQueue(std::shared_ptr<YCQueue<FrameData>> pFrameQueue);
	/* @brief OpenCV Mat를 다음 모듈에게 전달하기 위한 큐 설정
	@param pCVMatQueue OpenCV Mat 데이터를 위한 YCQueue 포인터 */
	void SetCVMatQueue(std::shared_ptr<YCQueue<cv::Mat>> pCVMatQueue);
	/* @brief 카메라 이름 가져오기 */
	const std::string& GetUserDefinedName();
	/* @brief 카메라 시리얼 번호 가져오기 */
	const std::string& GetSerialNumber();

protected:

private:
	static void OnStCallbackMethod(IStCallbackParamBase* pIStCallbackParamBase, void* pvContext);
	/* @brief StApi 콜백 함수
	@param pCallbackParam 콜백 파라미터 포인터 */
	void OnCallback(IStCallbackParamBase* pCallbackParam);

	IStInterface* m_pInterface; // GigE interface pointer
	CIStDevicePtr m_pDevice; // Camera device pointer
	CIStDataStreamPtr m_pDataStream; // Data stream pointer

	GenApi::CCommandPtr pICommandTriggerSoftware;
	std::shared_ptr<YCQueue<FrameData>> m_pFrameQueue;
	std::shared_ptr<YCQueue<cv::Mat>> m_pCVMatQueue;

	std::string m_strRootDir;
	std::string m_strSerialNumber;
	std::string m_strUDFName;
	std::string m_strDetailInfo; // detail information for saving images (e.g., "TOP", "BOTTOM", "hole1", ..., "hole10")

	std::shared_ptr<CamLogger> m_logger; // Logger for logging camera events
};