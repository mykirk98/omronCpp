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
	m_pSystem = CreateIStSystem();
	for (size_t i = 0; i < cameraCount; ++i)
	{
		auto worker = std::make_unique<CameraWorkerCB>();
		if (worker->initialize(m_pSystem))  // 수정된 initialize 함수 사용
		{
			m_workers.push_back(std::move(worker));
		}
		else
		{
			std::cerr << "Camera " << i << " initialization failed.\n";
			return false;
		}
	}
	return true;
}

void CameraManager::StartAcquisitionAll()
{
	for (auto& worker : m_workers)
	{
		worker->startAcquisition();
	}
}

void CameraManager::StopAcquisitionAll()
{
	for (auto& worker : m_workers)
	{
		worker->stopAcquisition();
	}
}

void CameraManager::TriggerAll()
{
	for (auto& worker : m_workers)
	{
		if (worker->pICommandTriggerSoftware)
			worker->pICommandTriggerSoftware->Execute();
	}
}

void CameraManager::SaveImageAll(const std::string& dstDir)
{
	for (auto& worker : m_workers)
	{
		worker->SaveImageToFile(dstDir);
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