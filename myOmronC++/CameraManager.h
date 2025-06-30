#pragma once
#include "CameraWorkerCB.h"
#include <vector>
#include <memory>

class CameraManager
{
public:
	/* @brief 클래스 생성자 */
	CameraManager();
	/* @brief 클래스 소멸자*/
	~CameraManager();

	/*
	@brief 모든 카메라 초기화 함수
	@param cameraCount : 초기화할 카메라의 개수
	*/
	bool InitializeAll(size_t cameraCount);
	/*
	@brief 모든 카메라의 이미지 획득 시작 함수
	*/
	void StartAcquisitionAll();
	/*
	@brief 모든 카메라의 이미지 획득 중지 함수
	*/
	void StopAcquisitionAll();
	/*
	@brief 모든 카메라에 트리거 신호를 전송하는 함수
	*/
	void TriggerAll();
	/*
	@brief 모든 카메라의 이미지를 지정된 디렉토리에 저장하는 함수
	@param dstDir : 이미지 저장할 디렉토리 경로
	*/
	void SaveImageAll(const std::string& dstDir);

private:
	/*
	@brief StApi 라이브러리 초기화 객체
	@brief StApi 라이브러리를 초기화하고 종료 시 자동으로 정리합니다.
	*/
	CStApiAutoInit m_StApiAutoInit;
	/*
	@brief 카메라 시스템 객체
	@brief 카메라 시스템은 카메라 디바이스를 관리하고, 디바이스와의 연결을 설정하는 역할을 합니다.
	*/
	CIStSystemPtr m_pSystem;

	/*
	@brief 카메라 작업자 객체들의 벡터
	@brief 각 카메라에 대한 작업을 수행하는 CameraWorkerCB 객체들의 벡터입니다.
	*/
	std::vector<std::unique_ptr<CameraWorkerCB>> m_workers;
};