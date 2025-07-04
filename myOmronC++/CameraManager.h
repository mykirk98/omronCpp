// CameraManager.h
#pragma once
#include <vector>
#include <memory>
#include <unordered_map>
#include <thread>
#include <mutex>
#include "TriggerCamera.h"

class CameraManager
{
public:
    CameraManager() = default;
    ~CameraManager();

    bool AddCamera(int index, const CIStSystemPtr& pSystem);
    void TriggerSelectedCameras(const std::vector<int>& indices);
    void StartAll();
    void StopAll();

private:
    std::unordered_map<int, std::shared_ptr<TriggerCamera>> m_cameras;
    std::mutex m_mutex;
};
