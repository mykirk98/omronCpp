#include "GigEManager.h"

GigEManager::GigEManager(std::string saveRootDir)
    : m_saveRootDir(saveRootDir)
{
}

GigEManager::GigEManager(std::string saveRootDir, std::shared_ptr<PathQueue> pathQueue)
    : m_saveRootDir(saveRootDir)
	, m_pathQueue(pathQueue)
{
}

GigEManager::~GigEManager()
{
    StopAll();
}

bool GigEManager::Initialize()
{
    try {
		m_frameQueue = std::make_shared<FrameQueue>();  //TODO: 생성자 안으로 이동
		m_ImageSaverThreadPool = std::make_shared<ImageSaverThreadPool>(5, m_saveRootDir, m_frameQueue, m_pathQueue); //TODO: 생성자 안으로 이동
		m_ImageSaverThreadPool->Start();

        m_pSystem = CreateIStSystem(StSystemVendor_Default, StInterfaceType_GigEVision);

        for (uint32_t ifaceIdx = 0; ifaceIdx < m_pSystem->GetInterfaceCount(); ++ifaceIdx)
        {
            IStInterface* pInterface = m_pSystem->GetIStInterface(ifaceIdx);

            std::cout << "Interface " << ifaceIdx << ": " << pInterface->GetIStInterfaceInfo()->GetDisplayName() << std::endl;
            std::cout << "DeviceCount = " << pInterface->GetDeviceCount() << std::endl;

            for (uint32_t deviceIdx = 0; deviceIdx < pInterface->GetDeviceCount(); ++deviceIdx)
            {
                std::cout << "-------------------------------------------" << std::endl;
                std::cout << "Device " << deviceIdx << ": " << pInterface->GetIStDeviceInfo(deviceIdx)->GetDisplayName() << std::endl;
                std::cout << "SerialNumber: " << pInterface->GetIStDeviceInfo(deviceIdx)->GetSerialNumber() << std::endl;

                std::shared_ptr<GigECamera> camera = std::make_shared<GigECamera>(m_saveRootDir);
				const std::string& cameraName = camera->GetCameraName();    //TODO: 이 시점에서 cameraName이 어떻게 결정된 것인지 확인 필요
                if (camera->Initialize(pInterface, deviceIdx))
                {
                    camera->SetFrameQueue(m_frameQueue);
                    m_cameras.push_back(camera);
                    m_cameraMap[cameraName] = camera;
                }
                else
                {
                    std::cerr << "[GigEManager] Failed to initialize camera " << deviceIdx << std::endl;
                }
            }
        }
        //return !m_workers.empty();
        return !m_cameras.empty();
    }
    catch (const GenICam::GenericException& e)
    {
        std::cerr << "[GigEManager] Initialization error: " << e.GetDescription() << std::endl;
        return false;
    }
}

void GigEManager::StartAll()
{
    m_running = true;
    for (std::shared_ptr<GigECamera>& camera : m_cameras)
    {
        try
        {
            camera->StartAcquisition();
			m_threads.emplace_back(&GigEManager::CameraLoop, this, camera);
        }
        catch (const std::exception& e)
        {
            std::cerr << "[GigEManager] Worker failed to start: " << e.what() << std::endl;
        }
    }
}

void GigEManager::StopAll()
{
    m_running = false;
    for (std::thread& thread : m_threads)
    {
        if (thread.joinable())
        {
            thread.join();
        }
    }
    for (std::shared_ptr<GigECamera>& camera : m_cameras)
    {
        camera->StopAcquisition();
    }
    m_threads.clear();
}

//void GigEManager::TriggerAll()
//{
//    for (std::shared_ptr<GigEWorker>& worker : m_workers)
//    {
//        worker->Trigger();
//    }
//}

void GigEManager::TriggerSingle(int index)
{
    if (index >= 0 && index < static_cast<int>(m_cameras.size()))
    {
		m_cameras[index]->pICommandTriggerSoftware->Execute();  //TODO: 래퍼로 감싸서 호출하기, 직접 자원에 접근하는 것은 좋지 않음
    }
    else
    {
		std::cerr << "[GigEManager] Invalid worker index: " << index << std::endl;
    }
}

void GigEManager::TriggerSingle(const std::string& cameraName)
{
	std::map<std::string, std::shared_ptr<GigECamera>>::iterator it = m_cameraMap.find(cameraName);
    //auto it = m_cameraMap.find(cameraName);
    if (it != m_cameraMap.end())
    {
		it->second->pICommandTriggerSoftware->Execute();  //TODO: 래퍼로 감싸서 호출하기, 직접 자원에 접근하는 것은 좋지 않음
    }
    else
    {
        std::cerr << "[GigEManager] Camera not found: " << cameraName << std::endl;
	}
}

void GigEManager::CameraLoop(std::shared_ptr<GigECamera> camera)
{
    while (m_running)
    {
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
        // Here you can add any additional logic for the camera loop if needed;
		// e.g., checking camera status, temperature, heartbeat, etc.
	}
}

// Example usage of GigEManager class
/*
#include "GigEManager.h"
#include "PathQueue.h"
#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>

int main()
{
    std::shared_ptr<PathQueue> pathQueue = std::make_shared<PathQueue>();
    std::string saveRootDir = "C:\\Users\\mykir\\Work\\Experiments\\"; // NOTE: LAB WINDOWS PC DIRECTORY
    //std::string saveRootDir = "C:\\Users\\USER\\Pictures\\"; // NOTE: HOME PC DIRECTORY
    //std::string saveRootDir = "/home/msis/Pictures/SentechExperiments/Experiments1/"; // NOTE: LAB LINUX PC DIRECTORY
    GigEManager manager(saveRootDir, pathQueue);

    if (!manager.Initialize())
    {
        std::cerr << "Failed to initialize cameras.\n";
        return -1;
    }

    manager.StartAll();

    //std::cout << "Enter indices to trigger:\n";
    //std::cout << "  0     => trigger ALL cameras\n";
    //std::cout << "  1     => trigger camera at index 0\n";
    //std::cout << "  2 3   => trigger cameras at indices 1 and 2\n";
    //std::cout << "Type 'q' to quit.\n";

    //std::string line;
    //while (true)
    //{
    //    std::cout << "> ";
    //    std::getline(std::cin, line);

    //    if (line == "q")
    //        break;

    //    std::istringstream iss(line);
    //    std::vector<int> inputs;
    //    int num;

    //    while (iss >> num)
    //    {
    //        inputs.push_back(num);
    //    }

    //    if (inputs.empty())
    //    {
    //        std::cerr << "No input detected.\n";
    //        continue;
    //    }

    //    if (std::find(inputs.begin(), inputs.end(), 0) != inputs.end())
    //    {
    //        manager.TriggerAll();
    //    }
    //    else
    //    {
    //        for (int input : inputs)
    //        {
    //            int index = input - 1;
    //            manager.TriggerSingle(index);
    //        }
    //    }
    //}

    for (int i = 0; i < 20; ++i)
    {
        manager.TriggerSingle("5MP_1");
        manager.TriggerSingle("5MP_2");
        manager.TriggerSingle("5MP_3");
        manager.TriggerSingle("12MP_1");
        manager.TriggerSingle("12MP_2");
        Sleep(150);
    }
    Sleep(1000);
    manager.StopAll();
    return 0;
}
*/