#include "CameraManager.h"

CameraManager::CameraManager()
{
}

CameraManager::~CameraManager()
{
	StopAcquisitionAll(); // ������ ����
}

bool CameraManager::InitializeAll(size_t cameraCount)
{
    try
    {
        // �ý��� ��ü ���� (��ġ �˻� �� ����)
        m_pSystem = CreateIStSystem();
        for (size_t i = 0; i < cameraCount; ++i)
        {
            // �� ī�޶� ���� CameraWorkerCB ��ü ���� �� �ʱ�ȭ
            std::unique_ptr<CameraWorkerCB> worker(new CameraWorkerCB());
            if (worker->Initialize(m_pSystem))
            {
                // ī�޶� �ʱ�ȭ ���� ��, �۾��ڸ� �̵�(move)�Ͽ� ���Ϳ� �߰�, ������ ����
                m_workers.push_back(std::move(worker));
            }
            else
            {
                std::cerr << "[Manager] Camera " << i << " initialization failed." << std::endl;    //TODO: ���� �α׿� ī�޶� �Ϸ� ��ȣ �߰��ϱ�
				return false; // �ʱ�ȭ ���� �� false ��ȯ
            }
        }
        return true;  // 또는 적절한 return 값
    }
    catch (const GenICam::GenericException& e)
    {
		std::cerr << "[Manager] initialization ALL error: " << e.GetDescription() << std::endl;
		return false;
    }
}

void CameraManager::StartAcquisitionAll()
{
	// ��� ī�޶� ���� �̹��� ȹ�� ����
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
    std::string saveDirectory = "C:\\Users\\mykir\\Work\\Experiments\\";  // �̹��� ���� ���丮
    size_t cameraCount = 2;  // ������ ī�޶� �� (��: 2��)

    CameraManager cameraManager;

    if (!cameraManager.InitializeAll(cameraCount))
    {
        std::cerr << "ī�޶� �ʱ�ȭ�� �����߽��ϴ�." << std::endl;
        return -1;
    }

    cameraManager.StartAcquisitionAll();

    while (true)
    {
        std::cout << "\n0: Ʈ���� �߻�" << std::endl;
        std::cout << "1: �̹��� ����" << std::endl;
        std::cout << "2: ����" << std::endl;
        std::cout << "�Է�: ";

        int choice;
        std::cin >> choice;

        if (choice == 0)
        {
            cameraManager.TriggerAll();
            std::cout << "Ʈ���� ���� �Ϸ�" << std::endl;
        }
        else if (choice == 1)
        {
            cameraManager.SaveImageAll(saveDirectory);
            std::cout << "�̹��� ���� �Ϸ�: " << saveDirectory << std::endl;
        }
        else if (choice == 2)
        {
            break;
        }
        else
        {
            std::cout << "�ùٸ��� ���� �Է��Դϴ�." << std::endl;
        }
    }

    cameraManager.StopAcquisitionAll();

    return 0;
}
*/