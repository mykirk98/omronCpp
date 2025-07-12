// GigEManager.cpp
#include "GigEManager.h"
#include <iostream>
#include <sstream>

GigEManager::GigEManager() {}

GigEManager::~GigEManager() {
    StopAll();
}

bool GigEManager::Initialize() {
    try {
        static CStApiAutoInit stApiAutoInit;
        m_pSystem = CreateIStSystem(StSystemVendor_Default, StInterfaceType_GigEVision);

        for (uint32_t i = 0; i < m_pSystem->GetInterfaceCount(); ++i) {
            IStInterface* pInterface = m_pSystem->GetIStInterface(i);

            std::cout << "Interface " << i << ": " << pInterface->GetIStInterfaceInfo()->GetDisplayName() << std::endl;
            std::cout << "DeviceCount = " << pInterface->GetDeviceCount() << std::endl;

            for (uint32_t j = 0; j < pInterface->GetDeviceCount(); ++j) {
                std::cout << "-------------------------------------------" << std::endl;
                std::cout << "Device " << j << ": " << pInterface->GetIStDeviceInfo(j)->GetDisplayName() << std::endl;
                std::cout << "SerialNumber: " << pInterface->GetIStDeviceInfo(j)->GetSerialNumber() << std::endl;

                auto camera = std::make_shared<GigECamera>();
                if (camera->Initialize(pInterface, j)) {
                    auto worker = std::make_shared<GigEWorker>(camera);
                    m_workers.push_back(worker);
                }
                else {
                    std::cerr << "[GigEManager] Failed to initialize camera " << j << std::endl;
                }
            }
        }
        return !m_workers.empty();
    }
    catch (const GenICam::GenericException& e) {
        std::cerr << "[GigEManager] Initialization error: " << e.GetDescription() << std::endl;
        return false;
    }
}

void GigEManager::StartAll() {
    for (auto& worker : m_workers) {
        worker->Start();
    }
}

void GigEManager::StopAll() {
    for (auto& worker : m_workers) {
        worker->Stop();
    }
}

void GigEManager::TriggerAll() {
    for (auto& worker : m_workers) {
        worker->Trigger();
    }
}

void GigEManager::TriggerSelected(const std::vector<int>& indices) {
    for (int idx : indices) {
        if (idx >= 0 && idx < static_cast<int>(m_workers.size())) {
            m_workers[idx]->Trigger();
        }
        else {
            std::cerr << "[GigEManager] Invalid camera index: " << idx << std::endl;
        }
    }
}

void GigEManager::RunInteractiveLoop() {
    std::cout << "Enter camera indices to trigger (e.g., 0 or 1 2), or 'exit' to quit:\n";
    std::string line;
    while (true) {
        std::cout << "> ";
        std::getline(std::cin, line);

        if (line == "exit") break;

        std::istringstream iss(line);
        std::vector<int> indices;
        int val;
        while (iss >> val) {
            indices.push_back(val);
        }

        if (indices.empty()) continue;

        if (indices.size() == 1 && indices[0] == 0) {
            TriggerAll();
        }
        else {
            TriggerSelected(indices);
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
    if (!manager.Initialize()) {
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