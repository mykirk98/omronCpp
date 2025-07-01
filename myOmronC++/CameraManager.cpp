#include "CameraManager.h"

CameraManager::CameraManager()
{
}

CameraManager::~CameraManager()
{
	StopAcquisitionAll();
}

bool CameraManager::InitializeAll(size_t cameraCount)
{
    try
    {
		// Create StApi system object
        m_pSystem = CreateIStSystem();
        for (size_t i = 0; i < cameraCount; ++i)
        {
			// Create CameraWorkerCB unique pointer object for each camera
            std::unique_ptr<CameraWorkerCB> worker(new CameraWorkerCB());
            if (worker->Initialize(m_pSystem))
            {
				// If initialization is successful, add the worker to the vector
                m_workers.push_back(std::move(worker));
            }
            else
            {
                std::cerr << "[Manager] Camera " << i << " initialization failed." << std::endl;    //TODO: instead of i, show camera serial number
				return false; // If any camera fails to initialize, return false
            }
        }
        return true;
    }
    catch (const GenICam::GenericException& e)
    {
		std::cerr << "[Manager] initialization ALL error: " << e.GetDescription() << std::endl;
		return false;
    }
}

void CameraManager::StartAcquisitionAll()
{
	// Start acquisition for all cameras
	for (std::vector<std::unique_ptr<CameraWorkerCB>>::iterator it = m_workers.begin(); it != m_workers.end(); ++it)
	{
        (*it)->StartAcquisition();
	}
}

void CameraManager::StopAcquisitionAll()
{
	// Stop acquisition for all cameras
    for (std::vector<std::unique_ptr<CameraWorkerCB>>::iterator it = m_workers.begin(); it != m_workers.end(); ++it)
    {
        (*it)->StopAcquisition();
    }
}

void CameraManager::TriggerAll()
{
	// Send trigger signal to all cameras
	for (std::vector<std::unique_ptr<CameraWorkerCB>>::iterator it = m_workers.begin(); it != m_workers.end(); ++it)
	{
		if ((*it)->pICommandTriggerSoftware)
            (*it)->pICommandTriggerSoftware->Execute();
	}
}

void CameraManager::SaveImageAll(const std::string& dstDir)
{
	// Save images from all cameras to the specified directory
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
        std::cerr << "Failed to initialize manager" << std::endl;
        return -1;
    }

    cameraManager.StartAcquisitionAll();

    while (true)
    {
        std::cout << "\n0: Send trigger" << std::endl;
        std::cout << "1: Save image" << std::endl;
        std::cout << "2: Terminate" << std::endl;
        std::cout << "Input: ";

        int choice;
        std::cin >> choice;

        if (choice == 0)
        {
            cameraManager.TriggerAll();
            std::cout << "Sending trigger completed." << std::endl;
        }
        else if (choice == 1)
        {
            cameraManager.SaveImageAll(saveDirectory);
            std::cout << "Saving image completed: " << saveDirectory << std::endl;
        }
        else if (choice == 2)
        {
            break;
        }
        else
        {
            std::cout << "Wrong input." << std::endl;
        }
    }

    cameraManager.StopAcquisitionAll();

    return 0;
}
*/