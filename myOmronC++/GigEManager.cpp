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

void GigEManager::TriggerSelected(const std::vector<int>& indices)
{
    for (int idx : indices)
    {
        if (idx >= 0 && idx < static_cast<int>(m_workers.size()))
        {
            m_workers[idx]->Trigger();
        }
        else
        {
            std::cerr << "[GigEManager] Invalid camera index: " << idx << std::endl;
        }
    }
}

void GigEManager::RunInteractiveLoop()
{
    std::cout << "Enter indices to trigger:\n";
    std::cout << "  0     => trigger ALL cameras\n";
    std::cout << "  1     => trigger camera at index 0\n";
    std::cout << "  2 3   => trigger cameras at indices 1 and 2\n";
    std::cout << "Type 'exit' to quit.\n";

    std::string line;
    while (true)
    {
        std::cout << "> ";
        std::getline(std::cin, line);

        if (line == "exit")
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
            std::cerr << "[GigEManager] No input detected.\n";
            continue;
        }

        // If input includes 0, treat as trigger all
        if (std::find(inputs.begin(), inputs.end(), 0) != inputs.end())
        {
            TriggerAll();
        }
        else
        {
            // Adjust 1-based input to 0-based camera indices
            std::vector<int> adjustedIndices;
            for (int n : inputs)
            {
                int index = n - 1;
                if (index >= 0 && index < static_cast<int>(m_workers.size()))
                {
                    adjustedIndices.push_back(index);
                }
                else
                {
                    std::cerr << "[GigEManager] Invalid camera number: " << n << std::endl;
                }
            }

            if (!adjustedIndices.empty())
                TriggerSelected(adjustedIndices);
        }
    }
}



// Example usage of GigEManager class
/*
#include "GigEManager.h"
#include <iostream>

int main() {
    GigEManager manager;

    // Step 1: 카메라 초기화
    if (!manager.Initialize())
    {
        std::cerr << "[main] Failed to initialize GigEManager." << std::endl;
        return -1;
    }

    // Step 2: 모든 워커 스레드 시작 (acquisition 시작)
    manager.StartAll();
    std::cout << "[main] All camera workers started." << std::endl;

    // Step 3: 사용자 입력 루프 실행
    manager.RunInteractiveLoop();

    // Step 4: 모든 워커 종료 (acquisition 정지)
    manager.StopAll();
    std::cout << "[main] All camera workers stopped. Exiting..." << std::endl;

    return 0;
}
*/