#include "CameraManager.h"

CameraManager::CameraManager()
{
}

CameraManager::~CameraManager()
{
	StopAcquisitionAll(); // 안전한 종료
}

bool CameraManager::InitializeAll(size_t cameraCount)
{
    try
    {
        // 시스템 객체 생성 (장치 검색 및 연결)
        m_pSystem = CreateIStSystem();
        for (size_t i = 0; i < cameraCount; ++i)
        {
            // 각 카메라에 대한 CameraWorkerCB 객체 생성 및 초기화
            std::unique_ptr<CameraWorkerCB> worker(new CameraWorkerCB());
            if (worker->Initialize(m_pSystem))
            {
                // 카메라 초기화 성공 시, 작업자를 이동(move)하여 벡터에 추가, 소유권 이전
                m_workers.push_back(std::move(worker));
            }
            else
            {
                std::cerr << "[Manager] Camera " << i << " initialization failed." << std::endl;    //TODO: 에러 로그에 카메라 일련 번호 추가하기
				return false; // 초기화 실패 시 false 반환
            }
        }
    }
    catch (const GenICam::GenericException& e)
    {
		std::cerr << "[Manager] initialization ALL error: " << e.GetDescription() << std::endl;
		return false;
    }
}

void CameraManager::StartAcquisitionAll()
{
	// 모든 카메라에 대해 이미지 획득 시작
	for (std::vector<std::unique_ptr<CameraWorkerCB>>::iterator it = m_workers.begin(); it != m_workers.end(); ++it)
	{
        (*it)->StartAcquisition();
	}
}

void CameraManager::StopAcquisitionAll()
{
    for (std::vector<std::unique_ptr<CameraWorkerCB>>::iterator it = m_workers.begin(); it != m_workers.end(); ++it)
    {
        (*it)->StopAcquisition();
    }
}

void CameraManager::TriggerAll()
{
	for (std::vector<std::unique_ptr<CameraWorkerCB>>::iterator it = m_workers.begin(); it != m_workers.end(); ++it)
	{
		if ((*it)->pICommandTriggerSoftware)
            (*it)->pICommandTriggerSoftware->Execute();
	}
}

void CameraManager::SaveImageAll(const std::string& dstDir)
{
	for (std::vector<std::unique_ptr<CameraWorkerCB>>::iterator it = m_workers.begin(); it != m_workers.end(); ++it)
	{
		(*it)->SaveImageToFile(dstDir);
	}
}

/*
#include "CameraManager.h"
#include <iostream>
#include <string>

int main()
{
    std::string saveDirectory = "C:\\Users\\mykir\\Work\\Experiments\\";  // 이미지 저장 디렉토리
    size_t cameraCount = 2;  // 연결할 카메라 수 (예: 2개)

    CameraManager cameraManager;

    if (!cameraManager.InitializeAll(cameraCount))
    {
        std::cerr << "카메라 초기화에 실패했습니다." << std::endl;
        return -1;
    }

    cameraManager.StartAcquisitionAll();

    while (true)
    {
        std::cout << "\n0: 트리거 발생" << std::endl;
        std::cout << "1: 이미지 저장" << std::endl;
        std::cout << "2: 종료" << std::endl;
        std::cout << "입력: ";

        int choice;
        std::cin >> choice;

        if (choice == 0)
        {
            cameraManager.TriggerAll();
            std::cout << "트리거 전송 완료" << std::endl;
        }
        else if (choice == 1)
        {
            cameraManager.SaveImageAll(saveDirectory);
            std::cout << "이미지 저장 완료: " << saveDirectory << std::endl;
        }
        else if (choice == 2)
        {
            break;
        }
        else
        {
            std::cout << "올바르지 않은 입력입니다." << std::endl;
        }
    }

    cameraManager.StopAcquisitionAll();

    return 0;
}
*/