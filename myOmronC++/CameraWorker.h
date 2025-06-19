#pragma once
#include <StApi_TL.h>	// TL : Transport Layer
#include <StApi_IP.h>	// IP : Image Processing
#include <iostream>
#include <ShlObj.h>

using namespace StApi;
/*
@brief 기본적인 카메라 작업을 수행하는 클래스.
@brief 단순한 이미지 획득 및 저장 기능을 제공하며, 카메라 초기화, 이미지 획득 시작 및 종료 기능을 포함합니다.
*/
class CameraWorker
{
public:
	/*
	@brief 클래스 생성자
	@param imageCount : 수집할 이미지 수
	*/
	explicit CameraWorker(uint64_t imageCount = 100);
	/* @brief 클래스 소멸자 */
	~CameraWorker();
	
	/* @brief 카메라 제어에 필요한 여러 객체 초기화 함수 */
	bool initialize();
	/* @brief 이미지 획득 시작 함수 */
	void StartAcquisition();
	/* @brief 이미지 획득 종료 함수 */
	void StopAcquisition();
	
protected:

private:
	/*
	@brief 이미지 저장 경로 설정 함수
	@return : 이미지 저장 경로 문자열
	*/
	GenICam::gcstring SetSavePath(GenICam::gcstring frameID);
	/* @brief 이미지 로드 함수 */
	//bool LoadImage();
	/* @brief 이미지 포맷 설정 함수 */
	//bool SetImageFormat();
	/* @brief BMP 이미지 저장 함수 */
	//bool SaveBMPImage();
	/*
	@brief BMP 이미지 저장 함수 (이미지 버퍼를 인자로 받음) 
	@param pImageBuffer : 저장할 이미지 버퍼 포인터
	*/
	void SaveBMPImage(CIStImageBufferPtr& pImageBuffer, GenICam::gcstring& savePath);
	/*
	@brief 픽셀 포맷 변환 함수
	@param pSrcImage : 변환할 원본 이미지 포인터
	@param dstFormat : 변환할 대상 픽셀 포맷
	@param pDstBuffer : 변환된 이미지를 저장할 이미지 버퍼 포인터
	*/
	void ConvertToBGR8(IStImage* pSrcImage, EStPixelFormatNamingConvention_t dstFormat, CIStImageBufferPtr& pDstBuffer);

	bool m_initialized;
	bool m_isImageSaved;

	uint64_t m_imageCount;	// 획득할 이미지 수
	
	CStApiAutoInit m_StApiAutoInit;	// StApi 초기화 객체
	// StApi 라이브러리의 자동 초기화를 위한 객체로,
	// 프로그램 시작 시 자동으로 StApi 라이브러리를 초기화하고 종료 시 자동으로 정리합니다.
	CIStSystemPtr m_pSystem;	// 시스템 객체
	CIStDevicePtr m_pDevice;	// 디바이스 객체
	CIStDataStreamPtr m_pDataStream;	// 데이터 스트림 객체
	
	std::wstring m_savePath;	// 이미지 저장 경로
};