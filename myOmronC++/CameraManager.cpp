// CameraManager.cpp
#include "CameraManager.h"

CameraManager::~CameraManager()
{
    StopAll();
}

bool CameraManager::AddCamera(int index, const CIStSystemPtr& pSystem)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if (m_cameras.count(index) > 0)
    {
        std::cerr << "[CameraManager] Camera with index " << index << " already exists." << std::endl;
        return false;
    }

    std::shared_ptr<TriggerCamera> camera = std::make_shared<TriggerCamera>();

    if (camera->Initialize(pSystem))
    {
        m_cameras[index] = camera;
        std::cout << "[CameraManager] Camera " << index << " added." << std::endl;
        return true;
    }

    std::cerr << "[CameraManager] Failed to initialize camera at index " << index << std::endl;
    return false;
}

void CameraManager::StartAll()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto& cam : m_cameras)
    {
        cam.second->StartAcquisition();
    }
    std::cout << "[CameraManager] All cameras started." << std::endl;
}

void CameraManager::StopAll()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto& cam : m_cameras)
    {
        cam.second->StopAcquisition();
    }
    std::cout << "[CameraManager] All cameras stopped." << std::endl;
}

void CameraManager::TriggerSelectedCameras(const std::vector<int>& indices)
{
    std::vector<std::thread> threads;

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (int idx : indices)
        {
            if (m_cameras.count(idx))
            {
                //threads.emplace_back([camera = m_cameras[idx]]() {
                //    camera->pICommandTriggerSoftware->Execute(); // 내부에 Software Trigger 실행
                //    });
            }
            else
            {
                std::cerr << "[CameraManager] Camera index " << idx << " not found." << std::endl;
            }
        }
    }

    for (auto& t : threads)
        t.join(); // 비동기 호출이지만 동시에 실행되게 하고, 종료까지 기다림
}
