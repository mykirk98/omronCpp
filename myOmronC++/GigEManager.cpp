#include "GigEManager.h"

GigEManager::GigEManager(std::string saveRootDir)
    : m_saveRootDir(saveRootDir)
{
}

GigEManager::~GigEManager()
{
    StopAll();
}

bool GigEManager::Initialize()
{
    try {
        m_pSystem = CreateIStSystem(StSystemVendor_Default, StInterfaceType_GigEVision);

        for (uint32_t i = 0; i < m_pSystem->GetInterfaceCount(); ++i)
        {
            IStInterface* pInterface = m_pSystem->GetIStInterface(i);

            std::cout << "Interface " << i << ": " << pInterface->GetIStInterfaceInfo()->GetDisplayName() << std::endl;
            std::cout << "DeviceCount = " << pInterface->GetDeviceCount() << std::endl;

            for (uint32_t j = 0; j < pInterface->GetDeviceCount(); ++j)
            {
                std::cout << "-------------------------------------------" << std::endl;
                std::cout << "Device " << j << ": " << pInterface->GetIStDeviceInfo(j)->GetDisplayName() << std::endl;
                std::cout << "SerialNumber: " << pInterface->GetIStDeviceInfo(j)->GetSerialNumber() << std::endl;

                std::unique_ptr<GigECamera> camera = std::make_unique<GigECamera>(m_saveRootDir);
                if (camera->Initialize(pInterface, j))
                {
                    std::shared_ptr<GigEWorker> worker = std::make_shared<GigEWorker>(std::move(camera));
                    m_workers.push_back(worker);
                }
                else
                {
                    std::cerr << "[GigEManager] Failed to initialize camera " << j << std::endl;
                }
            }
        }
        return !m_workers.empty();
    }
    catch (const GenICam::GenericException& e)
    {
        std::cerr << "[GigEManager] Initialization error: " << e.GetDescription() << std::endl;
        return false;
    }
}

void GigEManager::StartAll()
{
    for (std::shared_ptr<GigEWorker>& worker : m_workers)
    {
        try
        {
            worker->Start();
        }
        catch (const std::exception& e)
        {
            std::cerr << "[GigEManager] Worker failed to start: " << e.what() << std::endl;
        }
    }
}

void GigEManager::StopAll()
{
    for (std::shared_ptr<GigEWorker>& worker : m_workers)
    {
        worker->Stop();
    }
}

void GigEManager::TriggerAll()
{
    for (std::shared_ptr<GigEWorker>& worker : m_workers)
    {
        worker->Trigger();
    }
}

void GigEManager::TriggerSingle(int index)
{
    if (index >= 0 && index < static_cast<int>(m_workers.size()))
    {
        m_workers[index]->Trigger();
    }
    else
    {
		std::cerr << "[GigEManager] Invalid worker index: " << index << std::endl;
    }
}

// Example usage of GigEManager class
/*
#include "GigEManager.h"
#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>

int main()
{
    std::string saveRootDir = "C:\\Users\\mykir\\Work\\Experiments\\"; // NOTE: LAB WINDOWS PC DIRECTORY
    //std::string saveRootDir = "C:\\Users\\USER\\Pictures\\"; // NOTE: HOME PC DIRECTORY
    //std::string saveRootDir = "/home/msis/Pictures/SentechExperiments/Experiments1/"; // NOTE: LAB LINUX PC DIRECTORY
    GigEManager manager(saveRootDir);

    if (!manager.Initialize())
    {
        std::cerr << "Failed to initialize cameras.\n";
        return -1;
    }

    manager.StartAll();

    std::cout << "Enter indices to trigger:\n";
    std::cout << "  0     => trigger ALL cameras\n";
    std::cout << "  1     => trigger camera at index 0\n";
    std::cout << "  2 3   => trigger cameras at indices 1 and 2\n";
    std::cout << "Type 'q' to quit.\n";

    std::string line;
    while (true)
    {
        std::cout << "> ";
        std::getline(std::cin, line);

        if (line == "q")
            break;

        std::istringstream iss(line);
        std::vector<int> inputs;
        int num;

        while (iss >> num)
        {
            inputs.push_back(num);
        }

        if (inputs.empty())
        {
            std::cerr << "No input detected.\n";
            continue;
        }

        if (std::find(inputs.begin(), inputs.end(), 0) != inputs.end())
        {
            manager.TriggerAll();
        }
        else
        {
            for (int input : inputs)
            {
                int index = input - 1;
                manager.TriggerSingle(index);
            }
        }
    }

    manager.StopAll();
    return 0;
}
*/