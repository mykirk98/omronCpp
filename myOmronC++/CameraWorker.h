#pragma once
#include <StApi_TL.h>	// TL : Transport Layer
#include <StApi_IP.h>	// IP : Image Processing
#include <iostream>
#include <ShlObj.h>

using namespace StApi;

class CameraWorker
{
public:
	explicit CameraWorker(uint64_t imageCount = 100);		// 생성자
	~CameraWorker();	// 소멸자

	bool initialize();	// 카메라 초기화 및 장치 연결
	void StartAcquisition();	// 이미지 획득 시작
	void StopAcquisition();	// 이미지 획득 중지
	
protected:

private:
	GenICam::gcstring SetSavePath();	// 이미지 저장 경로 설정
	bool SaveImage(IStImage* pImage);
	bool LoadImage();
	bool SetImageFormat();
	bool SaveBMPImage();

	bool m_initialized;
	bool m_isImageSaved;

	uint64_t m_imageCount;	// 획득할 이미지 수
	
	
	CStApiAutoInit m_StApiAutoInit;	// StApi 초기화 객체
	// StApi 라이브러리의 자동 초기화를 위한 객체로,
	// 프로그램 시작 시 자동으로 StApi 라이브러리를 초기화하고 종료 시 자동으로 정리합니다.
	CIStSystemPtr m_pSystem;	// 시스템 객체
	CIStDevicePtr m_pDevice;	// 장치 객체
	CIStDataStreamPtr m_pDataStream;	// 데이터 스트림 객체
	
	std::wstring m_savePath;	// 이미지 저장 경로
	CIStImageBufferPtr m_pImageBuffer;	// 이미지 버퍼 포인터
};

