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

                std::shared_ptr<GigECamera> camera = std::make_shared<GigECamera>();
                if (camera->Initialize(pInterface, j)) {
                    m_cameras.push_back(camera);
                }
                else {
                    std::cerr << "[GigEManager] Failed to initialize camera " << j << std::endl;
                }
            }
        }
        return !m_cameras.empty();
    }
    catch (const GenICam::GenericException& e) {
        std::cerr << "[GigEManager] Initialization error: " << e.GetDescription() << std::endl;
        return false;
    }
}

void GigEManager::StartAll() {
    std::vector<std::thread> threads;
    for (auto& camera : m_cameras) {
        threads.emplace_back([camera]() {
            camera->StartAcquisition();
            });
    }
    for (auto& t : threads) t.join();
}

void GigEManager::StopAll() {
    std::vector<std::thread> threads;
    for (auto& camera : m_cameras) {
        threads.emplace_back([camera]() {
            camera->StopAcquisition();
            });
    }
    for (auto& t : threads) t.join();
}

void GigEManager::TriggerAll() {
    std::vector<std::thread> threads;
    for (auto& camera : m_cameras) {
        threads.emplace_back([camera]() {
            if (camera->pICommandTriggerSoftware) {
                camera->pICommandTriggerSoftware->Execute();
            }
            });
    }
    for (auto& t : threads) t.join();
}

void GigEManager::TriggerSelected(const std::vector<int>& indices) {
    std::vector<std::thread> threads;
    for (int idx : indices) {
        if (idx >= 0 && idx < static_cast<int>(m_cameras.size())) {
            threads.emplace_back([this, idx]() {
                if (m_cameras[idx]->pICommandTriggerSoftware) {
                    m_cameras[idx]->pICommandTriggerSoftware->Execute();
                }
                });
        }
        else {
            std::cerr << "[GigEManager] Invalid camera index: " << idx << std::endl;
        }
    }
    for (auto& t : threads) t.join();
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
